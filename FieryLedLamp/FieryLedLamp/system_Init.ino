// ************************************************************************* system_Init.ino ************************************************************

// ================================================================ ИНИЦИАЛИЗАЦИЯ ФАЙЛОВОЙ СИСТЕМЫ =====================================================
bool FS_init(void) {
  if (!LittleFS.begin(false)) {
#if GENERAL_LOG
    SYSLOG.add("Ошибка монтирования LittleFS, выполняется форматирование...");
#endif
    LittleFS.format();
    delay(200);
    if (!LittleFS.begin(false)) {
#if GENERAL_LOG
      SYSLOG.add("LittleFS не удалось смонтировать даже после форматирования!");
#endif
      return false;
    }
#if GENERAL_LOG
    SYSLOG.add("LittleFS отформатирована и смонтирована успешно");
#endif
  } else {
#if GENERAL_LOG
    SYSLOG.add("LittleFS смонтирована успешно");
#endif
  }

  HTTP.on("/edit", HTTP_GET, []() {
    if (!HTTP.authenticate(FILEMANAGER_USERNAME, FILEMANAGER_PASSWORD)) {
      HTTP.requestAuthentication();
      return;
    }
    handleFileRead("/edit.htm");
  });

  HTTP.on("/edit", HTTP_PUT, handleFileCreate);
  HTTP.on("/edit", HTTP_DELETE, handleFileDelete);
  HTTP.on("/edit", HTTP_POST, []() {}, handleFileUpload);
  HTTP.on("/list", HTTP_GET, handleFileList);

  HTTP.onNotFound([]() {
    if (!handleFileRead(HTTP.uri())) {
      handleFileRead("/index.html");
    }
  });

  return true;
}

// ----------------------------------------------------------------
bool FileCopy(const String& SourceFile, const String& TargetFile) {
#if USE_OTA
  if (Ota::instance().isOtaActive()) return false;
#endif
  LittleFS.remove(TargetFile);
  File s = LittleFS.open(SourceFile, "r");
  File t = LittleFS.open(TargetFile, "w");
  if (!s || !t) {
    if (s) s.close();
    return false;
  }
  uint8_t buf[512];
  while (s.available()) {
    size_t len = s.read(buf, sizeof(buf));
    t.write(buf, len);
    yield();
  }
  s.close(); t.close();
  return true;
}

// =================================================================== АППАРАТНАЯ ИНИЦИАЛИЗАЦИЯ ========================================================
void initHardware() {
  // SD карта
#if USE_SD && !FS_AS_SD
  if (SD.begin(SD_CS_PIN)) {
    sd_card_present = true;
#if SD_LOG
    SYSLOG.add("SD-карта инициализирована");
#endif

    for (uint8_t i = 0; i < MODE_AMOUNT; i++)
      effects_folders[i] = pgm_read_byte(&default_effects_folders[i]);
    jsonWrite(configSetup, "out_file", "/effects/effect133.out");
  } else {
    sd_card_present = false;
#if SD_LOG
    SYSLOG.add("Ошибка инициализации SD-карты");
#endif

  }
#endif

  // мосфет
#ifdef MOSFET_PIN
  pinMode(MOSFET_PIN, OUTPUT);
#ifdef MOSFET_LEVEL
  digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif
#endif

  // источник будильника
#ifdef ALARM_PIN
  pinMode(ALARM_PIN, OUTPUT);
#ifdef ALARM_LEVEL
  digitalWrite(ALARM_PIN, !ALARM_LEVEL);
#endif
#endif
}

// ------------------------------------------------------------------
fs::File openEffectFile(const String& filename) {
#if (FS_AS_SD == 1)
  return LittleFS.open("/effects/" + filename, "r");
#else
  return SD.open("/effects/" + filename, "r");
#endif
}
// ------------------------------------------------------------------
void loadOutEffect(uint8_t effectIndex) {
  if (!ONflag) return;
  String filename = "/effects/effect" + String(effectIndex) + ".out";
  fs::File file = openEffectFile(filename);

  if (!file || file.size() != usedLeds * 3) {
#if SD_LOG
    SYSLOG.add("Ошибка: файл .out повреждён");
#endif

    if (file) file.close();
    return;
  }

  for (uint16_t i = 0; i < usedLeds; i++) {
    leds[i] = CRGB(file.read(), file.read(), file.read());
  }
  file.close();
  FastLED.show();
}

// ==================================================================== ИНИЦИАЛИЗАЦИЯ МП3 ПЛЕЕРА =======================================================
void initMP3Hardware() {
#if USE_MP3_PLAYER
  mp3.begin(9600, SERIAL_8N1, MP3_RX_PIN, MP3_TX_PIN);
  delay(200);

  mp3_player_connect = 0;
  mp3_folder_last = 255;

#if MP3_LOG
  SYSLOG.add("MP3: инициализация ...");
#endif

  mp3_setup();

  String configSound = readFile(F("sound_list.json"), 2048);

  if (configSound.length() > 0 && configSound != "Failed" && configSound != "Large") {
    bool allOk = true;

    for (uint8_t k = 0; k < MODE_AMOUNT; k++) {
      char key[8];
      sprintf(key, "e%d", k);
      int val = jsonReadtoInt(configSound, key, 0);

      if (val == 0) {
#if MP3_LOG
        SYSLOG.add("Предупреждение: мелодия для режима %d не найдена, будет использована папка по умолчанию", k);
#endif
        allOk = false;
      }
      effects_folders[k] = (val != 0) ? val : 1;
    }

    if (allOk) {
#if MP3_LOG
      SYSLOG.add("Папки с мелодиями загружены из sound_list.json");
#endif
    } else {
#if MP3_LOG
      SYSLOG.add("Настройки звука загружены с некоторыми значениями по умолчанию");
#endif
    }
  }
  else {
#if MP3_LOG
    SYSLOG.add("Используется папка 1 для всех режимов");
#endif
    for (uint8_t k = 0; k < MODE_AMOUNT; k++) {
      effects_folders[k] = 1;
    }
  }

  // настройка текущей папки
  if (currentMode >= MODE_AMOUNT) currentMode = 0;
  uint8_t newFolder = effects_folders[currentMode];
  if (newFolder == 0) newFolder = 1;

  mp3_folder = newFolder;
  CurrentFolder = newFolder;
  mp3_folder_last = newFolder;
  CurrentFolder_last = newFolder;

  jsonWrite(configMP3, "fold_sel", newFolder);
  writeFile(F("config_mp3.json"), configMP3);

#if MP3_LOG
  SYSLOG.add("MP3: текущий режим=%d, папка со звуком установлена в %d", currentMode, newFolder);
#endif

#endif // USE_MP3_PLAYER
}

// ======================================================================= ИНИЦИАЛИЗАЦИЯ КНОПКИ ========================================================
void initButtonHardware() {
#if USE_BUTTON
  if (button_type) {  // сенсорная
    touch.setType(LOW_PULL);
    touch.setDebounce(BUTTON_SET_DEBOUNCE_SENSORY);
  } else {  // механическая
    touch.setType(HIGH_PULL);
    touch.setDebounce(BUTTON_SET_DEBOUNCE_MECHANICAL);
  }

  touch.setDirection(NORM_OPEN);
  touch.setTimeout(BUTTON_CLICK_TIMEOUT);
  touch.setStepTimeout(BUTTON_STEP_TIMEOUT);
#if BUTTON_LOG
  SYSLOG.add("Кнопка инициализирована: %s\n", button_type ? "Сенсорная" : "Механическая");
#endif
#endif // USE_BUTTON
}

// ============================================================== ИНИЦИАЛИЗАЦИЯ ДИСПЛЕЕВ TM1637 / ST7789 ===============================================
void initST7789() {
#if USE_ST7789
#if ST7789_LOG
  SYSLOG.add("Старт дисплея ST7789");
#endif
  TFT_Init();
  TFT_SetBrightness(tft_brightness);
  TFT_SetAutoBrightness(tft_auto_brightness);
  tftShowStartText();
#endif // USE_ST7789
}

void initTM1637() {
#if USE_TM1637
#if TM1637_LOG
  SYSLOG.add("Старт дисплея TM1637");
#endif
  tmr_clock = millis();
  display.setBrightness(DispBrightness);
  display.displayByte(_empty, _empty, _empty, _empty);
  display.displayByte(_dash, _dash, _dash, _dash);
#endif // USE_TM1637
}

// ========================================================================= ИНИЦИАЛИЗАЦИЯ IR / RF =====================================================
void initIR() {
#if USE_IR_RECEIVER
  irrecv.enableIRIn();
  IR_Tick_Timer = IR_Repeat_Timer = millis();
#endif
}

void initRF() {
#if USE_RF_RECEIVER
  rfReceiver.enableReceive(RF_RECEIVER_PIN);
#endif
}

// ======================================================== ИНИЦИАЛИЗАЦИЯ РАССВЕТА, ЗАКАТА, РАСПИСАНИЯ ЛАМПЫ ===========================================
void initAlarm() {
#if USE_DAWN
  first_entry = 1;
  handle_alarm();
  first_entry = 0;
#endif
}

void initSunset() {
#if USE_SUNSET
  first_entry = 1;
  handle_sunset();
  first_entry = 0;
#endif
}

void initSchedule() {
#if USE_SCHEDULE
  load_schedule();
#endif
}

// =================================================================== ИНИЦИАЛИЗАЦИЯ НОЧНЫХ ЧАСОВ ======================================================
void initNightClockSettings() {
  nightClockEnabled = false;
  jsonWrite(configLedPanel, "night_clock_enabled", "0");
  
  nightClockBrightness = jsonReadtoInt(configLedPanel, "night_clock_brightness", 1);
  if (nightClockBrightness < 1) nightClockBrightness = 1;
  nightClockHue = jsonReadtoInt(configLedPanel, "night_clock_hue", 0);
  nightModeBrightness = 0;
}

// ==================================================================== ИНИЦИАЛИЗАЦИЯ ТАЙМЕРОВ =========================================================
void initSystemTimers() {
  uint32_t now = millis();
#if USE_TM1637
  DisplayTimer = now;
#endif
  my_timer = now;
#if HEAP_SIZE_PRINT
  mem_timer = now;
#endif
  manualControlTimer = now;
  lastMedium = now;
  lastSlow = now;
  lastWiFi = now;
}

// ======================================================================= ИНИЦАЛИЗАЦИЯ MQTT ===========================================================
void initMQTT() {
#if USE_MQTT
  // Инициализация MQTT клиента
  if (MqttOn && Wifi::instance().isConnected()) {
    mqttClient = new AsyncMqttClient();
    if (mqttClient) {
      Mqtt::instance().begin(mqttClient, inputBuffer, &sendCurrent);
#if MQTT_LOG
      SYSLOG.add("MQTT клиент создан и настроен");
#endif
    } else {
#if MQTT_LOG
      SYSLOG.add("Ошибка: не удалось создать AsyncMqttClient");
#endif
    }
  } else {
    if (!MqttOn) {
#if MQTT_LOG
      SYSLOG.add("MQTT отключён в настройках");
#endif
    } else {
#if MQTT_LOG
      SYSLOG.add("Нет соединения с Wi-Fi - MQTT не запускается");
#endif
    }
  }
#endif
}

// ==================================================================== ИНИЦИАЛИЗАЦИЯ TELNET ===========================================================
void initTelnetDebug() {
#if DEBUG_ENABLED && DEBUG_TELNET
  initTelnet();
#endif
}

// ===================================================================== ИНИЦИАЛИЗАЦИЯ SSDP ============================================================
void SSDP_init(void) {
  HTTP.on(F("/description.xml"), HTTP_GET, []() {
    SSDP.schema(HTTP.client());
  });

  String lampName = jsonRead(configSetup, "ssdp");
  LAMP_NAME = lampName;
  SSDP.setName(lampName);
  SSDP.setDeviceType(F("upnp:rootdevice"));
  SSDP.setSchemaURL(F("description.xml"));
  SSDP.setHTTPPort(80);
  SSDP.setSerialNumber(get_Chip_ID());
  SSDP.setURL("/");
  SSDP.setModelName(F("FieryLedLamp"));
  SSDP.setModelNumber(lampName + VERSION);
  SSDP.setModelURL(F("https://github.com/MishanyaTS/FieryLedLamp"));
  SSDP.setManufacturer(F("MishanyaTS"));
  SSDP.setManufacturerURL(F("https://github.com/MishanyaTS/FieryLedLamp"));
  SSDP.begin();
}

// =================================================================== ПЕРЕИНИЦИАЛИЗАЦИЯ SSDP ==========================================================
void restartSSDP() {
  SSDP_init();
  ssdpInitialized = true;
  auto& wifi = Wifi::instance();
  if (wifi.isConnected()) {
    previousIP = wifi.localIP();
  } else {
    previousIP = wifi.apIP();
  }
}

// ====================================================================== ИНИЦИАЛИЗАЦИЯ WEB ============================================================
void initWebServices() {
  SSDP_init();
  Udp.begin(localPort);
  User_settings();
  HTTP_init();
}

void HTTP_init(void) {
  HTTP.on("/config.json", HTTP_GET, []() {
    HTTP.send(200, F("application/json"), configSetup);
  });

  HTTP.on("/restart", HTTP_GET, []() {
    String restart = HTTP.arg("device");
    if (restart == "ok") {
      HTTP.send(200, F("text / plain"), F("Reset OK"));
      delay(1000);
      ESP.restart();
    } else {
      HTTP.send(200, F("text / plain"), F("No Reset"));
    }
  });

  ElegantOTA.begin(&HTTP);
  HTTP.begin();
}

// ******************************************************************************************************************************************************

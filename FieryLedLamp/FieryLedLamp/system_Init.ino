// ************************************************************************* system_Init.ino ************************************************************

// ================================================================ ИНИЦИАЛИЗАЦИЯ ФАЙЛОВОЙ СИСТЕМЫ =====================================================

bool FS_init(void) {
  if (!LittleFS.begin(false)) {
#if GENERAL_LOG
    SYSLOG.add("LittleFS mount failed, retry...");
#endif
    delay(100);
    if (!LittleFS.begin(false)) {
#if GENERAL_LOG
      SYSLOG.add("LittleFS mount failed twice - форматирование ОТКЛЮЧЕНО");
      SYSLOG.add("Загрузите FS через /update_fs или веб-флешер");
#endif
      return false;
    }
  }
#if GENERAL_LOG
  SYSLOG.add("LittleFS OK: total=%u, used=%u, free=%u", (unsigned)LittleFS.totalBytes(), (unsigned)LittleFS.usedBytes(), (unsigned)(LittleFS.totalBytes() - LittleFS.usedBytes()));
  if (LittleFS.usedBytes() == 0) {
    SYSLOG.add("ВНИМАНИЕ: LittleFS пуста - загрузите образ FS");
  }
  {
    File root = LittleFS.open("/");
    if (root && root.isDirectory()) {
      File f = root.openNextFile();
      int n = 0;
      while (f && n < 500) {
        SYSLOG.add("FS: %s (%u bytes)", f.name(), (unsigned)f.size());
        f = root.openNextFile();
        n++;
      }
      SYSLOG.add("FS: total %d entries", n);
    }
  }
#endif
  return true;
}

// ===================================================================== ИНИЦИАЛИЗАЦИЯ SSDP ============================================================

void SSDP_init(void) {
  static bool routeRegistered = false;
  if (!routeRegistered) {
    HTTP.on("/description.xml", HTTP_GET, [](AsyncWebServerRequest * request) {
      String xml;
      xml.reserve(600);
      xml += F("<?xml version=\"1.0\"?>\r\n");
      xml += F("<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\r\n");
      xml += F("<specVersion><major>1</major><minor>0</minor></specVersion>\r\n");
      xml += F("<device>\r\n");
      xml += F("<deviceType>upnp:rootdevice</deviceType>\r\n");
      xml += F("<friendlyName>");
      xml += LAMP_NAME;
      xml += F("</friendlyName>\r\n");
      xml += F("<manufacturer>MishanyaTS</manufacturer>\r\n");
      xml += F("<manufacturerURL>https://github.com/MishanyaTS/FieryLedLamp</manufacturerURL>\r\n");
      xml += F("<modelName>FieryLedLamp</modelName>\r\n");
      xml += F("<modelNumber>");
      xml += LAMP_NAME + VERSION;
      xml += F("</modelNumber>\r\n");
      xml += F("<modelURL>https://github.com/MishanyaTS/FieryLedLamp</modelURL>\r\n");
      xml += F("<serialNumber>");
      xml += get_Chip_ID();
      xml += F("</serialNumber>\r\n");
      xml += F("<UDN>uuid:");
      xml += get_Chip_ID();
      xml += F("</UDN>\r\n");
      xml += F("</device>\r\n");
      xml += F("</root>\r\n");
      request->send(200, "text/xml", xml);
    });
    routeRegistered = true;
  }

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
#if GENERAL_LOG
  SYSLOG.add("initWebServices: start, free heap=%u", ESP.getFreeHeap());
#endif
  SSDP_init();
#if GENERAL_LOG
  SYSLOG.add("initWebServices: SSDP_init OK, free heap=%u", ESP.getFreeHeap());
#endif
  Udp.begin(localPort);
#if GENERAL_LOG
  SYSLOG.add("initWebServices: Udp.begin OK, free heap=%u", ESP.getFreeHeap());
#endif
  User_settings();
#if GENERAL_LOG
  SYSLOG.add("initWebServices: User_settings OK, free heap=%u", ESP.getFreeHeap());
#endif
  HTTP_init();
#if GENERAL_LOG
  SYSLOG.add("initWebServices: HTTP_init OK, free heap=%u", ESP.getFreeHeap());
#endif
}

// ----------------------------------------------------------------------------------------

void HTTP_init(void) {
  HTTP.on("/config.json", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "application/json", configSetup);
  });

  HTTP.on("/restart", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("device") && request->getParam("device")->value() == "ok") {
      request->send(200, "text/plain", "Reset OK");
      pendingRestart = true;
    } else {
      request->send(200, "text/plain", "No Reset");
    }
  });

  HTTP.on("/ping", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "PONG");
  });

  HTTP.on("/check_file", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->hasParam("path")) {
      request->send(400, "text/plain", "Missing ?path=");
      return;
    }
    String p = request->getParam("path")->value();
    if (!p.startsWith("/")) p = "/" + p;

    String out = "path: " + p + "\n";
    out += "exists(): ";
    out += LittleFS.exists(p) ? "YES" : "NO";
    out += "\n";

    File f = LittleFS.open(p);
    if (f) {
      out += "open(): OK, size=";
      out += (unsigned)f.size();
      out += "\n";
      out += "isDirectory(): ";
      out += f.isDirectory() ? "YES" : "NO";
      out += "\n";
      f.close();
    } else {
      out += "open(): FAILED\n";
    }
    request->send(200, "text/plain", out);
  });

  HTTP.on("/fs_info", HTTP_GET, [](AsyncWebServerRequest * request) {
    String out = "";
    out += "totalBytes: "; out += (unsigned)LittleFS.totalBytes(); out += "\n";
    out += "usedBytes: ";  out += (unsigned)LittleFS.usedBytes();  out += "\n";
    out += "freeBytes: ";  out += (unsigned)(LittleFS.totalBytes() - LittleFS.usedBytes()); out += "\n";
    request->send(200, "text/plain", out);
  });

  FS_routes();
#if GENERAL_LOG
  SYSLOG.add("HTTP_init: heap=%u, FS used=%u", ESP.getFreeHeap(), (unsigned)LittleFS.usedBytes());
#endif
  HTTP.begin();
}

// ----------------------------------------------------------------------------------------

void FS_routes() {
  HTTP.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!handleFileRead(request, "/index.htm")) {
      request->send(404, "text/plain", "index.htm not found in FS");
    }
  });

  HTTP.on("/index.htm", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!handleFileRead(request, "/index.htm")) {
      request->send(404, "text/plain", "index.htm not found in FS");
    }
  });

  HTTP.on("/test", HTTP_GET, [](AsyncWebServerRequest * request) {
#if GENERAL_LOG
    SYSLOG.add(">>> /test: HIT from %s", request->client()->remoteIP().toString().c_str());
#endif
    request->send(200, "text/plain", "TEST OK");
  });

  HTTP.on("/edit", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!requireFileManagerAuth(request, true)) return;
    handleFileRead(request, "/edit.htm");
  });

  HTTP.on("/edit", HTTP_PUT, [](AsyncWebServerRequest * request) { handleFileCreate(request); });

  HTTP.on("/edit", HTTP_DELETE, [](AsyncWebServerRequest * request) { handleFileDelete(request); });

  HTTP.on("/edit", HTTP_POST,
  [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "OK");
  }, handleFileUpload);

  HTTP.on("/edit_json", HTTP_POST,
  [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "OK");
  }, handleJsonUpload);

  HTTP.on("/list", HTTP_GET, [](AsyncWebServerRequest * request) { handleFileList(request); });

  HTTP.onNotFound([](AsyncWebServerRequest * request) {
#if GENERAL_LOG
    SYSLOG.add(">>> onNotFound: url=%s method=%d", request->url().c_str(), (int)request->method());
#endif
    if (!handleFileRead(request, request->url())) {
#if GENERAL_LOG
      SYSLOG.add(">>> onNotFound: handleFileRead returned false, sending 404 for %s", request->url().c_str());
#endif
      request->send(404, "text/plain", "Not found");
    }
  });
}

// =================================================================== АППАРАТНАЯ ИНИЦИАЛИЗАЦИЯ ========================================================

void initHardware() {
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

// ====================================================================== ИНИЦИАЛИЗАЦИЯ SD КАРТЫ =======================================================

void initSD() {
#if USE_SD
  if (!sdEnabled) return;

  if (sdType == 0) {
    if (SD.begin(SD_CS_PIN)) {
      sd_card_present = true;
      for (uint8_t i = 0; i < MODE_AMOUNT; i++)
        effects_folders[i] = pgm_read_byte(&default_effects_folders[i]);
      jsonWrite(configSetup, "out_file", "");
    } else {
      sd_card_present = false;
    }
  } else {
    sd_card_present = true;
    if (!LittleFS.exists("/effects/")) {
      if (LittleFS.mkdir("/effects/")) {
      } else {
      }
    }
  }
#endif // USE_SD
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
  if (button_type) { // сенсорная
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
#if DEBUG_TELNET
  initTelnet();
#endif
}

// ******************************************************************************************************************************************************

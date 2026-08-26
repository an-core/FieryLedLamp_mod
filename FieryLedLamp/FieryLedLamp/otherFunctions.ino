// *********************************************************************** otherFunctions.ino ***********************************************************
#include "Time.h"
#include "Constants.h"
// --------------------------

// идентификация чипа ESP32
uint32_t get_Chip_ID(void) {
  return (uint32_t)ESP.getEfuseMac();
}

// ----------------------------------------------------------------------
// логи
#if DEBUG_ENABLED
void loadSystemLogSettings() {
  bool enabled = (jsonReadtoInt(configSetup, "syslog_enabled", 1) == 1);
  SystemLog::instance().setEnabled(enabled);
}
#endif

// ----------------------------------------------------------------------
// ведущий ноль (для будильника Рассвет и заката)
String zeroPad(String str, uint8_t len) {
  while (str.length() < len) str = "0" + str;
  return str;
}

// ----------------------------------------------------------------------
// декодирование URL
String urldecode(String str) {
  String decoded = "";
  char ch;
  int i = 0;

  while (i < str.length()) {
    if (str[i] == '%') {
      if (i + 2 < str.length()) {
        String hex = str.substring(i + 1, i + 3);
        ch = (char)strtol(hex.c_str(), nullptr, 16);
        decoded += ch;
        i += 3;
      } else {
        decoded += str[i];
        i++;
      }
    } else if (str[i] == '+') {
      decoded += ' ';
      i++;
    } else {
      decoded += str[i];
      i++;
    }
  }
  return decoded;
}

// ----------------------------------------------------------------------
// кодирование URL
String urlencode(const String& str) {
  String encoded = "";
  char c;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded += c;
    } else if (c == '/') {
      encoded += c;
    } else {
      encoded += '%';
      char hex[3];
      sprintf(hex, "%02X", (unsigned char)c);
      encoded += hex;
    }
  }
  return encoded;
}

// ----------------------------------------------------------------------
// дата и время прошивки
String convertBuildDate(const char* dateStr) {
  const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  String date = String(dateStr);
  date.trim();

  String monthStr = date.substring(0, 3);
  int month = 1;
  for (int i = 0; i < 12; i++) {
    if (monthStr == months[i]) {
      month = i + 1;
      break;
    }
  }

  String dayStr = date.substring(4, 6);
  dayStr.trim();
  int day = dayStr.toInt();

  String yearStr = date.substring(7, 11);
  int year = yearStr.toInt();

  char result[11];
  sprintf(result, "%04d-%02d-%02d", year, month, day);
  return String(result);
}

String buildDateTimeString() {
  String datePart = convertBuildDate(__DATE__);
  String timePart = String(__TIME__);
  return datePart + "T" + timePart + "+03:00";
}

String getUTCDateTime() {
  time_t now = Time::now();
  struct tm tm;
  gmtime_r(&now, &tm);
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S+00:00", &tm);
  return String(buffer);
}

String convertUTCtoMSK(String utcTime) {
  int year = utcTime.substring(0, 4).toInt();
  int month = utcTime.substring(5, 7).toInt();
  int day = utcTime.substring(8, 10).toInt();
  int hour = utcTime.substring(11, 13).toInt();
  int minute = utcTime.substring(14, 16).toInt();

  hour += 3;
  if (hour >= 24) {
    hour -= 24;
    day += 1;

    int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (isLeap) daysInMonth[2] = 29;

    if (day > daysInMonth[month]) {
      day = 1;
      month += 1;
      if (month > 12) {
        month = 1;
        year += 1;
      }
    }
  }

  char buffer[30];
  sprintf(buffer, "%04d-%02d-%02dT%02d:%02d:00+03:00", year, month, day, hour, minute);
  return String(buffer);
}

String formatDateTimeForDisplay(String dateTime) {
  int year = dateTime.substring(0, 4).toInt();
  int month = dateTime.substring(5, 7).toInt();
  int day = dateTime.substring(8, 10).toInt();
  int hour = dateTime.substring(11, 13).toInt();
  int minute = dateTime.substring(14, 16).toInt();

  char buffer[25];
  sprintf(buffer, "%02d.%02d.%04d в %02d:%02d", day, month, year, hour, minute);
  return String(buffer);
}

// ----------------------------------------------------------------------
void performUpdateCheck() {
  if (!Wifi::instance().isConnected()) {
    DynamicJsonDocument doc(256);
    doc["has_update"] = false;
    doc["error"] = "Нет WiFi";
    String resp;
    serializeJson(doc, resp);
    updateCache.response = resp;
    updateCache.timestamp = millis();
    updateCache.valid = true;
    return;
  }

  DynamicJsonDocument doc(8192);
  String resp;
  String buildDateTime = buildDateTimeString();

  doc["current_version"] = String(VERSION);
  doc["build_datetime"] = buildDateTime;
  doc["folder_url"] = "https://github.com/an-core/FieryLedLamp_mod/tree/main/FieryLedLamp_mod";
  doc["update_folder_url"] = "https://github.com/an-core/FieryLedLamp_mod/tree/main/update";

  String changelogUrl = "https://gist.githubusercontent.com/an-core/76efbb63916515dda1843a5574208b8d/raw/changelog.json";
  bool changelogLoaded = false;

  WiFiClientSecure clientChangelog;
  clientChangelog.setInsecure();
  clientChangelog.setTimeout(10000);
  HTTPClient httpChangelog;
  httpChangelog.setTimeout(10000);

  if (httpChangelog.begin(clientChangelog, changelogUrl)) {
    httpChangelog.addHeader("User-Agent", "FieryLedLamp");
    int httpCodeChangelog = httpChangelog.GET();

    if (httpCodeChangelog == HTTP_CODE_OK) {
      String content = httpChangelog.getString();
      DynamicJsonDocument changelogJson(16384);
      DeserializationError error = deserializeJson(changelogJson, content);

      if (!error && changelogJson.containsKey("changes")) {
        JsonArray changes = changelogJson["changes"];
        if (changes && changes.size() > 0) {
          JsonArray changelog = doc.createNestedArray("changelog");
          for (JsonVariant item : changes) {
            changelog.add(item.as<JsonVariant>());
          }
          changelogLoaded = true;
        }
      } else if (!error) {
        JsonArray changelog = doc.createNestedArray("changelog");
        changelog.add("Список изменений пуст");
        changelogLoaded = true;
      }
    }
    httpChangelog.end();
  }
  clientChangelog.stop();

  if (!changelogLoaded) {
    JsonArray changelog = doc.createNestedArray("changelog");
    changelog.add("Список изменений временно недоступен");
    changelog.add("Попробуйте обновить страницу позже");
  }

  String plannedUrl = "https://gist.githubusercontent.com/an-core/2af384f891752661d020ae354274bc08/raw/planned.json";
  bool plannedLoaded = false;

  WiFiClientSecure clientPlanned;
  clientPlanned.setInsecure();
  clientPlanned.setTimeout(10000);
  HTTPClient httpPlanned;
  httpPlanned.setTimeout(10000);

  if (httpPlanned.begin(clientPlanned, plannedUrl)) {
    httpPlanned.addHeader("User-Agent", "FieryLedLamp");
    int httpCodePlanned = httpPlanned.GET();

    if (httpCodePlanned == HTTP_CODE_OK) {
      String contentPlanned = httpPlanned.getString();
      DynamicJsonDocument plannedJson(4096);
      DeserializationError errorPlanned = deserializeJson(plannedJson, contentPlanned);

      if (!errorPlanned && plannedJson.containsKey("planned")) {
        JsonArray planned = plannedJson["planned"];
        if (planned && planned.size() > 0) {
          JsonArray plannedList = doc.createNestedArray("planned");
          for (JsonVariant item : planned) {
            plannedList.add(item.as<JsonVariant>());
          }
          plannedLoaded = true;
        }
      }
    }
    httpPlanned.end();
  }
  clientPlanned.stop();

  if (!plannedLoaded) {
    JsonArray plannedList = doc.createNestedArray("planned");
    plannedList.add("Список планов временно недоступен");
  }

  String fileName = "bin_ESP32_ESP32S3.zip";
  String commitUrl = "https://api.github.com/repos/an-core/FieryLedLamp_mod/commits?path=update/" + fileName + "&per_page=1";

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(15000);

  HTTPClient http;
  http.setTimeout(15000);

  bool hasUpdate = false;
  String updateVersion = "";
  String fullDateTimeForDisplay = "";

  if (http.begin(client, commitUrl)) {
    http.addHeader("User-Agent", "FieryLedLamp");
    http.addHeader("Accept", "application/vnd.github.v3+json");

    if (http.GET() == HTTP_CODE_OK) {
      String payload = http.getString();
      DynamicJsonDocument commitDoc(16384);

      if (!deserializeJson(commitDoc, payload)) {
        String commitDate = commitDoc[0]["commit"]["author"]["date"].as<String>();
        if (commitDate.length() > 0) {
          String fileDateTimeMSK = convertUTCtoMSK(commitDate.substring(0, 19) + "+00:00");
          String buildDateTimeMSK = buildDateTimeString();

          hasUpdate = (fileDateTimeMSK > buildDateTimeMSK);

          if (hasUpdate) {
            updateVersion = fileName;
            fullDateTimeForDisplay = formatDateTimeForDisplay(fileDateTimeMSK);
          }
        }
      }
    }
    http.end();
  }
  client.stop();

  doc["has_update"] = hasUpdate;
  if (hasUpdate) {
    doc["update_version"] = updateVersion;
    doc["update_datetime"] = fullDateTimeForDisplay;
    doc["update_url"] = "https://github.com/an-core/FieryLedLamp_mod/tree/main/update";
  }
  serializeJson(doc, resp);

  updateCache.response = resp;
  updateCache.timestamp = millis();
  updateCache.valid = true;
}

// ----------------------------------------------------------------------
void Save_File_Changes() {
  if (save_file_changes && millis() - timeout_save_file_changes >= SAVE_FILE_DELAY_TIMEOUT) {
    if (save_file_changes & SAVE_CONFIG_BIT) {
      writeFile(F("config.json"), configSetup);
    }
    if (save_file_changes & SAVE_ALARMS_BIT) {
#if USE_DAWN
      save_alarms();
#endif
#if USE_SUNSET
      save_sunsets();
#endif
    }
    if (save_file_changes & SAVE_CYCLE_BIT) {
      cycle_get();
    }
#if USE_SCHEDULE
    if (save_file_changes & SAVE_SCHEDULE_BIT) {
      writeFile(F("config_schedule.json"), configSchedule);
    }
#endif
#if USE_MULTILAMP
    if (save_file_changes & SAVE_MULTILAMP_BIT) {
      writeFile(F("config_multilamp.json"), configMultilamp);
    }
#endif

    save_file_changes = 0;
  }
}

// ----------------------------------------------------------------------
// MQTT публикация
void publishMqttState() {
#if USE_MQTT
  if (!MqttOn || !Wifi::instance().isConnected()) return;
  static uint32_t mqtt_timer = 0;
  if (Mqtt::instance().needToPublish || (MqttPeriod && (millis() - mqtt_timer) >= (MqttPeriod * 1000UL))) {
    mqtt_timer = millis();
    if (strlen(inputBuffer) > 0) {
      processInputBuffer(inputBuffer, Mqtt::instance().mqttBuffer, true);
    }

#ifdef PUBLISH_STATE_IN_OLD_FORMAT
    Mqtt::instance().publishState(0);
#endif

    String MqttSnd = "{\"power\":\"ON\"}";
    jsonWrite(MqttSnd, "power", ONflag ? "ON" : "OFF");
    jsonWrite(MqttSnd, "cycle", Favorites::instance().FavoritesRunning ? "ON" : "OFF");

    for (uint8_t n = 0; n < MODE_AMOUNT; n++) {
      if (eff_num_correct[n] == currentMode) {
        jsonWrite(MqttSnd, "effect", String(n));
        break;
      }
    }

    jsonWrite(MqttSnd, "bri", String(modes[currentMode].Brightness));
    jsonWrite(MqttSnd, "spd", String(modes[currentMode].Speed));
    jsonWrite(MqttSnd, "sca", String(modes[currentMode].Scale));

#if USE_MP3_PLAYER
    jsonWrite(MqttSnd, "sound", eff_sound_on ? "ON" : "OFF");
    jsonWrite(MqttSnd, "vol", String(eff_volume));
#endif

    jsonWrite(MqttSnd, "runt", String(RuninTextOverEffects));
    jsonWrite(MqttSnd, "runc", String(ColorRunningText));
    jsonWrite(MqttSnd, "runf", String(ColorTextFon));
    jsonWrite(MqttSnd, "runs", String(SpeedRunningText));
    jsonWrite(MqttSnd, "rnde", String(Favorites::instance().rndCycle));
    jsonWrite(MqttSnd, "rndс", String(random_on));
    jsonWrite(MqttSnd, "rndf", String(selectedSettings));

    MqttSnd.toCharArray(Mqtt::instance().mqttBuffer, MqttSnd.length() + 1);
    Mqtt::instance().publishState(1);
  }
#endif
}

void handleMediumTasks() {
#if defined(GENERAL_DEBUG) && GENERAL_DEBUG_TELNET
  handleTelnetClient();
#endif
#if USE_MQTT
  Mqtt::instance().loop();
#endif
#if USE_BLYNK
  if (Wifi::instance().isConnected()) Blynk.run();
#endif
  handleMqttPublish();
  handleHeapMonitor();
}

// -----------------------
void handleSlowTasks() {
  printTime();
  updateAutoHueModes();

#if USE_WEATHER
  static bool firstWeatherUpdateDone = false;
  if (!firstWeatherUpdateDone && WiFi.status() == WL_CONNECTED) {
    Weather::instance().forceUpdate();
    Weather::instance().update();
    firstWeatherUpdateDone = true;
  }

#if LED_PANEL
  printWeather();
#endif // LED_PANEL
  Weather::instance().updateIfNeeded();
#endif // USE_WEATHER

  static bool ipShown = false;
  if (!ipShown) {
    IPAddress ip = WiFi.localIP();
    if (ip != IPAddress(0, 0, 0, 0)) {
      showIPOnMatrix();
      ipShown = true;
    } else {
      static uint32_t startTime = millis();
      if (millis() - startTime > 10000) {
        showIPOnMatrix();
        ipShown = true;
      }
    }
  }
  if (updateCheckPending && !updateCheckInProgress) {
    updateCheckInProgress = true;
    updateCheckPending = false;
    performUpdateCheck();
    updateCheckInProgress = false;
    updateCheckLastRun = millis();
  }
}

// -----------------------
void handleMqttPublish() {
#if USE_MQTT
  static uint32_t lastMqttPublish = 0;
  if (millis() - lastMqttPublish >= 900) {
    lastMqttPublish = millis();
    publishMqttState();
  }
#endif
}

// -----------------------
void handleHeapMonitor() {
#if HEAP_SIZE_PRINT
  static uint32_t mem_timer = 0;
  if (millis() - mem_timer >= 10000UL) {
    mem_timer = millis();
#if GENERAL_LOG
    SYSLOG.add("Heap Size = ");
    SYSLOG.add(ESP.getFreeHeap());
    SYSLOG.add("\n");
#endif
  }
#endif // HEAP_SIZE_PRINT
}

// -----------------------
void handleFavorites() {
  static uint32_t lastFav = 0;
  if (millis() - lastFav >= 100) {
    lastFav = millis();
    if (Favorites::instance().HandleFavorites(&ONflag, &currentMode, &loadingFlag,
#if USE_DAWN
        &dawnFlag,
#endif
#if USE_SUNSET
        &sunsetFlag,
#endif
        &random_on, &selectedSettings, udpBuffer)) {
#if USE_BLYNK
      updateRemoteBlynkParams();
#endif
      SetBrightness(modes[currentMode].Brightness);
    }
  }
}

// -----------------------
#if USE_TM1637
void handleTM1637() {
  if (!tm1637Enabled) return;
  static uint32_t tmr_clock = 0;
  if (millis() - tmr_clock >= 500UL) {
    tmr_clock = millis();
    if (inClockWeatherMode && showClock && DisplayFlag == 0) {
      dotFlag = !dotFlag;
      boolean points[4] = {0, 0, 0, 0};
      points[1] = dotFlag;
      display.setSegmentPoints(points);
    }
    Display_Timer();
  }
  static uint32_t lastBlink = 0;
  if ((dawnFlag == 1 || sunsetFlag == 1) && (millis() - lastBlink >= 250)) {
    lastBlink = millis();
    clockTicker_blink();
  }
}
#endif

// -----------------------
void handleRunningText() {
#if LED_PANEL || USE_TM1637 || USE_ST7789
  if (!runTextEnabled || !textIsRunning || !ONflag) return;

  uint32_t delayMs = map(constrain(SpeedRunningText, 20, 220), 20, 220, 80, 8);
  static uint32_t lastShift = 0;
  if (millis() - lastShift < delayMs) return;
  lastShift = millis();

  CRGB textColor = CHSV(ColorRunningText, 255U, 255U);
  if (runTextOver) {
    clearTextAreaOnly();
    Fill_String = fillString(TextTicker, textColor, true);
  } else {
    FastLED.clear();
    Fill_String = fillString(TextTicker, textColor, false);
  }
  FastLED.show();
  if (Fill_String) finishRunningText();
#endif
}

// ----------------------------------------------------------------------
// проверка активности лампы
inline bool isLampActive() {
  return ONflag ||
#if USE_DAWN
         dawnFlag ||
#endif
#if USE_SUNSET
         sunsetFlag ||
#endif
         Favorites::instance().FavoritesRunning;
}

// ----------------------------------------------------------------------
// рассвет (мп3)
#if USE_DAWN
void handleDawnMp3() {
#if USE_MP3_PLAYER
  if (dawnFlag == 1) {
    if (dawnflag_sound) {
      if (mp3Enabled && alarm_sound_flag && (millis() - alarm_timer > 1000)) {
        alarm_timer = millis();
        send_command(0x06, FEEDBACK, 0, min((uint8_t)(dawnPosition / 8), alarm_volume));
      }
      return;
    }

    // Отправляем команду паузы только если плеер включён
    if (mp3Enabled) {
      send_command(0x0E, FEEDBACK, 0, 0);
      mp3_stop = true;
    } else {
      mp3_stop = true; // просто сбросим флаг
    }
    dawnflag_sound = 1;

    if (mp3Enabled && alarm_sound_on) {
      delay(mp3_delay);
      mp3_folder = AlarmFolder;
      alarm_timer = millis();
      send_command(0x06, FEEDBACK, 0, 0);
      alarm_sound_flag = true;
      mp3_folder_last = mp3_folder;
      play_sound();
    }
    return;
  }
  else if (dawnflag_sound) {
    if (mp3Enabled) {
      send_command(0x06, FEEDBACK, 0, eff_volume);
      delay(mp3_delay);
      alarm_sound_flag = false;
      dawnflag_sound = 0;
      send_command(0x0E, FEEDBACK, 0, 0);
      mp3_stop = true;
      delay(mp3_delay);
    } else {
      // Плеер выключен – только сбрасываем флаги, без команд
      alarm_sound_flag = false;
      dawnflag_sound = 0;
      mp3_stop = true;
    }
  }
#endif // USE_MP3_PLAYER
}
#else
#define handleDawnMp3() // пустышка
#endif

// ----------------------------------------------------------------------
// закат (мп3)
#if USE_SUNSET
void handleSunsetMp3() {
#if USE_MP3_PLAYER
  if (sunsetFlag == 1) {
    if (sunsetflag_sound) {
      if (mp3Enabled && sunset_sound_flag && (millis() - sunset_timer > 1000)) {
        sunset_timer = millis();
        send_command(0x06, FEEDBACK, 0, min((uint8_t)(sunsetPosition / 8), sunset_volume));
      }
      return;
    }

    if (mp3Enabled) {
      send_command(0x0E, FEEDBACK, 0, 0);
      mp3_stop = true;
    } else {
      mp3_stop = true;
    }
    sunsetflag_sound = 1;

    if (mp3Enabled && sunset_sound_on) {
      delay(mp3_delay);
      mp3_folder = SunsetFolder;
      sunset_timer = millis();
      send_command(0x06, FEEDBACK, 0, 0);
      sunset_sound_flag = true;
      mp3_folder_last = mp3_folder;
      play_sound();
    }
    return;
  }
  else if (sunsetflag_sound) {
    if (mp3Enabled) {
      send_command(0x06, FEEDBACK, 0, eff_volume);
      delay(mp3_delay);
      sunset_sound_flag = false;
      sunsetflag_sound = 0;
      send_command(0x0E, FEEDBACK, 0, 0);
      mp3_stop = true;
      delay(mp3_delay);
    } else {
      sunset_sound_flag = false;
      sunsetflag_sound = 0;
      mp3_stop = true;
    }
  }
#endif // USE_MP3_PLAYER
}
#else
#define handleSunsetMp3() // и тут пустышка
#endif

// ----------------------------------------------------------------------
// вывод IP адреса в serial
void printIPInfo() {
  auto& wifi = Wifi::instance();
  Serial.println("=== ЛАМПА ГОТОВА ===");
  Serial.printf("Точка доступа: %s   IP: %s\n", AP_NAME.c_str(), wifi.apIP().toString().c_str());
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("WiFi подключён: %s IP: %s RSSI: %d dBm\n", wifi.getSSID().c_str(), wifi.localIP().toString().c_str(), wifi.getRSSI());
  } else {
    Serial.println("WiFi не подключён. Активна только точка доступа");
  }
  Serial.println("===================");
}

void waitForWiFiOrTimeout(uint32_t timeoutMs = 10000) {
  uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }
    delay(100);
    Wifi::instance().loop();
  }
  printIPInfo();
}

// ----------------------------------------------------------------------
// завершающие настройки
void finalizeSetup() {
  randomSeed(micros());
  changePower();
  loadingFlag = true;
  initTelnetDebug();
  waitForWiFiOrTimeout();
  delay(80);
  if (currentMode >= MODE_AMOUNT) currentMode = 0;
  loadBrightnessForMode(currentMode);
  FastLED.show();

#if GENERAL_LOG
  SYSLOG.add("Яркость после загрузки: %u", FastLED.getBrightness());
#endif
}

// ******************************************************************************************************************************************************

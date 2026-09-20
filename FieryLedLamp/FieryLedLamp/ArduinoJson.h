// ***************************************************************************** ArduinoJson.ino ********************************************************
#include "Prototypes.h"
#include "Extern.h"
#include "SystemLog.h"
#include <ArduinoJson.h> // Version 6.19.4
// -------------------------------------------

const size_t JSON_BUFFER_SIZE = 4096;

// ==================================================================== ЧТЕНИЕ JSON ===================================================================

String jsonRead(String &json, String name) {
  DynamicJsonDocument doc(JSON_BUFFER_SIZE);
  DeserializationError error = deserializeJson(doc, json);
  if (error) return "";
  JsonObject obj = doc.as<JsonObject>();
  return obj[name].as<String>();
}

int jsonReadtoInt(String &json, String name, int defaultValue = 0) {
  DynamicJsonDocument doc(JSON_BUFFER_SIZE);
  DeserializationError error = deserializeJson(doc, json);
  if (error) return defaultValue;
  if (!doc.containsKey(name)) return defaultValue;
  return doc[name].as<int>();
}

// ====================================================================== ЗАПИСЬ JSON ===================================================================

void jsonWrite(String &json, String name, String volume) {
  DynamicJsonDocument doc(JSON_BUFFER_SIZE);
  DeserializationError error = deserializeJson(doc, json);
  if (error) return;
  doc[name] = volume;
  json = "";
  serializeJson(doc, json);
}

void jsonWrite(String &json, String name, int volume) {
  DynamicJsonDocument doc(JSON_BUFFER_SIZE);
  DeserializationError error = deserializeJson(doc, json);
  if (error) return;
  doc[name] = volume;
  json = "";
  serializeJson(doc, json);
}

void jsonWriteMultiple(String &json, std::initializer_list<std::pair<const char*, String>> kvs) {
  DynamicJsonDocument doc(JSON_BUFFER_SIZE);
  DeserializationError error = deserializeJson(doc, json);
  if (error) return;
  for (const auto& kv : kvs) {
    doc[kv.first] = kv.second;
  }
  json = "";
  serializeJson(doc, json);
}

// ======================================================================= РАБОТА С ФАЙЛАМИ =============================================================

bool writeFile(const String& fileName, const String& strings) {
  if (!LittleFS.begin(false)) {
#if GENERAL_LOG
    SYSLOG.add("writeFile: LittleFS не смонтирован (%s)", fileName.c_str());
#endif
    return false;
  }

  File configFile = LittleFS.open("/" + fileName, "w");
  if (!configFile) {
#if GENERAL_LOG
    SYSLOG.add("writeFile: не удалось открыть файл %s для записи", fileName.c_str());
#endif
    return false;
  }

  size_t written = configFile.print(strings);
  configFile.close();

  if (written != strings.length()) {
#if GENERAL_LOG
    SYSLOG.add("writeFile: записано %u из %u байт (%s)", (unsigned)written, (unsigned)strings.length(), fileName.c_str());
#endif
    return false;
  }
  return true;
}

String readFile(const String& fileName, size_t maxSize = 8192) {
  File configFile = LittleFS.open("/" + fileName, "r");
  if (!configFile) return "Failed";

  size_t fileSize = configFile.size();
  if (fileSize > maxSize) {
    configFile.close();
    return "Large";
  }

  String content;
  content.reserve(fileSize + 1);

  uint8_t buffer[512];
  while (configFile.available()) {
    int bytesRead = configFile.read(buffer, sizeof(buffer));
    if (bytesRead <= 0) break;
    content.concat((char*)buffer, bytesRead);
  }

  configFile.close();

  if (content.length() == 0) return "Failed";
  return content;
}

String safeReadFile(const String& fileName, size_t maxSize) {
  String content = readFile(fileName, maxSize);
  if (content.isEmpty() || content == "Failed" || content == "Large") {
    return "{}";
  }

  DynamicJsonDocument doc(JSON_BUFFER_SIZE);
  DeserializationError err = deserializeJson(doc, content);
  if (err) {
#if GENERAL_LOG
    SYSLOG.add("safeReadFile: невалидный JSON в %s (%s), сброс в {}", fileName.c_str(), err.c_str());
#endif
    return "{}";
  }

  return content;
}

// ======================================================================== СОХРАНЕНИЕ КОНФИГОВ =========================================================

// Адресное сохранение: если which == nullptr - сохранить все
// Если which задан - сохранить только соответствующий файл
//
//   "setup" - config.json
//   "wifi" - config_wifi.json
//   "ledpanel" - config_led_panel.json
//   "ledinter" - config_led_interval.json
//   "ledmatr" - config_led_matrix.json
//   "cycle" - config_cycle.json
//   "button" - config_button.json
//   "mqtt" - config_mqtt.json
//   "weather" - config_weather.json
//   "mp3" - config_mp3.json
//   "st7789" - config_st7789.json
//   "multilamp" - config_multilamp.json
//   "alarm" - config_alarm.json
//   "sunset" - config_sunset.json
//   "schedule" - config_schedule.json
//
// sound_list.json здесь НЕ сохраняется - он пишется в handle_sound_set

bool saveConfig(const char* which = nullptr) {
  auto match = [&](const char* name) -> bool {
    return (which == nullptr) || (strcmp(which, name) == 0);
  };

  bool allSuccess = true;

  if (match("setup")) {
    if (!writeFile(F("config.json"), configSetup)) allSuccess = false;
  }
  if (match("wifi")) {
    if (!writeFile(F("config_wifi.json"), configWiFi)) allSuccess = false;
  }
  if (match("ledpanel")) {
    if (!writeFile(F("config_led_panel.json"), configLedPanel)) allSuccess = false;
  }
  if (match("ledinter")) {
    if (!writeFile(F("config_led_interval.json"), configLedInterval)) allSuccess = false;
  }
  if (match("ledmatr")) {
    if (!writeFile(F("config_led_matrix.json"), configLED)) allSuccess = false;
  }
  if (match("cycle")) {
    if (!writeFile(F("config_cycle.json"), configCycle)) allSuccess = false;
  }

#if USE_BUTTON
  if (match("button")) {
    if (!writeFile(F("config_button.json"), configButton)) allSuccess = false;
  }
#endif
#if USE_MQTT
  if (match("mqtt")) {
    if (!writeFile(F("config_mqtt.json"), configMQTT)) allSuccess = false;
  }
#endif
#if USE_WEATHER
  if (match("weather")) {
    if (!writeFile(F("config_weather.json"), configWeather)) allSuccess = false;
  }
#endif
#if USE_MP3_PLAYER
  if (match("mp3")) {
    if (!writeFile(F("config_mp3.json"), configMP3)) allSuccess = false;
  }
#endif
#if USE_ST7789
  if (match("st7789")) {
    if (!writeFile(F("config_st7789.json"), configST7789)) allSuccess = false;
  }
#endif
#if USE_MULTILAMP
  if (match("multilamp")) {
    if (!writeFile(F("config_multilamp.json"), configMultilamp)) allSuccess = false;
  }
#endif
#if USE_DAWN
  if (match("alarm")) {
    if (!writeFile(F("config_alarm.json"), configAlarm)) allSuccess = false;
  }
#endif
#if USE_SUNSET
  if (match("sunset"))   {
    if (!writeFile(F("config_sunset.json"), configSunset)) allSuccess = false;
  }
#endif
#if USE_SCHEDULE
  if (match("schedule")) {
    if (!writeFile(F("config_schedule.json"), configSchedule)) allSuccess = false;
  }
#endif

  if (!allSuccess) {
#if GENERAL_LOG
    SYSLOG.add("saveConfig: не все файлы сохранены (which=%s)", which ? which : "all");
#endif
  }

  return allSuccess;
}

// ****************************************************************************************************************************************************

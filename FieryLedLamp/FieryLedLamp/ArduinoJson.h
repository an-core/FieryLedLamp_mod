// ***************************************************************************** ArduinoJson.h **********************************************************
#include "Prototypes.h"
#include "Extern.h"
#include "SystemLog.h"
#include <ArduinoJson.h> // Version 6.19.4
// -------------------------------------------

const size_t JSON_BUFFER_SIZE = 8192;

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

bool writeFile(const String& fileName, const String& strings) {
  File configFile = LittleFS.open("/" + fileName, "w");
  if (!configFile) return false;
  size_t written = configFile.print(strings);
  configFile.close();
  return (written == strings.length());
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
    content.concat((char*)buffer, bytesRead);
  }

  configFile.close();

  if (content.length() == 0) return "Failed";
  return content;
}

String safeReadFile(const String& fileName, size_t maxSize) {
  String content = readFile(fileName, maxSize);
  if (content.isEmpty() || content == "Failed" || content == "Large") return "{}";
  return content;
}

void saveConfig() {
  bool allSuccess = true;

  if (!writeFile(F("config.json"), configSetup)) allSuccess = false;
  if (!writeFile(F("config_wifi.json"), configWiFi)) allSuccess = false;
  if (!writeFile(F("config_led_panel.json"), configLedPanel)) allSuccess = false;
  if (!writeFile(F("config_led_interval.json"), configLedInterval)) allSuccess = false;
  if (!writeFile(F("config_led_matrix.json"), configLED)) allSuccess = false;
  if (!writeFile(F("config_cycle.json"), configCycle)) allSuccess = false;
  
#if USE_BUTTON
    if (!writeFile(F("config_button.json"), configButton)) allSuccess = false;
#endif
#if USE_MQTT
    if (!writeFile(F("config_mqtt.json"), configMQTT)) allSuccess = false;
#endif
#if USE_WEATHER
    if (!writeFile(F("config_weather.json"), configWeather)) allSuccess = false;
#endif
#if USE_MP3_PLAYER
    if (!writeFile(F("sound_list.json"), soundList)) allSuccess = false;
    if (!writeFile(F("config_mp3.json"), configMP3)) allSuccess = false;
#endif
#if USE_ST7789
    if (!writeFile(F("config_st7789.json"), configST7789)) allSuccess = false;
#endif
#if USE_MULTILAMP
    if (!writeFile(F("config_multilamp.json"), configMultilamp)) allSuccess = false;
#endif
#if USE_DAWN
    if (!writeFile(F("config_alarm.json"), configAlarm)) allSuccess = false;
#endif
#if USE_SUNSET
    if (!writeFile(F("config_sunset.json"), configSunset)) allSuccess = false;
#endif
#if USE_SCHEDULE
    if (!writeFile(F("config_schedule.json"), configSchedule)) allSuccess = false;
#endif
} // void saveConfig()

// ******************************************************************************************************************************************************

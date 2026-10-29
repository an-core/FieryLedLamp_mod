// ****************************************************************************** Ardu_Json.h ***********************************************************
#pragma once
#include "SystemLog.h"
// -------------------------------------------

const size_t JSON_BUFFER_SIZE = 8192;

// ----------------------------------------------------------------------- ЧТЕНИЕ JSON -----------------------------------------------------------------

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

// ----------------------------------------------------------------------- ЗАПИСЬ JSON -----------------------------------------------------------------

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

// ----------------------------------------------------------------------- РАБОТА С ФАЙЛАМИ ------------------------------------------------------------

bool writeFile(const String& fileName, const String& strings) {
#if GENERAL_LOG
  SYSLOG.add(">>> writeFile START: %s (%u bytes)", fileName.c_str(), (unsigned)strings.length());
#endif
  File configFile = LittleFS.open("/" + fileName, "w");
  if (!configFile) {
#if GENERAL_LOG
    SYSLOG.add(">>> writeFile FAIL: %s", fileName.c_str());
#endif
    return false;
  }
  size_t written = configFile.print(strings);
  configFile.close();
#if GENERAL_LOG
  SYSLOG.add(">>> writeFile END: %s (written=%u)", fileName.c_str(), (unsigned)written);
#endif
  return written == strings.length();
}

String readFile(const String& fileName, size_t maxSize = 8192) {
  File configFile = LittleFS.open("/" + fileName, "r");

#if GENERAL_LOG
  SYSLOG.add("readFile(%s): open=%d", fileName.c_str(), (bool)configFile);
#endif

  if (!configFile) return "Failed";

  size_t fileSize = configFile.size();

#if GENERAL_LOG
  SYSLOG.add("readFile(%s): size=%u, maxSize=%u", fileName.c_str(), (unsigned)fileSize, (unsigned)maxSize);
#endif

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

#if GENERAL_LOG
  SYSLOG.add("readFile(%s): read=%u head=%.40s", fileName.c_str(), (unsigned)content.length(), content.c_str());
#endif

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

// ******************************************************************************************************************************************************

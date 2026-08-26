// ***************************************************************************** SystemLog.ino ***********************************************************
#include "SystemLog.h"
// --------------------

#if DEBUG_ENABLED
#include <esp_intr.h>

portMUX_TYPE SystemLog::logMutex = portMUX_INITIALIZER_UNLOCKED;

SystemLog::SystemLog() {
  addOutput(&Serial);
}

SystemLog::~SystemLog() {}

SystemLog& SystemLog::instance() {
  static SystemLog instance;
  return instance;
}

void SystemLog::add(const char* format, ...) {
  if (!enabled) return;
  if (minLevel > LOG_LEVEL_INFO) return;

  char buf[LINE_MAX_LEN];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);

  internalAdd(LOG_LEVEL_INFO, buf, false);
}

void SystemLog::addRaw(const char* msg) {
  if (!enabled) return;
  if (minLevel > LOG_LEVEL_INFO) return;
  internalAdd(LOG_LEVEL_INFO, msg, true);
}

void SystemLog::add(LogLevel level, const char* format, ...) {
  if (!enabled) return;
  if (level < minLevel) return;

  char buf[LINE_MAX_LEN];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);

  internalAdd(level, buf, false);
}

void SystemLog::addRaw(LogLevel level, const char* msg) {
  if (!enabled) return;
  if (level < minLevel) return;
  internalAdd(level, msg, true);
}

void SystemLog::internalAdd(LogLevel level, const char* msg, bool isRaw) {
  if (!msg) return;

  char fullMsg[LINE_MAX_LEN + 40];
  fullMsg[0] = '\0';

  if (!isRaw) {
    char timeStr[20] = "";
    if (useTimestamp) {
      unsigned long ts = timestampProvider ? timestampProvider() : millis();
      snprintf(timeStr, sizeof(timeStr), "[%6lu] ", ts);
    }
    
    const char* levelStr = "";
    switch (level) {
      case LOG_LEVEL_VERBOSE: levelStr = "V "; break;
      case LOG_LEVEL_DEBUG: levelStr = "D "; break;
      case LOG_LEVEL_INFO: levelStr = "I "; break;
      case LOG_LEVEL_WARN: levelStr = "W "; break;
      case LOG_LEVEL_ERROR: levelStr = "E "; break;
      default: break;
    }
    
    snprintf(fullMsg, sizeof(fullMsg), "%s%s%s", timeStr, levelStr, msg);
  } // if (!isRaw)
  else {
    strncpy(fullMsg, msg, sizeof(fullMsg) - 1);
    fullMsg[sizeof(fullMsg) - 1] = '\0';
  }
  
  // вывод в serial
  for (int i = 0; i < outputCount; i++) {
    if (outputs[i]) outputs[i]->println(fullMsg);
  }
  
  // буфер
  portENTER_CRITICAL(&SystemLog::logMutex);
  strncpy(buffer[head], fullMsg, LINE_MAX_LEN - 1);
  buffer[head][LINE_MAX_LEN - 1] = '\0';
  head = (head + 1) % MAX_LINES;
  if (count < MAX_LINES) count++;
  else tail = (tail + 1) % MAX_LINES;
  portEXIT_CRITICAL(&SystemLog::logMutex);
} // void SystemLog::internalAdd(LogLevel level, const char* msg, bool isRaw)

void SystemLog::clear() {
  portENTER_CRITICAL(&SystemLog::logMutex);
  head = tail = count = 0;
  portEXIT_CRITICAL(&SystemLog::logMutex);
}

void SystemLog::addOutput(Print* output) {
  if (!output || outputCount >= 8) return;
  for (int i = 0; i < outputCount; i++) {
    if (outputs[i] == output) return;
  }
  outputs[outputCount++] = output;
}

void SystemLog::removeOutput(Print* output) {
  for (int i = 0; i < outputCount; i++) {
    if (outputs[i] == output) {
      for (int j = i; j < outputCount - 1; j++) {
        outputs[j] = outputs[j + 1];
      }
      outputCount--;
      return;
    }
  }
}

void SystemLog::clearOutputs() {
  outputCount = 0;
}

void SystemLog::setEnabled(bool enable) {
  enabled = enable;
}

bool SystemLog::isEnabled() const {
  return enabled;
}

void SystemLog::setMinLevel(LogLevel level) {
  minLevel = level;
}

void SystemLog::setTimestamp(bool enable) {
  useTimestamp = enable;
}

void SystemLog::setTimestampProvider(unsigned long (*provider)()) {
  timestampProvider = provider ? provider : millis;
}

String SystemLog::getAll() {
  String out;
  int idx = tail;
  for (int i = 0; i < count; i++) {
    out += buffer[idx];
    out += "\n";
    idx = (idx + 1) % MAX_LINES;
  }
  return out;
}

#endif // DEBUG_ENABLED

// *******************************************************************************************************************************************************

// *************************************************************************** SystemLog.h ***************************************************************
#pragma once
// --------------
#include <Arduino.h>
#include "Types.h"
// ---------------------

#if DEBUG_ENABLED
#include <esp_intr.h>
#include <stdarg.h>

class SystemLog {
  public:
    static const int MAX_LINES = 200;
    static const int LINE_MAX_LEN = 128;

    void add(const char* format, ...);
    void addRaw(const char* msg);
    String getAll();
    void clear();
    void setEnabled(bool enabled);
    bool isEnabled() const;

    void add(LogLevel level, const char* format, ...);
    void addRaw(LogLevel level, const char* msg);

    void setMinLevel(LogLevel level);
    void setTimestamp(bool enable);
    void setTimestampProvider(unsigned long (*provider)());

    void addOutput(Print* output);
    void removeOutput(Print* output);
    void clearOutputs();

    static SystemLog& instance();

  private:
    SystemLog(); // это чтобы добавить Serial
    ~SystemLog();

    SystemLog(const SystemLog&) = delete;
    SystemLog& operator=(const SystemLog&) = delete;
    static portMUX_TYPE logMutex;

    void internalAdd(LogLevel level, const char* msg, bool isRaw);
    void formatTimestamp(char* buffer, size_t len);

    // кольцевой буфер
    char buffer[MAX_LINES][LINE_MAX_LEN];
    int head = 0, tail = 0, count = 0;
    bool enabled = true;
    LogLevel minLevel = LOG_LEVEL_INFO;
    bool useTimestamp = false;
    unsigned long (*timestampProvider)() = millis;

    Print* outputs[8];
    int outputCount = 0;
}; // class SystemLog

#else

#define LOG (void)0

#endif // DEBUG_ENABLED

// *******************************************************************************************************************************************************

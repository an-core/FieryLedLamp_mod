// ******************************************************************************* Time.h ****************************************************************
#pragma once
// --------------
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <esp_sntp.h>
#include <functional>
#include "Constants.h"
// ----------------------

class Time {
  private:
    Time();
    ~Time() = default;

    Time(const Time&) = delete;
    Time& operator=(const Time&) = delete;

    static void onSNTPTimeSync(struct timeval *tv);
    void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);
    void internalTimeSyncNotify();

    std::function<void()> timecb;
    String userntp;
    bool initialized;

  public:
    static Time& instance() {
      static Time instance;
      return instance;
    }

    void forcesync();
    void tzsetup(const char* tz);
    void setcustomntp(const char* ntp);
    String getserver(uint8_t idx);
    void attach_callback(std::function<void()> callback);
    void ntpodhcp(bool enable);
    void enable();
    void disable();
    void setOffset(int val);
    long int getOffset() const;
    bool isTimeSet() const;
    void getDateTimeString(String &buf, const time_t _tstamp = 0);
    String getFormattedShortTime();
    time_t setTime(const char* datetimestr);

    static time_t now();
    static int hour(time_t t);
    static int minute(time_t t);
    static int weekday(time_t t);
    static bool isLeapYear(uint16_t year);
    static bool isAM(time_t t) {
      return hour(t) < 12;
    }
    static bool isPM(time_t t) {
      return hour(t) >= 12;
    }
    int hour() const {
      return hour(now());
    }
    int minute() const {
      return minute(now());
    }
    int weekday() const {
      return weekday(now());
    }
    bool isAM() const {
      return hour() < 12;
    }
    bool isPM() const {
      return hour() >= 12;
    }
}; // class Time

// ******************************************************************************************************************************************************

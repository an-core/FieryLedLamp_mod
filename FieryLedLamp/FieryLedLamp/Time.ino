// ****************************************************************************** Time.ino **************************************************************
#include "Time.h"
#include <sys/time.h>
#include <esp_sntp.h>
// -------------------------

static constexpr const char* NTP_SERVER_PRIMARY  = "pool.ntp.org";
static constexpr const char* NTP_SERVER_SECONDARY = "time.nist.gov";
static constexpr const char* NTP_SERVER_GOOGLE = "time.google.com";

Time::Time() : initialized(false) {
  sntp_set_time_sync_notification_cb([](struct timeval * tv) {
    Time::onSNTPTimeSync(tv);
  });

#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  sntp_servermode_dhcp(1);
#endif

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  esp_sntp_stop();
#else
  sntp_stop();
#endif

  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
    this->onWiFiEvent(event, info);
  });
} // Time::Time() : initialized(false)

void Time::onSNTPTimeSync(struct timeval* tv) {
  Time::instance().internalTimeSyncNotify();
}

void Time::internalTimeSyncNotify() {
  if (timecb) timecb();
}

void Time::onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
#else
    case SYSTEM_EVENT_STA_GOT_IP:
#endif
     
      {
        const char* servers[] = { NTP_SERVER_PRIMARY, NTP_SERVER_SECONDARY, userntp.c_str() };
        for (int i = 0; i < 3; ++i) {
          if (servers[i] && servers[i][0] != '\0') {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
            esp_sntp_setservername(i, servers[i]);
#else
            sntp_setservername(i, const_cast<char*>(servers[i]));
#endif
          } // if (servers[i] && servers[i][0] != '\0')
        } // for (int i = 0; i < 3; ++i)
      }
      
      enable();
      break;

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
#else
    case SYSTEM_EVENT_STA_DISCONNECTED:
#endif

      disable();
      break;

    default:
      break;
  } // switch (event)
} // void Time::onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)

void Time::forcesync() {
  disable();
  enable();
}

void Time::enable() {
  if (initialized) return;
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  esp_sntp_init();
#else
  sntp_init();
#endif
  initialized = true;
}

void Time::disable() {
  if (!initialized) return;
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  esp_sntp_stop();
#else
  sntp_stop();
#endif
  initialized = false;
}

void Time::ntpodhcp(bool enable) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  esp_sntp_servermode_dhcp(enable);
  if (!enable) {
    esp_sntp_setservername(0, NTP_SERVER_PRIMARY);
    esp_sntp_setservername(1, NTP_SERVER_SECONDARY);
    if (userntp.length())
      esp_sntp_setservername(CUSTOM_NTP_INDEX, userntp.c_str());
  } // if (!enable)
#else
  sntp_servermode_dhcp(enable);
  if (!enable) {
    sntp_setservername(0, const_cast<char*>(NTP_SERVER_PRIMARY));
    sntp_setservername(1, const_cast<char*>(NTP_SERVER_SECONDARY));
    if (userntp.length())
      sntp_setservername(CUSTOM_NTP_INDEX, const_cast<char*>(userntp.c_str()));
  }
#endif
} // void Time::ntpodhcp(bool enable)

void Time::tzsetup(const char* tz) {
  if (!tz || !*tz) tz = "MSK-3";
  configTzTime(tz, NTP_SERVER_PRIMARY, NTP_SERVER_SECONDARY, NTP_SERVER_GOOGLE);
  forcesync();
}

void Time::setcustomntp(const char* ntp) {
  if (!ntp || !*ntp) return;
  userntp = ntp;
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  esp_sntp_setservername(CUSTOM_NTP_INDEX, userntp.c_str());
#else
  sntp_setservername(CUSTOM_NTP_INDEX, const_cast<char*>(userntp.c_str()));
#endif
}

String Time::getserver(uint8_t idx) {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
  const char* name = esp_sntp_getservername(idx);
  if (name) return String(name);
  ip_addr_t ip;
  
  if (esp_sntp_getserver(idx, &ip) == ESP_OK) {
    return IPAddress(ip.u_addr.ip4.addr).toString();
  }
  
#else
  const char* name = sntp_getservername(idx);
  if (name) return String(name);
  const ip_addr_t* ip = sntp_getserver(idx);
  if (ip) return IPAddress(ip->u_addr.ip4.addr).toString();
#endif
  return String();
} // String Time::getserver(uint8_t idx)

void Time::attach_callback(std::function<void()> callback) {
  timecb = std::move(callback);
}

time_t Time::now() {
  return time(nullptr);
}

bool Time::isTimeSet() const {
  time_t t = now();
  return (t > 946684800UL);
}

String Time::getFormattedShortTime() {
  char buffer[6];
  time_t t = now();
  struct tm tm;
  localtime_r(&t, &tm);
  sprintf_P(buffer, PSTR("%02u:%02u"), tm.tm_hour, tm.tm_min);
  return String(buffer);
}

void Time::getDateTimeString(String& buf, const time_t _tstamp) {
  char tmpBuf[20];
  time_t timestamp = (_tstamp != 0) ? _tstamp : now();
  struct tm tm;
  localtime_r(&timestamp, &tm);
  sprintf_P(tmpBuf, PSTR("%04u-%02u-%02uT%02u:%02u"), tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
  buf.concat(tmpBuf);
}

time_t Time::setTime(const char* datetimestr) {
  if (!datetimestr) return 0;
  struct tm tmStruct = {};
  const char* format = (strlen(datetimestr) < 19) ? "%Y-%m-%dT%H:%M" : "%Y-%m-%dT%H:%M:%S";
  
  if (strptime(datetimestr, format, &tmStruct) == nullptr) {
    return 0;
  }
  
  time_t t = mktime(&tmStruct);
  struct timeval tv = { t, 0 };
  settimeofday(&tv, nullptr);
  return t;
}

long int Time::getOffset() const {
  time_t t = now();
  struct tm tm;
  localtime_r(&t, &tm);
#ifdef __USE_GNU
  return tm.tm_gmtoff;
#else
  struct timezone tz;
  gettimeofday(nullptr, &tz);
  return -tz.tz_minuteswest * 60;
#endif
}

void Time::setOffset(int /*val*/) {
}

int Time::hour(time_t t) {
  struct tm tm;
  localtime_r(&t, &tm);
  return tm.tm_hour;
}

int Time::minute(time_t t) {
  struct tm tm;
  localtime_r(&t, &tm);
  return tm.tm_min;
}

int Time::weekday(time_t t) {
  struct tm tm;
  localtime_r(&t, &tm);
  return tm.tm_wday;
}

bool Time::isLeapYear(uint16_t year) {
  return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
}

// ******************************************************************************************************************************************************

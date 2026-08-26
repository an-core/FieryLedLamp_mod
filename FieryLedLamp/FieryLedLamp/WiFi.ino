#include "WiFi.h"
#include "SystemLog.h"
#include <WiFiClient.h>
#include <nvs_flash.h>

Wifi::Wifi()
  : staState(STA_IDLE)
  , connected(false)
  , apAlways(true)
  , useStaticIP(false)
  , forcedAPActive(false)
  , apActive(false)
  , internetAvailable(false)
  , internetCheckPending(false)
  , internetCheckStart(0)
  , noInternetStartTime(0)
  , wifiTimeoutMs(30000)
  , reconnectIntervalMs(30000)
  , checkIntervalMs(300000UL)
  , forcedApTimeoutMs(60000UL)
  , lastReconnectAttempt(0)
  , lastInternetCheck(0)
  , lastQuickCheck(0)
  , connectInProgress(false)
  , connectStartTime(0)
{
  apIPAddr.fromString("192.168.4.1");
  apGateway.fromString("192.168.4.1");
  apSubnet.fromString("255.255.255.0");
}

Wifi::~Wifi() {}

// -------------------------------------------------------------------
void Wifi::setAPSettings(const String& ssid, const String& password) {
  apSSID = ssid;
  apPassword = password;
#if WIFI_LOG
  SYSLOG.add("Настройки AP обновлены: SSID=%s", apSSID.c_str());
#endif
}

void Wifi::restartAP() {
#if WIFI_LOG
  SYSLOG.add("Перезапуск точки доступа...");
#endif
  WiFi.softAPdisconnect(true);
  delay(10);
  apActive = false;
  WiFi.mode(WIFI_AP_STA);
  initAP();
}

// ----------------------------------------------------------
void Wifi::begin() {
  static bool initialized = false;
  if (initialized) return;
  initialized = true;

  String main_ssid = jsonRead(configWiFi, "ssid");
  String main_pass = jsonRead(configWiFi, "password");
  if (main_ssid == "null" || main_ssid.isEmpty()) main_ssid = "";
  if (main_pass == "null" || main_pass.isEmpty()) main_pass = "";

  String ap_ssid = jsonRead(configWiFi, "ssidAP");
  String ap_pass = jsonRead(configWiFi, "passwordAP");
  if (ap_ssid.isEmpty() || ap_ssid == "null") ap_ssid = "FieryLedLamp";
  if (ap_pass.isEmpty() || ap_pass == "null") ap_pass = "";
  apSSID = ap_ssid;
  apPassword = ap_pass;

  apAlways = (jsonReadtoInt(configWiFi, "ap_always", 1) == 1);
  useStaticIP = (jsonReadtoInt(configWiFi, "s_IP", 0) == 1);
  int wifi_multi_enabled = jsonReadtoInt(configWiFi, "wifi_multi", 0);

  wifiTimeoutMs = jsonReadtoInt(configWiFi, "wifi_timeout", 60) * 1000UL;
  if (wifiTimeoutMs == 0) wifiTimeoutMs = 60000UL;

  forcedApTimeoutMs = jsonReadtoInt(configWiFi, "forced_ap_after", 2) * 60UL * 1000UL;
  if (forcedApTimeoutMs == 0) forcedApTimeoutMs = 120000UL;

  checkIntervalMs = jsonReadtoInt(configWiFi, "wifi_check_interval", 5) * 60UL * 1000UL;
  if (checkIntervalMs == 0) checkIntervalMs = 300000UL;

  reconnectIntervalMs = jsonReadtoInt(configWiFi, "wifi_reconnect_interval", 30) * 1000UL;
  if (reconnectIntervalMs == 0) reconnectIntervalMs = 30000UL;

  if (useStaticIP) {
    String ip_str = jsonRead(configWiFi, "ip");
    String gw_str = jsonRead(configWiFi, "gateway");
    String subnet_str = jsonRead(configWiFi, "subnet");
    String dns_str = jsonRead(configWiFi, "dns");
    if (ip_str != "" && ip_str != "null") staticIP.fromString(ip_str);
    if (gw_str != "" && gw_str != "null") gateway.fromString(gw_str);
    if (subnet_str != "" && subnet_str != "null") subnet.fromString(subnet_str);
    if (dns_str != "" && dns_str != "null") dns1.fromString(dns_str);
    else dns1 = IPAddress(8, 8, 8, 8);
  }

#if WIFI_LOG
  SYSLOG.add("=== WiFi настройки ===");
  SYSLOG.add("Основной SSID: %s", main_ssid.c_str());
  SYSLOG.add("AP Always: %d", apAlways);
  SYSLOG.add("Статический IP: %d", useStaticIP);
  SYSLOG.add("WiFi Multi: %d", wifi_multi_enabled);
  SYSLOG.add("Таймаут: %lu сек", wifiTimeoutMs / 1000);
  SYSLOG.add("======================");
#endif

  WiFi.persistent(false);

  if (apAlways) {
    WiFi.mode(WIFI_AP_STA);
    initAP();
  } else {
    WiFi.mode(WIFI_STA);
    apActive = false;
  }

  wifiMulti = WiFiMulti();
  networkCount = 0;

  if (useStaticIP) {
    WiFi.config(staticIP, gateway, subnet, dns1);
  } else {
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  }

  if (!main_ssid.isEmpty()) {
    wifiMulti.addAP(main_ssid.c_str(), main_pass.c_str());
    networkCount++;
  }

  if (wifi_multi_enabled) {
    for (int i = 2; i <= 5; i++) {
      String ssidKey = "ssid" + String(i);
      String passKey = "password" + String(i);
      String ssidVal = jsonRead(configWiFi, ssidKey);
      String passVal = jsonRead(configWiFi, passKey);
      if (!ssidVal.isEmpty() && ssidVal != "null") {
        wifiMulti.addAP(ssidVal.c_str(), passVal.c_str());
        networkCount++;
#if WIFI_LOG
        SYSLOG.add("Добавлена сеть %d: %s", i, ssidVal.c_str());
#endif
      }
    }
  }

  if (networkCount > 0) {
    initSTA();
  } else {
#if WIFI_LOG
    SYSLOG.add("Нет сохранённых сетей, запускаем AP");
#endif
    if (!apActive) {
      WiFi.mode(WIFI_AP_STA);
      initAP();
    }
  }
}

// ----------------------------------------------------------
void Wifi::loop() {
  if (WiFi.status() == WL_CONNECTED && !connected) {
    connected = true;
    staState = STA_CONNECTED;
    connectInProgress = false;
    if (forcedAPActive) {
      stopForcedAP();
    }
  }

  unsigned long now = millis();

  manageConnection();

  if (!apActive && !forcedAPActive && !connectInProgress) {
    if (now - lastQuickCheck >= 5000UL) {
      lastQuickCheck = now;
      if (WiFi.status() != WL_CONNECTED) {
#if WIFI_LOG
        SYSLOG.add("WiFi отключен - инициируем переподключение");
#endif
        initSTA();
      }
    }
  }

  if (connected && (now - lastInternetCheck >= 10000UL)) {
    lastInternetCheck = now;
    if (!internetCheckPending) {
      checkInternetAsync();
    }
  }

  if ((apActive || forcedAPActive) && !connectInProgress && !connected && hasAnyNetworks()) {
    if (now - lastReconnectAttempt >= reconnectIntervalMs) {
      lastReconnectAttempt = now;
      initSTA();
    }
  }

  if (!forcedAPActive && !apActive && connected) {
    if (!internetAvailable) {
      if (noInternetStartTime == 0) noInternetStartTime = now;
      else if (now - noInternetStartTime >= forcedApTimeoutMs && forcedApTimeoutMs > 0) {
#if WIFI_LOG
        SYSLOG.add("Нет интернета %lu сек - включаем AP", forcedApTimeoutMs / 1000);
#endif
        startForcedAP();
        return;
      }
    } else {
      noInternetStartTime = 0;
    }
  }

  if (!apActive && !forcedAPActive && !connectInProgress && WiFi.status() != WL_CONNECTED) {
    if (now - lastReconnectAttempt > 60000) {
      lastReconnectAttempt = now;
      ensureAP();
    }
  }
}

// ----------------------------------------------------------
void Wifi::setAPAlways(bool enable) {
  apAlways = enable;
  jsonWrite(configWiFi, "ap_always", enable ? "1" : "0");
  saveConfig();

  if (enable) {
    if (WiFi.getMode() == WIFI_STA) {
      WiFi.mode(WIFI_AP_STA);
      initAP();
    } else if (!apActive) {
      initAP();
    }
  } else {
    if (apActive && !forcedAPActive) {
      WiFi.softAPdisconnect(true);
      apActive = false;
      if (WiFi.getMode() == WIFI_AP_STA) {
        WiFi.mode(WIFI_STA);
      }
    }
  }
}

// ----------------------------------------------------------
void Wifi::ensureAP() {
  if (!apActive && !forcedAPActive && !isConnected()) {
#if WIFI_LOG
    SYSLOG.add("ensureAP: нет WiFi, запускаем AP");
#endif
    startForcedAP();
  }
}

// ----------------------------------------------------------
bool Wifi::isConnected() const {
  return connected && (WiFi.status() == WL_CONNECTED);
}

IPAddress Wifi::localIP() const {
  return WiFi.localIP();
}

IPAddress Wifi::apIP() const {
  return WiFi.softAPIP();
}

String Wifi::getSSID() const {
  return WiFi.SSID();
}

int32_t Wifi::getRSSI() const {
  return WiFi.RSSI();
}

// ----------------------------------------------------------
void Wifi::checkInternetAsync() {
  if (internetCheckPending) return;
  if (WiFi.status() != WL_CONNECTED) {
    internetAvailable = false;
    return;
  }

  internetCheckPending = true;
  internetCheckStart = millis();

  WiFiClient client;
  client.setTimeout(2000);
  if (client.connect(IPAddress(8, 8, 8, 8), 53)) {
    client.stop();
    onInternetCheckResult(true);
  } else {
    onInternetCheckResult(false);
  }
}

void Wifi::onInternetCheckResult(bool has) {
  internetAvailable = has;
  internetCheckPending = false;
  if (has) {
#if WIFI_LOG
    SYSLOG.add("Интернет доступен");
#endif
    if (!myTime.isTimeSet()) {
      initTimeAndTimezone();
    }
  } else {
#if WIFI_LOG
    SYSLOG.add("Интернет недоступен");
#endif
  }
}

// ----------------------------------------------------------
void Wifi::initAP() {
  if (apActive) {
#if WIFI_LOG
    SYSLOG.add("initAP: AP уже активна");
#endif
    return;
  }

  if (apSSID.isEmpty()) {
    extern String AP_NAME;
    extern String AP_PASS;
    apSSID = AP_NAME;
    apPassword = AP_PASS;
    if (apSSID.isEmpty() || apSSID == "null") apSSID = "FieryLedLamp";
    if (apPassword.isEmpty() || apPassword == "null") apPassword = "";
  }

  if (!WiFi.softAPConfig(apIPAddr, apGateway, apSubnet)) {
#if WIFI_LOG
    SYSLOG.add("Ошибка конфигурации AP");
#endif
  }

  bool ok = WiFi.softAP(apSSID.c_str(), apPassword.c_str());
  if (ok) {
    apActive = true;
#if WIFI_LOG
    SYSLOG.add("Точка доступа запущена");
    SYSLOG.add("SSID: %s", apSSID.c_str());
    SYSLOG.add("IP: %s", WiFi.softAPIP().toString().c_str());
    SYSLOG.add("Пароль: %s", apPassword.isEmpty() ? "(нет)" : "****");
    SYSLOG.add("================================");
#endif
  } else {
    apActive = false;
#if WIFI_LOG
    SYSLOG.add("Ошибка запуска точки доступа");
#endif
  }
}

// ----------------------------------------------------------
void Wifi::initSTA() {
  if (connectInProgress) {
#if WIFI_LOG
    SYSLOG.add("initSTA: уже идёт подключение");
#endif
    return;
  }

  if (networkCount == 0) {
#if WIFI_LOG
    SYSLOG.add("initSTA: нет сетей для подключения");
#endif
    return;
  }

  connectInProgress = true;
  connectStartTime = millis();
  staState = STA_CONNECTING;
#if WIFI_LOG
  SYSLOG.add("Запущено неблокирующее подключение к WiFi");
#endif
}

IPAddress Wifi::getCurrentIP() const {
  if (WiFi.status() == WL_CONNECTED) {
    IPAddress ip = WiFi.localIP();
    if (ip != IPAddress(0, 0, 0, 0)) {
      return ip;
    }
  }
  return WiFi.softAPIP();
}

void Wifi::manageConnection() {
  if (WiFi.status() == WL_CONNECTED && !connected) {
    connected = true;
    staState = STA_CONNECTED;
    connectInProgress = false;
    if (forcedAPActive) {
      stopForcedAP();
    }
#if WIFI_LOG
    SYSLOG.add("WiFi уже подключён");
#endif
    return;
  }

  if (!connectInProgress) return;

  unsigned long now = millis();
  if (now - connectStartTime >= wifiTimeoutMs) {
    connectInProgress = false;
    staState = STA_FAILED;
    connected = false;
#if WIFI_LOG
    SYSLOG.add("Таймаут подключения к WiFi");
#endif
    return;
  }

  wl_status_t status = (wl_status_t)wifiMulti.run();
#if WIFI_LOG
  SYSLOG.add("WiFi статус: %d", status);
#endif

  if (status == WL_CONNECTED) {
    connectInProgress = false;
    staState = STA_CONNECTED;
    connected = true;
    noInternetStartTime = 0;
    if (forcedAPActive) {
      stopForcedAP();
    }
#if WIFI_LOG
    SYSLOG.add("WiFi подключён! SSID: %s IP: %s", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
#endif
    initTimeAndTimezone();
  }
}

// ----------------------------------------------------------
void Wifi::forceReconnect() {
#if WIFI_LOG
  SYSLOG.add("Принудительное переподключение WiFi");
#endif
  WiFi.disconnect();
  delay(10);
  connectInProgress = false;
  staState = STA_IDLE;
  connected = false;
  initSTA();
}

// ----------------------------------------------------------
void Wifi::startForcedAP() {
  if (apActive) {
#if WIFI_LOG
    SYSLOG.add("AP уже активна, пропускаем");
#endif
    return;
  }
#if WIFI_LOG
  SYSLOG.add("Запуск принудительного AP");
#endif
  WiFi.disconnect();
  delay(10);
  WiFi.mode(WIFI_AP_STA);
  initAP();
  forcedAPActive = true;
  connected = false;
  staState = STA_IDLE;
  connectInProgress = false;
#if WIFI_LOG
  SYSLOG.add("Принудительный AP активирован");
#endif
}

void Wifi::stopForcedAP() {
  if (!forcedAPActive) return;
  forcedAPActive = false;
  noInternetStartTime = 0;
  internetAvailable = false;

  if (apAlways) {
    if (!apActive) {
      WiFi.mode(WIFI_AP_STA);
      initAP();
    }
  } else {
    WiFi.softAPdisconnect(true);
    apActive = false;
    WiFi.mode(WIFI_STA);
  }

  if (WiFi.status() != WL_CONNECTED && hasAnyNetworks()) {
    initSTA();
  } else if (WiFi.status() == WL_CONNECTED) {
    connected = true;
    staState = STA_CONNECTED;
  }
}

// ----------------------------------------------------------
void Wifi::clearWiFiCache() {
#if WIFI_LOG
  SYSLOG.add("Очистка кэша WiFi...");
#endif
  WiFi.disconnect(true);
  delay(10);
  WiFi.mode(WIFI_OFF);
  delay(10);
  WiFi.mode(WIFI_STA);
  delay(10);
#if WIFI_LOG
  SYSLOG.add("Кэш WiFi очищен");
#endif
}

bool Wifi::hasAnyNetworks() {
  String main_ssid = jsonRead(configWiFi, "ssid");
  if (main_ssid.length() > 0 && main_ssid != "null" && main_ssid != "") return true;

  int wifi_multi_enabled = jsonReadtoInt(configWiFi, "wifi_multi", 0);
  if (wifi_multi_enabled) {
    for (int i = 2; i <= 5; i++) {
      String ssidKey = "ssid" + String(i);
      String ssidVal = jsonRead(configWiFi, ssidKey);
      if (ssidVal.length() > 0 && ssidVal != "null" && ssidVal != "") return true;
    }
  }
  return false;
}

// ----------------------------------------------------------
void Wifi::setStaticIP(const IPAddress& ip, const IPAddress& gw, const IPAddress& subnet, const IPAddress& dns) {
  useStaticIP = true;
  staticIP = ip; gateway = gw; this->subnet = subnet; dns1 = dns;
  jsonWrite(configWiFi, "s_IP", "1");
  jsonWrite(configWiFi, "ip", ip.toString());
  jsonWrite(configWiFi, "gateway", gw.toString());
  jsonWrite(configWiFi, "subnet", subnet.toString());
  jsonWrite(configWiFi, "dns", dns.toString());
  saveConfig();
}

void Wifi::disableStaticIP() {
  useStaticIP = false;
  jsonWrite(configWiFi, "s_IP", "0");
  saveConfig();
}

void Wifi::addNetwork(const char* ssid, const char* password) {
  wifiMulti.addAP(ssid, password);
}

void Wifi::clearNetworks() {
  wifiMulti = WiFiMulti();
}

void Wifi::setWiFiTimeout(uint32_t timeoutSec) {
  wifiTimeoutMs = timeoutSec * 1000UL;
  jsonWrite(configWiFi, "wifi_timeout", timeoutSec);
  saveConfig();
}

void Wifi::setReconnectInterval(uint32_t intervalSec) {
  reconnectIntervalMs = intervalSec * 1000UL;
  jsonWrite(configWiFi, "wifi_reconnect_interval", intervalSec);
  saveConfig();
}

void Wifi::setCheckInterval(uint32_t intervalSec) {
  checkIntervalMs = intervalSec * 1000UL;
  jsonWrite(configWiFi, "wifi_check_interval", intervalSec);
  saveConfig();
}

void Wifi::setForcedAPTimeout(uint32_t timeoutSec) {
  forcedApTimeoutMs = timeoutSec * 1000UL;
  jsonWrite(configWiFi, "forced_ap_after", timeoutSec);
  saveConfig();
}

void Wifi::reconnect() {
  forceReconnect();
}

bool Wifi::hasInternet() {
  return internetAvailable;
}

// **************************************************************************** WiFi.ino ****************************************************************

#include "WiFi.h"
#include "SystemLog.h"
#include <WiFiClient.h>
#include <nvs_flash.h>
#include <esp_wifi.h>
// ----------------------

Wifi::Wifi()
  : staState(STA_IDLE)
  , connected(false)
  , apAlways(true)
  , useStaticIP(false)
  , forcedAPActive(false)
  , apActive(false)
  , internetAvailable(false)
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

// ----------------------------------------------------------

void Wifi::restartAP() {
#if WIFI_LOG
  SYSLOG.add("Перезапуск точки доступа...");
#endif
  WiFi.softAPdisconnect(true);
  vTaskDelay(pdMS_TO_TICKS(10));
  apActive = false;
  WiFi.mode(WIFI_AP_STA);
  initAP();
}

// ----------------------------------------------------------

void Wifi::begin() {
  static bool initialized = false;
  if (initialized) return;
  initialized = true;
  xTaskCreatePinnedToCore(
  [](void* arg) {
    static_cast<Wifi*>(arg)->wifiManagerTask();
  },
  "wifi_mgr", 4096, this, 1, &wifiManagerTaskHandle, 0);

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
    initAP();                          // сначала AP
    WiFi.setSleep(false);              // потом sleep off
    esp_wifi_set_ps(WIFI_PS_NONE);     // потом power save off
  } else {
    WiFi.mode(WIFI_STA);
    apActive = false;
    WiFi.setSleep(false);
    esp_wifi_set_ps(WIFI_PS_NONE);
  }

  wifiMulti = WiFiMulti();
  networkCount = 0;

  if (useStaticIP) {
    WiFi.config(staticIP, gateway, subnet, dns1);
  }
  // else: НЕ вызываем WiFi.config() — DHCP по умолчанию

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
  unsigned long now = millis();

  // --- Синхронизация connected с реальным статусом ---
  if (WiFi.status() == WL_CONNECTED) {
    if (!connected) {
      connected = true;
      staState = STA_CONNECTED;
      connectInProgress = false;
      wifiRunActive = false;
      if (forcedAPActive) stopForcedAP();
      Time::instance().enable();
    }
  } else {
    if (connected) {
      connected = false;
      staState = STA_IDLE;
    }
  }

  manageConnection();

  // --- Триггер переподключения ВНЕ блока wifiRunActive ---
  if (!apActive && !forcedAPActive && !connectInProgress && !connected && hasAnyNetworks()) {
    if (now - lastReconnectAttempt >= checkIntervalMs) {
      lastReconnectAttempt = now;
#if WIFI_LOG
      SYSLOG.add("Попытка подключиться к STA");
#endif
      initSTA();
    }
  }

  // --- Сама попытка подключения (раз в 30 с, пока не подключились) ---
  if (wifiRunActive && WiFi.status() != WL_CONNECTED) {
    static uint32_t lastRun = millis() - 30000;   // первая попытка сразу
    if (now - lastRun >= 30000) {
      lastRun = now;
      wl_status_t st = (wl_status_t)wifiMulti.run(2000);  // не 5000!
      // если WL_CONNECTED — верхний блок подхватит на следующем проходе
    }
  }

  // --- Форсированный AP если долго не выходит ---
  if (!apActive && !forcedAPActive && !connected && WiFi.status() != WL_CONNECTED) {
    static uint32_t failSince = 0;
    if (failSince == 0) failSince = now;
    if (now - failSince > 60000) {
      failSince = 0;
      needStartForcedAP = true;
    }
  } else {
    // связь восстановилась или уже есть AP — обнуляем счётчик
    static uint32_t resetDummy = 0;
    (void)resetDummy;
  }
}

  // ----------------------------------------------------------

  void Wifi::setAPAlways(bool enable) {
    apAlways = enable;
    jsonWrite(configWiFi, "ap_always", enable ? "1" : "0");
    saveConfig();

    needSetAPAlways = true;
    apAlwaysNewValue = enable;
  }

  // ----------------------------------------------------------

  void Wifi::ensureAP() {
    // ВАЖНО: проверяем connected, а не isConnected(),
    // потому что WiFi.status() может мигнуть на 0 при сканировании,
    // а STA реально ещё подключён (connected == true).
    if (!apActive && !forcedAPActive && !connected) {
#if WIFI_LOG
      SYSLOG.add("ensureAP: STA реально не подключён, запрашиваем AP");
#endif
      needStartForcedAP = true;
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

  void deferredTasksTask(void* pv) {
    esp_task_wdt_delete(NULL);
    for (;;) {
      if (g_flagAutoTz) {
        g_flagAutoTz = false;
        if (Wifi::instance().isConnected()) {
          autoDetectTimezone();
        }
      }
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }

  void initDeferredTasks() {
    xTaskCreatePinnedToCore(deferredTasksTask, "deferred", 4096, NULL, 1, NULL, 1);
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

    bool ok = WiFi.softAP(apSSID.c_str(), apPassword.c_str(), 6, false, 4);
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
    if (connectInProgress) return;
    if (networkCount == 0) return;
    if (!hasAnyNetworks()) return;

    connectInProgress = true;
    wifiRunActive = true;
    connectStartTime = millis();
    staState = STA_CONNECTING;
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

  // ----------------------------------------------------------

  void Wifi::wifiManagerTask() {
    esp_task_wdt_delete(NULL);

    for (;;) {
#if WIFI_LOG
      static uint32_t lastLogState = 0;
      if (millis() - lastLogState > 5000) {
        lastLogState = millis();
        SYSLOG.add("wifiMgr: wifiRunActive=%d, status=%d, apActive=%d, forced=%d, connected=%d, netCount=%d",
                   wifiRunActive, WiFi.status(), apActive, forcedAPActive, connected, networkCount);
      }
#endif
      if (needRestartAP) {
        needRestartAP = false;
        restartAP();
        vTaskDelay(pdMS_TO_TICKS(200));
        continue;
      }

      if (needSetAPAlways) {
        needSetAPAlways = false;
        bool enable = apAlwaysNewValue;

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
        vTaskDelay(pdMS_TO_TICKS(200));
        continue;
      }

      if (needStartForcedAP) {
        needStartForcedAP = false;
        startForcedAP();
        vTaskDelay(pdMS_TO_TICKS(200));
        continue;
      }

      if (needStopForcedAP) {
        needStopForcedAP = false;
        stopForcedAP();
        vTaskDelay(pdMS_TO_TICKS(200));
        continue;
      }

      if (needInitSTA) {
        needInitSTA = false;
        if (WiFi.status() != WL_CONNECTED) {
          WiFi.disconnect();
          WiFi.mode(WIFI_OFF);
          vTaskDelay(pdMS_TO_TICKS(100));
          WiFi.mode(WIFI_AP_STA);
          vTaskDelay(pdMS_TO_TICKS(100));
          initSTA();
        } else {
          connected = true;
          staState = STA_CONNECTED;
          connectInProgress = false;
          wifiRunActive = false;
        }
      }

      // --- Правка A: wifiMulti.run() раз в 30 секунд, а не в горячем цикле ---
      if (wifiRunActive && WiFi.status() != WL_CONNECTED) {
        static uint32_t lastRunAttempt = millis() - 30000;   // первая попытка сразу
        uint32_t nowMs = millis();
        if (nowMs - lastRunAttempt >= 30000) {
          lastRunAttempt = nowMs;
#if WIFI_LOG
          SYSLOG.add("wifiMulti.run(): попытка подключения");
#endif
          wl_status_t status = (wl_status_t)wifiMulti.run(5000);
          if (status == WL_CONNECTED) {
            wifiRunActive = false;
            connected = true;
            staState = STA_CONNECTED;
            connectInProgress = false;
          }
        }
        vTaskDelay(pdMS_TO_TICKS(200));
      } else {
        if (wifiRunActive) {
          wifiRunActive = false;
          connected = true;
          staState = STA_CONNECTED;
          connectInProgress = false;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
      }
    }
  }

  // ----------------------------------------------------------

  void Wifi::manageConnection() {
    if (WiFi.status() == WL_CONNECTED && !connected) {
      connected = true;
      staState = STA_CONNECTED;
      connectInProgress = false;
      wifiRunActive = false;
      if (forcedAPActive) needStopForcedAP = true;
#if WIFI_LOG
      SYSLOG.add("WiFi уже подключён");
#endif
      return;
    }

    if (!connectInProgress) {
      return;
    }

    unsigned long now = millis();
    if (now - connectStartTime >= wifiTimeoutMs) {
      connectInProgress = false;
      wifiRunActive = false;
      staState = STA_FAILED;
      connected = false;
#if WIFI_LOG
      SYSLOG.add("Таймаут подключения к WiFi");
#endif
    }
  }

  // ----------------------------------------------------------

  void Wifi::forceReconnect() {
#if WIFI_LOG
    SYSLOG.add("Принудительное переподключение WiFi");
#endif
    needInitSTA = true;
    connectInProgress = false;
    connected = false;
    staState = STA_IDLE;
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
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect();
      vTaskDelay(pdMS_TO_TICKS(10));
      // STA реально нет — сбрасываем флаги
      connected = false;
      staState = STA_IDLE;
      connectInProgress = false;
      wifiRunActive = false;
    }
    // Если STA был подключён — НЕ трогаем connected/wifiRunActive,
    // STA продолжает работать в AP+STA режиме.
    WiFi.mode(WIFI_AP_STA);
    initAP();
    forcedAPActive = true;
#if WIFI_LOG
    SYSLOG.add("Принудительный AP активирован");
#endif
  }

  // ----------------------------------------------------------

  void Wifi::stopForcedAP() {
    if (!forcedAPActive) return;
    forcedAPActive = false;
    wifiRunActive = false;
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
    vTaskDelay(pdMS_TO_TICKS(10));
    WiFi.mode(WIFI_OFF);
    vTaskDelay(pdMS_TO_TICKS(10));
    WiFi.mode(WIFI_STA);
    vTaskDelay(pdMS_TO_TICKS(10));
#if WIFI_LOG
    SYSLOG.add("Кэш WiFi очищен");
#endif
  }

  // ----------------------------------------------------------

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

  void Wifi::setStaticIP(const IPAddress & ip, const IPAddress & gw, const IPAddress & subnet, const IPAddress & dns) {
    useStaticIP = true;
    staticIP = ip; gateway = gw; this->subnet = subnet; dns1 = dns;
    jsonWrite(configWiFi, "s_IP", "1");
    jsonWrite(configWiFi, "ip", ip.toString());
    jsonWrite(configWiFi, "gateway", gw.toString());
    jsonWrite(configWiFi, "subnet", subnet.toString());
    jsonWrite(configWiFi, "dns", dns.toString());
    saveConfig();
  }

  // ----------------------------------------------------------

  void Wifi::disableStaticIP() {
    useStaticIP = false;
    jsonWrite(configWiFi, "s_IP", "0");
    saveConfig();
  }

  // ----------------------------------------------------------

  void Wifi::addNetwork(const char* ssid, const char* password) {
    wifiMulti.addAP(ssid, password);
  }

  // ----------------------------------------------------------

  void Wifi::clearNetworks() {
    wifiMulti = WiFiMulti();
  }

  // ----------------------------------------------------------

  void Wifi::setWiFiTimeout(uint32_t timeoutSec) {
    wifiTimeoutMs = timeoutSec * 1000UL;
    jsonWrite(configWiFi, "wifi_timeout", timeoutSec);
    saveConfig();
  }

  // ----------------------------------------------------------

  void Wifi::setReconnectInterval(uint32_t intervalSec) {
    reconnectIntervalMs = intervalSec * 1000UL;
    jsonWrite(configWiFi, "wifi_reconnect_interval", intervalSec);
    saveConfig();
  }

  // ----------------------------------------------------------

  void Wifi::setCheckInterval(uint32_t intervalMin) {
    checkIntervalMs = intervalMin * 60UL * 1000UL;
    jsonWrite(configWiFi, "wifi_check_interval", intervalMin);
    saveConfig();
  }

  // ----------------------------------------------------------

  void Wifi::setForcedAPTimeout(uint32_t timeoutMin) {
    forcedApTimeoutMs = timeoutMin * 60UL * 1000UL;
    jsonWrite(configWiFi, "forced_ap_after", timeoutMin);
    saveConfig();
  }

  // ----------------------------------------------------------

  void Wifi::reconnect() {
    forceReconnect();
  }

  // ----------------------------------------------------------

  bool Wifi::hasInternet() {
    return internetAvailable;
  }

  // ******************************************************************************************************************************************************

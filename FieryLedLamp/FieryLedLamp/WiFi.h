// **************************************************************************** wifi.h *******************************************************************

#pragma once
#include <WiFi.h>
#include <WiFiMulti.h>
#include <IPAddress.h>
#include "Extern.h"
#include "Prototypes.h"
// -------------------------

class Wifi {
  public:
    static Wifi& instance() {
      static Wifi instance;
      return instance;
    }

    void begin();
    void loop();
    bool isConnected() const;
    IPAddress localIP() const;
    IPAddress apIP() const;
    String getSSID() const;
    int32_t getRSSI() const;

    void setAPAlways(bool enable);
    bool isAPAlways() const {
      return apAlways;
    }

    void setStaticIP(const IPAddress& ip, const IPAddress& gw, const IPAddress& subnet, const IPAddress& dns = IPAddress(8, 8, 8, 8));
    void disableStaticIP();
    void addNetwork(const char* ssid, const char* password);
    void clearNetworks();
    void clearWiFiCache();
    void setWiFiTimeout(uint32_t timeoutSec);
    void setReconnectInterval(uint32_t intervalSec);
    void setCheckInterval(uint32_t intervalSec);
    void setForcedAPTimeout(uint32_t timeoutSec);
    void setAPSettings(const String& ssid, const String& password);
    void restartAP();
    void reconnect();
    bool hasInternet();
    void forceReconnect();
    void startForcedAP();
    void stopForcedAP();
    void ensureAP();
    bool hasAnyNetworks();
    int networkCount;
    IPAddress getCurrentIP() const;

  private:
    Wifi();
    ~Wifi();
    Wifi(const Wifi&) = delete;
    Wifi& operator=(const Wifi&) = delete;

    enum WiFiState {
      STA_IDLE,
      STA_CONNECTING,
      STA_CONNECTED,
      STA_FAILED
    };

    void initAP();
    void initSTA();
    void manageConnection();
    void checkInternetAsync();
    void onInternetCheckResult(bool has);

    WiFiMulti wifiMulti;
    WiFiState staState;
    bool connected;
    bool apAlways;
    bool useStaticIP;
    bool forcedAPActive;
    bool apActive;
    bool internetAvailable;
    bool internetCheckPending;
    unsigned long internetCheckStart;
    unsigned long noInternetStartTime;

    IPAddress staticIP, gateway, subnet, dns1;
    uint32_t wifiTimeoutMs;
    uint32_t reconnectIntervalMs;
    uint32_t checkIntervalMs;
    uint32_t forcedApTimeoutMs;

    unsigned long lastReconnectAttempt;
    unsigned long lastInternetCheck;
    unsigned long lastQuickCheck;

    IPAddress apIPAddr, apGateway, apSubnet;
    String apSSID, apPassword;

    unsigned long connectStartTime;
    bool connectInProgress;
};

// ******************************************************************************************************************************************************

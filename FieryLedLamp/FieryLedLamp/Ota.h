// **************************************************************************** Ota.h *******************************************************************
#pragma once
// --------------
#if USE_OTA
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include "Constants.h"
#include "Types.h"
#include "Extern.h"
#include "Prototypes.h"
#include "WiFi.h"
// -----------------------------

class Ota {
  public:
    static Ota& instance() {
      static Ota instance;
      return instance;
    }

    void setWarningDelegate(ShowWarningDelegate delegate) {
      showWarningDelegate = delegate;
    }

    bool RequestOtaUpdate() {
      if (!Wifi::instance().isConnected()) return false;
      if (otaFlag == OtaPhase::None) {
        momentOfOtaStart = millis();

        prepareForOTA();

#if BACKUP_CFG_FILES
        if (saveWifiSettingsToBackup()) {
          if (showWarningDelegate != nullptr) showWarningDelegate(CRGB::Cyan, 500U, 200U);
        }
#endif
        if (showWarningDelegate != nullptr) showWarningDelegate(CRGB::Yellow, 2000U, 500U);
        startOtaUpdate();
        return true;
      }
      return false;
    }

    void HandleOtaUpdate() {
      if (otaFlag == OtaPhase::None && momentOfOtaStart != 0 && millis() - momentOfOtaStart >= ESP_CONF_TIMEOUT * 1000UL) {
        if (showWarningDelegate != nullptr) showWarningDelegate(CRGB::Red, 2000U, 500U);
        momentOfOtaStart = 0;
        otaFlag = OtaPhase::None;
        ESP.restart();
        return;
      }

      if (otaFlag == OtaPhase::InProgress) {
        ArduinoOTA.handle();

        if (millis() - uploadStartTime > 300000UL) {
          if (showWarningDelegate != nullptr) showWarningDelegate(CRGB::Red, 2000U, 500U);
          otaFlag = OtaPhase::None;
          momentOfOtaStart = 0;
          ESP.restart();
        }

        static uint32_t lastOtaActivityCheck = 0;
        if (millis() - lastOtaActivityCheck >= 30000UL) {
          lastOtaActivityCheck = millis();
        }

      } else {
        uploadStartTime = 0;
      }

    }

    bool isOtaActive() const {
      return (otaFlag == OtaPhase::InProgress);
    }

    void abortOta() {
      if (isOtaActive()) {
        otaFlag = OtaPhase::None;
        momentOfOtaStart = 0;
        if (showWarningDelegate != nullptr) showWarningDelegate(CRGB::Red, 1000U, 250U);
      }
    }

  private:
    Ota() = default;
    ~Ota() = default;

    Ota(const Ota&) = delete;
    Ota& operator=(const Ota&) = delete;

    OtaPhase otaFlag = OtaPhase::None;
    uint32_t momentOfOtaStart = 0;
    uint32_t uploadStartTime = 0;
    ShowWarningDelegate showWarningDelegate = nullptr;

    bool saveWifiSettingsToBackup() {
#if BACKUP_CFG_FILES
      extern String configSetup;
      if (configSetup.length() > 0 && configSetup != "{}") {
        return false;
      }
#endif
      return false;
    }

    void prepareForOTA() {
      if (nightClockEnabled) {
        jsonWrite(configLedPanel, "night_clock_enabled", "1");
        jsonWrite(configLedPanel, "night_clock_brightness", nightClockBrightness);
        jsonWrite(configLedPanel, "night_clock_hue", nightClockHue);
      } else {
        jsonWrite(configLedPanel, "night_clock_enabled", "0");
      }

      jsonWrite(configSetup, "eff_sel", currentMode);
      jsonWrite(configSetup, "brightness", modes[currentMode].Brightness);

      ONflag = false;
      jsonWrite(configSetup, "Power", 0);

      FastLED.setBrightness(0);
      FastLED.clear();
      FastLED.show();

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
      digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif

      systemShuttingDown = false;
      justPoweredOn = false;
      loadingFlag = false;
      nightModeBrightness = 0;

      saveConfig();
      delay(100);
    }

    void startOtaUpdate() {
      if (!Wifi::instance().isConnected()) {
        otaFlag = OtaPhase::None;
        return;
      }

      char espHostName[65];
      char lamp_name[LAMP_NAME.length() + 1];
      LAMP_NAME.toCharArray(lamp_name, LAMP_NAME.length() + 1);

#ifdef ESP32_USED
      snprintf_P(espHostName, sizeof(espHostName), PSTR("%s-%06lX"), lamp_name, ESP.getEfuseMac() >> 24);
#else
      snprintf_P(espHostName, sizeof(espHostName), PSTR("%s-%08X"), lamp_name, ESP.getChipId());
#endif

      ArduinoOTA.setPort(ESP_OTA_PORT);
      ArduinoOTA.setHostname(espHostName);

      ArduinoOTA.onStart([this]() {
        otaFlag = OtaPhase::InProgress;
        if (showWarningDelegate != nullptr) showWarningDelegate(CRGB::Blue, 500U, 100U);
      });

      ArduinoOTA.onEnd([this]() {
        if (showWarningDelegate != nullptr) showWarningDelegate(CRGB::Green, 1000U, 200U);
        otaFlag = OtaPhase::Done;
        saveConfig();
        ESP.restart();
      });

      ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {});

      ArduinoOTA.onError([this](ota_error_t error) {
        if (showWarningDelegate != nullptr) showWarningDelegate(CRGB::Red, 2000U, 500U);
        otaFlag = OtaPhase::None;
        momentOfOtaStart = 0;
        ESP.restart();
      });

      ArduinoOTA.setRebootOnSuccess(false);
      ArduinoOTA.begin();
      otaFlag = OtaPhase::InProgress;
    }
}; // class Ota
#endif // USE_OTA

// ******************************************************************************************************************************************************

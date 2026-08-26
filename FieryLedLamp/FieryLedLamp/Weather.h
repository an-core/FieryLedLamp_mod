// ******************************************************************************* Weather.h *************************************************************
#pragma once
// -----------
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Constants.h"
#include "Types.h"
#include "Extern.h"
// ----------------------------

class Weather {
  public:
    static Weather& instance() {
      static Weather instance;
      return instance;
    }

    void loadSettings();
    void update();
    void updateIfNeeded();
    float getTemperature() const {
      return currentTemp;
    }
    String getCondition() const {
      return currentCondition;
    }
    bool isAvailable() const {
      return (currentTemp > -999.0f);
    }
    String getProviderName() const {
      return actualYandex ? "Yandex" : "OpenWeather";
    }
    bool isUsingYandex() const {
      return actualYandex;
    }
    void forceUpdate();
    String getCity() const {
      return weatherCity;
    }
    bool isYandexPreferred() const {
      return preferYandex;
    }
    bool isClockWeatherMode() const {
      return inClockWeatherMode;
    }

    String getYandexGeoId() const { return yandexGeoId; }
    String getWeatherCity() const { return weatherCity; }

  private:
    Weather() = default;
    ~Weather() = default;
    Weather(const Weather&) = delete;
    Weather& operator=(const Weather&) = delete;

    uint32_t lastUpdateTime = 0;

    bool fetchFromYandex();
    bool fetchFromOpenWeather();

    String buildWeatherDescription(String baseDesc, float windSpeed, int weatherId, const String& provider, float pressureMm);
    String getYandexRussianDescription(const String& engCond, float windSpeed, int precType, float precStrength, float temp, float humidity, float pressure);

    String weatherApiKey;
    String yandexGeoId;
    String customYandexGeoId;
    String weatherCity;
    String customWeatherCity;

    bool inClockWeatherMode = false;
    bool preferYandex = true;
    bool actualYandex = true;

    float currentTemp = -999.0f;
    String currentCondition;

    WiFiClientSecure yandexClient;
    HTTPClient yandexHttp;
};

// *******************************************************************************************************************************************************

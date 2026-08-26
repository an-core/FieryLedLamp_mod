// **************************************************************************** Weather.ino **************************************************************
#include "Extern.h"
#include "Prototypes.h"
#include "Constants.h"
#include "Weather.h"
// ---------------------

void Weather::loadSettings() {
#if USE_WEATHER
  weatherApiKey = jsonRead(configWeather, "openweather_key");

  String yandexGeoFromConfig = jsonRead(configWeather, "yandex_geo");
  String customYandexGeoFromConfig = jsonRead(configWeather, "custom_yandex_geo");

  if (yandexGeoFromConfig.length() > 0 && yandexGeoFromConfig != "0") {
    yandexGeoId = yandexGeoFromConfig;
  } else if (customYandexGeoFromConfig.length() > 0) {
    yandexGeoId = customYandexGeoFromConfig;
  } else {
    yandexGeoId = "213";
  }

  customYandexGeoId = customYandexGeoFromConfig;

  String cityFromConfig = jsonRead(configWeather, "city");
  String customCityFromConfig = jsonRead(configWeather, "custom_city");

  if (cityFromConfig.length() > 0) {
    weatherCity = cityFromConfig;
  } else if (customCityFromConfig.length() > 0) {
    weatherCity = customCityFromConfig;
  } else {
    weatherCity = "";
  }

  customWeatherCity = customCityFromConfig;

  int show = jsonReadtoInt(configWeather, "show_weather", 0);
  ::inClockWeatherMode = (show == 1);

  if (show == 0) {
    jsonWrite(configWeather, "show_weather", 1);
    saveConfig();
    ::inClockWeatherMode = true;
  }

  preferYandex = (jsonReadtoInt(configWeather, "weather_source") == 0);
  lastUpdateTime = millis() - WEATHER_UPDATE_INTERVAL;

#endif // USE_WEATHER
} // void Weather::loadSettings()

// --------------------------------------------------------------------------------
#if USE_WEATHER
// ----------------------
void Weather::update() {
  if (!Wifi::instance().isConnected()) {
    return;
  }

  bool success = false;
  actualYandex = false;

  if (preferYandex && yandexGeoId.length() > 0 && yandexGeoId != "0") {
    success = fetchFromYandex();
    if (success) actualYandex = true;
  }

  if (!success && weatherApiKey.length() > 10 && weatherCity.length() > 0) {
    success = fetchFromOpenWeather();
    if (success) actualYandex = false;
  }

  if (!success) {
    currentTemp = -999.0f;
    currentCondition = "";
  } else if (currentTemp > -999.0f) {
#if WEATHER_LOG
    SYSLOG.add("На улице: %+.1f°C, %s", currentTemp, currentCondition.c_str());
#endif
  }
} // void Weather::update()

// --------------------------------------------------
void Weather::forceUpdate() {
  lastUpdateTime = 0;
}

// --------------------------------------------------
void Weather::updateIfNeeded() {
  if (millis() - lastUpdateTime >= WEATHER_UPDATE_INTERVAL) {
    update();
    lastUpdateTime = millis();
  }
}

// --------------------------------------------------------------------------------
// Яндекс.Погода
bool Weather::fetchFromYandex() {
  if (!Wifi::instance().isConnected()) return false;
  String url = "https://yandex.com/time/sync.json?geo=" + yandexGeoId + "&lang=ru";
  yandexClient.setInsecure();
  yandexClient.setTimeout(5000);
  
  if (yandexHttp.begin(yandexClient, url)) {
    yandexHttp.setTimeout(5000);
    yandexHttp.addHeader("User-Agent", "FieryLedLamp");
    int httpCode = yandexHttp.GET();
    
    if (httpCode == HTTP_CODE_OK) {
      String payload = yandexHttp.getString();
      DynamicJsonDocument doc(8192);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        JsonObject clock = doc["clocks"][yandexGeoId];
        
        if (!clock.isNull() && clock.containsKey("weather")) {
          JsonObject weatherObj = clock["weather"];
          currentTemp = weatherObj["temp"].as<float>();
          String engCond = weatherObj["condition"] | "";
          float windSpeed = weatherObj["wind_speed"] | 0.0f;
          int precType = weatherObj["prec_type"] | 0;
          float precStrength = weatherObj["prec_strength"] | 0.0f;
          float humidity = weatherObj["humidity"] | 0.0f;
          float pressure = weatherObj["pressure_pa"] | 0.0f;
          currentCondition = getYandexRussianDescription(engCond, windSpeed, precType, precStrength, currentTemp, humidity, pressure);
#if WEATHER_LOG
          SYSLOG.add("Яндекс.Погода: описание '%s'", currentCondition.c_str());
          SYSLOG.add("Яндекс.Погода: температура %.1f", currentTemp);
#endif
          return true;
          
        } //if (!clock.isNull() && clock.containsKey("weather"))
      } // if (!error)
    } // if (httpCode == HTTP_CODE_OK)
    
    yandexHttp.end();
    
  } // if (yandexHttp.begin(yandexClient, url))
  
  return false;
  
} // bool Weather::fetchFromYandex()

// --------------------------------------------------------------------------------
// Openweather
bool Weather::fetchFromOpenWeather() {
  if (weatherCity.length() == 0 || weatherApiKey.length() < 15) {
#if WEATHER_LOG
    SYSLOG.add("OpenWeather: Ошибка: не указан город или ключ");
#endif
    return false;
  } // if (weatherCity.length() == 0 || weatherApiKey.length() < 15)
  
  String url = "https://api.openweathermap.org/data/2.5/weather?q=" + weatherCity + "&appid=" + weatherApiKey + "&units=metric&lang=ru";
#if WEATHER_LOG
  SYSLOG.add("OpenWeather: Запрос: %s", url.c_str());
#endif

  WiFiClientSecure client;

  client.setInsecure();
  client.setTimeout(10000);
  HTTPClient http;
  http.setTimeout(10000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  if (!http.begin(client, url)) {
#if WEATHER_LOG
    SYSLOG.add("OpenWeather: Не удалось инициализировать HTTP");
#endif
    return false;
  }

  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    String payload = http.getString();
#if WEATHER_LOG
    SYSLOG.add("OpenWeather: HTTP ошибка: %d", httpCode);
#endif
    if (payload.length() > 0) {
#if WEATHER_LOG
      SYSLOG.add("OpenWeather: Ответ: %s", payload.c_str());
#endif
    } else {
#if WEATHER_LOG
      SYSLOG.add("OpenWeather: Ответ пустой (соединение не установлено)");
#endif
    } // else {
    http.end();
    return false;
  } // if (httpCode != HTTP_CODE_OK)

  String payload = http.getString();
  http.end();

#if WEATHER_LOG
  SYSLOG.add("OpenWeather: Получен ответ длиной %d байт", payload.length());
#endif

  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
#if WEATHER_LOG
    SYSLOG.add("OpenWeather: ошибка JSON: %s", error.c_str());
#endif
    return false;
  }

  if (doc.containsKey("cod") && doc["cod"] != 200) {
    const char* message = doc["message"] | "no message";
#if WEATHER_LOG
    SYSLOG.add("OpenWeather: API ошибка: cod=%s, message=%s", doc["cod"].as<String>().c_str(), message);
#endif
    return false;
  }

  currentTemp = doc["main"]["temp"].as<float>();
  currentCondition = doc["weather"][0]["description"] | "";
#if WEATHER_LOG
  SYSLOG.add("OpenWeather: %.1f°C, %s", currentTemp, currentCondition.c_str());
#endif
  return true;
} // bool Weather::fetchFromOpenWeather()

// --------------------------------------------------------------------------------
// описание
String Weather::buildWeatherDescription(String baseDesc, float windSpeed, int weatherId, const String& provider, float pressureMm) {
  String desc = baseDesc;
  bool hasRain = desc.indexOf("дождь") != -1 || desc.indexOf("ливень") != -1 || desc.indexOf("осадки") != -1;
  bool hasSnow = desc.indexOf("снег") != -1;
  bool hasThunder = desc.indexOf("гроза") != -1;

  if (provider == "openweather") {
    if (weatherId >= 200 && weatherId < 700) {
      if (!hasRain && !hasSnow && !hasThunder) {
        if (weatherId >= 500 && weatherId < 600) desc += ", идёт дождь";
        else if (weatherId >= 600 && weatherId < 700) desc += ", идёт снег";
        else if (weatherId >= 200 && weatherId < 300) desc += ", гроза";
      } // if (!hasRain && !hasSnow && !hasThunder)
    } // if (weatherId >= 200 && weatherId < 700)
  } // if (provider == "openweather")

  if (windSpeed > 7.0) desc += ", ветрено";
  if (pressureMm > 0) {
    if (pressureMm >= 770) desc += ", повышенное давление";
    else if (pressureMm <= 740) desc += ", пониженное давление";
  }
  return desc;
} // String Weather::buildWeatherDescription

// --------------------------------------------------------------------------------
String Weather::getYandexRussianDescription(const String& engCond, float windSpeed, int precType, float precStrength, float temp, float humidity, float pressure) {
  String desc = "";

  if (engCond.length() > 0) {
    if (engCond == "clear") desc = "ясно";
    else if (engCond == "partly-cloudy") desc = "малооблачно";
    else if (engCond == "cloudy") desc = "облачно";
    else if (engCond == "overcast") desc = "пасмурно";
    else if (engCond.indexOf("rain") >= 0 || engCond.indexOf("drizzle") >= 0) desc = "дождь";
    else if (engCond.indexOf("snow") >= 0) desc = "снег";
    else if (engCond.indexOf("thunder") >= 0) desc = "гроза";
    else desc = engCond;
    if (desc == "дождь") {
      if (engCond.indexOf("thunder") >= 0 || precStrength > 0.5) {
        desc = "дождь с грозой";
      } else if (precType == 3 || precType == 4) {
        desc = "ливень";
        if (precStrength > 0.7) {
          desc = "сильный дождь с грозой";
        }
      } // else if (precType == 3 || precType == 4)
      else if (precType == 1) {
        desc = "слабый дождь";
      }
    } // if (desc == "дождь")
    else if (desc == "гроза") {
      desc += " с дождём";
    }
  } // if (engCond.length() > 0)

  if (desc.length() == 0) {
    if (temp <= -18) desc = "сильный мороз";
    else if (temp <= -8) desc = "мороз";
    else if (temp <= 0) desc = "морозно";
    else if (temp <= 7) desc = "холодно";
    else if (temp <= 13) desc = "прохладно";
    else if (temp <= 20) desc = "комфортно";
    else if (temp <= 25) desc = "тепло";
    else if (temp <= 30) desc = "жарко";
    else desc = "очень жарко";
  }

  if (windSpeed >= 7.0) {
    desc += ", ветрено";
  }

  return desc;
} // String Weather::getYandexRussianDescription

// ---------------------------------------------------

// города для Яндекса (по Geo-id)
String getCityNameByGeo(String geo) {
  if (geo == "213") return "Москва";
  if (geo == "2") return "Санкт-Петербург";
  if (geo == "54") return "Екатеринбург";
  if (geo == "43") return "Казань";
  if (geo == "51") return "Самара";
  if (geo == "47") return "Нижний Новгород";
  if (geo == "239") return "Сочи";
  if (geo == "35") return "Краснодар";
  if (geo == "36") return "Ставрополь";
  if (geo == "38") return "Волгоград";
  if (geo == "39") return "Ростов-на-Дону";
  if (geo == "50") return "Пермь";
  if (geo == "56") return "Челябинск";
  if (geo == "62") return "Красноярск";
  if (geo == "63") return "Иркутск";
  if (geo == "65") return "Новосибирск";
  if (geo == "66") return "Омск";
  if (geo == "67") return "Томск";
  if (geo == "75") return "Владивосток";
  if (geo == "76") return "Хабаровск";
  if (geo == "172") return "Уфа";
  if (geo == "193") return "Воронеж";
  if (geo == "197") return "Барнаул";
  if (geo == "973") return "Сургут";
  if (geo == "4") return "Белгород";
  if (geo == "5") return "Иваново";
  if (geo == "7") return "Кострома";
  if (geo == "8") return "Курск";
  if (geo == "9") return "Липецк";
  if (geo == "10") return "Орёл";
  if (geo == "11") return "Рязань";
  if (geo == "12") return "Смоленск";
  if (geo == "13") return "Тамбов";
  if (geo == "14") return "Тверь";
  if (geo == "15") return "Тула";
  if (geo == "16") return "Ярославль";
  if (geo == "20") return "Архангельск";
  if (geo == "21") return "Вологда";
  if (geo == "22") return "Калининград";
  if (geo == "23") return "Мурманск";
  if (geo == "24") return "Великий Новгород";
  if (geo == "25") return "Псков";
  if (geo == "28") return "Махачкала";
  if (geo == "30") return "Нальчик";
  if (geo == "33") return "Владикавказ";
  if (geo == "37") return "Астрахань";
  if (geo == "41") return "Йошкар-Ола";
  if (geo == "42") return "Саранск";
  if (geo == "45") return "Чебоксары";
  if (geo == "48") return "Оренбург";
  if (geo == "49") return "Пенза";
  if (geo == "53") return "Курган";
  if (geo == "64") return "Кемерово";
  if (geo == "77") return "Благовещенск";
  if (geo == "191") return "Брянск";
  if (geo == "192") return "Владимир";
  if (geo == "195") return "Ульяновск";
  return "Город " + geo;
}

// города для OpenWeather
String getRussianCityNameForOpenWeather(String city) {
  if (city == "Moscow") return "Москва";
  if (city == "Saint Petersburg") return "Санкт-Петербург";
  if (city == "Yekaterinburg") return "Екатеринбург";
  if (city == "Kazan") return "Казань";
  if (city == "Nizhny Novgorod") return "Нижний Новгород";
  if (city == "Samara") return "Самара";
  if (city == "Omsk") return "Омск";
  if (city == "Novosibirsk") return "Новосибирск";
  if (city == "Krasnoyarsk") return "Красноярск";
  if (city == "Sochi") return "Сочи";
  if (city == "Vladivostok") return "Владивосток";
  if (city == "Khabarovsk") return "Хабаровск";
  if (city == "Perm") return "Пермь";
  if (city == "Volgograd") return "Волгоград";
  if (city == "Rostov-on-Don") return "Ростов-на-Дону";
  if (city == "Ufa") return "Уфа";
  if (city == "Chelyabinsk") return "Челябинск";
  if (city == "Voronezh") return "Воронеж";
  if (city == "Krasnodar") return "Краснодар";
  if (city == "Barnaul") return "Барнаул";
  if (city == "Irkutsk") return "Иркутск";
  if (city == "Tomsk") return "Томск";
  if (city == "Orenburg") return "Оренбург";
  if (city == "Kemerovo") return "Кемерово";
  if (city == "Tula") return "Тула";
  if (city == "Ryazan") return "Рязань";
  if (city == "Astrakhan") return "Астрахань";
  if (city == "Penza") return "Пенза";
  if (city == "Lipetsk") return "Липецк";
  if (city == "Kirov") return "Киров";
  if (city == "Cheboksary") return "Чебоксары";
  if (city == "Kaliningrad") return "Калининград";
  if (city == "Bryansk") return "Брянск";
  if (city == "Ivanovo") return "Иваново";
  if (city == "Tver") return "Тверь";
  if (city == "Belgorod") return "Белгород";
  if (city == "Kursk") return "Курск";
  if (city == "Yaroslavl") return "Ярославль";
  if (city == "Vladimir") return "Владимир";
  if (city == "Smolensk") return "Смоленск";
  if (city == "Tambov") return "Тамбов";
  if (city == "Kostroma") return "Кострома";
  if (city == "Orel") return "Орёл";
  if (city == "Arkhangelsk") return "Архангельск";
  if (city == "Vologda") return "Вологда";
  if (city == "Murmansk") return "Мурманск";
  if (city == "Pskov") return "Псков";
  if (city == "Veliky Novgorod") return "Великий Новгород";
  if (city == "Makhachkala") return "Махачкала";
  if (city == "Nalchik") return "Нальчик";
  if (city == "Vladikavkaz") return "Владикавказ";
  if (city == "Stavropol") return "Ставрополь";
  if (city == "Yakutsk") return "Якутск";
  if (city == "Magadan") return "Магадан";
  if (city == "Petropavlovsk-Kamchatsky") return "Петропавловск-Камчатский";
  if (city == "Tyumen") return "Тюмень";
  if (city == "Surgut") return "Сургут";
  if (city == "Magnitogorsk") return "Магнитогорск";
  if (city == "Tolyatti") return "Тольятти";
  if (city == "Ulyanovsk") return "Ульяновск";
  if (city == "Saratov") return "Саратов";
  if (city == "Izhevsk") return "Ижевск";
  if (city == "Minsk") return "Минск";
  if (city == "Kiev") return "Киев";
  if (city == "Kharkiv") return "Харьков";
  if (city == "Odessa") return "Одесса";
  if (city == "Dnipro") return "Днепр";
  if (city == "Donetsk") return "Донецк";
  if (city == "Lviv") return "Львов";
  if (city == "Zaporizhzhia") return "Запорожье";
  if (city == "Kryvyi Rih") return "Кривой Рог";
  return city;
}

// ------------------------
#endif // USE_WEATHER

// ******************************************************************************************************************************************************

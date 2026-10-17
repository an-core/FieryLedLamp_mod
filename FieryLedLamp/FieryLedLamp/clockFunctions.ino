// ********************************************************************** clockFunctions.ino ************************************************************
#include "Extern.h"
// --------------------

// получение названия населённого пункта по координатам
String getCityNameFromCoords(float lat, float lon) {
  if (!Wifi::instance().isConnected()) return "";

  String url = "https://nominatim.openstreetmap.org/reverse?lat=" + String(lat, 6) + "&lon=" + String(lon, 6) + "&format=json&accept-language=ru";

  WiFiClientSecure client;
  client.setInsecure();
  client.setTimeout(8000);

  HTTPClient http;
  http.setTimeout(8000);

  if (!http.begin(client, url)) {
    return "";
  }

  http.addHeader("User-Agent", "FieryLedLamp/1.0");

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    http.end();
    return "";
  }

  String payload = http.getString();
  http.end();

  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, payload);
  if (error) return "";

  JsonObject address = doc["address"];
  String city = address["city"] | "";
  if (city.length() == 0) city = address["town"] | "";
  if (city.length() == 0) city = address["city_district"] | "";
  if (city.length() == 0) city = address["village"] | "";
  if (city.length() == 0) city = address["county"] | "";
  if (city.length() == 0) city = address["state"] | "";

  return city;
}

// определение IANA-часового пояса по координатам
String geoToTimezoneOnline(float lat, float lon) {
  if (!Wifi::instance().isConnected()) return "";

  String iana = "";
  {
    HTTPClient https;
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(8000);
    https.setTimeout(8000);

    String url = "https://timeapi.io/api/TimeZone/coordinate?latitude=" + String(lat, 6) + "&longitude=" + String(lon, 6);

    if (https.begin(client, url)) {
      https.addHeader("User-Agent", "FieryLedLamp/1.0");
      int code = https.GET();
      if (code == HTTP_CODE_OK) {
        String payload = https.getString();
        DynamicJsonDocument doc(1024);
        if (!deserializeJson(doc, payload)) {
          iana = doc["timeZone"] | "";
          iana.trim();
        }
      }
      https.end();
    }
  }

  if (iana.length() == 0) {
    HTTPClient http;
    WiFiClient client;
    client.setTimeout(8000);
    http.setTimeout(8000);

    String url = "https://api.bigdatacloud.net/data/reverse-geocode-client?latitude=" + String(lat, 6) + "&longitude=" + String(lon, 6) + "&localityLanguage=en";

    if (http.begin(client, url)) {
      http.addHeader("User-Agent", "FieryLedLamp/1.0");
      int code = http.GET();
      if (code == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(2048);
        if (!deserializeJson(doc, payload)) {
          JsonArray info = doc["localityInfo"]["informative"];
          for (JsonObject item : info) {
            String desc = item["description"] | "";
            if (desc == "timezone" || desc == "Time Zone") {
              iana = item["name"] | "";
              iana.trim();
              break;
            }
          }

          if (iana.length() == 0) {
            iana = doc["timeZone"] | "";
            iana.trim();
          }
        }
      }
      http.end();
    }
  }

  if (iana.length() == 0) return "";

  // маппинг IANA - POSIX
  String posix = ianaToPosix(iana);
  if (posix != iana) return posix;   // ianaToPosix знает зону (РФ/СНГ)

  // Ближний Восток
  if (iana == "Asia/Dubai")              return "GST-4";
  if (iana == "Asia/Abu_Dhabi")          return "GST-4";
  if (iana == "Asia/Muscat")             return "GST-4";
  if (iana == "Asia/Riyadh")             return "AST-3";
  if (iana == "Asia/Qatar")              return "AST-3";
  if (iana == "Asia/Kuwait")             return "AST-3";
  if (iana == "Asia/Bahrain")            return "AST-3";
  if (iana == "Asia/Tehran")             return "IRST-3:30";
  if (iana == "Asia/Jerusalem")          return "IST-2IDT,M3.4.4/26,M10.5.0";
  if (iana == "Asia/Beirut")             return "EET-2EEST,M3.5.0/0,M10.5.0/0";
  if (iana == "Asia/Amman")              return "EET-2EEST,M3.5.4/0,M10.5.5/1";

  // Азия
  if (iana == "Asia/Tokyo")              return "JST-9";
  if (iana == "Asia/Seoul")              return "KST-9";
  if (iana == "Asia/Shanghai")           return "CST-8";
  if (iana == "Asia/Hong_Kong")          return "HKT-8";
  if (iana == "Asia/Singapore")          return "SGT-8";
  if (iana == "Asia/Kuala_Lumpur")       return "MYT-8";
  if (iana == "Asia/Bangkok")            return "ICT-7";
  if (iana == "Asia/Ho_Chi_Minh")        return "ICT-7";
  if (iana == "Asia/Jakarta")            return "WIB-7";
  if (iana == "Asia/Manila")             return "PST-8";
  if (iana == "Asia/Kolkata")            return "IST-5:30";
  if (iana == "Asia/Karachi")            return "PKT-5";
  if (iana == "Asia/Dhaka")              return "BDT-6";
  if (iana == "Asia/Kathmandu")          return "NPT-5:45";
  if (iana == "Asia/Colombo")            return "IST-5:30";

  // Европа
  if (iana == "Europe/London")           return "GMT0BST,M3.5.0/1,M10.5.0";
  if (iana == "Europe/Dublin")           return "IST-1GMT0,M10.5.0,M3.5.0/1";
  if (iana == "Europe/Lisbon")           return "WET0WEST,M3.5.0/1,M10.5.0";
  if (iana == "Europe/Berlin")           return "CET-1CEST,M3.5.0,M10.5.0/3";
  if (iana == "Europe/Paris")            return "CET-1CEST,M3.5.0,M10.5.0/3";
  if (iana == "Europe/Rome")             return "CET-1CEST,M3.5.0,M10.5.0/3";
  if (iana == "Europe/Madrid")           return "CET-1CEST,M3.5.0,M10.5.0/3";
  if (iana == "Europe/Amsterdam")        return "CET-1CEST,M3.5.0,M10.5.0/3";
  if (iana == "Europe/Brussels")         return "CET-1CEST,M3.5.0,M10.5.0/3";
  if (iana == "Europe/Vienna")           return "CET-1CEST,M3.5.0,M10.5.0/3";
  if (iana == "Europe/Warsaw")           return "CET-1CEST,M3.5.0,M10.5.0/3";
  if (iana == "Europe/Prague")           return "CET-1CEST,M3.5.0,M10.5.0/3";
  if (iana == "Europe/Budapest")         return "CET-1CEST,M3.5.0,M10.5.0/3";
  if (iana == "Europe/Athens")           return "EET-2EEST,M3.5.0/3,M10.5.0/4";
  if (iana == "Europe/Helsinki")         return "EET-2EEST,M3.5.0/3,M10.5.0/4";
  if (iana == "Europe/Bucharest")        return "EET-2EEST,M3.5.0/3,M10.5.0/4";
  if (iana == "Europe/Istanbul")         return "TRT-3";
  if (iana == "Europe/Moscow")           return "MSK-3";
  if (iana == "Europe/Kiev")             return "EET-2EEST,M3.5.0/3,M10.5.0/4";

  // Америка
  if (iana == "America/New_York")        return "EST5EDT,M3.2.0,M11.1.0";
  if (iana == "America/Chicago")         return "CST6CDT,M3.2.0,M11.1.0";
  if (iana == "America/Denver")          return "MST7MDT,M3.2.0,M11.1.0";
  if (iana == "America/Los_Angeles")     return "PST8PDT,M3.2.0,M11.1.0";
  if (iana == "America/Toronto")         return "EST5EDT,M3.2.0,M11.1.0";
  if (iana == "America/Vancouver")       return "PST8PDT,M3.2.0,M11.1.0";
  if (iana == "America/Sao_Paulo")       return "BRT3";
  if (iana == "America/Argentina/Buenos_Aires") return "ART3";
  if (iana == "America/Mexico_City")     return "CST6";
  if (iana == "America/Bogota")          return "<-05>5";
  if (iana == "America/Lima")            return "<-05>5";

  // Австралия / Океания
  if (iana == "Australia/Sydney")        return "AEST-10AEDT,M10.1.0,M4.1.0/3";
  if (iana == "Australia/Melbourne")     return "AEST-10AEDT,M10.1.0,M4.1.0/3";
  if (iana == "Australia/Brisbane")      return "AEST-10";
  if (iana == "Australia/Perth")         return "AWST-8";
  if (iana == "Australia/Adelaide")      return "ACST-9:30ACDT,M10.1.0,M4.1.0/3";
  if (iana == "Pacific/Auckland")        return "NZST-12NZDT,M9.5.0,M4.1.0/3";

  // Африка
  if (iana == "Africa/Cairo")            return "EET-2EEST,M4.5.5/0,M10.5.4/24";
  if (iana == "Africa/Johannesburg")     return "SAST-2";
  if (iana == "Africa/Lagos")            return "WAT-1";
  if (iana == "Africa/Nairobi")          return "EAT-3";

  return "";
}

// -----------------------------------------

// определение часового пояса по координатам
String geoToTimezone(float lat, float lon) {
  // UTC+2 (Калининградская область)
  if (lat >= 54.0 && lat <= 55.5 && lon >= 19.0 && lon <= 23.0) {
    return "EET-2";
  }

  // UTC+3 (Москва и большая часть Европейской России), Центральная Россия, Поволжье, Черноземье, Северо-Запад
  if (lat >= 48.0 && lat <= 62.0 && lon >= 26.0 && lon <= 48.0) {
    return "MSK-3";
  }

  // Северо-Запад (СПб, Карелия, Мурманск, Архангельск)
  if (lat >= 58.0 && lat <= 70.0 && lon >= 26.0 && lon <= 48.0) {
    return "MSK-3";
  }

  // Крым, Краснодар, Ставрополье, Ростов
  if (lat >= 43.0 && lat <= 48.0 && lon >= 33.0 && lon <= 48.0) {
    return "MSK-3";
  }

  // Ненецкий АО (Нарьян-Мар)
  if (lat >= 66.0 && lat <= 70.0 && lon >= 44.0 && lon <= 60.0) {
    return "MSK-3";
  }

  // Республика Коми (Сыктывкар, Ухта, Воркута)
  if (lat >= 60.0 && lat <= 68.0 && lon >= 48.0 && lon <= 66.0) {
    return "MSK-3";
  }

  // UTC+4 (Самара, Саратов, Ульяновск, Астрахань)
  if (lat >= 45.0 && lat <= 56.0 && lon >= 44.0 && lon <= 52.0) {
    return "SAMT-4";
  }

  // UTC+5 (Урал, ХМАО, ЯНАО, Тюменская обл.), Свердловская, Челябинская, Курганская обл., Башкортостан, Пермский край
  if (lat >= 49.0 && lat <= 62.0 && lon >= 52.0 && lon <= 66.0) {
    return "YEKT-5";
  }

  // ХМАО (Сургут, Нижневартовск, Ханты-Мансийск)
  if (lat >= 58.0 && lat <= 66.0 && lon >= 66.0 && lon <= 78.0) {
    return "YEKT-5";
  }

  // ЯНАО (Салехард, Новый Уренгой, Ноябрьск)
  if (lat >= 62.0 && lat <= 70.0 && lon >= 62.0 && lon <= 80.0) {
    return "YEKT-5";
  }

  // Тюменская область (Тюмень, Тобольск)
  if (lat >= 55.0 && lat <= 62.0 && lon >= 64.0 && lon <= 76.0) {
    return "YEKT-5";
  }

  // UTC+6 (Омская область)
  if (lat >= 53.0 && lat <= 58.0 && lon >= 70.0 && lon <= 76.0) {
    return "OMST-6";
  }

  // UTC+7 (Красноярский край, Новосибирская, Томская, Кемеровская обл., Алтай)
  if (lat >= 49.0 && lat <= 62.0 && lon >= 76.0 && lon <= 93.0) {
    return "KRAT-7";
  }

  // UTC+8 (Иркутская область, Бурятия)
  if (lat >= 49.0 && lat <= 60.0 && lon >= 93.0 && lon <= 110.0) {
    return "IRKT-8";
  }

  // UTC+9 (Якутия, Забайкальский край, Амурская область)
  if (lat >= 49.0 && lat <= 68.0 && lon >= 110.0 && lon <= 130.0) {
    return "YAKT-9";
  }

  // UTC+10 (Приморский край, Хабаровский край)
  if (lat >= 42.0 && lat <= 50.0 && lon >= 130.0 && lon <= 141.0) {
    return "VLAT-10";
  }

  // UTC+11 (Магаданская, Сахалинская обл.)
  if (lat >= 45.0 && lat <= 65.0 && lon >= 141.0 && lon <= 160.0) {
    return "MAGT-11";
  }

  // UTC+12 (Камчатка, Чукотка)
  if (lat >= 50.0 && lat <= 72.0 && lon >= 160.0) {
    return "PETT-12";
  }

  // UTC+5 (Казахстан)
  if (lat >= 40.0 && lat <= 55.0 && lon >= 46.0 && lon <= 88.0) {
    return "ALMT-5";
  }

  // Ближний Восток — ОАЭ (Дубай, Абу-Даби, Шарджа) - UTC+4
  if (lat >= 22.0 && lat <= 27.0 && lon >= 51.0 && lon <= 57.0) {
    return "GST-4";
  }

  // Саудовская Аравия, Катар, Бахрейн, Кувейт - UTC+3
  if (lat >= 16.0 && lat <= 32.0 && lon >= 34.0 && lon <= 56.0) {
    return "AST-3";
  }

  if (lat >= 36.0 && lat <= 60.0 && lon >= -11.0 && lon <= 2.0) {
    return "GMT0BST,M3.5.0/1,M10.5.0";
  }
  if (lat >= 36.0 && lat <= 60.0 && lon > 2.0 && lon <= 30.0) {
    return "CET-1CEST,M3.5.0,M10.5.0/3";
  }

  // Северная Америка
  if (lat >= 24.0 && lat <= 50.0 && lon >= -125.0 && lon <= -66.0) {
    if (lon >= -87.0)  return "EST5EDT,M3.2.0,M11.1.0";
    if (lon >= -100.0) return "CST6CDT,M3.2.0,M11.1.0";
    if (lon >= -115.0) return "MST7MDT,M3.2.0,M11.1.0";
    return "PST8PDT,M3.2.0,M11.1.0";
  }

  // Азия (кроме РФ/СНГ)
  if (lat >= 24.0 && lat <= 46.0 && lon >= 122.0 && lon <= 146.0) {
    return "JST-9";  // Япония
  }
  if (lat >= 18.0 && lat <= 54.0 && lon >= 73.0 && lon <= 135.0) {
    return "CST-8";  // Китай
  }
  if (lat >= 6.0 && lat <= 36.0 && lon >= 68.0 && lon <= 98.0) {
    return "IST-5:30";  // Индия
  }

  // Австралия
  if (lat >= -40.0 && lat <= -10.0 && lon >= 140.0 && lon <= 155.0) {
    return "AEST-10AEDT,M10.1.0,M4.1.0/3";
  }

  return "MSK-3";
}

// -----------------------------------------

// валидация POSIX
bool isValidPosixFormat(const String& tz) {
  if (tz.length() < 3) return false;

  bool hasSign = false;
  bool hasDigits = false;
  bool hasComma = false;

  for (int i = 0; i < tz.length(); i++) {
    char c = tz[i];
    if (c == '+' || c == '-') hasSign = true;
    if (isdigit(c)) hasDigits = true;
    if (c == ',') hasComma = true;
  }

  if (tz.startsWith("UTC") || tz.startsWith("GMT")) {
    return hasSign && hasDigits;
  }
  if (hasSign && hasDigits && !hasComma) {
    return true;
  }
  if (hasComma && hasSign) {
    return true;
  }

  return false;
}

// -----------------------------------------

// маппинги
String ianaToPosix(const String& iana) {
  if (iana == "Europe/Kaliningrad") return "EET-2";
  if (iana == "Europe/Moscow") return "MSK-3";
  if (iana == "Europe/Volgograd") return "MSK-3";
  if (iana == "Europe/Simferopol") return "MSK-3";
  if (iana == "Europe/Minsk") return "MSK-3";
  if (iana == "Europe/Kirov") return "MSK-3";
  if (iana == "Europe/Astrakhan") return "MSK-3";
  if (iana == "Europe/Saratov") return "MSK-3";
  if (iana == "Europe/Ulyanovsk") return "MSK-3";
  if (iana == "Europe/Kursk") return "MSK-3";
  if (iana == "Europe/Bryansk") return "MSK-3";
  if (iana == "Europe/Orel") return "MSK-3";
  if (iana == "Europe/Tula") return "MSK-3";
  if (iana == "Europe/Ryazan") return "MSK-3";
  if (iana == "Europe/Voronezh") return "MSK-3";
  if (iana == "Europe/Lipetsk") return "MSK-3";
  if (iana == "Europe/Tambov") return "MSK-3";
  if (iana == "Europe/Penza") return "MSK-3";
  if (iana == "Europe/Samara") return "MSK-3"; // Самара исторически UTC+4, но с 2010-х - UTC+3
  if (iana == "Europe/Smolensk") return "MSK-3";
  if (iana == "Europe/Belgorod") return "MSK-3";
  if (iana == "Europe/Ivanovo") return "MSK-3";
  if (iana == "Europe/Kostroma") return "MSK-3";
  if (iana == "Europe/Vologda") return "MSK-3";
  if (iana == "Europe/Yaroslavl") return "MSK-3";
  if (iana == "Europe/Vladimir") return "MSK-3";
  if (iana == "Europe/Tver") return "MSK-3";
  if (iana == "Europe/Pskov") return "MSK-3";
  if (iana == "Europe/VelikyNovgorod") return "MSK-3";
  if (iana == "Europe/Arkhangelsk") return "MSK-3";
  if (iana == "Europe/Murmansk") return "MSK-3";
  if (iana == "Europe/Kazan") return "MSK-3";
  if (iana == "Europe/Ufa") return "MSK-3";
  if (iana == "Europe/Orenburg") return "MSK-3";
  if (iana == "Europe/Saratov") return "MSK-3";
  if (iana == "Europe/Cheboksary") return "MSK-3";
  if (iana == "Europe/Saransk") return "MSK-3";
  if (iana == "Europe/YoshkarOla") return "MSK-3";
  if (iana == "Europe/Makhachkala") return "MSK-3";
  if (iana == "Europe/Stavropol") return "MSK-3";
  if (iana == "Europe/Rostov-on-Don") return "MSK-3";
  if (iana == "Europe/Krasnodar") return "MSK-3";
  if (iana == "Europe/Sochi") return "MSK-3";
  if (iana == "Europe/Simferopol") return "MSK-3";
  if (iana == "Asia/Yekaterinburg") return "YEKT-5";
  if (iana == "Asia/Chelyabinsk") return "YEKT-5";
  if (iana == "Asia/Perm") return "YEKT-5";
  if (iana == "Asia/Ufa") return "YEKT-5";
  if (iana == "Asia/Orenburg") return "YEKT-5";
  if (iana == "Asia/Kurgan") return "YEKT-5";
  if (iana == "Asia/Tyumen") return "YEKT-5";
  if (iana == "Asia/Omsk") return "OMST-6";
  if (iana == "Asia/Krasnoyarsk") return "KRAT-7";
  if (iana == "Asia/Novosibirsk") return "KRAT-7";
  if (iana == "Asia/Tomsk") return "KRAT-7";
  if (iana == "Asia/Barnaul") return "KRAT-7";
  if (iana == "Asia/Kemerovo") return "KRAT-7";
  if (iana == "Asia/Novokuznetsk") return "KRAT-7";
  if (iana == "Asia/Kyzyl") return "KRAT-7";
  if (iana == "Asia/Abakan") return "KRAT-7";
  if (iana == "Asia/Irkutsk") return "IRKT-8";
  if (iana == "Asia/Ulan-Ude") return "IRKT-8";
  if (iana == "Asia/Bratsk") return "IRKT-8";
  if (iana == "Asia/Yakutsk") return "YAKT-9";
  if (iana == "Asia/Khandyga") return "YAKT-9";
  if (iana == "Asia/Blagoveshchensk") return "YAKT-9";
  if (iana == "Asia/Chita") return "YAKT-9";
  if (iana == "Asia/Vladivostok") return "VLAT-10";
  if (iana == "Asia/Khabarovsk") return "VLAT-10";
  if (iana == "Asia/Ussuriysk") return "VLAT-10";
  if (iana == "Asia/Komsomolsk-on-Amur") return "VLAT-10";
  if (iana == "Asia/Ust-Nera") return "VLAT-10";
  if (iana == "Asia/Magadan") return "MAGT-11";
  if (iana == "Asia/Sakhalin") return "MAGT-11";
  if (iana == "Asia/Yuzhno-Sakhalinsk") return "MAGT-11";
  if (iana == "Asia/Srednekolymsk") return "MAGT-11";
  if (iana == "Asia/Kamchatka") return "PETT-12";
  if (iana == "Asia/Anadyr") return "PETT-12";
  if (iana == "Asia/Petropavlovsk-Kamchatsky") return "PETT-12";
  if (iana == "Asia/Almaty") return "ALMT-5";
  if (iana == "Asia/Astana") return "ALMT-5";
  if (iana == "Asia/Qostanay") return "ALMT-5";
  if (iana == "Asia/Aqtau") return "ALMT-5";
  if (iana == "Asia/Aqtobe") return "ALMT-5";
  if (iana == "Asia/Atyrau") return "ALMT-5";
  if (iana == "Asia/Oral") return "ALMT-5";

  return iana;
}

// -----------------------------------------

// преобразование русского названия города в POSIX
String russianCityToPosix(String city) {
  city.trim();
  city.replace(" (UTC+3)", "");
  city.replace(" (UTC+4)", "");
  city.replace(" (UTC+5)", "");
  city.replace(" (UTC+6)", "");
  city.replace(" (UTC+7)", "");
  city.replace(" (UTC+8)", "");
  city.replace(" (UTC+9)", "");
  city.replace(" (UTC+10)", "");
  city.replace(" (UTC+11)", "");
  city.replace(" (UTC+12)", "");

  int parenPos = city.indexOf('(');
  if (parenPos > 0) {
    city = city.substring(0, parenPos);
    city.trim();
  }

  // -----------------------------------------
  // POSIX форматы
  if (city == "Москва" || city == "Москва, Санкт-Петербург") return "MSK-3";
  if (city == "Санкт-Петербург") return "MSK-3_SPB";
  if (city == "Ростов-на-Дону") return "MSK-3_RND";
  if (city == "Нижний Новгород") return "MSK-3_NN";
  if (city == "Казань") return "MSK-3_Kazan";
  if (city == "Воронеж") return "MSK-3_Voronezh";
  if (city == "Краснодар") return "MSK-3_Krasnodar";
  if (city == "Сочи") return "MSK-3_Sochi";
  if (city == "Киров") return "MSK-3_Kirov";
  if (city == "Тула") return "MSK-3_Tula";
  if (city == "Тверь") return "MSK-3_Tver";
  if (city == "Рязань") return "MSK-3_Ryazan";
  if (city == "Ярославль") return "MSK-3_Yaroslavl";
  if (city == "Владимир") return "MSK-3_Vladimir";
  if (city == "Брянск") return "MSK-3_Bryansk";
  if (city == "Белгород") return "MSK-3_Belgorod";
  if (city == "Курск") return "MSK-3_Kursk";
  if (city == "Липецк") return "MSK-3_Lipetsk";
  if (city == "Орёл" || city == "Орел") return "MSK-3_Orel";
  if (city == "Смоленск") return "MSK-3_Smolensk";
  if (city == "Тамбов") return "MSK-3_Tambov";
  if (city == "Иваново") return "MSK-3_Ivanovo";
  if (city == "Кострома") return "MSK-3_Kostroma";
  if (city == "Архангельск") return "MSK-3_Arkhangelsk";
  if (city == "Вологда") return "MSK-3_Vologda";
  if (city == "Мурманск") return "MSK-3_Murmansk";
  if (city == "Великий Новгород") return "MSK-3_VelikyNovgorod";
  if (city == "Псков") return "MSK-3_Pskov";
  if (city == "Махачкала") return "MSK-3_Makhachkala";
  if (city == "Нальчик") return "MSK-3_Nalchik";
  if (city == "Владикавказ") return "MSK-3_Vladikavkaz";
  if (city == "Ставрополь") return "MSK-3_Stavropol";
  if (city == "Волгоград") return "MSK-3_Volgograd";
  if (city == "Йошкар-Ола") return "MSK-3_YoshkarOla";
  if (city == "Саранск") return "MSK-3_Saransk";
  if (city == "Чебоксары") return "MSK-3_Cheboksary";
  if (city == "Оренбург") return "MSK-3_Orenburg";
  if (city == "Пенза") return "MSK-3_Penza";
  if (city == "Ульяновск") return "MSK-3_Ulyanovsk";
  if (city == "Зеленодольск") return "MSK-3_Zelenodolsk";
  if (city == "Подольск") return "MSK-3_Podolsk";
  if (city == "Люберцы") return "MSK-3_Lubertsy";
  if (city == "Мытищи") return "MSK-3_Mytishchi";
  if (city == "Балашиха") return "MSK-3_Balashikha";
  if (city == "Химки") return "MSK-3_Khimki";
  if (city == "Королёв" || city == "Королев") return "MSK-3_Korolev";
  if (city == "Электросталь") return "MSK-3_Elektrostal";
  if (city == "Коломна") return "MSK-3_Kolomna";
  if (city == "Одинцово") return "MSK-3_Odintsovo";
  if (city == "Домодедово") return "MSK-3_Domodedovo";
  if (city == "Сергиев Посад") return "MSK-3_SergievPosad";
  if (city == "Пушкино") return "MSK-3_Pushkino";
  if (city == "Жуковский") return "MSK-3_Zhukovsky";
  if (city == "Реутов") return "MSK-3_Reutov";
  if (city == "Долгопрудный") return "MSK-3_Dolgoprudny";
  if (city == "Щёлково" || city == "Щелково") return "MSK-3_Shchelkovo";
  if (city == "Орехово-Зуево") return "MSK-3_OrekhovoZuevo";
  if (city == "Серпухов") return "MSK-3_Serpukhov";
  if (city == "Воскресенск") return "MSK-3_Voskresensk";
  if (city == "Лобня") return "MSK-3_Lobnya";
  if (city == "Ивантеевка") return "MSK-3_Ivanteevka";
  if (city == "Дубна") return "MSK-3_Dubna";
  if (city == "Егорьевск") return "MSK-3_Egoryevsk";
  if (city == "Чехов") return "MSK-3_Chekhov";
  if (city == "Ступино") return "MSK-3_Stupino";
  if (city == "Павловский Посад") return "MSK-3_PavlovskyPosad";
  if (city == "Наро-Фоминск") return "MSK-3_NaroFominsk";
  if (city == "Клин") return "MSK-3_Klin";
  if (city == "Красногорск") return "MSK-3_Krasnogorsk";
  if (city == "Калининград") return "EET-2";
  if (city == "Самара") return "SAMT-4";
  if (city == "Астрахань") return "SAMT-4_Astrakhan";
  if (city == "Саратов") return "SAMT-4_Saratov";
  if (city == "Тольятти") return "SAMT-4_Tolyatti";
  if (city == "Екатеринбург") return "YEKT-5";
  if (city == "Челябинск") return "YEKT-5_Chelyabinsk";
  if (city == "Пермь") return "YEKT-5_Perm";
  if (city == "Уфа") return "YEKT-5_Ufa";
  if (city == "Курган") return "YEKT-5_Kurgan";
  if (city == "Тюмень") return "YEKT-5_Tyumen";
  if (city == "Омск") return "OMST-6";
  if (city == "Красноярск") return "KRAT-7";
  if (city == "Новосибирск") return "KRAT-7_Novosibirsk";
  if (city == "Томск") return "KRAT-7_Tomsk";
  if (city == "Барнаул") return "KRAT-7_Barnaul";
  if (city == "Кемерово") return "KRAT-7_Kemerovo";
  if (city == "Новокузнецк") return "KRAT-7_Novokuznetsk";
  if (city == "Иркутск") return "IRKT-8";
  if (city == "Улан-Удэ") return "IRKT-8_UlanUde";
  if (city == "Якутск") return "YAKT-9";
  if (city == "Чита") return "YAKT-9_Chita";
  if (city == "Благовещенск") return "YAKT-9_Blagoveshchensk";
  if (city == "Владивосток") return "VLAT-10";
  if (city == "Хабаровск") return "VLAT-10_Khabarovsk";
  if (city == "Уссурийск") return "VLAT-10_Ussuriysk";
  if (city == "Комсомольск-на-Амуре") return "VLAT-10_Komsomolsk";
  if (city == "Магадан") return "MAGT-11";
  if (city == "Сахалин") return "MAGT-11_Sakhalin";
  if (city == "Южно-Сахалинск") return "MAGT-11_YuzhnoSakhalinsk";
  if (city == "Камчатка") return "PETT-12";
  if (city == "Анадырь") return "PETT-12_Anadyr";
  if (city == "Петропавловск-Камчатский") return "PETT-12_Petropavlovsk";
  if (city == "Алматы") return "ALMT-5";
  if (city == "Астана") return "ALMT-5_Astana";
  if (city == "Караганда") return "ALMT-5_Karaganda";
  if (city == "Костанай") return "ALMT-5_Kostanay";
  if (city == "Актау") return "ALMT-5_Aktau";
  if (city == "Актобе") return "ALMT-5_Aktobe";
  if (city == "Шымкент") return "ALMT-5_Shymkent";
  if (city == "Павлодар") return "ALMT-5_Pavlodar";
  if (city == "Приозёрск" || city == "Приозерск") return "ALMT-5_Prz";

  // IANA форматы
  if (city == "Europe/Moscow") return "MSK-3";
  if (city == "Europe/Volgograd") return "MSK-3_Volgograd";
  if (city == "Europe/Simferopol") return "MSK-3_Simferopol";
  if (city == "Europe/Minsk") return "MSK-3_Minsk";
  if (city == "Europe/Kirov") return "MSK-3_Kirov";
  if (city == "Europe/Kaliningrad") return "EET-2";
  if (city == "Europe/Samara") return "SAMT-4";
  if (city == "Europe/Ulyanovsk") return "MSK-3_Ulyanovsk";
  if (city == "Europe/Astrakhan") return "SAMT-4_Astrakhan";
  if (city == "Europe/Saratov") return "SAMT-4_Saratov";
  if (city == "Asia/Yekaterinburg") return "YEKT-5";
  if (city == "Asia/Orenburg") return "MSK-3_Orenburg";
  if (city == "Asia/Omsk") return "OMST-6";
  if (city == "Asia/Novosibirsk") return "KRAT-7_Novosibirsk";
  if (city == "Asia/Barnaul") return "KRAT-7_Barnaul";
  if (city == "Asia/Krasnoyarsk") return "KRAT-7";
  if (city == "Asia/Tomsk") return "KRAT-7_Tomsk";
  if (city == "Asia/Irkutsk") return "IRKT-8";
  if (city == "Asia/Yakutsk") return "YAKT-9";
  if (city == "Asia/Khandyga") return "YAKT-9_Khandyga";
  if (city == "Asia/Vladivostok") return "VLAT-10";
  if (city == "Asia/Ust-Nera") return "VLAT-10_UstNera";
  if (city == "Asia/Magadan") return "MAGT-11";
  if (city == "Asia/Srednekolymsk") return "MAGT-11_Srednekolymsk";
  if (city == "Asia/Kamchatka") return "PETT-12";
  if (city == "Asia/Anadyr") return "PETT-12_Anadyr";
  if (city == "Asia/Almaty") return "ALMT-5";
  if (city == "Asia/Qostanay") return "ALMT-5_Kostanay";
  if (city == "Asia/Aqtau") return "ALMT-5_Aktau";
  if (city == "Asia/Aqtobe") return "ALMT-5_Aktobe";
  if (city == "Asia/Chita") return "YAKT-9_Chita";
  if (city == "Asia/Sakhalin") return "MAGT-11_Sakhalin";
  if (city == "Asia/Blagoveshchensk") return "YAKT-9_Blagoveshchensk";
  if (city == "Asia/Kemerovo") return "KRAT-7_Kemerovo";
  if (city == "Asia/Kurgan") return "YEKT-5_Kurgan";
  if (city == "Asia/Perm") return "YEKT-5_Perm";
  if (city == "Asia/Chelyabinsk") return "YEKT-5_Chelyabinsk";
  return city;
}

// -----------------------------------------

// получение информации о текущем часовом поясе
String getTimezoneInfo() {
  DynamicJsonDocument doc(512);

  String currentTz = jsonRead(configSetup, "tz");
  String customTz = jsonRead(configSetup, "tz_custom");
  bool isCustom = (jsonReadtoInt(configSetup, "tz_is_custom") == 1);
  bool useAuto = (jsonReadtoInt(configSetup, "use_auto_tz") == 1);

  doc["current"] = currentTz;
  doc["custom"] = customTz;
  doc["is_custom"] = isCustom;
  doc["use_auto"] = useAuto;
  doc["auto_detected"] = autoDetectedTz;

  // -----------------------------------------

  String displayName = currentTz;
  if (isCustom && customTz.length() > 0) {
    displayName = customTz + " (пользовательский)";
  } else if (useAuto && autoDetectedTz.length() > 0) {
    displayName = autoDetectedTz + " (автоопределён)";
  } else {
    if (currentTz == "MSK-3") displayName = "MSK-3 (Москва, UTC+3)";
    else if (currentTz == "SAMT-4") displayName = "SAMT-4 (Самара, UTC+4)";
    else if (currentTz == "YEKT-5") displayName = "YEKT-5 (Екатеринбург, UTC+5)";
    else if (currentTz == "OMST-6") displayName = "OMST-6 (Омск, UTC+6)";
    else if (currentTz == "KRAT-7") displayName = "KRAT-7 (Красноярск, UTC+7)";
    else if (currentTz == "IRKT-8") displayName = "IRKT-8 (Иркутск, UTC+8)";
    else if (currentTz == "YAKT-9") displayName = "YAKT-9 (Якутск, UTC+9)";
    else if (currentTz == "VLAT-10") displayName = "VLAT-10 (Владивосток, UTC+10)";
    else if (currentTz == "MAGT-11") displayName = "MAGT-11 (Магадан, UTC+11)";
    else if (currentTz == "PETT-12") displayName = "PETT-12 (Камчатка, UTC+12)";
    else if (currentTz == "ALMT-5") displayName = "ALMT-5 (Алматы, UTC+5)";
  }
  doc["display_name"] = displayName;

  JsonObject presets = doc.createNestedObject("presets");
  presets["MSK-3"] = "Москва, Санкт-Петербург, Ростов-на-Дону (UTC+3)";
  presets["SAMT-4"] = "Самара, Ульяновск, Астрахань (UTC+4)";
  presets["YEKT-5"] = "Екатеринбург, Челябинск, Пермь (UTC+5)";
  presets["OMST-6"] = "Омск (UTC+6)";
  presets["KRAT-7"] = "Красноярск, Новосибирск, Томск, Барнаул (UTC+7)";
  presets["IRKT-8"] = "Иркутск (UTC+8)";
  presets["YAKT-9"] = "Якутск, Чита (UTC+9)";
  presets["VLAT-10"] = "Владивосток, Хабаровск, Магадан (UTC+10)";
  presets["MAGT-11"] = "Магадан, Сахалин (UTC+11)";
  presets["PETT-12"] = "Камчатка, Анадырь (UTC+12)";
  presets["ALMT-5"] = "Алматы, Астана, Караганда, Приозёрск (UTC+5, Казахстан)";

  String output;
  serializeJson(doc, output);
  return output;
}

// -----------------------------------------

// установка ручного часового пояса
bool setCustomTimezone(String customTz) {
  customTz.trim();
  if (customTz.length() == 0) return false;

  String posixTz = ianaToPosix(customTz);

  if (posixTz == customTz) {
    if (customTz.indexOf('/') > 0) {
      return false;
    } else {
      if (!isValidPosixFormat(customTz)) {
        return false;
      }
      posixTz = customTz;
    }
  }

  jsonWrite(configSetup, "tz", posixTz);
  jsonWrite(configSetup, "tz_custom", customTz);
  jsonWrite(configSetup, "tz_is_custom", "1");
  jsonWrite(configSetup, "use_auto_tz", "0");
  saveConfig();

  myTime.tzsetup(posixTz.c_str());
  if (Wifi::instance().isConnected()) {
    g_flagForceSync = true;
  }

#if GENERAL_LOG
  SYSLOG.add("Установлен ручной часовой пояс: %s (POSIX: %s)", customTz.c_str(), posixTz.c_str());
#endif

  return true;
}

// -----------------------------------------

// автоопределение часового пояса по IP
void autoDetectTimezone() {
  if (!Wifi::instance().isConnected()) {
#if GENERAL_LOG
    SYSLOG.add("Авто TZ: нет интернета (AP режим)");
#endif
    return;
  }

  HTTPClient https;
  WiFiClientSecure client;
  client.setInsecure();

  String ianaTz = "";

  if (https.begin(client, "http://ip-api.com/json")) {
    https.setTimeout(3000);
    int httpCode = https.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = https.getString();
      int pos = payload.indexOf("\"timezone\":\"");
      if (pos >= 0) {
        pos += 12;
        int end = payload.indexOf("\"", pos);
        if (end > pos) ianaTz = payload.substring(pos, end);
      }
    }
    https.end();
  }

  if (ianaTz.length() == 0 && https.begin(client, "https://ipapi.co/timezone/")) {
    https.setTimeout(3000);
    int httpCode = https.GET();
    if (httpCode == HTTP_CODE_OK) {
      ianaTz = https.getString();
      ianaTz.trim();
    }
    https.end();
  }

  if (ianaTz.length() == 0 || ianaTz == "undefined") {
#if GENERAL_LOG
    SYSLOG.add("Авто TZ: не удалось определить");
#endif
    return;
  }

  autoDetectedTz = ianaTz;

  ianaTz.trim();
  String posixTz = ianaToPosix(ianaTz);
  String current = jsonRead(configSetup, "tz");

  myTime.tzsetup(posixTz.c_str());

  if (current != posixTz) {
    jsonWrite(configSetup, "tz", posixTz);
    jsonWrite(configSetup, "tz_custom", ianaTz);
    jsonWrite(configSetup, "tz_is_custom", "0");
    jsonWrite(configSetup, "use_auto_tz", "1");
    saveConfig();
    myTime.forcesync();
  }
}

// -----------------------------------------

// инициализация
void initTimeAndTimezone() {
  static uint32_t lastSyncAttempt = 0;
  static bool firstRun = true;

  if (!firstRun) {
    if (myTime.isTimeSet() && (millis() - lastSyncAttempt < 300000UL)) return;
    if (!myTime.isTimeSet() && (millis() - lastSyncAttempt < 5000UL)) return;
  }
  firstRun = false;
  lastSyncAttempt = millis();

  String n1 = jsonRead(configSetup, "ntp1");
  if (n1.length() > 0) NTP_SERVER1 = n1;

  String n2 = jsonRead(configSetup, "ntp2");
  if (n2.length() > 0) NTP_SERVER2 = n2;

  if (jsonReadtoInt(configSetup, "geo_tz_enabled") == 1) {
    String latS = jsonRead(configSetup, "geo_tz_lat");
    String lonS = jsonRead(configSetup, "geo_tz_lon");
    latS.trim(); lonS.trim();
    if (latS.length() > 0 && lonS.length() > 0) {
      String posixTz = geoToTimezoneOnline(latS.toFloat(), lonS.toFloat());
      if (posixTz.length() == 0) {
        posixTz = geoToTimezone(latS.toFloat(), lonS.toFloat());
      }
      jsonWrite(configSetup, "tz", posixTz);
      saveConfig();
      myTime.tzsetup(posixTz.c_str());
#if GENERAL_LOG
      SYSLOG.add("TZ по координатам: %s -> %s", latS.c_str(), posixTz.c_str());
#endif
      return;
    }
  }

  String saved_tz = jsonRead(configSetup, "tz");
  saved_tz.trim();

  if (saved_tz.length() == 0 || saved_tz == "auto") {
    jsonWrite(configSetup, "tz", "MSK-3");
    saveConfig();
    myTime.tzsetup("MSK-3");
  } else {
    myTime.tzsetup(saved_tz.c_str());
  }
}

// ******************************************************************************************************************************************************

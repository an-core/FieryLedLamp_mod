// ********************************************************************** clockFunctions.ino ************************************************************
#include "Extern.h"
// --------------------

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
String ianaToPosix(const String& iana) {
  if (iana == "Europe/Moscow" || iana == "Europe/Volgograd" || iana == "Europe/Simferopol" || iana == "Europe/Minsk" || iana == "Europe/Kirov") {
    return "MSK-3";
  }
  if (iana == "Europe/Kaliningrad") return "EET-2";
  if (iana == "Europe/Samara" || iana == "Europe/Ulyanovsk") return "SAMT-4";
  if (iana == "Asia/Yekaterinburg" || iana == "Asia/Orenburg") return "YEKT-5";
  if (iana == "Asia/Omsk") return "OMST-6";
  if (iana == "Asia/Novosibirsk" || iana == "Asia/Barnaul" || iana == "Asia/Krasnoyarsk" || iana == "Asia/Tomsk") return "KRAT-7";
  if (iana == "Asia/Irkutsk") return "IRKT-8";
  if (iana == "Asia/Yakutsk" || iana == "Asia/Khandyga") return "YAKT-9";
  if (iana == "Asia/Vladivostok" || iana == "Asia/Ust-Nera") return "VLAT-10";
  if (iana == "Asia/Magadan" || iana == "Asia/Srednekolymsk") return "MAGT-11";
  if (iana == "Asia/Kamchatka" || iana == "Asia/Anadyr") return "PETT-12";
  if (iana == "Asia/Almaty" || iana == "Asia/Qostanay" || iana == "Asia/Aqtau" || iana == "Asia/Aqtobe") return "ALMT-5";
  if (iana.startsWith("Europe/")) return "CET-1CEST,M3.5.0/2,M10.5.0/3";
  return iana;
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
    myTime.forcesync();
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
void initTimeAndTimezone() {
  static uint32_t lastSyncAttempt = 0;
  
  if (myTime.isTimeSet() && (millis() - lastSyncAttempt < 300000UL)) return;
  if (!myTime.isTimeSet() && (millis() - lastSyncAttempt < 5000UL)) return;
  
  lastSyncAttempt = millis();

  String n1 = jsonRead(configSetup, "ntp1");
  if (n1.length() > 0) NTP_SERVER1 = n1;

  String n2 = jsonRead(configSetup, "ntp2");
  if (n2.length() > 0) NTP_SERVER2 = n2;

  configTime(0, 0, NTP_SERVER1.c_str(), NTP_SERVER2.c_str(), "time.google.com");

  String saved_tz = jsonRead(configSetup, "tz");
  saved_tz.trim();

  if (saved_tz.length() == 0 || saved_tz == "auto") {
    jsonWrite(configSetup, "tz", "MSK-3");
    saveConfig();
    myTime.tzsetup("MSK-3");
  } else {
    myTime.tzsetup(saved_tz.c_str());
  }

  myTime.forcesync();
}

// ******************************************************************************************************************************************************

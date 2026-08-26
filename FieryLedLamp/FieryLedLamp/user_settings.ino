// ********************************************************************** user_settings.ino ****************************************************************
#include "Constants.h"
#include "Extern.h"
#include "Prototypes.h"
#include "Time.h"
#include "IRManager.h"
#include "Types.h"
// ---------------------------

// ==================================================================== РЕГИСТРАЦИЯ МАРШРУТОВ =========================================================
void User_settings() {

  // ----------------------------------------------------------------------------------------------------------------------------------------------------
  HTTP.on("/get_settings", HTTP_GET, []() {
    DynamicJsonDocument doc(4096);
    deserializeJson(doc, configSetup);
    String output;
    serializeJson(doc, output);
    HTTP.send(200, "application/json", output);
  });

  // ----------------------------------------------------------------------------------------------------------------------------------------------------
  HTTP.on("/PassOn", handle_PassOn);                         // Пароль на страницу настроек
  HTTP.on("/Power", handle_Power);                           // Управление питанием
  HTTP.on("/index", handle_index);                           // Переключение на финальную страницу
  HTTP.on("/lang", handle_lang);                             // Язык

  // ------------------------------------------------------------ РЕГУЛИРОВКА НАСТРОЕК ЭФФЕКТОВ ---------------------------------------------------------
  HTTP.on("/all_br", handle_all_br);                         // Общая яркость всех эффектов
  HTTP.on("/br", handle_br);                                 // Яркость эффекта
  HTTP.on("/brm", handle_brm);                               // Уменьшение яркости (-1)
  HTTP.on("/brp", handle_brp);                               // Увеличение яркости (+1)
  HTTP.on("/sp", handle_sp);                                 // Скорость эффекта
  HTTP.on("/spm", handle_spm);                               // Уменьшение скорости (-1)
  HTTP.on("/spp", handle_spp);                               // Увеличение скорости (+1)
  HTTP.on("/sc", handle_sc);                                 // Масштаб / Цвет эффекта
  HTTP.on("/scm", handle_scm);                               // Уменьшение масштаба (-1)
  HTTP.on("/scp", handle_scp);                               // Увеличение масштаба (+1)
  HTTP.on("/auto_bri", handle_auto_bri);                     // Автояркость по времени суток
  HTTP.on("/eff", handle_eff);                               // Следующий / Предыдущий эффект
  HTTP.on("/eff_all", handle_eff_all);                       // Выбрать все
  HTTP.on("/eff_clr", handle_eff_clr);                       // Сбросить выбор
  HTTP.on("/time_eff", handle_time_eff);                     // Время переключения
  HTTP.on("/eff_save", handle_eff_save);                     // Сохранить настройки эффектов в файл
  HTTP.on("/eff_read", handle_eff_read);                     // Загрузить настройки из файла
  HTTP.on("/def", handle_def);                               // Настройки текущего эффекта по умолчанию
  HTTP.on("/eff_reset", handle_eff_reset);                   // Сброс настроек всех эффектов по умолчанию
  HTTP.on("/favorit", handle_favorit);                       // Включить / Выключить переход кнопкой только по эффектам из избранного
  HTTP.on("/random_on", handle_random);                      // Случайные настройки эффектов в режиме цикл
  HTTP.on("/effect_always", handle_effect_always);           // Не возобновлять эффекты после обесточивания лампы
  HTTP.on("/eff_sel", handle_eff_sel);                       // Выбор эффекта из списка

  HTTP.on("/button_enable", handle_button_enable);
  HTTP.on("/ir_enable", handle_ir_enable);
  HTTP.on("/rf_enable", handle_rf_enable);
  HTTP.on("/tm1637_enable", handle_tm1637_enable);
  HTTP.on("/st7789_enable", handle_st7789_enable);
  HTTP.on("/mp3_enable", handle_mp3_enable);

  HTTP.on("/get_module_states", HTTP_GET, []() {
    DynamicJsonDocument doc(256);
    doc["button_enable"] = buttonEnabled ? "1" : "0";
    doc["ir_enable"] = irEnabled ? "1" : "0";
    doc["rf_enable"] = rfEnabled ? "1" : "0";
    doc["tm1637_enable"] = tm1637Enabled ? "1" : "0";
    doc["st7789_enable"] = st7789Enabled ? "1" : "0";
    doc["mp3_enable"] = mp3Enabled ? "1" : "0";
    String response;
    serializeJson(doc, response);
    HTTP.send(200, "application/json", response);
  });

#if USE_BUTTON
  HTTP.on("/button_type", handle_button_type);               // Сенсорная / механическая кнопка
  HTTP.on("/save_btn_clicks", handle_save_btn_clicks);       // Настройки действий по количеству нажатий на кнопку
#endif

#if USE_SUNSET
  HTTP.on("/sunset", handle_sunset); // Закат
  HTTP.on("/config_sunset.json", HTTP_GET, []() {
    HTTP.send(200, "application/json", configSunset);
  });
#endif

#if USE_DAWN
  HTTP.on("/alarm", handle_alarm); // Рассвет
  HTTP.on("/config_alarm.json", HTTP_GET, []() {
    HTTP.send(200, "application/json", configAlarm);
  });
#endif

#if USE_SCHEDULE
  HTTP.on("/schedule", handle_schedule); // Расписание
  HTTP.on("/config_schedule.json", HTTP_GET, []() {
    HTTP.send(200, "application/json", configSchedule);
  });
#endif

  // -------------------------------------------------------------------------- ЦИКЛ --------------------------------------------------------------------
  HTTP.on("/cycle_on", handle_cycle_on);             // Вкл/Выкл режим Цикл
  HTTP.on("/rnd_cycle", handle_rnd_cycle);           // Перемешивать или по порядку
  HTTP.on("/cycle_always", handle_cycle_always);     // Запускать цикл после включения
  HTTP.on("/cycle_set", handle_cycle_set);           // Выбор эффектов для цикла

  // -------------------------------------------------------------- УПРАВЛЕНИЕ НЕСКОЛЬКИМИ ЛАМПАМИ ------------------------------------------------------
#if USE_MULTILAMP
  HTTP.on("/multi", HTTP_GET, handle_multiple_lamp);
  HTTP.on("/config_multilamp.json", HTTP_GET, []() {
    HTTP.send(200, "application/json", configMultilamp);
  });
#endif

  HTTP.on("/setup_multilamp", HTTP_GET, []() {
#ifndef USE_MULTILAMP
    HTTP.send(404, "text/plain", "Управление несколькими лампами отключено в прошивке");
#else
    handleFileRead("/setup_multilamp.htm");
#endif
  });

  // --------------------------------------------------------------------- ДИСПЛЕЙ ST7789 ---------------------------------------------------------------
#if USE_ST7789
  HTTP.on("/tft_clock_color", handle_tft_clock_color);
  HTTP.on("/tft_weather_color", handle_tft_weather_color);
  HTTP.on("/tft_ticker_on", handle_tft_ticker_on);
  HTTP.on("/tft_ticker_color", handle_tft_ticker_color);
  HTTP.on("/tft_ticker_speed", handle_tft_ticker_speed);
  HTTP.on("/tft_ticker_period", handle_tft_ticker_period);
  HTTP.on("/tft_ticker_text", handle_tft_ticker_text);
  HTTP.on("/tft_brightness", handle_tft_brightness);
  HTTP.on("/tft_auto_brightness", handle_tft_auto_brightness);
  HTTP.on("/tft_date_color", HTTP_GET, handle_tft_date_color);
#endif

  // --------------------------------------------------------------------- ЭФФЕКТЫ С SD -----------------------------------------------------------------
#if USE_SD && !FS_AS_SD
  HTTP.on("/list_out_files", handle_list_out_files);       // список всех .out файлов
  HTTP.on("/out_file", handle_out_file);                   // выбор и активация .out файла
#else
  // Заглушки, когда SD-карта отключена
  HTTP.on("/list_out_files", []() {
    HTTP.send(404, F("text/plain"), F("SD card not enabled"));
  });
  HTTP.on("/out_file", []() {
    HTTP.send(404, F("text/plain"), F("SD card not enabled"));
  });
#endif

  // -------------------------------------------------------------------------- ВРЕМЯ -------------------------------------------------------------------
  HTTP.on("/save_brightness", handle_save_brightness);
  HTTP.on("/save_time", handle_save_time);
  HTTP.on("/night_time", handle_night_time);
  HTTP.on("/print_time", handle_print_time);
  HTTP.on("/clock_vert", handle_clock_vert);
  HTTP.on("/clock_leading_zero", handle_clock_leading_zero);
  HTTP.on("/auto_move_clock", handle_auto_move_clock);
  HTTP.on("/night_clock_enabled", handle_night_clock_enabled);
  HTTP.on("/night_clock_brightness", handle_night_clock_brightness);
  HTTP.on("/get_time", HTTP_GET, []() {
    DynamicJsonDocument doc(256);

    if (!myTime.isTimeSet()) {
      doc["time"] = "--:--:--";
      doc["city"] = "";
      doc["date"] = "";
      String out;
      serializeJson(doc, out);
      HTTP.send(200, "application/json", out);
      return;
    }

    time_t now_time = myTime.now();
    struct tm tm;
    localtime_r(&now_time, &tm);
    char buf[9];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);

    char dateBuf[20];
    strftime(dateBuf, sizeof(dateBuf), "%d %B %Y", &tm);
    String dateStr = String(dateBuf);

    dateStr.replace("January", "января");
    dateStr.replace("February", "февраля");
    dateStr.replace("March", "марта");
    dateStr.replace("April", "апреля");
    dateStr.replace("May", "мая");
    dateStr.replace("June", "июня");
    dateStr.replace("July", "июля");
    dateStr.replace("August", "августа");
    dateStr.replace("September", "сентября");
    dateStr.replace("October", "октября");
    dateStr.replace("November", "ноября");
    dateStr.replace("December", "декабря");

    String cityName = "";
    String tz = jsonRead(configSetup, "tz");

    // POSIX-форматы
    if (tz == "MSK-3") cityName = "Москва";
    else if (tz == "MSK-3_SPB") cityName = "Санкт-Петербург";
    else if (tz == "MSK-3_RND") cityName = "Ростов-на-Дону";
    else if (tz == "MSK-3_NN") cityName = "Нижний Новгород";
    else if (tz == "MSK-3_Kazan") cityName = "Казань";
    else if (tz == "MSK-3_Voronezh") cityName = "Воронеж";
    else if (tz == "MSK-3_Krasnodar") cityName = "Краснодар";
    else if (tz == "MSK-3_Sochi") cityName = "Сочи";
    else if (tz == "MSK-3_Kirov") cityName = "Киров";
    else if (tz == "MSK-3_Tula") cityName = "Тула";
    else if (tz == "MSK-3_Tver") cityName = "Тверь";
    else if (tz == "MSK-3_Ryazan") cityName = "Рязань";
    else if (tz == "MSK-3_Yaroslavl") cityName = "Ярославль";
    else if (tz == "MSK-3_Vladimir") cityName = "Владимир";
    else if (tz == "MSK-3_Bryansk") cityName = "Брянск";
    else if (tz == "MSK-3_Belgorod") cityName = "Белгород";
    else if (tz == "MSK-3_Kursk") cityName = "Курск";
    else if (tz == "MSK-3_Lipetsk") cityName = "Липецк";
    else if (tz == "MSK-3_Orel") cityName = "Орёл";
    else if (tz == "MSK-3_Smolensk") cityName = "Смоленск";
    else if (tz == "MSK-3_Tambov") cityName = "Тамбов";
    else if (tz == "MSK-3_Ivanovo") cityName = "Иваново";
    else if (tz == "MSK-3_Kostroma") cityName = "Кострома";
    else if (tz == "MSK-3_Arkhangelsk") cityName = "Архангельск";
    else if (tz == "MSK-3_Vologda") cityName = "Вологда";
    else if (tz == "MSK-3_Murmansk") cityName = "Мурманск";
    else if (tz == "MSK-3_VelikyNovgorod") cityName = "Великий Новгород";
    else if (tz == "MSK-3_Pskov") cityName = "Псков";
    else if (tz == "MSK-3_Makhachkala") cityName = "Махачкала";
    else if (tz == "MSK-3_Nalchik") cityName = "Нальчик";
    else if (tz == "MSK-3_Vladikavkaz") cityName = "Владикавказ";
    else if (tz == "MSK-3_Stavropol") cityName = "Ставрополь";
    else if (tz == "MSK-3_Volgograd") cityName = "Волгоград";
    else if (tz == "MSK-3_YoshkarOla") cityName = "Йошкар-Ола";
    else if (tz == "MSK-3_Saransk") cityName = "Саранск";
    else if (tz == "MSK-3_Cheboksary") cityName = "Чебоксары";
    else if (tz == "MSK-3_Orenburg") cityName = "Оренбург";
    else if (tz == "MSK-3_Penza") cityName = "Пенза";
    else if (tz == "MSK-3_Ulyanovsk") cityName = "Ульяновск";
    else if (tz == "MSK-3_Simferopol") cityName = "Симферополь";
    else if (tz == "MSK-3_Minsk") cityName = "Минск";

    else if (tz == "MSK-3_Zelenodolsk") cityName = "Зеленодольск";
    else if (tz == "MSK-3_Podolsk") cityName = "Подольск";
    else if (tz == "MSK-3_Lubertsy") cityName = "Люберцы";
    else if (tz == "MSK-3_Mytishchi") cityName = "Мытищи";
    else if (tz == "MSK-3_Balashikha") cityName = "Балашиха";
    else if (tz == "MSK-3_Khimki") cityName = "Химки";
    else if (tz == "MSK-3_Korolev") cityName = "Королёв";
    else if (tz == "MSK-3_Elektrostal") cityName = "Электросталь";
    else if (tz == "MSK-3_Kolomna") cityName = "Коломна";
    else if (tz == "MSK-3_Odintsovo") cityName = "Одинцово";
    else if (tz == "MSK-3_Domodedovo") cityName = "Домодедово";
    else if (tz == "MSK-3_SergievPosad") cityName = "Сергиев Посад";
    else if (tz == "MSK-3_Pushkino") cityName = "Пушкино";
    else if (tz == "MSK-3_Zhukovsky") cityName = "Жуковский";
    else if (tz == "MSK-3_Reutov") cityName = "Реутов";
    else if (tz == "MSK-3_Dolgoprudny") cityName = "Долгопрудный";
    else if (tz == "MSK-3_Shchelkovo") cityName = "Щёлково";
    else if (tz == "MSK-3_OrekhovoZuevo") cityName = "Орехово-Зуево";
    else if (tz == "MSK-3_Serpukhov") cityName = "Серпухов";
    else if (tz == "MSK-3_Voskresensk") cityName = "Воскресенск";
    else if (tz == "MSK-3_Lobnya") cityName = "Лобня";
    else if (tz == "MSK-3_Ivanteevka") cityName = "Ивантеевка";
    else if (tz == "MSK-3_Dubna") cityName = "Дубна";
    else if (tz == "MSK-3_Egoryevsk") cityName = "Егорьевск";
    else if (tz == "MSK-3_Chekhov") cityName = "Чехов";
    else if (tz == "MSK-3_Stupino") cityName = "Ступино";
    else if (tz == "MSK-3_PavlovskyPosad") cityName = "Павловский Посад";
    else if (tz == "MSK-3_NaroFominsk") cityName = "Наро-Фоминск";
    else if (tz == "MSK-3_Klin") cityName = "Клин";
    else if (tz == "MSK-3_Krasnogorsk") cityName = "Красногорск";

    else if (tz == "EET-2") cityName = "Калининград";

    else if (tz == "SAMT-4") cityName = "Самара";
    else if (tz == "SAMT-4_Astrakhan") cityName = "Астрахань";
    else if (tz == "SAMT-4_Saratov") cityName = "Саратов";
    else if (tz == "SAMT-4_Tolyatti") cityName = "Тольятти";

    else if (tz == "YEKT-5") cityName = "Екатеринбург";
    else if (tz == "YEKT-5_Chelyabinsk") cityName = "Челябинск";
    else if (tz == "YEKT-5_Perm") cityName = "Пермь";
    else if (tz == "YEKT-5_Ufa") cityName = "Уфа";
    else if (tz == "YEKT-5_Orenburg") cityName = "Оренбург";
    else if (tz == "YEKT-5_Kurgan") cityName = "Курган";
    else if (tz == "YEKT-5_Tyumen") cityName = "Тюмень";

    else if (tz == "OMST-6") cityName = "Омск";

    else if (tz == "KRAT-7") cityName = "Красноярск";
    else if (tz == "KRAT-7_Novosibirsk") cityName = "Новосибирск";
    else if (tz == "KRAT-7_Tomsk") cityName = "Томск";
    else if (tz == "KRAT-7_Barnaul") cityName = "Барнаул";
    else if (tz == "KRAT-7_Kemerovo") cityName = "Кемерово";
    else if (tz == "KRAT-7_Novokuznetsk") cityName = "Новокузнецк";
    else if (tz == "KRAT-7_Sayanogorsk") cityName = "Саяногорск";

    else if (tz == "IRKT-8") cityName = "Иркутск";
    else if (tz == "IRKT-8_UlanUde") cityName = "Улан-Удэ";

    else if (tz == "YAKT-9") cityName = "Якутск";
    else if (tz == "YAKT-9_Chita") cityName = "Чита";
    else if (tz == "YAKT-9_Blagoveshchensk") cityName = "Благовещенск";
    else if (tz == "YAKT-9_Khandyga") cityName = "Хандыга";

    else if (tz == "VLAT-10") cityName = "Владивосток";
    else if (tz == "VLAT-10_Khabarovsk") cityName = "Хабаровск";
    else if (tz == "VLAT-10_Ussuriysk") cityName = "Уссурийск";
    else if (tz == "VLAT-10_Komsomolsk") cityName = "Комсомольск-на-Амуре";
    else if (tz == "VLAT-10_UstNera") cityName = "Усть-Нера";

    else if (tz == "MAGT-11") cityName = "Магадан";
    else if (tz == "MAGT-11_Sakhalin") cityName = "Сахалин";
    else if (tz == "MAGT-11_YuzhnoSakhalinsk") cityName = "Южно-Сахалинск";
    else if (tz == "MAGT-11_Srednekolymsk") cityName = "Среднеколымск";

    else if (tz == "PETT-12") cityName = "Камчатка";
    else if (tz == "PETT-12_Anadyr") cityName = "Анадырь";
    else if (tz == "PETT-12_Petropavlovsk") cityName = "Петропавловск-Камчатский";

    else if (tz == "ALMT-5") cityName = "Алматы";
    else if (tz == "ALMT-5_Astana") cityName = "Астана";
    else if (tz == "ALMT-5_Karaganda") cityName = "Караганда";
    else if (tz == "ALMT-5_Shymkent") cityName = "Шымкент";
    else if (tz == "ALMT-5_Pavlodar") cityName = "Павлодар";
    else if (tz == "ALMT-5_Prz") cityName = "Приозёрск";
    else if (tz == "ALMT-5_Kostanay") cityName = "Костанай";
    else if (tz == "ALMT-5_Aktau") cityName = "Актау";
    else if (tz == "ALMT-5_Aktobe") cityName = "Актобе";

    // IANA-форматы
    else if (tz == "Europe/Moscow") cityName = "Москва";
    else if (tz == "Europe/Volgograd") cityName = "Волгоград";
    else if (tz == "Europe/Simferopol") cityName = "Симферополь";
    else if (tz == "Europe/Minsk") cityName = "Минск";
    else if (tz == "Europe/Kirov") cityName = "Киров";
    else if (tz == "Europe/Kaliningrad") cityName = "Калининград";
    else if (tz == "Europe/Samara") cityName = "Самара";
    else if (tz == "Europe/Ulyanovsk") cityName = "Ульяновск";
    else if (tz == "Europe/Astrakhan") cityName = "Астрахань";
    else if (tz == "Europe/Saratov") cityName = "Саратов";
    else if (tz == "Asia/Yekaterinburg") cityName = "Екатеринбург";
    else if (tz == "Asia/Orenburg") cityName = "Оренбург";
    else if (tz == "Asia/Omsk") cityName = "Омск";
    else if (tz == "Asia/Novosibirsk") cityName = "Новосибирск";
    else if (tz == "Asia/Barnaul") cityName = "Барнаул";
    else if (tz == "Asia/Krasnoyarsk") cityName = "Красноярск";
    else if (tz == "Asia/Tomsk") cityName = "Томск";
    else if (tz == "Asia/Irkutsk") cityName = "Иркутск";
    else if (tz == "Asia/Yakutsk") cityName = "Якутск";
    else if (tz == "Asia/Khandyga") cityName = "Хандыга";
    else if (tz == "Asia/Vladivostok") cityName = "Владивосток";
    else if (tz == "Asia/Ust-Nera") cityName = "Усть-Нера";
    else if (tz == "Asia/Magadan") cityName = "Магадан";
    else if (tz == "Asia/Srednekolymsk") cityName = "Среднеколымск";
    else if (tz == "Asia/Kamchatka") cityName = "Камчатка";
    else if (tz == "Asia/Anadyr") cityName = "Анадырь";
    else if (tz == "Asia/Almaty") cityName = "Алматы";
    else if (tz == "Asia/Qostanay") cityName = "Костанай";
    else if (tz == "Asia/Aqtau") cityName = "Актау";
    else if (tz == "Asia/Aqtobe") cityName = "Актобе";
    else if (tz == "Asia/Chita") cityName = "Чита";
    else if (tz == "Asia/Sakhalin") cityName = "Сахалин";
    else if (tz == "Asia/Blagoveshchensk") cityName = "Благовещенск";
    else if (tz == "Asia/Kemerovo") cityName = "Кемерово";
    else if (tz == "Asia/Kurgan") cityName = "Курган";
    else if (tz == "Asia/Perm") cityName = "Пермь";
    else if (tz == "Asia/Chelyabinsk") cityName = "Челябинск";

    doc["time"] = String(buf);
    doc["city"] = cityName;
    doc["date"] = dateStr;

    String out;
    serializeJson(doc, out);
    HTTP.send(200, "application/json", out);
  });

  // сохранение времени
  HTTP.on("/save_time", HTTP_GET, []() {
    bool changed = false;

    String tz = HTTP.arg("tz");
    tz.trim();
    if (tz.length() > 0) {
      String currentTz = jsonRead(configSetup, "tz");
      if (currentTz != tz) {
        bool isPreset = false;
        const char* presets[] = {"MSK-3", "SAMT-4", "YEKT-5", "OMST-6", "KRAT-7", "IRKT-8", "YAKT-9", "VLAT-10", "MAGT-11", "PETT-12", "ALMT-5"};
        for (int i = 0; i < 11; i++) {
          if (tz == presets[i]) {
            isPreset = true;
            break;
          }
        }

        if (isPreset) {
          jsonWrite(configSetup, "tz", tz);
          jsonWrite(configSetup, "tz_custom", tz);
          jsonWrite(configSetup, "tz_is_custom", "0");
          jsonWrite(configSetup, "use_auto_tz", "0");
        } else {
          String posixTz = ianaToPosix(tz);
          if (posixTz != tz || isValidPosixFormat(tz)) {
            jsonWrite(configSetup, "tz", (posixTz != tz) ? posixTz : tz);
            jsonWrite(configSetup, "tz_custom", tz);
            jsonWrite(configSetup, "tz_is_custom", "1");
            jsonWrite(configSetup, "use_auto_tz", "0");
          } else {
            HTTP.send(400, "text/plain", "INVALID_TZ");
            return;
          }
        }
        changed = true;
      }
    }

    String ntp1_arg = HTTP.arg("ntp1"); ntp1_arg.trim();
    String ntp2_arg = HTTP.arg("ntp2"); ntp2_arg.trim();
    String ntp3_arg = HTTP.arg("ntp3"); ntp3_arg.trim();

    if (ntp1_arg.length() > 0 && jsonRead(configSetup, "ntp1") != ntp1_arg) {
      jsonWrite(configSetup, "ntp1", ntp1_arg);
      changed = true;
    }
    if (ntp2_arg.length() > 0 && jsonRead(configSetup, "ntp2") != ntp2_arg) {
      jsonWrite(configSetup, "ntp2", ntp2_arg);
      changed = true;
    }
    if (ntp3_arg.length() > 0 && jsonRead(configSetup, "ntp3") != ntp3_arg) {
      jsonWrite(configSetup, "ntp3", ntp3_arg);
      changed = true;
    }

    if (!changed) {
      HTTP.send(200, "text/plain", "NO_CHANGE");
      return;
    }

    saveConfig();

    String current_tz = jsonRead(configSetup, "tz");
    current_tz.trim();
    if (current_tz.length() == 0) {
      current_tz = "MSK-3";
      jsonWrite(configSetup, "tz", current_tz);
      saveConfig();
    }

    String new_ntp1 = jsonRead(configSetup, "ntp1");
    if (new_ntp1.length() == 0) new_ntp1 = "ntp3.vniiftri.ru";

    String new_ntp2 = jsonRead(configSetup, "ntp2");
    if (new_ntp2.length() == 0) new_ntp2 = "ru.pool.ntp.org";

    String new_ntp3 = jsonRead(configSetup, "ntp3");

    NTP_SERVER1 = new_ntp1;
    NTP_SERVER2 = new_ntp2;

    myTime.ntpodhcp(false);
    myTime.tzsetup(current_tz.c_str());

    if (new_ntp3.length() > 0) {
      myTime.setcustomntp(new_ntp3.c_str());
    }

    if (Wifi::instance().isConnected()) {
      myTime.forcesync();
    }

    HTTP.send(200, "text/plain", "OK");
  });

  HTTP.on("/force_time_sync", HTTP_GET, []() {
    if (Wifi::instance().isConnected()) {
      myTime.forcesync();
      HTTP.send(200, "text/plain", "SYNC_STARTED");
    } else {
      HTTP.send(503, "text/plain", "Нет подключения к интернету.\nИспользуйте ручную установку времени.");
    }
  });

  // автоопределение часового пояса
  HTTP.on("/auto_tz", HTTP_GET, []() {
    jsonWrite(configSetup, "tz", "");
    jsonWrite(configSetup, "use_auto_tz", "1");
    saveConfig();
    autoDetectTimezone();

    String detectedTz = jsonRead(configSetup, "tz");
    if (detectedTz.length() > 0) {
      myTime.tzsetup(detectedTz.c_str());
      if (Wifi::instance().isConnected()) {
        myTime.forcesync();
      }
      HTTP.send(200, "text/plain", "AUTO_OK");
    } else {
      HTTP.send(500, "text/plain", "AUTO_FAILED");
    }
  });

  // установка времени с клиента
  HTTP.on("/set_client_time", HTTP_GET, []() {
    bool timeChanged = false;
    bool tzChanged = false;

    if (HTTP.hasArg("time")) {
      unsigned long clientTime = HTTP.arg("time").toInt();
      if (clientTime > 946684800UL) {
        myTime.disable();
        struct timeval tv = { (time_t)clientTime, 0 };
        settimeofday(&tv, nullptr);
        timeChanged = true;
      }
    }

    if (HTTP.hasArg("tz")) {
      String tzStr = urldecode(HTTP.arg("tz"));
      tzStr.trim();

      if (tzStr.length() > 0 && tzStr != "undefined" && tzStr != "null") {
        String posixTz = ianaToPosix(tzStr);
        if (posixTz.length() == 0) posixTz = tzStr;

        if (isValidPosixFormat(posixTz) || posixTz != tzStr) {
          jsonWrite(configSetup, "tz", posixTz);
          jsonWrite(configSetup, "tz_custom", tzStr);
          jsonWrite(configSetup, "tz_is_custom", "1");
          jsonWrite(configSetup, "use_auto_tz", "0");
          saveConfig();
          myTime.tzsetup(posixTz.c_str());
          tzChanged = true;
        }
      }
    }

    if (!tzChanged) {
      String currentTz = jsonRead(configSetup, "tz");
      if (currentTz.length() == 0 || currentTz == "auto") {
        jsonWrite(configSetup, "tz", "MSK-3");
        jsonWrite(configSetup, "use_auto_tz", "0");
        saveConfig();
        myTime.tzsetup("MSK-3");
      }
    }

    myTime.enable();
    if (!timeChanged && !tzChanged) {
      HTTP.send(400, "text/plain", "Missing parameters");
      return;
    }

    HTTP.send(200, "text/plain", "OK_TIME_SET");
  });

  // установка часового пояса
  HTTP.on("/tz", HTTP_GET, []() {
    if (!HTTP.hasArg("tz")) {
      HTTP.send(400, "text/plain", "Missing tz parameter");
      return;
    }

    String newTz = HTTP.arg("tz");
    newTz.trim();

    if (newTz.length() == 0 || newTz == "auto") {
      jsonWrite(configSetup, "tz", "");
      jsonWrite(configSetup, "use_auto_tz", "1");
      saveConfig();
      autoDetectTimezone();
      String detectedTz = jsonRead(configSetup, "tz");
      if (detectedTz.length() > 0) {
        myTime.tzsetup(detectedTz.c_str());
      } else {
        myTime.tzsetup("MSK-3");
      }
    } else {
      bool isPreset = false;
      const char* presets[] = {"MSK-3", "SAMT-4", "YEKT-5", "OMST-6", "KRAT-7", "IRKT-8", "YAKT-9", "VLAT-10", "MAGT-11", "PETT-12", "ALMT-5"};
      for (int i = 0; i < 11; i++) {
        if (newTz == presets[i]) {
          isPreset = true;
          break;
        }
      }

      String posixTz = newTz;
      if (!isPreset) {
        posixTz = ianaToPosix(newTz);
        if (posixTz == newTz && !isValidPosixFormat(newTz)) {
          HTTP.send(400, "text/plain", "INVALID_TZ_FORMAT");
          return;
        }
      }

      jsonWrite(configSetup, "tz", posixTz);
      jsonWrite(configSetup, "tz_custom", newTz);
      jsonWrite(configSetup, "tz_is_custom", isPreset ? "0" : "1");
      jsonWrite(configSetup, "use_auto_tz", "0");
      saveConfig();
      myTime.tzsetup(posixTz.c_str());
    }

    if (Wifi::instance().isConnected()) {
      myTime.forcesync();
    }

    HTTP.send(200, "text/plain", "OK_TZ_SET");
  });

  // установка пользовательского часового пояса
  HTTP.on("/set_custom_tz", HTTP_GET, []() {
    if (!HTTP.hasArg("tz")) {
      HTTP.send(400, "text/plain", "Missing tz parameter");
      return;
    }

    String customTz = urldecode(HTTP.arg("tz"));
    customTz.trim();

    if (customTz.length() == 0) {
      HTTP.send(400, "text/plain", "Empty timezone");
      return;
    }

    String posixTz = russianCityToPosix(customTz);

    if (posixTz == customTz) {
      bool isPreset = false;
      const char* presets[] = {"MSK-3", "SAMT-4", "YEKT-5", "OMST-6", "KRAT-7", "IRKT-8", "YAKT-9", "VLAT-10", "MAGT-11", "PETT-12", "ALMT-5"};
      for (int i = 0; i < 11; i++) {
        if (customTz == presets[i]) {
          isPreset = true;
          break;
        }
      }

      if (!isPreset) {
        posixTz = ianaToPosix(customTz);
        if (posixTz == customTz && !isValidPosixFormat(customTz)) {
          HTTP.send(400, "text/plain", "Invalid timezone format");
          return;
        }
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
    SYSLOG.add("Установлен часовой пояс: %s (POSIX: %s)", customTz.c_str(), posixTz.c_str());
#endif

    HTTP.send(200, "text/plain", "OK_TZ_SET");
  });

  HTTP.on("/get_tz_info", HTTP_GET, []() {
    String info = getTimezoneInfo();
    HTTP.send(200, "application/json", info);
  });

  // разноцветные часы на матрице (которые в списке эффектов)
  HTTP.on("/rainbow_clock", HTTP_GET, []() {
    if (HTTP.hasArg("value")) {
      rainbowClock = (HTTP.arg("value") == "1" || HTTP.arg("value") == "true");
      jsonWrite(configLedPanel, "rainbow_clock", rainbowClock ? "1" : "0");
      saveConfig();
      loadingFlag = true;
    }
    HTTP.send(200, "text/plain", rainbowClock ? "1" : "0");
  });

  // --------------------------------------------------------------- ПОГОДА (ЯНДЕКС, OPENWEATHER) -------------------------------------------------------
#if USE_WEATHER
  HTTP.on("/print_weather", handle_print_weather); // Интервал вывода погоды
  HTTP.on("/run_weather_text_enabled", handle_run_weather_text_enabled); // Включение/выключение погоды бегущей строкой
  HTTP.on("/get_weather", HTTP_GET, []() {
    DynamicJsonDocument doc(1024);

    auto& weather = Weather::instance();
    if (weather.isAvailable()) {
      String source = weather.isUsingYandex() ? "Яндекс" : "OpenWeather";
      String text = source + ": " + String((int)round(weather.getTemperature())) + "°C";
      String condition = weather.getCondition();
      if (condition.length() > 0) {
        text += ", " + condition;
      }

      String cityName = "";
      int weatherSource = jsonReadtoInt(configWeather, "weather_source");

      if (weatherSource == 0) { // Яндекс
        String yandexGeo = jsonRead(configWeather, "yandex_geo");
        String customYandexGeo = jsonRead(configWeather, "custom_yandex_geo");
        if (customYandexGeo.length() > 0) {
          cityName = getCityNameByGeo(customYandexGeo);
          if (cityName.length() == 0) cityName = "Город " + customYandexGeo;
        } else if (yandexGeo.length() > 0) {
          cityName = getCityNameByGeo(yandexGeo);
        } else {
          cityName = "Москва";
        }
      } else { // OpenWeather
        String city = jsonRead(configWeather, "city");
        String customCity = jsonRead(configWeather, "custom_city");
        if (customCity.length() > 0) {
          cityName = getRussianCityNameForOpenWeather(customCity);
        } else if (city.length() > 0) {
          cityName = getRussianCityNameForOpenWeather(city);
        } else {
          cityName = "Moscow";
        }
      }

      doc["text"] = text;
      doc["temp"] = round(weather.getTemperature());
      doc["city"] = cityName;
    } else {
      doc["text"] = "—";
      doc["city"] = "";
    }
    doc["init"] = inClockWeatherMode;
    doc["provider"] = weather.isUsingYandex() ? "yandex" : "openweather";

    String out;
    serializeJson(doc, out);
    HTTP.send(200, "application/json", out);
  });

  HTTP.on("/save_weather_param", HTTP_GET, []() {
    if (!HTTP.hasArg("key") || !HTTP.hasArg("value")) {
      HTTP.send(400, "text/plain", "Missing key or value");
      return;
    }

    String key = HTTP.arg("key");
    String value = HTTP.arg("value");
    bool needForceUpdate = false;
    bool configChanged = false;

    String oldValue = jsonRead(configWeather, key);
    if (oldValue != value) {
      jsonWrite(configWeather, key, value);
      configChanged = true;
    }

    if (key == "show_weather") {
      inClockWeatherMode = (value == "1");
      if (inClockWeatherMode && Wifi::instance().isConnected()) {
        needForceUpdate = true;
      }
    }
    if (configChanged) {
      saveConfig();
      Weather::instance().loadSettings();
    }

    if (needForceUpdate && Wifi::instance().isConnected()) {
      Weather::instance().forceUpdate();
    }

    HTTP.send(200, "text/plain", "OK");
  });

  // кнопка "Применить"
  HTTP.on("/save_weather_params", HTTP_GET, []() {
    bool configChanged = false;

    String weather_source = HTTP.arg("weather_source");
    String yandex_geo = HTTP.arg("yandex_geo");
    String custom_yandex_geo = HTTP.arg("custom_yandex_geo");
    String city = HTTP.arg("city");
    String custom_city = HTTP.arg("custom_city");

    if (HTTP.hasArg("weather_source")) {
      String value = HTTP.arg("weather_source");
      String oldValue = jsonRead(configWeather, "weather_source");
      if (oldValue != value) {
        jsonWrite(configWeather, "weather_source", value);
        configChanged = true;
      }
    }

    if (HTTP.hasArg("yandex_geo")) {
      String value = HTTP.arg("yandex_geo");
      String oldValue = jsonRead(configWeather, "yandex_geo");
      if (oldValue != value) {
        jsonWrite(configWeather, "yandex_geo", value);
        configChanged = true;
      }
    }

    if (HTTP.hasArg("custom_yandex_geo")) {
      String value = HTTP.arg("custom_yandex_geo");
      String oldValue = jsonRead(configWeather, "custom_yandex_geo");
      if (oldValue != value) {
        jsonWrite(configWeather, "custom_yandex_geo", value);
        configChanged = true;
      }
    }

    if (HTTP.hasArg("city")) {
      String value = HTTP.arg("city");
      String oldValue = jsonRead(configWeather, "city");
      if (oldValue != value) {
        jsonWrite(configWeather, "city", value);
        configChanged = true;
      }
    }

    if (HTTP.hasArg("custom_city")) {
      String value = HTTP.arg("custom_city");
      String oldValue = jsonRead(configWeather, "custom_city");
      if (oldValue != value) {
        jsonWrite(configWeather, "custom_city", value);
        configChanged = true;
      }
    }

    if (configChanged) {
      saveConfig();
      Weather::instance().loadSettings();

      if (Wifi::instance().isConnected()) {
        Weather::instance().forceUpdate();
      }
    }

    HTTP.send(200, "text/plain", "OK");
  });

  // текущие настройки
  HTTP.on("/get_weather_settings", HTTP_GET, []() {
    DynamicJsonDocument doc(2048);

    String weather_source = jsonRead(configWeather, "weather_source");
    String yandex_geo = jsonRead(configWeather, "yandex_geo");
    String custom_yandex_geo = jsonRead(configWeather, "custom_yandex_geo");
    String city = jsonRead(configWeather, "city");
    String custom_city = jsonRead(configWeather, "custom_city");

    doc["weather_source"] = weather_source.length() > 0 ? weather_source : "0";
    doc["yandex_geo"] = yandex_geo.length() > 0 ? yandex_geo : "";
    doc["custom_yandex_geo"] = custom_yandex_geo.length() > 0 ? custom_yandex_geo : "";
    doc["city"] = city.length() > 0 ? city : "";
    doc["custom_city"] = custom_city.length() > 0 ? custom_city : "";

    String out;
    serializeJson(doc, out);
    HTTP.send(200, "application/json", out);
  });

#else
  // заглушки
  // возвращает статус "ОТКЛЮЧЕНО В ПРОШИВКЕ"
  HTTP.on("/get_weather", HTTP_GET, []() {
    DynamicJsonDocument doc(128);
    doc["enabled"] = false;
    doc["text"] = "ОТКЛЮЧЕНО В ПРОШИВКЕ";
    doc["temp"] = 0;
    doc["init"] = false;
    doc["city"] = "";
    doc["provider"] = "";
    String out;
    serializeJson(doc, out);
    HTTP.send(200, "application/json", out);
  });

  HTTP.on("/save_weather_param", HTTP_GET, []() {
    HTTP.send(200, "text/plain", "OK (weather disabled)");
  });

  HTTP.on("/save_weather_params", HTTP_GET, []() {
    HTTP.send(200, "text/plain", "OK (weather disabled)");
  });

  HTTP.on("/get_weather_settings", HTTP_GET, []() {
    DynamicJsonDocument doc(128);
    doc["weather_source"] = "0";
    doc["yandex_geo"] = "";
    doc["custom_yandex_geo"] = "";
    doc["city"] = "";
    doc["custom_city"] = "";
    String out;
    serializeJson(doc, out);
    HTTP.send(200, "application/json", out);
  });
#endif

  // ------------------------------------------------------------------------ НАСТРОЙКИ МАТРИЦЫ ---------------------------------------------------------
  HTTP.on("/m_t", handle_matrix_tipe);
  HTTP.on("/m_o", handle_matrix_orientation);

  // чип светодиодов, порядок цветов
  HTTP.on("/save_led_settings", HTTP_GET, []() {
    String configLED = readFile(F("config_led_matrix.json"), 2048);
    if (configLED == "Failed" || configLED == "Large" || configLED.isEmpty()) {
      configLED = "{}";
    }

    bool changed = false;
    if (HTTP.hasArg("led_chip")) {
      String val = HTTP.arg("led_chip");
      String oldVal = jsonRead(configLED, "led_chip");
      if (oldVal != val) {
        jsonWrite(configLED, "led_chip", val);
        changed = true;
      }
    }

    if (HTTP.hasArg("color_order")) {
      String val = HTTP.arg("color_order");
      String oldVal = jsonRead(configLED, "color_order");
      if (oldVal != val) {
        jsonWrite(configLED, "color_order", val);
        changed = true;
      }
    }

    if (changed) {
      writeFile(F("config_led_matrix.json"), configLED);
      HTTP.send(200, "text/plain", "OK_REBOOT");
      delay(800);
      ESP.restart();
    } else {
      HTTP.send(200, "text/plain", "NO_CHANGE");
    }
  });

  HTTP.on("/save_matrix", HTTP_GET, []() {
    bool changed = false;
    bool needReboot = false;
    String configLED = readFile(F("config_led_matrix.json"), 2048);
    if (configLED == "Failed" || configLED == "Large") configLED = "{}";

    // с перезагрузкой
    if (HTTP.hasArg("led_chip")) {
      String val = HTTP.arg("led_chip");
      String old = jsonRead(configLED, "led_chip");
      if (old != val) {
        jsonWrite(configLED, "led_chip", val);
        changed = true;
        needReboot = true;
      }
    }

    if (HTTP.hasArg("color_order")) {
      String val = HTTP.arg("color_order");
      String old = jsonRead(configLED, "color_order");
      if (old != val) {
        jsonWrite(configLED, "color_order", val);
        changed = true;
        needReboot = true;
      }
    }

    if (HTTP.hasArg("width")) {
      String val = HTTP.arg("width");
      String old = jsonRead(configLED, "width");
      if (old != val) {
        uint16_t newW = val.toInt();
        if (newW >= 1 && newW <= 128) {
          jsonWrite(configLED, "width", String(newW));
          changed = true;
          needReboot = true;
        }
      }
    }

    if (HTTP.hasArg("height")) {
      String val = HTTP.arg("height");
      String old = jsonRead(configLED, "height");
      if (old != val) {
        uint16_t newH = val.toInt();
        if (newH >= 1 && newH <= 128) {
          jsonWrite(configLED, "height", String(newH));
          changed = true;
          needReboot = true;
        }
      }
    }

#if MULTI_MATRIX
    if (HTTP.hasArg("segMatrix_w")) {
      String val = HTTP.arg("segMatrix_w");
      String old = jsonRead(configLED, "segMatrix_w");
      if (old != val) {
        uint8_t newW = val.toInt();
        if (newW >= 1 && newW <= 8) { // ограничение 8 матриц (которые 16х16)
          jsonWrite(configLED, "segMatrix_w", String(newW));
          changed = true;
          needReboot = true;
        }
      }
    }

    if (HTTP.hasArg("segMatrix_h")) {
      String val = HTTP.arg("segMatrix_h");
      String old = jsonRead(configLED, "segMatrix_h");
      if (old != val) {
        uint8_t newH = val.toInt();
        if (newH >= 1 && newH <= 8) {
          jsonWrite(configLED, "segMatrix_h", String(newH));
          changed = true;
          needReboot = true;
        }
      }
    }
    // перезагрузка автоматическая
    if (HTTP.hasArg("panel_flip")) {
      String val = HTTP.arg("panel_flip");
      String old = jsonRead(configLED, "panel_flip");
      if (old != val) {
        jsonWrite(configLED, "panel_flip", val);
        changed = true;
        needReboot = true;
        panelFlip = (val == "1");
      }
    }
#endif

    // без перезагрузки
    if (HTTP.hasArg("m_t")) {
      String val = HTTP.arg("m_t");
      String old = jsonRead(configLED, "m_t");
      if (old != val) {
        jsonWrite(configLED, "m_t", val);
        changed = true;
        MatrixType = val.toInt();
      }
    }

    if (HTTP.hasArg("m_o")) {
      String val = HTTP.arg("m_o");
      String old = jsonRead(configLED, "m_o");
      if (old != val) {
        jsonWrite(configLED, "m_o", val);
        changed = true;
        MatrixOrientation = val.toInt();
      }
    }

    if (HTTP.hasArg("cur_lim")) {
      String val = HTTP.arg("cur_lim");
      String old = jsonRead(configLED, "cur_lim");
      if (old != val) {
        jsonWrite(configLED, "cur_lim", val);
        changed = true;
        current_limit = val.toInt();
        if (current_limit > CURRENT_LIMIT) current_limit = CURRENT_LIMIT;
        if (current_limit == 0) current_limit = 0xFFFF;
        FastLED.setMaxPowerInVoltsAndMilliamps(5, current_limit);
      }
    }

    if (changed) {
      writeFile(F("config_led_matrix.json"), configLED);

      if (needReboot) {
        HTTP.send(200, "text/plain", "OK_REBOOT");
        delay(500);
        ESP.restart();
      } else {
        HTTP.send(200, "text/plain", "OK");
      }
    } else {
      HTTP.send(200, "text/plain", "NO_CHANGE");
    }
  });

  // ------------------------------------------------------------------------- LED ПАНЕЛЬ ---------------------------------------------------------------
  // бегущая строка
#if LED_PANEL
  HTTP.on("/spt", handle_spt);                             // Скорость бегущей строки
  HTTP.on("/sct", handle_sct);                             // Цвет бегущей строки
  HTTP.on("/font_size", handle_font_size);                 // Шрифт текста
  HTTP.on("/run_text", handle_run_text);                   // Текст бегущей строки
  HTTP.on("/run_text_enabled", handle_run_text_enabled);   // Чекбокс "Включить бегущую строку"
  HTTP.on("/run_text_over", handle_run_text_over);         // Чекбокс "Бегущая строка поерх эффекта"
  HTTP.on("/interval_run_text", handle_interval_run_text); // Интервал вывода текста бегущей строкой"
  HTTP.on("/run_time_text_enabled", handle_run_time_text_enabled); // Включение/выключение часов бегущей строкой
  HTTP.on("/rainbow_text", HTTP_GET, []() {                // Чекбокс "Разноцветный текст"
    if (HTTP.hasArg("value")) {
      rainbowText = (HTTP.arg("value") == "1" || HTTP.arg("value") == "true");
      if (rainbowText) {
        runTextColorCycle = false;
        autoRunTextHue = false;
        jsonWrite(configLedPanel, "run_text_cycle", "0");
      }
      jsonWrite(configLedPanel, "rainbow_text", rainbowText ? "1" : "0");
      saveConfig();
      loadingFlag = true;
      textIsRunning = true;
      offset = matrixWidth + 10;
      scrollTimer = millis();
    }
    HTTP.send(200, "text/plain", rainbowText ? "1" : "0");
  });

  // оттенок текста
  HTTP.on("/run_text_hue", []() {
    if (HTTP.hasArg("value")) {
      int val = HTTP.arg("value").toInt();
      if (val >= 253) {
        runTextColorCycle = true;
        runTextHue = 0;
        jsonWrite(configLedPanel, "run_text_cycle", "1");
      } else {
        runTextColorCycle = false;
        runTextHue = (uint8_t)val;
        jsonWrite(configLedPanel, "run_text_cycle", "0");
      }
      jsonWrite(configLedPanel, "run_text_hue", String(runTextHue));
      saveConfig();
      loadingFlag = true;
    }
    HTTP.send(200, "text/plain", String(runTextHue));
  });

  // если интервал вывода бегущей строки установлен 0, то текст будет бежать непрерывно
  HTTP.on("/run_text_cycle", []() {
    if (HTTP.hasArg("value")) {
      runTextColorCycle = (HTTP.arg("value") == "1");
      jsonWrite(configLedPanel, "run_text_cycle", runTextColorCycle ? "1" : "0");
      saveConfig();
      loadingFlag = true;
    }
    HTTP.send(200, "text/plain", runTextColorCycle ? "1" : "0");
  });
  HTTP.on("/text_y_offset", handle_text_y_offset);              // Смещение текста по Y (по высоте)
  // часы
  HTTP.on("/clock_x_offset", handle_clock_x_offset);            // Смещение текста по X (по ширине)
  HTTP.on("/clock_y_offset", handle_clock_y_offset);            // Смещение текста по Y (по высоте)
  HTTP.on("/clock_hue", handle_clock_hue); // Оттенок часов
  // дата
  HTTP.on("/date_enabled", handle_date_enabled);                 // Чекбокс "Включить дату"
  HTTP.on("/date_y_offset", handle_date_y_offset);               // Смещение даиы по Y (по ширине)
  HTTP.on("/date_x_offset", handle_date_x_offset);               // Смещение даты по X (по ширине)
  HTTP.on("/date_hue", handle_date_hue);                         // Оттенок даты
  HTTP.on("/date_full_year", handle_date_full_year);             // Показывать 4 цифры года
  HTTP.on("/date_separator_blink", handle_date_separator_blink); // Плавное мигание разделительных точек
  HTTP.on("/date_show_year", handle_date_show_year);             // Показывать год

  HTTP.on("/rainbow_date", HTTP_GET, []() {
    if (HTTP.hasArg("value")) {
      String val = HTTP.arg("value");
      rainbowDate = (val == "1" || val.equalsIgnoreCase("true") || val == "on");
      jsonWrite(configLedPanel, "rainbow_date", rainbowDate ? "1" : "0");
      saveConfig();
      dateNeedRedraw = true;
      loadingFlag = true;
    }
    HTTP.send(200, "text/plain", rainbowDate ? "1" : "0");
  });

  // погода
#if USE_WEATHER
  HTTP.on("/weather_enabled", handle_weather_enabled);   // Чекбокс "Включить погоду"
  HTTP.on("/weather_hue", handle_weather_hue);           // Регулировка цвета
  HTTP.on("/weather_x_offset", handle_weather_x_offset); // Смещение по X
  HTTP.on("/weather_y_offset", handle_weather_y_offset); // Смещение по Y
  HTTP.on("/degree_blink", handle_degree_blink);         // Мигание знака градуса (маленький кружок)
  // таймеры и интервалы
  HTTP.on("/timer_c_d_w", handle_timer_c_d_w);                   // таймер часы/дата/погоды
  HTTP.on("/timer_clock_fixed", handle_timer_clock_fixed);       // таймер часы + дата/погоды
  HTTP.on("/interval_c_d_w", handle_interval_c_d_w);             // интервал часы/дата/погоды
  HTTP.on("/interval_clock_fixed", handle_interval_clock_fixed); // интервал часы + дата/погоды
  HTTP.on("/timer_d_w", handle_timer_d_w);                       // таймер дата/погода
  HTTP.on("/interval_d_w", handle_interval_d_w);                 // интервал дата/погода
  HTTP.on("/timer_c_w", handle_timer_c_w);                       // таймер часы/погода
  HTTP.on("/interval_c_w", handle_interval_c_w);                 // интервал часы/погода

  HTTP.on("/rainbow_weather", HTTP_GET, []() {
    if (HTTP.hasArg("value")) {
      rainbowWeather = (HTTP.arg("value") == "1" || HTTP.arg("value") == "true");
      jsonWrite(configLedPanel, "rainbow_weather", rainbowWeather ? "1" : "0");
      saveConfig();
      weatherNeedRedraw = true;
      loadingFlag = true;
    }
    HTTP.send(200, "text/plain", rainbowWeather ? "1" : "0");
  });
#endif // USE_WEATHER
  // таймер и интервал часы/дата
  HTTP.on("/timer_c_d",  handle_timer_c_d);
  HTTP.on("/interval_c_d", handle_interval_c_d);
#endif // LED_PANEL

  // ------------------------------------------------------------------------- МП3 ПЛЕЕР ----------------------------------------------------------------
#if USE_MP3_PLAYER
  HTTP.on("/save_sound_settings", HTTP_GET, handle_save_sound_settings);
  HTTP.on("/on_sound", handle_on_sound);
  HTTP.on("/on_day_adv", handle_day_advert_on_sound);
  HTTP.on("/on_night_adv", handle_night_advert_on_sound);
  HTTP.on("/on_alm_adv", handle_alarm_advert_sound_on);
  HTTP.on("/vol", handle_volume);
  HTTP.on("/alm_vol", handle_alarm_volume);
  HTTP.on("/sun_vol", handle_sunset_volume);
  HTTP.on("/day_vol", handle_day_advert_volume);
  HTTP.on("/night_vol", handle_night_advert_volume);
  HTTP.on("/tim_h", handle_tim_h);
  HTTP.on("/tim_m", handle_tim_m);
  HTTP.on("/delay", handle_delay);
  HTTP.on("/on_alm_snd", handle_alarm_on_sound);
  HTTP.on("/on_sun_snd", handle_sunset_on_sound);
  HTTP.on("/alm_fold_sel", handle_alarm_fold_sel);
  HTTP.on("/sun_fold_sel", handle_sunset_fold_sel);
  HTTP.on("/sound_set", handle_sound_set);
  HTTP.on("/track_down", handle_folder_down);
  HTTP.on("/track_up", handle_folder_up);
  HTTP.on("/fold_sel", handle_folder_select);
  HTTP.on("/eq", handle_equalizer);
  HTTP.on("/play_time", HTTP_GET, handle_play_time);
  HTTP.on("/play_weather", HTTP_GET, handle_play_weather);
  HTTP.on("/weather_speak", HTTP_GET, handle_weather_speak);
  HTTP.on("/time_speak", HTTP_GET, handle_time_speak);
  HTTP.on("/weather_temp_delay", HTTP_GET, handle_weather_temp_delay);
  HTTP.on("/weather_desc_delay", HTTP_GET, handle_weather_desc_delay);
  HTTP.on("/weather_day_vol", handle_weather_day_volume);
  HTTP.on("/weather_night_vol", handle_weather_night_volume);
  HTTP.on("/day_weather_adv", handle_day_weather_adv);
  HTTP.on("/day_weather_desc", handle_day_weather_desc);
  HTTP.on("/night_weather_adv", handle_night_weather_adv);
  HTTP.on("/night_weather_desc", handle_night_weather_desc);
  HTTP.on("/show_weather_desc", handle_show_weather_desc);
  HTTP.on("/time_always", handle_time_always);
  HTTP.on("/weather_always", handle_weather_always);

  HTTP.on("/stop_weather_sound", HTTP_GET, []() {
    if (mp3_player_connect == 4) {
      send_command(0x0E, 0x01, 0, 0); // Стоп
      delay(100);
      mp3_stop = true;
      pause_on = false;
      HTTP.send(200, "text/plain", "OK");
    } else {
      HTTP.send(503, "text/plain", "MP3 player not ready");
    }
  });
#endif // USE_MP3_PLAYER

  HTTP.on("/setup_sound", HTTP_GET, []() {
#ifndef USE_MP3_PLAYER
    HTTP.send(404, "text/plain", "Настройки звука отключены в прошивке");
#else
    handleFileRead("/setup_sound.htm");
#endif
  });

  // ------------------------------------------------------------------ УПРАВЛЕНИЕ ИК ПУЛЬТАМИ ---------------------------------------------------------
#if USE_IR_RECEIVER
  // список всех пультов
  HTTP.on("/api/ir/remotes", HTTP_GET, []() {
    String remotes = irManager.getRemotesList();
    HTTP.send(200, "application/json; charset=utf-8", remotes);
  });

  // страница настройки ИК пульта
  HTTP.on("/setup_ir", HTTP_GET, []() {
    String page = readFile("setup_ir.htm", 32768);
    if (page == "Failed" || page.isEmpty()) {
      HTTP.send(500, "text/plain; charset=utf-8", "Ошибка загрузки страницы");
      return;
    }

    HTTP.send(200, "text/html; charset=utf-8", page);
  });

  // текущий пульт
  HTTP.on("/api/ir/current", HTTP_GET, []() {
    DynamicJsonDocument doc(256);
    doc["id"] = irManager.getCurrentRemoteId();
    doc["name"] = irManager.getCurrentRemoteName();
    String response;
    serializeJson(doc, response);
    HTTP.send(200, "application/json; charset=utf-8", response);
  });

  // установка пульта
  HTTP.on("/api/ir/set", HTTP_POST, []() {
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, HTTP.arg("plain"));

    if (error) {
      HTTP.send(400, "application/json; charset=utf-8", "{\"success\":false,\"message\":\"Invalid JSON\"}");
      return;
    }

    String remoteId = doc["remote_id"];
    if (irManager.loadRemote(remoteId)) {
      DynamicJsonDocument response(256);
      response["success"] = true;
      response["name"] = irManager.getCurrentRemoteName();
      response["id"] = irManager.getCurrentRemoteId();
      String resp;
      serializeJson(response, resp);
      HTTP.send(200, "application/json; charset=utf-8", resp);
    } else {
      HTTP.send(400, "application/json; charset=utf-8", "{\"success\":false,\"message\":\"Remote not found\"}");
    }
  });

  // обучение пульта
  String learningRemoteName = "";
  DynamicJsonDocument learningDoc(4096);
  bool isLearning = false;

  HTTP.on("/api/ir/learn/save", HTTP_POST, []() {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, HTTP.arg("plain"));

    if (error) {
      HTTP.send(400, "application/json; charset=utf-8", "{\"success\":false,\"message\":\"Invalid JSON\"}");
      return;
    }

    String remoteName = doc["name"].as<String>();
    if (remoteName.length() == 0) remoteName = "Мой пульт";

    File file = LittleFS.open("/ir_codes.json", "r");
    DynamicJsonDocument jsonDoc(8192);
    if (file) {
      deserializeJson(jsonDoc, file);
      file.close();
    } else {
      jsonDoc["remotes"] = JsonArray();
    }

    String remoteId = "custom_" + String(millis());

    // новый пульт
    JsonObject newRemote = jsonDoc["remotes"].createNestedObject();
    newRemote["id"] = remoteId;
    newRemote["name"] = remoteName;
    JsonObject codes = newRemote.createNestedObject("codes");
    JsonObject learnedCodes = doc["codes"];
    for (JsonPair kv : learnedCodes) {
      codes[kv.key()] = kv.value().as<String>();
    }

    file = LittleFS.open("/ir_codes.json", "w");
    if (!file) {
      HTTP.send(500, "application/json; charset=utf-8", "{\"success\":false,\"message\":\"Cannot save file\"}");
      return;
    }
    serializeJson(jsonDoc, file);
    file.close();

    irManager.loadRemote(remoteId);

    HTTP.send(200, "application/json; charset=utf-8", "{\"success\":true,\"id\":\"" + remoteId + "\",\"name\":\"" + remoteName + "\"}");
  });

  // получение последнего ИК кода
  HTTP.on("/api/ir/last_code", HTTP_GET, []() {
    DynamicJsonDocument doc(64);
    doc["code"] = String(lastIRCode, HEX);
    String response;
    serializeJson(doc, response);
    HTTP.send(200, "application/json; charset=utf-8", response);
  });
#endif

  // ------------------------------------------------------------------------- НАСТРОЙКА ШРИФТА ---------------------------------------------------------
  HTTP.on("/static_font", HTTP_GET, []() {
    if (!HTTP.hasArg("static_font")) {
      HTTP.send(400, "text/plain", "Missing static_font parameter");
      return;
    }
    String arg = HTTP.arg("static_font");
    int val = arg.toInt();
    if (val < 0 || val > 3) {
      HTTP.send(400, "text/plain", "Invalid static_font value (must be 0-3)");
      return;
    }
    if (staticFont == val) {
      HTTP.send(200, "text/plain", "OK (no change)");
      return;
    }
    staticFont = val;
    jsonWrite(configLedPanel, "static_font", String(staticFont));
    saveConfig();
    loadingFlag = true;
    clockNeedRedraw = true;
    dateNeedRedraw = true;
    weatherNeedRedraw = true;
    needFullRedraw = true;
    HTTP.send(200, "text/plain", "OK");
  });

  // ------------------------------------------------------------- МОДАЛЬНЫЕ ОКНА МЕНЮ ВЭБ-ИНТЕРФЕЙСА ---------------------------------------------------
#if STATUS_DEVICE
  HTTP.on("/sd_status", HTTP_GET, handle_sd_status);
  HTTP.on("/ota_status", HTTP_GET, handle_ota_status);
  HTTP.on("/mqtt_status", HTTP_GET, handle_mqtt_status);
  HTTP.on("/multilamp_status", HTTP_GET, handle_multilamp_status);
  HTTP.on("/mp3_status", HTTP_GET, handle_mp3_status);
  HTTP.on("/tm1637_status", HTTP_GET, handle_tm1637_status);
  HTTP.on("/button_status", HTTP_GET, handle_button_status);
  HTTP.on("/ir_rf_status", HTTP_GET, handle_ir_rf_status);
  HTTP.on("/st7789_status", HTTP_GET, handle_st7789_status);
  HTTP.on("/show_weather", HTTP_GET, handle_show_weather);
  HTTP.on("/version", HTTP_GET, []() { // Инфа о прошивке
    DynamicJsonDocument doc(1024);
#ifdef VERSION
    doc["version"] = VERSION;
#else
    doc["version"] = "";
#endif
    doc["build_date"] = __DATE__ " " __TIME__;

#ifdef ESP32_S3_USED
    doc["chip"] = "ESP32-S3";
#else
    doc["chip"] = "ESP32";
#endif

    doc["cpu_freq"] = ESP.getCpuFreqMHz();
    doc["flash_size"] = ESP.getFlashChipSize() / 1024 / 1024;
    doc["free_heap"] = ESP.getFreeHeap();
    doc["uptime"] = millis() / 1000UL;
    doc["welcome_page"] = Eeprom::instance().IsWelcomePageActive() ? "АКТИВНА" : "ОТКЛЮЧЕНА";
    doc["t_flag"] = T_flag;
    doc["direct_to_main"] = (T_flag == 1);

    // Wi-Fi режим
    wifi_mode_t mode = WiFi.getMode();
    String wifiModeStr;
    if (mode == WIFI_AP_STA)  wifiModeStr = "AP + Station";
    else if (mode == WIFI_AP) wifiModeStr = "AP only";
    else if (mode == WIFI_STA) wifiModeStr = "Station only";
    else                        wifiModeStr = "WiFi выключен";
    doc["wifi_mode"] = wifiModeStr;

    auto& wifi = Wifi::instance();
    bool staConnected = wifi.isConnected();
    String currentIP = staConnected ? wifi.localIP().toString() : wifi.apIP().toString();
    if (currentIP == "0.0.0.0") currentIP = "192.168.4.1";
    doc["ip_address"] = currentIP;
    doc["sta_connected"] = staConnected;
    doc["sta_ssid"] = staConnected ? wifi.getSSID() : "";
    doc["sta_rssi"] = staConnected ? wifi.getRSSI() : 0;
    doc["lamp_on"] = ONflag;
    doc["current_effect"] = currentMode;
    doc["effect_count"] = MODE_AMOUNT;

    // Статус модулей
#if USE_MP3_PLAYER
    doc["mp3_enabled"] = true;
#else
    doc["mp3_enabled"] = false;
#endif
#if USE_TM1637
    doc["tm1637_enabled"] = true;
#else
    doc["tm1637_enabled"] = false;
#endif
#if USE_ST7789
    doc["st7789_enabled"] = true;
#else
    doc["st7789_enabled"] = false;
#endif
#if USE_MQTT
    doc["mqtt_enabled"] = true;
#else
    doc["mqtt_enabled"] = false;
#endif

    String response;
    serializeJson(doc, response);
    HTTP.send(200, "application/json; charset=utf-8", response);
  });

  // IP адрес
  HTTP.on("/wifi_ip", HTTP_GET, []() {
    DynamicJsonDocument doc(512);
    auto& wifi = Wifi::instance();
    bool staConnected = wifi.isConnected();
    doc["connected"] = staConnected;
    doc["sta_ip"] = staConnected ? wifi.localIP().toString() : "Не подключён";
    doc["ap_ip"] = wifi.apIP().toString();
    doc["ssid"] = staConnected ? wifi.getSSID() : "";
    doc["rssi"] = staConnected ? wifi.getRSSI() : 0;
    String ap = doc["ap_ip"].as<String>();
    if (ap == "0.0.0.0") doc["ap_ip"] = "192.168.4.1";
    String response;
    serializeJson(doc, response);
    HTTP.send(200, "application/json; charset=utf-8", response);
  });

#endif // STATUS_DEVICE

  // ------------------------------------------------------------
#if SOFT_INFO
  // информация о памяти
  HTTP.on("/heap", HTTP_GET, []() {
    DynamicJsonDocument doc(1280);
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t freePsram = 0;
    uint32_t totalPsram = 0;
    bool hasPsram = false;
    String chip = "ESP32";

#ifdef ESP32_S3_USED
    totalPsram = ESP.getPsramSize();
    freePsram = ESP.getFreePsram();
    hasPsram = true;
    chip = "ESP32-S3";
#else
    if (psramFound()) {
      totalPsram = ESP.getPsramSize();
      freePsram = ESP.getFreePsram();
      hasPsram = true;
    }
#endif // ESP32_S3_USED

    doc["chip"] = chip;
    doc["free"] = freeHeap;
    doc["total_dram"] = totalHeap;
    doc["free_psram"] = freePsram;
    doc["total_psram"] = totalPsram;
    doc["total"] = totalHeap + totalPsram;
    doc["free_total"] = freeHeap + freePsram;
    doc["psram"] = hasPsram;
    doc["uptime"] = millis() / 1000UL;

    String response;
    serializeJson(doc, response);
    HTTP.send(200, "application/json", response);
  });

  // ----------------------------------------------------------------------
  HTTP.on("/get_version", HTTP_GET, []() {
    StaticJsonDocument<512> doc;
    String resp;

    doc["current_version"] = String(VERSION);
    doc["build_datetime"] = buildDateTimeString();
    doc["folder_url"] = "https://github.com/an-core/FieryLedLamp_mod/tree/main/FieryLedLamp_mod";

    serializeJson(doc, resp);
    HTTP.send(200, "application/json; charset=utf-8", resp);
  });

  HTTP.on("/check_update", HTTP_GET, []() {
    if (updateCache.valid && (millis() - updateCache.timestamp < 600000)) {
      HTTP.send(200, "application/json; charset=utf-8", updateCache.response);
      return;
    }

    if (updateCheckInProgress) {
      DynamicJsonDocument doc(256);
      doc["has_update"] = false;
      doc["message"] = "Проверка обновлений уже выполняется, попробуйте позже";
      String resp;
      serializeJson(doc, resp);
      HTTP.send(200, "application/json; charset=utf-8", resp);
      return;
    }

    if (!updateCheckPending) {
      updateCheckPending = true;
      DynamicJsonDocument doc(256);
      doc["has_update"] = false;
      doc["message"] = "Проверка обновлений запущена, обновите страницу через минуту";
      String resp;
      serializeJson(doc, resp);
      HTTP.send(200, "application/json; charset=utf-8", resp);
      return;
    }

    HTTP.send(200, "application/json; charset=utf-8", "{\"has_update\":false,\"message\":\"Попробуйте позже\"}");
  });

#endif // SOFT_INFO

  // --------------------------------------------------------------------- СКРЫТИЕ ПУНКТОВ МЕНЮ ---------------------------------------------------------
  HTTP.on("/features", HTTP_GET, []() {
    DynamicJsonDocument doc(512);
    doc["mqtt"]           = !!USE_MQTT;
    doc["mp3"]            = !!USE_MP3_PLAYER;
    doc["sd"]             = !!USE_SD;
    doc["ota"]            = !!USE_OTA;
    doc["tm1637"]         = !!USE_TM1637;
    doc["st7789"]         = !!USE_ST7789;
    doc["button"]         = !!USE_BUTTON;
    doc["ir"]             = !!USE_IR_RECEIVER;
    doc["ir_settings"]    = !!USE_IR_RECEIVER;
    doc["rf"]             = !!USE_RF_RECEIVER;
    doc["weather"]        = !!USE_WEATHER;
    doc["multilamp"]      = !!USE_MULTILAMP;
    doc["out_files"]      = (USE_SD && !FS_AS_SD);
    doc["dawn"]           = !!USE_DAWN;
    doc["sunset"]         = !!USE_SUNSET;
    doc["schedule"]       = !!USE_SCHEDULE;
    doc["status_modal"]   = !!STATUS_DEVICE;
    doc["software_modal"] = !!SOFT_INFO;
    doc["memory_info"]    = !!SOFT_INFO;
    doc["led_panel"]      = !!LED_PANEL;
    doc["backup"]         = !!BACKUP_CFG_FILES;
    doc["logs"]           = !!DEBUG_ENABLED;
    doc["syslog"]         = !!DEBUG_ENABLED;
    doc["hardware"]       = USE_BUTTON || USE_IR_RECEIVER || USE_RF_RECEIVER || USE_TM1637 || USE_ST7789 || USE_MP3_PLAYER;

    String response;
    serializeJson(doc, response);
    HTTP.send(200, "application/json", response);
  });

  // ----------------------------------------------------------------------- MQTT ПОДКЛЮЧЕНИЕ -----------------------------------------------------------
#if USE_MQTT
  HTTP.on("/mqtt_set", handle_mqtt_set);
  HTTP.on("/mqtt_on", handle_mqtt_on);
  HTTP.on("/mqtt_prd", handle_mqtt_period);
#endif

  HTTP.on("/setup_mqtt.json.gz", HTTP_GET, []() {
#ifndef USE_MQTT
    HTTP.send(404, "text/plain", "MQTT disabled in firmware");
#else
    File file = LittleFS.open("/setup_mqtt.json.gz", "r");
    if (!file) {
      HTTP.send(404, "text/plain", "File not found");
      return;
    }
    HTTP.streamFile(file, "application/octet-stream");
    file.close();
#endif
  });


  // ------------------------------------------------------------ ОБНОВЛЕНИЕ ПРОШИВКИ ПО ВОЗДУХУ (OTA) --------------------------------------------------
#if USE_OTA
  // страница OTA ОБНОВЛЕНИЕ
  HTTP.on("/update", HTTP_GET, []() {
    if (!Wifi::instance().isConnected()) {
      HTTP.send(403, "text/plain; charset=utf-8", "WiFi не подключён");
      return;
    }

    String page = readFile("update.htm", 32768);
    if (page == "Failed" || page.isEmpty()) {
      HTTP.send(500, "text/plain; charset=utf-8", "Ошибка загрузки страницы обновления");
      return;
    }

    String platform;
#ifdef ESP32
#ifdef ESP32_S3_USED
    platform = "ESP32-S3";
#else
    platform = "ESP32";
#endif
#endif

    page.replace("{LAMP_NAME}", LAMP_NAME);
    page.replace("{IP_ADDRESS}", Wifi::instance().localIP().toString());
    page.replace("{OTA_TIMEOUT}", String(ESP_CONF_TIMEOUT));
    page.replace("{PLATFORM}", platform);

    HTTP.send(200, "text/html; charset=utf-8", page);
  });

  // обновление прошивки
  HTTP.on("/update_upload", HTTP_POST, []() {
    HTTP.send(200, "text/plain", "OK");
  }, []() {
    static bool updateStarted = false;
    static uint32_t expectedSize = 0;
    static uint32_t totalReceived = 0;

    HTTPUpload& upload = HTTP.upload();

    if (upload.status == UPLOAD_FILE_START) {
      updateStarted = false;
      expectedSize = upload.totalSize;
      totalReceived = 0;

      if (expectedSize == 0) {
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace, U_FLASH)) {
          upload.status = UPLOAD_FILE_ABORTED;
          return;
        }
        updateStarted = true;
      } else {
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;

        if (expectedSize > maxSketchSpace) {
          upload.status = UPLOAD_FILE_ABORTED;
          return;
        }

        if (!Update.begin(expectedSize, U_FLASH)) {
          upload.status = UPLOAD_FILE_ABORTED;
          return;
        }
        updateStarted = true;
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (updateStarted) {
        size_t written = Update.write(upload.buf, upload.currentSize);
        if (written != upload.currentSize) {
          Update.abort();
          upload.status = UPLOAD_FILE_ABORTED;
        } else {
          totalReceived += written;
          if (expectedSize > 0) {
          }
        }
      }
    } else if (upload.status == UPLOAD_FILE_END) {

      if (updateStarted && Update.end(true)) {

        HTTP.send(200, "text/plain", "SUCCESS");
        delay(100);
        delay(1000);
        ESP.restart();
      } else {
        HTTP.send(500, "text/plain", "Update failed");
      }
      updateStarted = false;
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
      if (updateStarted) {
        Update.abort();
      }
      updateStarted = false;
    }
  });

  // Обновление файловой системы
  HTTP.on("/update_fs", HTTP_POST, []() {}, []() {
    static bool updateStarted = false;
    static uint32_t expectedSize = 0;
    static uint32_t totalReceived = 0;

    HTTPUpload& upload = HTTP.upload();

    if (upload.status == UPLOAD_FILE_START) {
      updateStarted = false;
      expectedSize = upload.totalSize;
      totalReceived = 0;

      LittleFS.end();
      delay(200);

      uint32_t maxFSSize = 0;

      const esp_partition_t* partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);

      if (partition) {
        maxFSSize = partition->size;
      } else {
        partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "littlefs");
        if (partition) {
          maxFSSize = partition->size;
        }
      }

      if (maxFSSize == 0) {
        maxFSSize = 4 * 1024 * 1024;
      }

      if (expectedSize > 0 && expectedSize > maxFSSize) {
        upload.status = UPLOAD_FILE_ABORTED;
        LittleFS.begin();
        return;
      }

      if (!Update.begin(maxFSSize, U_SPIFFS)) {
        upload.status = UPLOAD_FILE_ABORTED;
        LittleFS.begin();
        return;
      }

      updateStarted = true;
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (updateStarted) {
        size_t written = Update.write(upload.buf, upload.currentSize);
        if (written != upload.currentSize) {
          Update.abort();
          upload.status = UPLOAD_FILE_ABORTED;
        } else {
          totalReceived += written;
          if (expectedSize > 0) {
          }
        }
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (updateStarted && Update.end(true)) {
        HTTP.send(200, "text/plain", "FS_UPDATE_SUCCESS");
        delay(100);
        delay(1000);
        ESP.restart();
      } else {
        HTTP.send(500, "text/plain", "FS update failed");
        LittleFS.begin();
      }
      updateStarted = false;
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
      if (updateStarted) {
        Update.abort();
      }
      LittleFS.begin();
      updateStarted = false;
    }
  });

  // OTA обновление из Arduino IDE
  HTTP.on("/ota_request", HTTP_GET, []() {
    if (!Wifi::instance().isConnected()) {
      HTTP.send(403, "text/plain; charset=utf-8", "WiFi не подключён");
      return;
    }

    if (Ota::instance().RequestOtaUpdate()) {
      HTTP.send(200, "text/plain", "OK");
    } else {
      HTTP.send(400, "text/plain", "Не удалось запустить OTA. Возможно обновление уже выполняется?");
    }
  });

#endif // USE_OTA

  // --------------------------------------------------- ПЕРЕКЛЮЧЕНИЕ "ЧАСЫ / ПОГОДА" НА ДИСПЛЕЯХ TM1637 и ST7789 ---------------------------------------
#if (USE_TM1637 || USE_ST7789)
  HTTP.on("/save_display_switch", HTTP_GET, []() {
    int interval = HTTP.arg("value").toInt();
    if (interval >= 5 && interval <= 60) {
      jsonWrite(configSetup, "disp_switch", interval);
      saveConfig();
      DISPLAY_SWITCH_INTERVAL = (uint32_t)interval * 1000UL;
      displaySwitchTimer = millis();
      HTTP.send(200, "text/plain", "OK");
    } else if (interval == 0) {
      jsonWrite(configSetup, "disp_switch", 0);
      saveConfig();
      DISPLAY_SWITCH_INTERVAL = 0;
      HTTP.send(200, "text/plain", "OK");
    } else {
      HTTP.send(400, "text/plain", "Invalid value. Use 0 or 5-60 seconds");
    }
  });
#endif

  // ----------------------------------------------------------------------- ПАРОЛЬ ДЛЯ РЕДАКТОРА -------------------------------------------------------
  HTTP.on("/set_local_auth", HTTP_GET, []() {
    if (!HTTP.hasArg("value")) {
      HTTP.send(400, "text/plain", "Missing value");
      return;
    }
    String val = HTTP.arg("value");
    bool newState = (val == "1" || val.equalsIgnoreCase("true") || val.equalsIgnoreCase("on"));

    if (ONflag) changePower();
    delay(200);

    jsonWrite(configSetup, "local_auth", newState ? "1" : "0");
    saveConfig();

    if (LittleFS.exists("/auth_local.txt")) LittleFS.remove("/auth_local.txt");

    HTTP.send(200, "text/html", "<html><body>OK, restart...</body></html>");
    delay(500);
    ESP.restart();
  });

  // ---------------------------------------------------------------------------------- ДЕБАГ -----------------------------------------------------------
#if DEBUG_ENABLED
  // Чекбокс "Включить получение новых логов"
  HTTP.on("/set_syslog", HTTP_GET, []() {
    if (!HTTP.hasArg("value")) {
      HTTP.send(400, "text/plain", "Missing value");
      return;
    }

    bool newState = (HTTP.arg("value") == "1");

    SystemLog::instance().setEnabled(newState);

    jsonWrite(configSetup, "syslog_enabled", newState ? "1" : "0");
    saveConfig();

    HTTP.send(200, "text/plain", newState ? "1" : "0");
  });

  HTTP.on("/logs", HTTP_GET, []() {
    String logs = SystemLog::instance().getAll();
    HTTP.send(200, "text/plain; charset=utf-8", logs);
  });

  HTTP.on("/logs_clear", HTTP_GET, []() {
    SystemLog::instance().clear();
    HTTP.send(200, "text/plain", "OK");
  });

  HTTP.on("/logs_page", HTTP_GET, []() {
    handleFileRead("/logs.htm");
  });
#else
  // Заглушки, если логи отключены
  HTTP.on("/set_syslog", HTTP_GET, []() {
    HTTP.send(404, "text/plain", "Logs disabled");
  });
  HTTP.on("/logs", HTTP_GET, []() {
    HTTP.send(404, "text/plain", "Logs disabled");
  });
  HTTP.on("/logs_clear", HTTP_GET, []() {
    HTTP.send(404, "text/plain", "Logs disabled");
  });
  HTTP.on("/logs_page", HTTP_GET, []() {
    HTTP.send(404, "text/plain", "Logs disabled");
  });
#endif // DEBUG_ENABLED

  // ------------------------------------------------------------------------ Wi-Fi ПОДКЛЮЧЕНИЕ ---------------------------------------------------------
  HTTP.on("/ssdp", handle_ssdp); // Имя лампы
  HTTP.on("/save_wifi", HTTP_GET, handle_save_wifi);
  HTTP.on("/wifi_reconnect_interval", handle_wifi_reconnect_interval);
  HTTP.on("/wifi_check_interval", handle_wifi_check_interval);
  HTTP.on("/ap_always", handle_ap_always); // Чекбокс "Показывать точку доступа"
  HTTP.on("/reset_to_default", handle_reset_to_default);
  HTTP.on("/s_IP", handle_use_static_ip);
  HTTP.on("/set_ip", handle_set_static_ip);
  HTTP.on("/wifi_multi", HTTP_GET, []() {
    if (HTTP.hasArg("wifi_multi")) {
      int val = HTTP.arg("wifi_multi").toInt();
      jsonWrite(configWiFi, "wifi_multi", val);
      saveConfig();
      wifi_multi_enabled = val;
      HTTP.send(200, "text/plain", "OK");
    } else {
      HTTP.send(400, "text/plain", "Missing parameter");
    }
  });

  // Точка доступа
  HTTP.on("/ssidap", HTTP_GET, []() {
    jsonWrite(configWiFi, "ssidAP", HTTP.arg("ssidAP"));
    jsonWrite(configWiFi, "passwordAP", HTTP.arg("passwordAP"));
    saveConfig();
    HTTP.send(200, "text/plain", "OK");
  });

} // User_settings()

// ========================================================================== ОБРАБОТЧИКИ =============================================================

// Имя лампы
void handle_ssdp() {
  jsonWrite(configSetup, "ssdp", HTTP.arg("ssdp"));
  SSDP.setName(jsonRead(configSetup, "ssdp"));
  saveConfig();
  LAMP_NAME = jsonRead(configSetup, "ssdp");
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

// ----------------------------------------------------------------------------- WI-FI ----------------------------------------------------------------
// Показывать точку доступа всегда
void handle_ap_always() {
  if (!HTTP.hasArg("ap_always")) {
    HTTP.send(400, "text/plain", "Missing parameter");
    return;
  }

  bool newVal = (HTTP.arg("ap_always") == "1" || HTTP.arg("ap_always").toInt() == 1);
  bool oldVal = Always_ap;

  if (newVal == oldVal) {
    HTTP.send(200, "text/plain", newVal ? "1" : "0");
    return;
  }

  Wifi::instance().setAPAlways(newVal);
  Always_ap = newVal;

  HTTP.send(200, "text/plain", newVal ? "1" : "0");
}

void handle_wifi_reconnect_interval() {
  if (!HTTP.hasArg("interval")) {
    HTTP.send(400, "text/plain", "Parameter missing");
    return;
  }

  uint32_t interval = HTTP.arg("interval").toInt();
  interval = constrain(interval, 10, 300);
  Wifi::instance().setReconnectInterval(interval);

  jsonWrite(configWiFi, "wifi_reconnect_interval", String(interval));
  saveConfig();

  HTTP.send(200, "text/plain", "OK");
}

void handle_wifi_check_interval() {
  if (!HTTP.hasArg("interval")) {
    HTTP.send(400, "text/plain", "Parameter missing");
    return;
  }

  uint32_t interval = HTTP.arg("interval").toInt();
  interval = constrain(interval, 60, 3600); // от 1 до 60 минут
  Wifi::instance().setCheckInterval(interval);

  jsonWrite(configWiFi, "wifi_check_interval", String(interval));
  saveConfig();

  HTTP.send(200, "text/plain", "OK");
}

void handle_save_wifi() {
  jsonWrite(configWiFi, "ssid", HTTP.arg("ssid"));
  jsonWrite(configWiFi, "password", HTTP.arg("password"));

  for (int i = 2; i <= 5; i++) {
    String ssidKey = "ssid" + String(i);
    String passKey = "password" + String(i);

    if (HTTP.hasArg(ssidKey)) {
      jsonWrite(configWiFi, ssidKey, HTTP.arg(ssidKey));
    }
    if (HTTP.hasArg(passKey)) {
      jsonWrite(configWiFi, passKey, HTTP.arg(passKey));
    }
  }

  if (HTTP.hasArg("wifi_timeout")) {
    int to = HTTP.arg("wifi_timeout").toInt();
    if (to < 10 || to > 600) to = 30;
    jsonWrite(configWiFi, "wifi_timeout", to);
    Wifi::instance().setWiFiTimeout(to);
  }

  if (HTTP.hasArg("forced_ap_after")) {
    int minutes = HTTP.arg("forced_ap_after").toInt();
    if (minutes >= 1 && minutes <= 30) {
      jsonWrite(configWiFi, "forced_ap_after", minutes);
      Wifi::instance().setForcedAPTimeout(minutes);
    }
  }

  if (HTTP.hasArg("wifi_reconnect_interval")) {
    int reconnectInterval = HTTP.arg("wifi_reconnect_interval").toInt();
    reconnectInterval = constrain(reconnectInterval, 30, 300);
    jsonWrite(configWiFi, "wifi_reconnect_interval", reconnectInterval);
    Wifi::instance().setReconnectInterval(reconnectInterval);
  }

  if (HTTP.hasArg("wifi_check_interval")) {
    int checkInterval = HTTP.arg("wifi_check_interval").toInt();
    checkInterval = constrain(checkInterval, 1, 30);
    jsonWrite(configWiFi, "wifi_check_interval", checkInterval);
    Wifi::instance().setCheckInterval(checkInterval * 60);
  }

  if (HTTP.hasArg("wifi_multi")) {
    int multi = HTTP.arg("wifi_multi").toInt();
    jsonWrite(configWiFi, "wifi_multi", multi);
  }

  if (HTTP.hasArg("ap_always")) {
    int apAlways = HTTP.arg("ap_always").toInt();
    Wifi::instance().setAPAlways(apAlways);
    Always_ap = apAlways;
  }

  saveConfig();

  Wifi::instance().begin();

  HTTP.send(200, "text/plain", "OK");
}

void handle_use_static_ip() {
  bool enable = (HTTP.arg("s_IP").toInt() == 1);
  jsonWrite(configWiFi, "s_IP", enable ? "1" : "0");
  saveConfig();
  Wifi::instance().begin();
  HTTP.send(200, "text/plain", "OK");
}

void handle_set_static_ip() {
  IPAddress ip, gateway, subnet, dns;
  if (!dns.fromString(HTTP.arg("dns"))) {
    dns = IPAddress(8, 8, 8, 8);
  }
  if (ip.fromString(HTTP.arg("ip")) &&
      gateway.fromString(HTTP.arg("gateway")) &&
      subnet.fromString(HTTP.arg("subnet")) &&
      dns.fromString(HTTP.arg("dns"))) {
    Wifi::instance().setStaticIP(ip, gateway, subnet, dns);
    saveConfig();
    HTTP.send(200, "application/json", "{\"should_refresh\": \"true\"}");
  } else {
    HTTP.send(400, "text/plain", "Invalid IP format");
  }
}

// ----------------------------------------------------------------------- НАСТРОЙКИ КНОПКИ -----------------------------------------------------------
#if USE_BUTTON
void handle_button_type() {
  if (!HTTP.hasArg("button_type")) {
    HTTP.send(400, "text/plain", "Missing button_type");
    return;
  }
  String configButton = readFile(F("config_button.json"), 1024);
  if (configButton == "Failed" || configButton == "Large" || configButton.isEmpty()) {
    configButton = "{}";
  }
  button_type = HTTP.arg("button_type").toInt();
  if (button_type > 1) button_type = 1;
  jsonWrite(configButton, "button_type", button_type);
  writeFile(F("config_button.json"), configButton);

  if (button_type) {
    touch.setType(LOW_PULL);
    touch.setDebounce(BUTTON_SET_DEBOUNCE_SENSORY);
  } else {
    touch.setType(HIGH_PULL);
    touch.setDebounce(BUTTON_SET_DEBOUNCE_MECHANICAL);
  }
  touch.setDirection(NORM_OPEN);
  touch.setTimeout(BUTTON_CLICK_TIMEOUT);
  touch.setStepTimeout(BUTTON_STEP_TIMEOUT);

  HTTP.send(200, "application/json", "{\"should_refresh\": \"true\"}");
}

void handle_save_btn_clicks() {
  const char* required[] = {"btn_click_power", "btn_click_next", "btn_click_prev", "btn_click_action4", "btn_click_ip", "btn_click_time", "btn_click_sound", "btn_click_weather"};
  for (const char* arg : required) {
    if (!HTTP.hasArg(arg)) {
      HTTP.send(400, "text/plain", String("Missing ") + arg);
      return;
    }
  }

  String configButton = readFile(F("config_button.json"), 1024);
  if (configButton == "Failed" || configButton == "Large" || configButton.isEmpty()) {
    configButton = "{}";
  }

  btn_click_power = HTTP.arg("btn_click_power").toInt();
  btn_click_next = HTTP.arg("btn_click_next").toInt();
  btn_click_prev = HTTP.arg("btn_click_prev").toInt();
  btn_click_action4 = HTTP.arg("btn_click_action4").toInt();
  btn_click_ip = HTTP.arg("btn_click_ip").toInt();
  btn_click_time = HTTP.arg("btn_click_time").toInt();
  btn_click_sound = HTTP.arg("btn_click_sound").toInt();
  btn_click_weather = HTTP.arg("btn_click_weather").toInt();

  jsonWrite(configButton, "btn_click_power", btn_click_power);
  jsonWrite(configButton, "btn_click_next", btn_click_next);
  jsonWrite(configButton, "btn_click_prev", btn_click_prev);
  jsonWrite(configButton, "btn_click_action4", btn_click_action4);
  jsonWrite(configButton, "btn_click_ip", btn_click_ip);
  jsonWrite(configButton, "btn_click_time", btn_click_time);
  jsonWrite(configButton, "btn_click_sound", btn_click_sound);
  jsonWrite(configButton, "btn_click_weather", btn_click_weather);

  writeFile(F("config_button.json"), configButton);

  HTTP.send(200, "application/json", "{\"should_refresh\": \"true\"}");
}

#endif // USE_BUTTON

// ----------------------------------------------------------------------- НАСТРОЙКИ МП3 ПЛЕЕРА -------------------------------------------------------
#if USE_MP3_PLAYER
// Сохранение настроек
void handle_save_sound_settings() {
  uint8_t tmp;
  if (HTTP.hasArg("tim_h")) {
    tmp = HTTP.arg("tim_h").toInt();
    ADVERT_TIMER_H = 100UL * tmp;
    jsonWrite(configMP3, "tim_h", tmp);
  }
  if (HTTP.hasArg("tim_m")) {
    tmp = HTTP.arg("tim_m").toInt();
    ADVERT_TIMER_M = 100UL * tmp;
    jsonWrite(configMP3, "tim_m", tmp);
  }
  if (HTTP.hasArg("delay")) {
    tmp = HTTP.arg("delay").toInt();
    mp3_delay = 10UL * tmp;
    jsonWrite(configMP3, "delay", tmp);
  }
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// --------------------- Чекбоксы ------------------------
// Чекбокс "Включить звук"
void handle_on_sound() {
  eff_sound_on = HTTP.arg("on_sound").toInt();
  jsonWrite(configMP3, "on_sound", eff_sound_on);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("text/plain"), F("OK"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Озвучивание времени днём"
void handle_day_advert_on_sound() {
  day_advert_sound_on = HTTP.arg("on_day_adv").toInt();
  jsonWrite(configMP3, "on_day_adv", day_advert_sound_on);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("text/plain"), F("OK"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Озвучивание времени ночью"
void handle_night_advert_on_sound() {
  night_advert_sound_on = HTTP.arg("on_night_adv").toInt();
  jsonWrite(configMP3, "on_night_adv", night_advert_sound_on);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("text/plain"), F("OK"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Озвучивать время на выключенной лампе"
void handle_time_always() {
  time_always = HTTP.arg("time_always").toInt();
  jsonWrite(configMP3, "time_always", time_always);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("text/plain"), F("OK"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Озвучивать погоду на выключенной лампе"
void handle_weather_always() {
  weather_always = HTTP.arg("weather_always").toInt();
  jsonWrite(configMP3, "weather_always", weather_always);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("text/plain"), F("OK"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Озвучивать температуру погоды днём"
void handle_day_weather_adv() {
  day_weather_temp_on = HTTP.arg("on_day_wadv").toInt();
  jsonWrite(configMP3, "on_day_wadv", day_weather_temp_on);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("text/plain"), F("OK"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Озвучивать описание погоды днём"
void handle_day_weather_desc() {
  day_weather_desc_on = HTTP.arg("on_day_wdesc").toInt();
  jsonWrite(configMP3, "on_day_wdesc", day_weather_desc_on);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("text/plain"), F("OK"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Озвучивать температуру погоды ночью"
void handle_night_weather_adv() {
  night_weather_temp_on = HTTP.arg("on_night_wadv").toInt();
  jsonWrite(configMP3, "on_night_wadv", night_weather_temp_on);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("text/plain"), F("OK"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Озвучивать описание погоды ночью"
void handle_night_weather_desc() {
  night_weather_desc_on = HTTP.arg("on_night_wdesc").toInt();
  jsonWrite(configMP3, "on_night_wdesc", night_weather_desc_on);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("text/plain"), F("OK"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Озвучивать время будильником"
void handle_alarm_advert_sound_on() {
  alarm_advert_sound_on = HTTP.arg("on_alm_adv").toInt();
  jsonWrite(configMP3, "on_alm_adv", alarm_advert_sound_on);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("text/plain"), F("OK"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Озвучивать погоду" (аналог времени: отключает автоматическое озвучивание, но кнопка всегда работает)
void handle_weather_speak() {
  if (HTTP.hasArg("value")) {
    weatherSpeakEnabled = (HTTP.arg("value") == "1");
    jsonWrite(configMP3, "weather_speak", weatherSpeakEnabled ? "1" : "0");
    writeFile(F("config_mp3.json"), configMP3);
  }
  HTTP.send(200, "text/plain", weatherSpeakEnabled ? "1" : "0");
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Озвучивать время" (аналог погоды: отключает автоматическое озвучивание, но кнопка всегда работает)
void handle_time_speak() {
  if (HTTP.hasArg("value")) {
    timeAnnounceEnabled = (HTTP.arg("value") == "1");
    jsonWrite(configMP3, "time_speak", timeAnnounceEnabled ? "1" : "0");
    writeFile(F("config_mp3.json"), configMP3);
  }
  HTTP.send(200, "text/plain", timeAnnounceEnabled ? "1" : "0");
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Включить звук будильника"
void handle_alarm_on_sound() {
  alarm_sound_on = HTTP.arg("on_alm_snd").toInt();
  jsonWrite(configMP3, "on_alm_snd", alarm_sound_on);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("text/plain"), F("OK"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Включить звук режима Закат"
void handle_sunset_on_sound() {
  sunset_sound_on = HTTP.arg("on_sun_snd").toInt();
  jsonWrite(configMP3, "on_sun_snd", sunset_sound_on);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("text/plain"), F("OK"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Показывать описание погоды"
void handle_show_weather_desc() {
  if (HTTP.hasArg("value")) {
    show_weather_desc = HTTP.arg("value").toInt();
    jsonWrite(configMP3, "show_weather_desc", show_weather_desc);
    writeFile(F("config_mp3.json"), configMP3);
  }
  HTTP.send(200, F("text/plain"), show_weather_desc ? "1" : "0");
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// --------------------- Регулировка ------------------------
// Общая гроикость
void handle_volume() {
  eff_volume = constrain(HTTP.arg("vol").toInt(), 0, 30);
  jsonWrite(configMP3, "vol", eff_volume);
  if (mp3Enabled && !dawnflag_sound && !sunsetflag_sound && !isAnnouncing)
    send_command(0x06, FEEDBACK, 0, eff_volume);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Громкость будильника Рассвет
void handle_alarm_volume() {
  alarm_volume = constrain(HTTP.arg("alm_vol").toInt(), 0, 30);
  jsonWrite(configMP3, "alm_vol", alarm_volume);
  if (mp3Enabled && dawnflag_sound) send_command(0x06, FEEDBACK, 0, alarm_volume);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Громкость режима Закат
void handle_sunset_volume() {
  sunset_volume = constrain(HTTP.arg("sun_vol").toInt(), 0, 30);
  jsonWrite(configMP3, "sun_vol", sunset_volume);
  if (mp3Enabled && sunsetflag_sound) send_command(0x06, FEEDBACK, 0, sunset_volume);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Громкость озвучивания времени днём
void handle_day_advert_volume() {
  day_advert_volume = constrain(HTTP.arg("day_vol").toInt(), 0, 30);
  jsonWrite(configMP3, "day_vol", day_advert_volume);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Громкость озвучивания времени ночью
void handle_night_advert_volume() {
  night_advert_volume = constrain(HTTP.arg("night_vol").toInt(), 0, 30);
  jsonWrite(configMP3, "night_vol", night_advert_volume);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Длительность звучания часов
void handle_tim_h() {
  uint8_t tmp = constrain(HTTP.arg("tim_h").toInt(), 8, 40);
  ADVERT_TIMER_H = 100UL * tmp;
  jsonWrite(configMP3, "tim_h", tmp);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Длительность звучания минут
void handle_tim_m() {
  uint8_t tmp = constrain(HTTP.arg("tim_m").toInt(), 8, 45);
  ADVERT_TIMER_M = 100UL * tmp;
  jsonWrite(configMP3, "tim_m", tmp);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Задержка между командами
void handle_delay() {
  uint8_t tmp = constrain(HTTP.arg("delay").toInt(), 3, 23);
  mp3_delay = 10UL * tmp;
  jsonWrite(configMP3, "delay", tmp);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Громкость погоды днём
void handle_weather_day_volume() {
  weather_day_volume = constrain(HTTP.arg("val").toInt(), 0, 30);
  jsonWrite(configMP3, "weather_day_vol", weather_day_volume);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Громкость погоды ночью
void handle_weather_night_volume() {
  weather_night_volume = constrain(HTTP.arg("val").toInt(), 0, 30);
  jsonWrite(configMP3, "weather_night_vol", weather_night_volume);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Задержка после температуры (перед описанием)
void handle_weather_temp_delay() {
  uint8_t tmp = constrain(HTTP.arg("weather_temp_delay").toInt(), 5, 60);
  ADVERT_TIMER_1 = 100UL * tmp; // 500 - 6000 мс
  jsonWrite(configMP3, "weather_temp_delay", tmp);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Задержка после описания (перед завершением озвучки)
void handle_weather_desc_delay() {
  uint8_t tmp = constrain(HTTP.arg("weather_desc_delay").toInt(), 10, 80);
  ADVERT_TIMER_2 = 100UL * tmp; // 1000 - 8000 мс
  jsonWrite(configMP3, "weather_desc_delay", tmp);
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// --------------------- Озвучка ------------------------
// Озвучка времени (по кнопке)
void handle_play_time() {
  if (mp3Enabled && mp3_player_connect == 4 && eff_sound_on && !isAnnouncing && !alarm_sound_flag && !sunset_sound_flag) {
    Time::instance().forcesync();
    previous_folder = mp3_folder;
    play_time_ADVERT(true);
    HTTP.send(200, "text/plain", "OK");
  } else {
    HTTP.send(503, "text/plain", "MP3 not ready, sound off or already speaking");
  }
}

// Озвучка погоды (по кнопке)
void handle_play_weather() {
#if USE_WEATHER
  if (mp3Enabled && mp3_player_connect >= 4 && eff_sound_on && !isAnnouncing && !alarm_sound_flag && !sunset_sound_flag) {
    play_weather(true);
    HTTP.send(200, "text/plain", "OK");
  }
  else {
    HTTP.send(503, "text/plain", "MP3 not ready, sound off or already speaking");
  }
#endif
}

// ----------------------------------------------------------------------
// Папка будильника
void handle_alarm_fold_sel() {
  AlarmFolder = HTTP.arg("alm_fold").toInt();
  jsonWrite(configMP3, "alm_fold", AlarmFolder);
  writeFile(F("config_mp3.json"), configMP3);
  if (mp3Enabled && alarm_sound_flag) {
    mp3_folder = AlarmFolder;
    play_sound();
  }
  HTTP.send(200, F("text/plain"), F("OK"));
}

// Папка Заката
void handle_sunset_fold_sel() {
  SunsetFolder = HTTP.arg("sun_fold").toInt();
  jsonWrite(configMP3, "sun_fold", SunsetFolder);
  writeFile(F("config_mp3.json"), configMP3);
  if (mp3Enabled && sunset_sound_flag) {
    mp3_folder = SunsetFolder;
    play_sound();
  }
  HTTP.send(200, F("text/plain"), F("OK"));
}

void handle_sound_set() {
  char i[4];
  String configSound = readFile(F("sound_list.json"), 2048);
  for (uint8_t k = 0; k < MODE_AMOUNT; k++) {
    itoa(k, i, 10);
    String e = "e" + String(i);
    if (!first_entry)
      jsonWrite(configSound, e, HTTP.arg(e).toInt());
    effects_folders[k] = jsonReadtoInt(configSound, e);
    yield();
  }
  if (!first_entry) {
    writeFile(F("sound_list.json"), configSound);
  }
  HTTP.send(200, F("text/plain"), F("OK"));
}

void handle_folder_down() {
  CurrentFolder = constrain(CurrentFolder - 1, 0, 99);
  jsonWrite(configMP3, "fold_sel", CurrentFolder);
  writeFile(F("config_mp3.json"), configMP3);
  if (mp3Enabled && !pause_on && !mp3_stop && eff_sound_on && !isAnnouncing) {
    send_command(0x17, FEEDBACK, 0, CurrentFolder);
    delay(mp3_delay);
  }
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
}

void handle_folder_up() {
  CurrentFolder = constrain(CurrentFolder + 1, 0, 99);
  jsonWrite(configMP3, "fold_sel", CurrentFolder);
  writeFile(F("config_mp3.json"), configMP3);
  if (mp3Enabled && !pause_on && !mp3_stop && eff_sound_on && !isAnnouncing) {
    send_command(0x17, FEEDBACK, 0, CurrentFolder);
    delay(mp3_delay);
  }
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
}

void handle_folder_select() {
  CurrentFolder = HTTP.arg("fold_sel").toInt();
  jsonWrite(configMP3, "fold_sel", CurrentFolder);
  writeFile(F("config_mp3.json"), configMP3);
  if (mp3Enabled && !pause_on && !mp3_stop && eff_sound_on && !isAnnouncing) {
    send_command(0x17, FEEDBACK, 0, CurrentFolder);
    delay(mp3_delay);
  }
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
}
// ----------------------------------------------------------------------
// Эквалайзер
void handle_equalizer() {
  Equalizer = HTTP.arg("eq").toInt();
  jsonWrite(configMP3, "eq", Equalizer);
  if (mp3Enabled) {
    send_command(0x07, FEEDBACK, 0, Equalizer);
  }
  writeFile(F("config_mp3.json"), configMP3);
  HTTP.send(200, F("text/plain"), F("OK"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

#endif // USE_MP3_PLAYER

// ------------------------------------------------------ РЕГУЛИРОВКА НАСТРОЕК ЭФФЕКТОВ НА ГЛАВНОЙ СТРАНИЦЕ -------------------------------------------
// Общая яркость
void handle_all_br() {
  jsonWrite(configSetup, "all_br", HTTP.arg("all_br").toInt());
  uint8_t ALLbri = jsonReadtoInt(configSetup, "all_br");

  for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
    modes[i].Brightness = ALLbri;
  }

  jsonWrite(configSetup, "br", ALLbri);
  SetBrightness(ALLbri);
  saveConfig();
  Eeprom::instance().EepromPut(modes);
  loadingFlag = true;

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
}

// Яркость
void handle_br() {
  if (!ONflag) {
    HTTP.send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  int newBri = constrain(HTTP.arg("br").toInt(), 1, 255);
  modes[currentMode].Brightness = newBri;
  jsonWrite(configSetup, "br", newBri);
  SetBrightness(newBri);

  saveConfig();
  Eeprom::instance().EepromPut(modes);

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Скорость
void handle_sp() {
  if (!ONflag) {
    HTTP.send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  uint8_t newSpeed = constrain(HTTP.arg("sp").toInt(), 1, 255);
  modes[currentMode].Speed = newSpeed;
  jsonWrite(configSetup, "sp", newSpeed);
  loadingFlag = true;

  Eeprom::instance().EepromPut(modes);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Масштаб
void handle_sc() {
  if (!ONflag) {
    HTTP.send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  uint8_t newScale = constrain(HTTP.arg("sc").toInt(), 1, 100);
  modes[currentMode].Scale = newScale;
  jsonWrite(configSetup, "sc", newScale);
  loadingFlag = true;

  Eeprom::instance().EepromPut(modes);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Яркость -1
void handle_brm() {
  if (!ONflag) {
    HTTP.send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  modes[currentMode].Brightness = constrain(modes[currentMode].Brightness - 1, 1, 255);
  jsonWrite(configSetup, "br", modes[currentMode].Brightness);
  SetBrightness(modes[currentMode].Brightness);

  Eeprom::instance().EepromPut(modes);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Яркость +1
void handle_brp() {
  if (!ONflag) {
    HTTP.send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  modes[currentMode].Brightness = constrain(modes[currentMode].Brightness + 1, 1, 255);
  jsonWrite(configSetup, "br", modes[currentMode].Brightness);
  SetBrightness(modes[currentMode].Brightness);

  Eeprom::instance().EepromPut(modes);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Скорость -1
void handle_spm() {
  if (!ONflag) {
    HTTP.send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  modes[currentMode].Speed = constrain(modes[currentMode].Speed - 1, 1, 255);
  jsonWrite(configSetup, "sp", modes[currentMode].Speed);
  loadingFlag = true;

  Eeprom::instance().EepromPut(modes);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Скорость +1
void handle_spp() {
  if (!ONflag) {
    HTTP.send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  modes[currentMode].Speed = constrain(modes[currentMode].Speed + 1, 1, 255);
  jsonWrite(configSetup, "sp", modes[currentMode].Speed);
  loadingFlag = true;

  Eeprom::instance().EepromPut(modes);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Масштаб -1
void handle_scm() {
  if (!ONflag) {
    HTTP.send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  modes[currentMode].Scale = constrain(modes[currentMode].Scale - 1, 1, 100);
  jsonWrite(configSetup, "sc", modes[currentMode].Scale);
  loadingFlag = true;

  Eeprom::instance().EepromPut(modes);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Масштаб +1
void handle_scp() {
  if (!ONflag) {
    HTTP.send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  modes[currentMode].Scale = constrain(modes[currentMode].Scale + 1, 1, 100);
  jsonWrite(configSetup, "sc", modes[currentMode].Scale);
  loadingFlag = true;

  Eeprom::instance().EepromPut(modes);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

void handle_eff_sel () {
  uint8_t temp = (HTTP.arg("eff_sel").toInt());
  jsonWrite(configSetup, "eff_sel", temp);
  currentMode = eff_num_correct[temp];
  jsonWrite(configSetup, "br", modes[currentMode].Brightness);
  jsonWrite(configSetup, "sp", modes[currentMode].Speed);
  jsonWrite(configSetup, "sc", modes[currentMode].Scale);
  SetBrightness(modes[currentMode].Brightness);
  loadingFlag = true;
  if (random_on && Favorites::instance().FavoritesRunning)
    selectedSettings = 1U;
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
#if USE_BLYNK
  updateRemoteBlynkParams();
#endif
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
}

// ----------------------------------------------------------------------------------------------------------------------------------------------------
void handle_night_time() {
  String day_hour_str = HTTP.arg("day_time_hour");
  String day_min_str = HTTP.arg("day_time_minute");
  String night_hour_str = HTTP.arg("night_time_hour");
  String night_min_str = HTTP.arg("night_time_minute");

  int day_hour = day_hour_str.toInt();
  int day_min = day_min_str.toInt();
  int night_hour = night_hour_str.toInt();
  int night_min  = night_min_str.toInt();

  day_hour = constrain(day_hour, 0, 23);
  day_min = constrain(day_min, 0, 59);
  night_hour = constrain(night_hour, 0, 23);
  night_min = constrain(night_min, 0, 59);

  char buf[3];
  snprintf(buf, sizeof(buf), "%02d", day_hour);
  day_hour_str = buf;
  snprintf(buf, sizeof(buf), "%02d", day_min);
  day_min_str = buf;

  snprintf(buf, sizeof(buf), "%02d", night_hour);
  night_hour_str = buf;
  snprintf(buf, sizeof(buf), "%02d", night_min);
  night_min_str = buf;

  jsonWrite(configSetup, "day_time_hour",   day_hour_str);
  jsonWrite(configSetup, "day_time_minute", day_min_str);
  jsonWrite(configSetup, "night_time_hour", night_hour_str);
  jsonWrite(configSetup, "night_time_minute", night_min_str);

  uint16_t day_time_minutes   = day_hour * 60 + day_min;
  uint16_t night_time_minutes = night_hour * 60 + night_min;

  jsonWrite(configSetup, "day_time", day_time_minutes);
  jsonWrite(configSetup, "night_time", night_time_minutes);

  uint8_t day_bright = constrain(HTTP.arg("day_bright").toInt(), 0, 255);
  uint8_t night_bright = constrain(HTTP.arg("night_bright").toInt(), 0, 255);

  jsonWrite(configSetup, "day_bright", day_bright);
  jsonWrite(configSetup, "night_bright", night_bright);

  saveConfig();

  NIGHT_HOURS_START = night_time_minutes;
  NIGHT_HOURS_STOP = day_time_minutes;
  NIGHT_HOURS_BRIGHTNESS = night_bright;
  DAY_HOURS_BRIGHTNESS = day_bright;

  getBrightnessForPrintTime();

#if USE_DAWN
  if (ONflag && !dawnFlag)
#else
  if (ONflag)
#endif
    SetBrightness(modes[currentMode].Brightness);

#if USE_TM1637
  if (tm1637Enabled) {
    clockTicker_blink();
  }
#endif
#if USE_ST7789
  TFT_ApplyBrightnessNow();
#endif

  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  HTTP.send(200, F("text/plain"), F("OK"));
}

// ----------------------------------------------------------------------------------------------------------------------------------------------------
void handle_save_time() {
  String day_hour_str = HTTP.arg("day_time_hour");
  String day_min_str = HTTP.arg("day_time_minute");
  String night_hour_str = HTTP.arg("night_time_hour");
  String night_min_str = HTTP.arg("night_time_minute");

  int day_hour = day_hour_str.toInt();
  int day_min = day_min_str.toInt();
  int night_hour = night_hour_str.toInt();
  int night_min = night_min_str.toInt();

  day_hour = constrain(day_hour, 0, 23);
  day_min = constrain(day_min, 0, 59);
  night_hour = constrain(night_hour, 0, 23);
  night_min = constrain(night_min, 0, 59);

  char buf[3];
  snprintf(buf, sizeof(buf), "%02d", day_hour);
  day_hour_str = buf;
  snprintf(buf, sizeof(buf), "%02d", day_min);
  day_min_str = buf;
  snprintf(buf, sizeof(buf), "%02d", night_hour);
  night_hour_str = buf;
  snprintf(buf, sizeof(buf), "%02d", night_min);
  night_min_str = buf;

  jsonWrite(configSetup, "day_time_hour", day_hour_str);
  jsonWrite(configSetup, "day_time_minute", day_min_str);
  jsonWrite(configSetup, "night_time_hour", night_hour_str);
  jsonWrite(configSetup, "night_time_minute", night_min_str);

  uint16_t day_time_minutes = day_hour * 60 + day_min;
  uint16_t night_time_minutes = night_hour * 60 + night_min;

  jsonWrite(configSetup, "day_time", day_time_minutes);
  jsonWrite(configSetup, "night_time", night_time_minutes);
  saveConfig();

  NIGHT_HOURS_START = night_time_minutes;
  NIGHT_HOURS_STOP = day_time_minutes;

  getBrightnessForPrintTime();

#if USE_DAWN
  if (ONflag && !dawnFlag)
#else
  if (ONflag)
#endif
    SetBrightness(modes[currentMode].Brightness);

  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  HTTP.send(200, F("text/plain"), F("OK"));
}

// ----------------------------------------------------------------------------------------------------------------------------------------------------
// Чекбокс "Включать лампу после обесточивания" (изменённая логика, было наоборот)
void handle_effect_always () {
  int value = HTTP.arg("effect_always").toInt();
  jsonWrite(configSetup, "effect_always", value);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);
  HTTP.send(200, F("text/plain"), F("OK"));
}

// --------------------------------------------------------------------------- ИЗБРАННОЕ --------------------------------------------------------------
void handle_favorit() {
  jsonWrite(configSetup, "favorit", HTTP.arg("favorit").toInt());
  timeout_save_file_changes = millis();
  bitSet (save_file_changes, 0);
  Favorit_only = jsonReadtoInt(configSetup, "favorit");
  HTTP.send(200, F("text/plain"), F("OK"));
}

void handle_random() {
  jsonWrite(configSetup, "random_on", HTTP.arg("random_on").toInt());
  timeout_save_file_changes = millis();
  bitSet (save_file_changes, 0);
  random_on = jsonReadtoInt(configSetup, "random_on");
  HTTP.send(200, F("text/plain"), F("OK"));
}

// ----------------------------------------------------------------------- УПРАВЛЕНИЕ ПИТАНИЕМ --------------------------------------------------------
void handle_Power() {
  static uint32_t lastPowerCommandMs = 0;
  uint32_t nowMs = millis();
  if (nowMs - lastPowerCommandMs < 500UL) {
    HTTP.send(200, "application/json", "{\"should_refresh\":\"false\"}");
    return;
  }
  lastPowerCommandMs = nowMs;

#if USE_DAWN
  if (dawnFlag == 1) {
    manualOff = true;
    dawnFlag = 2;
#if USE_TM1637
    if (tm1637Enabled) {
      clockTicker_blink();
    }
#endif
    SetBrightness(modes[currentMode].Brightness);
    changePower();
    HTTP.send(200, "application/json", "{\"should_refresh\":\"true\"}");
    return;
  }
#endif

#if USE_SUNSET
  if (sunsetFlag == 1) {
    manualsOff = true;
    sunsetFlag = 2;
#if USE_TM1637
    if (tm1637Enabled) {
      clockTicker_blink();
    }
#endif
    SetBrightness(modes[currentMode].Brightness);
    changePower();
    HTTP.send(200, "application/json", "{\"should_refresh\":\"true\"}");
    return;
  }
#endif

  // в режиме ночных часов после нажатия на кнопку Power лампа просто выключится
  if (ONflag && nightModeBrightness > 0 && currentMode == EFF_CLOCK) {
    nightClockEnabled = false;
    jsonWrite(configLedPanel, "night_clock_enabled", "0");
    nightModeBrightness = 0;

    // ВЫКЛ лампы
    ONflag = false;
    jsonWrite(configSetup, "Power", ONflag);
    saveConfig();

    FastLED.setBrightness(0);
    FastLED.clear();
    FastLED.show();

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif

    systemShuttingDown = false;
    justPoweredOn = false;
    loadingFlag = false;

    HTTP.send(200, "application/json", "{\"should_refresh\":\"true\"}");

#if USE_MULTILAMP
    multiple_lamp_control();
#endif
#if USE_MQTT
    if (Wifi::instance().isConnected()) {
      Mqtt::instance().needToPublish = true;
    }
#endif
    return;
  }

  // обычная логика включения/выключения
  uint8_t requestedState = HTTP.arg("Power").toInt();
  if (requestedState == 2) {
    requestedState = ONflag ? 0 : 1;
  }

  bool wasOn = ONflag;

  if (!wasOn && requestedState == 1) {
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
#endif
  }

  ONflag = requestedState;
  jsonWrite(configSetup, "Power", ONflag);
  saveConfig();

  if (wasOn && !ONflag) {
    manualOverrideOff = true;
  } else if (!wasOn && ONflag) {
    manualOverrideOff = false;
  }

  manualControlTimer = millis() + 60000UL;

  if (ONflag) {
    Eeprom::instance().EepromGet(modes);
    loadingFlag = true;
    // пользовательская яркость для обычных часов
    if (currentMode == EFF_CLOCK && nightModeBrightness == 0) {
      uint8_t normalBrightness = userClockBrightness;
      if (normalBrightness < 2) normalBrightness = 30;
      modes[currentMode].Brightness = normalBrightness;
      jsonWrite(configSetup, "br", normalBrightness);
    }

    changePower();
    timeout_save_file_changes = millis();
    bitSet(save_file_changes, 0);
  } else {
    changePower();
  }

  HTTP.send(200, "application/json", "{\"should_refresh\":\"true\"}");

#if USE_MULTILAMP
  if (ONflag) repeat_multiple_lamp_control = true;
  else multiple_lamp_control();
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// ----------------------------------------------------------------------------- РАССВЕТ --------------------------------------------------------------
#if USE_DAWN
bool firstStartAlarm = true;
void handle_alarm() {
  if (configAlarm.isEmpty() || configAlarm == "null") {
    configAlarm = "{}";
  }

  bool saveNeeded = false;

  for (uint8_t k = 0; k < 7; k++) {
    char idx[2];
    itoa(k + 1, idx, 10);

    String key_a = "a" + String(idx);
    String key_h = "h" + String(idx);
    String key_m = "m" + String(idx);

    String hoursStr = HTTP.arg(key_h);
    String minutesStr = HTTP.arg(key_m);
    uint8_t state = HTTP.arg(key_a).toInt();

    if (hoursStr.length() == 0) {
      hoursStr = jsonRead(configAlarm, key_h);
      if (hoursStr.length() == 0) hoursStr = "07";
    }
    if (minutesStr.length() == 0) {
      minutesStr = jsonRead(configAlarm, key_m);
      if (minutesStr.length() == 0) minutesStr = "00";
    }
    if (HTTP.arg(key_a).length() == 0) {
      state = jsonReadtoInt(configAlarm, key_a);
    }
    hoursStr   = zeroPad(hoursStr, 2);
    minutesStr = zeroPad(minutesStr, 2);

    if (jsonRead(configAlarm, key_h) != hoursStr) {
      jsonWrite(configAlarm, key_h, hoursStr);
      saveNeeded = true;
    }
    if (jsonRead(configAlarm, key_m) != minutesStr) {
      jsonWrite(configAlarm, key_m, minutesStr);
      saveNeeded = true;
    }
    if (jsonReadtoInt(configAlarm, key_a) != state) {
      jsonWrite(configAlarm, key_a, state);
      saveNeeded = true;
    }

    alarms[k].State = state;
    alarms[k].Time = hoursStr.toInt() * 60 + minutesStr.toInt();
  }

  uint8_t dawnModeNew = HTTP.arg("t").toInt();
  uint8_t dawnTimeoutNew = HTTP.arg("after").toInt();
  uint8_t dawnBrightNew  = HTTP.arg("a_br").toInt();

  if (dawnModeNew == 0) dawnModeNew = jsonReadtoInt(configAlarm, "t");
  if (dawnTimeoutNew == 0) dawnTimeoutNew = jsonReadtoInt(configAlarm, "after");
  if (dawnBrightNew == 0)  dawnBrightNew = jsonReadtoInt(configAlarm, "a_br");

  if (jsonReadtoInt(configAlarm, "t") != dawnModeNew) {
    jsonWrite(configAlarm, "t", dawnModeNew);
    saveNeeded = true;
  }
  if (jsonReadtoInt(configAlarm, "after") != dawnTimeoutNew) {
    jsonWrite(configAlarm, "after", dawnTimeoutNew);
    saveNeeded = true;
  }
  if (jsonReadtoInt(configAlarm, "a_br") != dawnBrightNew) {
    jsonWrite(configAlarm, "a_br", dawnBrightNew);
    saveNeeded = true;
  }

  dawnMode = dawnModeNew - 1;
  DAWN_TIMEOUT = dawnTimeoutNew;
  DAWN_BRIGHT = dawnBrightNew;

  if (saveNeeded || firstStartAlarm) {
    save_file_changes |= SAVE_ALARMS_BIT;
    timeout_save_file_changes = millis();
    firstStartAlarm = false;
  }

  HTTP.send(200, F("application/json"), F("{\"should_refresh\":\"true\"}"));
}

void save_alarms() {
  if (configAlarm.isEmpty()) configAlarm = "{}";

  bool changed = false;

  for (uint8_t i = 0; i < 7; i++) {
    char idx[2];
    itoa(i + 1, idx, 10);

    String key_a = "a" + String(idx);
    String key_h = "h" + String(idx);
    String key_m = "m" + String(idx);

    uint8_t currentState = jsonReadtoInt(configAlarm, key_a);
    uint8_t currentHours = jsonReadtoInt(configAlarm, key_h);
    uint8_t currentMins  = jsonReadtoInt(configAlarm, key_m);
    uint16_t currentTime = currentHours * 60 + currentMins;

    if (alarms[i].State != currentState || alarms[i].Time != currentTime) {
      changed = true;

      jsonWrite(configAlarm, key_a, alarms[i].State);

      char buf[3];
      sprintf(buf, "%02d", alarms[i].Time / 60);
      jsonWrite(configAlarm, key_h, String(buf));
      sprintf(buf, "%02d", alarms[i]. Time % 60);
      jsonWrite(configAlarm, key_m, String(buf));
    }
  }

  if (dawnMode + 1 != jsonReadtoInt(configAlarm, "t")) {
    jsonWrite(configAlarm, "t", dawnMode + 1);
    changed = true;
  }
  if (DAWN_TIMEOUT != jsonReadtoInt(configAlarm, "after")) {
    jsonWrite(configAlarm, "after", DAWN_TIMEOUT);
    changed = true;
  }
  if (DAWN_BRIGHT != jsonReadtoInt(configAlarm, "a_br")) {
    jsonWrite(configAlarm, "a_br", DAWN_BRIGHT);
    changed = true;
  }

  if (changed) {
    saveAlarmConfig(configAlarm);
    configAlarm = readFile(F("config_alarm.json"), 2048);
  }
}

void saveAlarmConfig(const String & data) {
  File file = LittleFS.open(F("/config_alarm.json"), "w");
  if (!file) {
    return;
  }
  file.print(data);
  file.flush();
  delay(5);
  file.close();
}
#endif // USE_DAWN

// ------------------------------------------------------------------------------ ЗАКАТ ---------------------------------------------------------------
#if USE_SUNSET
bool firstStartSunset = true;

void saveSunsetConfig(const String & data) {
  writeFile(F("config_sunset.json"), data);
}

void handle_sunset() {
  if (configSunset.isEmpty() || configSunset == "null") {
    configSunset = "{}";
  }

  bool saveNeeded = false;

  for (uint8_t k = 0; k < 7; k++) {
    char idx[2];
    itoa(k + 1, idx, 10);

    String key_a = "a" + String(idx);
    String key_h = "h" + String(idx);
    String key_m = "m" + String(idx);

    String hoursStr = HTTP.arg(key_h);
    String minutesStr = HTTP.arg(key_m);
    uint8_t state = HTTP.arg(key_a).toInt();

    if (hoursStr.length() == 0) {
      hoursStr = jsonRead(configSunset, key_h);
      if (hoursStr.length() == 0) hoursStr = "21";
    }
    if (minutesStr.length() == 0) {
      minutesStr = jsonRead(configSunset, key_m);
      if (minutesStr.length() == 0) minutesStr = "00";
    }
    if (HTTP.arg(key_a).length() == 0) {
      state = jsonReadtoInt(configSunset, key_a);
    }

    hoursStr = zeroPad(hoursStr, 2);
    minutesStr = zeroPad(minutesStr, 2);

    if (jsonRead(configSunset, key_h) != hoursStr) {
      jsonWrite(configSunset, key_h, hoursStr);
      saveNeeded = true;
    }
    if (jsonRead(configSunset, key_m) != minutesStr) {
      jsonWrite(configSunset, key_m, minutesStr);
      saveNeeded = true;
    }
    if (jsonReadtoInt(configSunset, key_a) != state) {
      jsonWrite(configSunset, key_a, state);
      saveNeeded = true;
    }

    sunsets[k].State = state;
    sunsets[k].Time = hoursStr.toInt() * 60 + minutesStr.toInt();

    yield();
  }

  uint8_t sunsetModeNew = HTTP.arg("t").toInt();
  uint8_t sunsetBrightNew = HTTP.arg("s_br").toInt();

  if (sunsetModeNew == 0) sunsetModeNew = jsonReadtoInt(configSunset, "t");
  if (sunsetBrightNew == 0) sunsetBrightNew = jsonReadtoInt(configSunset, "s_br");

  if (jsonReadtoInt(configSunset, "t") != sunsetModeNew) {
    jsonWrite(configSunset, "t", sunsetModeNew);
    saveNeeded = true;
  }
  if (jsonReadtoInt(configSunset, "s_br") != sunsetBrightNew) {
    jsonWrite(configSunset, "s_br", sunsetBrightNew);
    saveNeeded = true;
  }

  sunsetMode = sunsetModeNew - 1;
  SUNSET_BRIGHT = sunsetBrightNew;

  if (saveNeeded || firstStartSunset) {
    save_file_changes |= SAVE_ALARMS_BIT;
    timeout_save_file_changes = millis();
    firstStartSunset = false;
  }

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

void save_sunsets() {
  if (configSunset.isEmpty() || configSunset == "null") {
    configSunset = "{}";
  }

  bool saveNeeded = false;

  for (uint8_t i = 0; i < 7; i++) {
    char idx[2];
    itoa(i + 1, idx, 10);

    String key_a = "a" + String(idx);
    String key_h = "h" + String(idx);
    String key_m = "m" + String(idx);

    uint8_t currentState = jsonReadtoInt(configSunset, key_a);
    uint8_t currentHour = jsonReadtoInt(configSunset, key_h);
    uint8_t currentMin = jsonReadtoInt(configSunset, key_m);
    uint16_t currentTime = currentHour * 60U + currentMin;

    if (sunsets[i].State != currentState || sunsets[i].Time != currentTime) {
      saveNeeded = true;
      jsonWrite(configSunset, key_a, sunsets[i].State);
      jsonWrite(configSunset, key_h, zeroPad(String(sunsets[i].Time / 60U), 2));
      jsonWrite(configSunset, key_m, zeroPad(String(sunsets[i].Time % 60U), 2));
    }

    yield();
  }

  int8_t configMode = jsonReadtoInt(configSunset, "t") - 1;
  if (sunsetMode != configMode) {
    saveNeeded = true;
    jsonWrite(configSunset, "t", (sunsetMode + 1));
  }

  if (SUNSET_BRIGHT != jsonReadtoInt(configSunset, "s_br")) {
    saveNeeded = true;
    jsonWrite(configSunset, "s_br", SUNSET_BRIGHT);
  }

  if (saveNeeded) {
    writeFile(F("config_sunset.json"), configSunset);
    configSunset = readFile(F("config_sunset.json"), 512);
  }
}
#endif // USE_SUNSET

// ----------------------------------------------------------------------------- РАСПИСАНИЕ -----------------------------------------------------------
#if USE_SCHEDULE
void handle_schedule() {
  if (configSchedule.isEmpty() || configSchedule == "null") {
    configSchedule = "{}";
  }

  bool changed = false;

  for (uint8_t i = 0; i < MAX_SCHEDULE_ENTRIES; i++) {
    schedule[i].State = 0;
    schedule[i].Day = 0;
    schedule[i].Time = 0;
    schedule[i].Action = 0;
    schedule[i].EffectNum = 255;
  }

  String oldEnable = jsonRead(configSchedule, "schedule_enabled");
  String enableVal = HTTP.hasArg("schedule_enabled") ? HTTP.arg("schedule_enabled") : oldEnable;
  if (enableVal != "1") enableVal = "0";
  jsonWrite(configSchedule, "schedule_enabled", enableVal);
  if (enableVal != oldEnable) changed = true;

  uint8_t timerIndex = 0;

  for (uint8_t slot = 1; slot <= 6; slot++) {
    String aKey = "a" + String(slot);
    String hKey = "h" + String(slot);
    String mKey = "m" + String(slot);
    String effKey = (slot == 3) ? "eff3" : "";

    String actionStr = HTTP.hasArg(aKey) ? HTTP.arg(aKey) : jsonRead(configSchedule, aKey);
    if (actionStr == "" || actionStr == "null") actionStr = "0";
    uint8_t action = actionStr.toInt();

    String hStr = HTTP.hasArg(hKey) ? HTTP.arg(hKey) : jsonRead(configSchedule, hKey);
    if (hStr == "" || hStr == "null" || hStr == "0") hStr = "00";
    String mStr = HTTP.hasArg(mKey) ? HTTP.arg(mKey) : jsonRead(configSchedule, mKey);
    if (mStr == "" || mStr == "null" || mStr == "0") mStr = "00";

    uint8_t h = constrain(hStr.toInt(), 0, 23);
    uint8_t m = constrain(mStr.toInt(), 0, 59);
    char buf[3];
    sprintf(buf, "%02d", h); String hFormatted = buf;
    sprintf(buf, "%02d", m); String mFormatted = buf;

    uint8_t effect = 255;

    if (slot == 3 && action == 3) {
      String effStr = HTTP.hasArg(effKey) ? HTTP.arg(effKey) : jsonRead(configSchedule, effKey);
      if (effStr != "" && effStr != "null") {
        effect = constrain(effStr.toInt(), 0, MODE_AMOUNT - 1);
      }
    }

    bool slotChanged = false;
    if (HTTP.hasArg(aKey)) {
      jsonWrite(configSchedule, aKey, actionStr);
      slotChanged = true;
    }
    if (HTTP.hasArg(hKey)) {
      jsonWrite(configSchedule, hKey, hFormatted);
      slotChanged = true;
    }
    if (HTTP.hasArg(mKey)) {
      jsonWrite(configSchedule, mKey, mFormatted);
      slotChanged = true;
    }
    if (slot == 3 && HTTP.hasArg(effKey)) {
      jsonWrite(configSchedule, effKey, String(effect));
      slotChanged = true;
    }
    if (slotChanged) changed = true;

    if (action != 0 && timerIndex < MAX_SCHEDULE_ENTRIES) {
      schedule[timerIndex].State = 1;
      schedule[timerIndex].Day = 0;
      schedule[timerIndex].Time = h * 60 + m;

      if (slot == 4) { // цикл
        schedule[timerIndex].Action = action;
        schedule[timerIndex].EffectNum = 255;
      } else if (slot == 5) { // обычные часы
        schedule[timerIndex].Action = 4;
        schedule[timerIndex].EffectNum = EFF_CLOCK;
      } else if (slot == 6) { // ночные часы
        schedule[timerIndex].Action = 7;
        schedule[timerIndex].EffectNum = EFF_CLOCK;
      } else { // 1,2,3
        schedule[timerIndex].Action = action;
        if (slot == 3 && action == 3) {
          schedule[timerIndex].EffectNum = effect;
        } else {
          schedule[timerIndex].EffectNum = 255;
        }
      }
      timerIndex++;
    }
  }

  if (changed) {
    save_file_changes |= SAVE_SCHEDULE_BIT;
    timeout_save_file_changes = millis();
  }

  DynamicJsonDocument resp(4096);
  resp["should_refresh"] = "true";
  resp["schedule_enabled"] = jsonRead(configSchedule, "schedule_enabled");

  for (uint8_t slot = 1; slot <= 6; slot++) {
    String aKey = "a" + String(slot);
    String hKey = "h" + String(slot);
    String mKey = "m" + String(slot);
    resp[aKey] = jsonRead(configSchedule, aKey);
    resp[hKey] = jsonRead(configSchedule, hKey);
    resp[mKey] = jsonRead(configSchedule, mKey);
    if (slot == 3) {
      String effKey = "eff3";
      String effVal = jsonRead(configSchedule, effKey);
      if (effVal == "" || effVal == "null") effVal = "255";
      resp[effKey] = effVal;
    }
  }
  String respStr;
  serializeJson(resp, respStr);
  HTTP.send(200, "application/json", respStr);
}

// загрузка расписания
void load_schedule() {
  if (configSchedule.isEmpty() || configSchedule == "null") {
    configSchedule = "{}";
  }

  for (uint8_t i = 0; i < MAX_SCHEDULE_ENTRIES; i++) {
    schedule[i].State = 0;
    schedule[i].Day = 0;
    schedule[i].Time = 0;
    schedule[i].Action = 0;
    schedule[i].EffectNum = 255;
  }

  String enableStr = jsonRead(configSchedule, "schedule_enabled");
  if (enableStr != "1") return;

  uint8_t timerIndex = 0;

  for (uint8_t slot = 1; slot <= 6 && timerIndex < MAX_SCHEDULE_ENTRIES; slot++) {
    String aKey = "a" + String(slot);
    String hKey = "h" + String(slot);
    String mKey = "m" + String(slot);
    String effKey = (slot == 3) ? "eff3" : "";

    uint8_t action = jsonReadtoInt(configSchedule, aKey, 0);
    if (action == 0) continue;

    String hStr = jsonRead(configSchedule, hKey);
    if (hStr == "" || hStr == "null") hStr = "00";
    String mStr = jsonRead(configSchedule, mKey);
    if (mStr == "" || mStr == "null") mStr = "00";

    uint8_t hour = constrain(hStr.toInt(), 0, 23);
    uint8_t minute = constrain(mStr.toInt(), 0, 59);
    uint16_t timeInMinutes = hour * 60 + minute;

    uint8_t effectNum = 255;

    if (slot == 3 && action == 3) {
      String effStr = jsonRead(configSchedule, effKey);
      if (effStr != "" && effStr != "null") {
        effectNum = constrain(effStr.toInt(), 0, MODE_AMOUNT - 1);
      }
    }

    schedule[timerIndex].State = 1;
    schedule[timerIndex].Day = 0;
    schedule[timerIndex].Time = timeInMinutes;
    schedule[timerIndex].Action = action;

    if (slot == 3 && action == 3) {
      // включить указанный эффект
      schedule[timerIndex].EffectNum = effectNum;
    } else if (slot == 5) {
      // обычные часы
      schedule[timerIndex].EffectNum = EFF_CLOCK;
      schedule[timerIndex].Action = 4;
    } else if (slot == 6) {
      // ночные часы
      schedule[timerIndex].EffectNum = EFF_CLOCK;
      schedule[timerIndex].Action = 7;
    } else {
      schedule[timerIndex].EffectNum = 255;
    }

    timerIndex++;
  }
}

#endif // USE_SCHEDULE

// -------------------------------------------------------------- УПРАВЛЕНИЕ НЕСКОЛЬКИМИ ЛАМПАМИ ------------------------------------------------------
#if USE_MULTILAMP
void handle_multiple_lamp() {
  jsonWrite(configMultilamp, "ml1", HTTP.arg("ml1").toInt());
  jsonWrite(configMultilamp, "ml2", HTTP.arg("ml2").toInt());
  jsonWrite(configMultilamp, "ml3", HTTP.arg("ml3").toInt());
  jsonWrite(configMultilamp, "ml4", HTTP.arg("ml4").toInt());
  jsonWrite(configMultilamp, "ml5", HTTP.arg("ml5").toInt());

  jsonWrite(configMultilamp, "host1", HTTP.arg("host1"));
  jsonWrite(configMultilamp, "host2", HTTP.arg("host2"));
  jsonWrite(configMultilamp, "host3", HTTP.arg("host3"));
  jsonWrite(configMultilamp, "host4", HTTP.arg("host4"));
  jsonWrite(configMultilamp, "host5", HTTP.arg("host5"));

  jsonWrite(configMultilamp, "comment1", HTTP.arg("comment1"));
  jsonWrite(configMultilamp, "comment2", HTTP.arg("comment2"));
  jsonWrite(configMultilamp, "comment3", HTTP.arg("comment3"));
  jsonWrite(configMultilamp, "comment4", HTTP.arg("comment4"));
  jsonWrite(configMultilamp, "comment5", HTTP.arg("comment5"));

  writeFile(F("config_multilamp.json"), configMultilamp);

  ml1 = jsonReadtoInt(configMultilamp, "ml1");
  ml2 = jsonReadtoInt(configMultilamp, "ml2");
  ml3 = jsonReadtoInt(configMultilamp, "ml3");
  ml4 = jsonReadtoInt(configMultilamp, "ml4");
  ml5 = jsonReadtoInt(configMultilamp, "ml5");

  String str = jsonRead(configMultilamp, "host1");
  str.toCharArray(Host1, sizeof(Host1));

  str = jsonRead(configMultilamp, "host2");
  str.toCharArray(Host2, sizeof(Host2));

  str = jsonRead(configMultilamp, "host3");
  str.toCharArray(Host3, sizeof(Host3));

  str = jsonRead(configMultilamp, "host4");
  str.toCharArray(Host4, sizeof(Host4));

  str = jsonRead(configMultilamp, "host5");
  str.toCharArray(Host5, sizeof(Host5));

#if USE_MP3_PLAYER
  send_sound = HTTP.arg("s_s").toInt();
  jsonWrite(configMP3, "s_s", send_sound);
  send_eff_volume = HTTP.arg("s_e_v").toInt();
  if (!send_sound) {
    send_eff_volume = 0;
  }
  jsonWrite(configMP3, "s_e_v", send_eff_volume);
  writeFile(F("config_mp3.json"), configMP3);
#endif // USE_MP3_PLAYER
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

void multilamp_get() {
  ml1 = jsonReadtoInt(configMultilamp, "ml1");
  ml2 = jsonReadtoInt(configMultilamp, "ml2");
  ml3 = jsonReadtoInt(configMultilamp, "ml3");
  ml4 = jsonReadtoInt(configMultilamp, "ml4");
  ml5 = jsonReadtoInt(configMultilamp, "ml5");

  String str = jsonRead(configMultilamp, "host1");
  str.toCharArray(Host1, sizeof(Host1));
  str = jsonRead(configMultilamp, "host2");
  str.toCharArray(Host2, sizeof(Host2));
  str = jsonRead(configMultilamp, "host3");
  str.toCharArray(Host3, sizeof(Host3));
  str = jsonRead(configMultilamp, "host4");
  str.toCharArray(Host4, sizeof(Host4));
  str = jsonRead(configMultilamp, "host5");
  str.toCharArray(Host5, sizeof(Host5));
}

void multiple_lamp_control() {
  char outputBuffer[64];
  if (connect) {
#if USE_MP3_PLAYER
    if (send_sound && !send_eff_volume) {
      snprintf_P(outputBuffer, sizeof(outputBuffer), PSTR("MULTI,%u,%u,%u,%u,%u,%u"), (uint8_t)ONflag, currentMode, modes[currentMode].Brightness, modes[currentMode].Speed, modes[currentMode].Scale, CurrentFolder);
    }
    else if (send_sound && send_eff_volume) {
      snprintf_P(outputBuffer, sizeof(outputBuffer), PSTR("MULTI,%u,%u,%u,%u,%u,%u,%u,%u"), (uint8_t)ONflag, currentMode, modes[currentMode].Brightness, modes[currentMode].Speed, modes[currentMode].Scale, eff_sound_on, eff_volume, CurrentFolder);
    }
    else {
      snprintf_P(outputBuffer, sizeof(outputBuffer), PSTR("MULTI,%u,%u,%u,%u,%u"), (uint8_t)ONflag, currentMode, modes[currentMode].Brightness, modes[currentMode].Speed, modes[currentMode].Scale);
    }
#else
    snprintf_P(outputBuffer, sizeof(outputBuffer), PSTR("MULTI,%u,%u,%u,%u,%u"), (uint8_t)ONflag, currentMode, modes[currentMode].Brightness, modes[currentMode].Speed, modes[currentMode].Scale);
#endif // USE_MP3_PLAYER

    if (ml1) {
      Udp.beginPacket(Host1, localPort);
      Udp.print(outputBuffer);
      Udp.endPacket();
#if DEBUG_ENABLED
      SYSLOG.add("Передача MULTI на %s: %s", Host1, outputBuffer);
#endif
    }
    if (ml2) {
      Udp.beginPacket(Host2, localPort);
      Udp.print(outputBuffer);
      Udp.endPacket();
#if DEBUG_ENABLED
      SYSLOG.add("Передача MULTI на %s: %s", Host2, outputBuffer);
#endif
    }
    if ( ml3 ) {
      Udp.beginPacket(Host3, localPort);
      Udp.print(outputBuffer);
      Udp.endPacket();
#if DEBUG_ENABLED
      SYSLOG.add("Передача MULTI на %s: %s", Host3, outputBuffer);
#endif
    }
    if ( ml4 ) {
      Udp.beginPacket(Host4, localPort);
      Udp.print(outputBuffer);
      Udp.endPacket();
#if DEBUG_ENABLED
      SYSLOG.add("Передача MULTI на %s: %s", Host4, outputBuffer);
#endif
    }
    if ( ml5 ) {
      Udp.beginPacket(Host5, localPort);
      Udp.print(outputBuffer);
      Udp.endPacket();
#if DEBUG_ENABLED
      SYSLOG.add("Передача MULTI на %s: %s", Host5, outputBuffer);
#endif
    }
    outputBuffer[0] = '\0';
  }
}

#endif // USE_MULTILAMP

// ----------------------------------------------------------------------- РЕЖИМ ЦИКЛ -----------------------------------------------------------------
// Включение/выключение режима Цикл
void handle_cycle_on() {
  uint8_t tmp;
  tmp = HTTP.arg("cycle_on").toInt();
  if (tmp == 2) tmp = (jsonReadtoInt(configSetup, "cycle_on") == 0) ? 1 : 0;

  if (ONflag && tmp)   {
    jsonWrite(configSetup, "cycle_on", 1);
    Favorites::instance().FavoritesRunning = 1;
    Eeprom::instance().EepromPut(modes);
  } else {
    Favorites::instance().FavoritesRunning = 0;
    Favorites::instance().nextModeAt = 0;
    jsonWrite(configSetup, "cycle_on", 0);
  }

  saveConfig();
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Время переключения цикла
void handle_time_eff () {
  jsonWrite(configSetup, "time_eff", HTTP.arg("time_eff").toInt());
  Favorites::instance().Interval = jsonReadtoInt(configSetup, "time_eff");
  jsonWrite(configSetup, "disp", HTTP.arg("disp").toInt());
  Favorites::instance().Dispersion = jsonReadtoInt(configSetup, "disp");
  timeout_save_file_changes = millis();
  bitSet (save_file_changes, 0);
  HTTP.send(200, F("text/plain"), F("OK"));
}

// Перемешать выбранные или по порядку
void handle_rnd_cycle () {
  jsonWrite(configSetup, "rnd_cycle", HTTP.arg("rnd_cycle").toInt());
  Favorites::instance().rndCycle = jsonReadtoInt(configSetup, "rnd_cycle");
  timeout_save_file_changes = millis();
  bitSet (save_file_changes, 0);
  HTTP.send(200, F("text/plain"), F("OK"));
}

// Запускать режим цикл после выкл/вкл лампы или нет
void handle_cycle_always () {
  jsonWrite(configSetup, "cycle_always", HTTP.arg("cycle_always").toInt());
  Favorites::instance().UseSavedFavoritesRunning = jsonReadtoInt(configSetup, "cycle_always");

  if (!ONflag && !Favorites::instance().UseSavedFavoritesRunning)   {
    Favorites::instance().FavoritesRunning = 0;
    jsonWrite(configSetup, "cycle_on", 0);
  }

  timeout_save_file_changes = millis();
  bitSet (save_file_changes, 0);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

// Выбор эффектов для Цикла
void handle_cycle_set () {
  char i[4];
  String configCycle = readFile(F("config_cycle.json"), 2048);

  for (uint8_t k = 0; k < MODE_AMOUNT; k++) {
    itoa ((k), i, 10);
    String e = "e" + String (i) ;
    if (!first_entry)
      jsonWrite(configCycle, e, HTTP.arg(e).toInt());
    Favorites::instance().FavoriteModes[k] = jsonReadtoInt(configCycle, e);
    yield();
  }

  if (!first_entry) {
    writeFile(F("config_cycle.json"), configCycle );
  }

  HTTP.send(200, F("text/plain"), F("OK"));
}

// сохранение выбранных эффектов в файл
void cycle_get () {
  char i[4];
  bool cycle_change = false;
  String configCycle = readFile(F("config_cycle.json"), 2048);

  for (uint8_t k = 0; k < MODE_AMOUNT; k++) {
    itoa ((k), i, 10);
    String e = "e" + String (i) ;
    if (Favorites::instance().FavoriteModes[k] != jsonReadtoInt(configCycle, e)) {
      jsonWrite(configCycle, e, Favorites::instance().FavoriteModes[k]);
      cycle_change = true;
    }
    yield();
  }

  if (cycle_change) {
    writeFile(F("config_cycle.json"), configCycle );
  }
}

// Выбрать эффекты
void handle_eff() {
  int temp = jsonReadtoInt(configSetup, "eff_sel");
  bool next = HTTP.arg("eff").toInt();

  if (Favorit_only) {
    uint8_t lastMode = currentMode;
    do {
      next ? temp++ : temp--;
      if (temp >= MODE_AMOUNT) temp = 0;
      if (temp < 0) temp = MODE_AMOUNT - 1;
      currentMode = eff_num_correct[temp];
    }
    while (Favorites::instance().FavoriteModes[currentMode] == 0 && currentMode != lastMode);

    if (currentMode == lastMode) {  // ни один не избранный - всё равно переключимся
      next ? temp++ : temp--;
      if (temp >= MODE_AMOUNT) temp = 0;
      if (temp < 0) temp = MODE_AMOUNT - 1;
      currentMode = eff_num_correct[temp];
    }
  } else {
    next ? temp++ : temp--;
    if (temp >= MODE_AMOUNT) temp = 0;
    if (temp < 0) temp = MODE_AMOUNT - 1;
    currentMode = eff_num_correct[temp];
  }

  jsonWrite(configSetup, "eff_sel", temp);
  jsonWrite(configSetup, "br", modes[currentMode].Brightness);
  jsonWrite(configSetup, "sp", modes[currentMode].Speed);
  jsonWrite(configSetup, "sc", modes[currentMode].Scale);
  SetBrightness(modes[currentMode].Brightness);
  loadingFlag = true;

  if (random_on && Favorites::instance().FavoritesRunning) selectedSettings = 1U;

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif

#if USE_BLYNK
  updateRemoteBlynkParams();
#endif

  HTTP.send(200, "application/json", "{\"should_refresh\": \"true\"}");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
}

// Выбрать все эффекты
void handle_eff_all () {
  char i[4];
  String configCycle = readFile(F("config_cycle.json"), 2048);

  for (uint8_t k = 0; k < MODE_AMOUNT; k++) {
    itoa ((k), i, 10);
    String e = "e" + String (i) ;
    jsonWrite(configCycle, e, 1U);
    yield();
  }
  writeFile(F("config_cycle.json"), configCycle );
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

// Очистить все эффекты
void handle_eff_clr () {
  char i[4];
  String configCycle = readFile(F("config_cycle.json"), 2048);

  for (uint8_t k = 0; k < MODE_AMOUNT; k++) {
    itoa ((k), i, 10);
    String e = "e" + String (i) ;
    jsonWrite(configCycle, e, 0U);
    yield();
  }
  writeFile(F("config_cycle.json"), configCycle );
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

// --------------------------------------------------------- СОХРАНЕНИЕ, СБРОС И ЗАГРУЗКА НАСТРОЕК ЭФФЕКТОВ -------------------------------------------
// Сброс настроек текущего эффекта по умолчанию
void handle_def() {
  setModeSettings();
  updateSets();

  ONflag = false;
  systemShuttingDown = true;
  loadingFlag = false;

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
  digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif

  if (leds != nullptr) {
    FastLED.clear();
    FastLED.show();
    effectsTick();
  }
#if USE_WEATHER
  Weather::instance().update();
#endif
  resetTimerState();
  manualOverride = true;
  manualOverrideUntil = millis() + 300000UL;

  showWarning(CRGB::Blue, 2000U, 500U);

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
}

// Сброс настроек всех эффектов по умолчанию
void handle_eff_reset() {
  restoreSettings();
  updateSets();

  ONflag = false;
  systemShuttingDown = true;
  loadingFlag = false;

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
  digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif

  if (leds != nullptr) {
    FastLED.clear();
    FastLED.show();
    effectsTick();
  }
#if USE_WEATHER
  Weather::instance().update();
#endif
  resetTimerState();
  manualOverride = true;
  manualOverrideUntil = millis() + 300000UL;

  jsonWrite(configSetup, "br", modes[currentMode].Brightness);
  jsonWrite(configSetup, "sp", modes[currentMode].Speed);
  jsonWrite(configSetup, "sc", modes[currentMode].Scale);

  showWarning(CRGB::Blue, 2000U, 500U);

#if USE_BLYNK
  updateRemoteBlynkParams();
#endif

  HTTP.send(200, F("text/plain"), F("OK"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
}

// Сохранить настройки эффектов
void handle_eff_save() {
  if (!ONflag) {
    HTTP.send(403, "text/plain", "Lamp is off");
    return;
  }

  if (!LittleFS.begin(false)) {
    HTTP.send(500, "text/plain", "FS mount failed");
    return;
  }

  File file = LittleFS.open("/effect.ini", "w");
  if (!file) {
    HTTP.send(500, "text/plain", "Cannot create file");
    return;
  }

  for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
    file.write(modes[i].Brightness);
    file.write(modes[i].Speed);
    file.write(modes[i].Scale);
    yield();
  }
  file.close();

  showWarning(CRGB::Blue, 2000U, 500U);
  HTTP.send(200, "text/plain", "OK");
}

// Загрузить настройки эффектов из файла
void handle_eff_read() {
  if (!LittleFS.begin(false)) {
    HTTP.send(500, "text/plain", "FS mount failed");
    return;
  }

  File file = LittleFS.open("/effect.ini", "r");
  if (!file) {
    HTTP.send(404, "text/plain", "File not found");
    return;
  }

  uint16_t file_size = file.size();
  if (file_size < 3 || file_size > MODE_AMOUNT * 3 || (file_size % 3) != 0) {
    file.close();
    HTTP.send(400, "text/plain", "Invalid file size");
    return;
  }

  uint16_t count = file_size / 3;
  if (count > MODE_AMOUNT) count = MODE_AMOUNT;

  for (uint16_t i = 0; i < count; i++) {
    uint8_t b = file.read();
    uint8_t s = file.read();
    uint8_t sc = file.read();

    if (b >= 1 && b <= 255) modes[i].Brightness = b;
    if (s >= 1 && s <= 255) modes[i].Speed = s;
    if (sc >= 1 && sc <= 100) modes[i].Scale = sc;

    yield();
  }
  file.close();

  SetBrightness(modes[currentMode].Brightness);
  loadingFlag = true;

  jsonWrite(configSetup, "br", modes[currentMode].Brightness);
  jsonWrite(configSetup, "sp", modes[currentMode].Speed);
  jsonWrite(configSetup, "sc", modes[currentMode].Scale);

  HTTP.send(200, "text/plain", "OK");

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// ------------------------------------------------- АВТОЯРКОСТЬ ЭФФЕКТОВ (в зависимости от времени суток) --------------------------------------------
void handle_auto_bri () {
  AutoBrightness = HTTP.arg("auto_bri").toInt();
  jsonWrite(configSetup, "auto_bri", AutoBrightness);

#if USE_DAWN
  if (ONflag && !dawnFlag) {
#else
  if (ONflag) {
#endif
    SetBrightness(modes[currentMode].Brightness);
  }
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

// ----------------------------------------------------------------- НАСТРОЙКИ ДИСПЛЕЯ ST7789 ---------------------------------------------------------
#if USE_ST7789
void handle_tft_clock_color() {
  int val = HTTP.arg("tft_clock_color").toInt();
  jsonWrite(configST7789, "tft_clock_color", val);
  saveConfig();
  tft_clock_color = val;
  loadingFlag = true;
  HTTP.send(200, F("text/plain"), F("OK"));
}

void handle_tft_date_color() {
  if (HTTP.hasArg("tft_date_color")) {
    uint8_t val = HTTP.arg("tft_date_color").toInt();
    if (val <= 20) {
      tft_date_color = val;
      jsonWrite(configST7789, "tft_date_color", tft_date_color);
      saveConfig();
      loadingFlag = true;
    }
  }
  HTTP.send(200, F("text/plain"), F("OK"));
}

void handle_tft_weather_color() {
  int val = HTTP.arg("tft_weather_color").toInt();
  jsonWrite(configST7789, "tft_weather_color", val);
  saveConfig();
  tft_weather_color = val;
  loadingFlag = true;
  HTTP.send(200, F("text/plain"), F("OK"));
}

void handle_tft_ticker_on() {
  bool val = (HTTP.arg("tft_ticker_on").toInt() == 1);
  jsonWrite(configST7789, "tft_ticker_on", val ? "1" : "0");
  saveConfig();
  tft_ticker_on = val;
  if (st7789Enabled) {
    tftTickerStop();
    tftTickerNextStart = millis() + tftTickerPeriodMs();
  }
  HTTP.send(200, F("text/plain"), F("OK"));
}

void handle_tft_ticker_color() {
  int val = HTTP.arg("tft_ticker_color").toInt();
  jsonWrite(configST7789, "tft_ticker_color", val);
  saveConfig();
  tft_ticker_color = val;
  if (st7789Enabled) {
    tftTickerStop();
    tftTickerNextStart = millis() + tftTickerPeriodMs();
  }
  HTTP.send(200, F("text/plain"), F("OK"));
}

void handle_tft_ticker_speed() {
  uint16_t val = constrain(HTTP.arg("tft_ticker_speed").toInt(), 50, 500);
  jsonWrite(configST7789, "tft_ticker_speed", val);
  saveConfig();
  tft_ticker_speed = val;
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

void handle_tft_ticker_period() {
  uint16_t val = constrain(HTTP.arg("tft_ticker_period").toInt(), 0, 3600);
  jsonWrite(configST7789, "tft_ticker_period", val);
  saveConfig();
  tft_ticker_period = val;
  if (st7789Enabled) {
    tftTickerStop();
    tftTickerNextStart = millis() + tftTickerPeriodMs();
  }
  HTTP.send(200, F("text/plain"), F("OK"));
}

void handle_tft_ticker_text() {
  String s = HTTP.arg("tft_ticker_text");
  if (s.length() > 120) s.remove(120);
  jsonWrite(configST7789, "tft_ticker_text", s);
  saveConfig();
  s.toCharArray(TFTTickerText, sizeof(TFTTickerText));
  if (st7789Enabled) {
    tftTickerStop();
    tftTickerNextStart = millis() + tftTickerPeriodMs();
  }
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

void handle_tft_brightness() {
  if (!HTTP.hasArg("tft_brightness")) {
    HTTP.send(400, "text/plain", "Missing parameter");
    return;
  }
  uint8_t brightness = constrain(HTTP.arg("tft_brightness").toInt(), 0, 255);
  if (brightness != tft_brightness) {
    tft_brightness = brightness;
    if (st7789Enabled) {
      TFT_SetBrightness(tft_brightness);
    }
    jsonWrite(configST7789, "tft_brightness", brightness);
    if (tft_auto_brightness) {
      tft_auto_brightness = false;
      jsonWrite(configST7789, "tft_auto_brightness", "0");
      if (st7789Enabled) {
        TFT_SetAutoBrightness(false);
      }
    }
    saveConfig();
  }
  HTTP.send(200, "text/plain", "OK");
}

void handle_tft_auto_brightness() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, "text/plain", "Missing value");
    return;
  }
  bool enable = (HTTP.arg("value") == "1");
  if (enable != tft_auto_brightness) {
    tft_auto_brightness = enable;
    jsonWrite(configST7789, "tft_auto_brightness", enable ? "1" : "0");
    saveConfig();
    if (st7789Enabled) {
      TFT_SetAutoBrightness(enable);
    }
  }
  HTTP.send(200, "text/plain", "OK");
}

#endif // USE_ST7789
// ---------------------------------------------------------------------------------
void handle_save_brightness() {
  uint8_t day_bright = constrain(HTTP.arg("day_bright").toInt(), 0, 255);
  uint8_t night_bright = constrain(HTTP.arg("night_bright").toInt(), 0, 255);

  jsonWrite(configSetup, "day_bright", day_bright);
  jsonWrite(configSetup, "night_bright", night_bright);
  saveConfig();

  DAY_HOURS_BRIGHTNESS = day_bright;
  NIGHT_HOURS_BRIGHTNESS = night_bright;

#if USE_ST7789
  jsonWrite(configST7789, "day_bright", day_bright);
  jsonWrite(configST7789, "night_bright", night_bright);
  writeFile(F("config_st7789.json"), configST7789);

  TFT_DAY_BRIGHTNESS = day_bright;
  TFT_NIGHT_BRIGHTNESS = night_bright;
  if (st7789Enabled) {
    if (!tft_auto_brightness) {
      TFT_SetBrightness(tft_brightness);
    } else {
      TFT_ApplyBrightnessNow();
    }
  }
#endif

  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  HTTP.send(200, F("text/plain"), F("OK"));
}

// --------------------------------------------------------------------- НАСТРОЙКИ МАТРИЦЫ ------------------------------------------------------------
void handle_matrix_tipe() {
  String configLED = readFile(F("config_led_matrix.json"), 8192);
  if (configLED == "Failed" || configLED == "Large") {
    configLED = "{}";
  }

  int newType = HTTP.arg("m_t").toInt();
  if (newType != 0 && newType != 1) {
    newType = 0;
  }

  String oldType = jsonRead(configLED, "m_t");
  if (oldType.toInt() != newType) {
    jsonWrite(configLED, "m_t", newType);
    writeFile(F("config_led_matrix.json"), configLED);
    MatrixType = newType;
    HTTP.send(200, "text/plain", "OK");
  } else {
    HTTP.send(200, "text/plain", "NO_CHANGE");
  }
}

void handle_matrix_orientation() {
  String configLED = readFile(F("config_led_matrix.json"), 8192);
  if (configLED == "Failed" || configLED == "Large") configLED = "{}";

  int newOri = HTTP.arg("m_o").toInt();
  if (newOri < 0 || newOri > 7) newOri = 0;

  String oldOri = jsonRead(configLED, "m_o");
  if (oldOri.toInt() != newOri) {
    jsonWrite(configLED, "m_o", newOri);
    writeFile(F("config_led_matrix.json"), configLED);
    MatrixOrientation = newOri;
    loadingFlag = true;
    needFullRedraw = true;
    HTTP.send(200, "text/plain", "OK");
  } else {
    HTTP.send(200, "text/plain", "NO_CHANGE");
  }
}

// ------------------------------------------------------------------------- НАСТРОЙКИ MQTT -----------------------------------------------------------
#if USE_MQTT
void handle_mqtt_set() {
  String mq_ip = HTTP.arg("mq_ip");
  uint16_t mq_port = HTTP.arg("mq_port").toInt();
  String mq_user = HTTP.arg("mq_user");
  String mq_pass = HTTP.arg("mq_pass");
  String mq_topic = HTTP.arg("topic");

  if (mq_port < 1 || mq_port > 65535) mq_port = 1883;

  if (!MqttServer.fromString(mq_ip)) {
    mq_ip.toCharArray(MqttHost, sizeof(MqttHost) - 1);
    MqttHost[sizeof(MqttHost) - 1] = '\0';
    mqttIPaddr = false;
  } else {
    mqttIPaddr = true;
  }

  mq_user.toCharArray(MqttUser, sizeof(MqttUser) - 1);
  mq_pass.toCharArray(MqttPassword, sizeof(MqttPassword) - 1);
  mq_topic.toCharArray(TopicBase, sizeof(TopicBase) - 1);

  MqttUser[sizeof(MqttUser) - 1] = '\0';
  MqttPassword[sizeof(MqttPassword) - 1] = '\0';
  TopicBase[sizeof(TopicBase) - 1] = '\0';

  jsonWrite(configMQTT, "mq_ip", mq_ip);
  jsonWrite(configMQTT, "mq_port", mq_port);
  jsonWrite(configMQTT, "mq_user", mq_user);
  jsonWrite(configMQTT, "mq_pass", mq_pass);
  jsonWrite(configMQTT, "topic", mq_topic);
  jsonWrite(configMQTT, "TopicS", String(Mqtt::instance().getClientId()) + '/' + String(TopicCmnd));
  jsonWrite(configMQTT, "TopicP", String(Mqtt::instance().getClientId()) + '/' + String(TopicSnd));

  writeFile(F("config_mqtt.json"), configMQTT);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

void handle_mqtt_on() {
  MqttOn = HTTP.arg("mq_on").toInt();
  jsonWrite(configMQTT, "mq_on", MqttOn);
  writeFile(F("config_mqtt.json"), configMQTT);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

void handle_mqtt_period() {
  MqttPeriod = HTTP.arg("mq_prd").toInt();
  if (MqttPeriod < 10) MqttPeriod = 10;
  if (MqttPeriod > 3600) MqttPeriod = 3600;

  jsonWrite(configMQTT, "mq_prd", MqttPeriod);
  writeFile(F("config_mqtt.json"), configMQTT);
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

#endif // USE_MQTT

// ------------------------------------------------------------------- СПИСОК ЭФФЕКТОВ ЛАМПЫ ----------------------------------------------------------
void EffectList(const __FlashStringHelper * filename) {
  String effList = String(filename);
  effList += jsonRead(configSetup, "lang");
  effList += F(".ini");

  File file = LittleFS.open(effList, "r");
  if (!file) {
    return;
  }

  String content = file.readString();
  file.close();

  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.print(content);
  Udp.endPacket();
}

// ---------------------------------------------------------------------- НАСТРОЙКИ SD КАРТЫ ----------------------------------------------------------
// Эффекты с SD карты
#if USE_SD && !FS_AS_SD
void handle_out_file() {
  String outFileName = HTTP.arg("out_file");
  if (outFileName.length() == 0 || !outFileName.endsWith(".out")) {
    HTTP.send(400, F("text/plain"), F("Invalid or empty file name"));
    return;
  }

  jsonWrite(configSetup, "out_file", outFileName);

  if (currentMode != EFF_OUT_EFFECT) {
    currentMode = EFF_OUT_EFFECT;
    jsonWrite(configSetup, "eff_sel", EFF_OUT_EFFECT);
  }

  loadingFlag = true;
  saveConfig();
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": true}"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Список файлов
void handle_list_out_files() {
  String fileList = "[";
  File root = SD.open("/");
  if (!root || !root.isDirectory()) {
    if (root) root.close();
    HTTP.send(404, F("text/plain"), F("Failed to open directory"));
    return;
  }

  bool first = true;
  File file = root.openNextFile();
  while (file) {
    String name = file.name();

    if (name.endsWith(".out")) {
      if (!first) fileList += ",";
      fileList += "\"" + name + "\"";
      first = false;
    }

    File temp = file;
    file = root.openNextFile();
    temp.close();
  }

  root.close();
  fileList += "]";

  HTTP.send(200, F("application/json"), fileList);
}

#else

// Заглушки, когда SD отключена
void handle_out_file() {
  HTTP.send(404, F("text/plain"), F("SD card not enabled"));
}

void handle_list_out_files() {
  HTTP.send(404, F("text/plain"), F("SD card not enabled"));
}

#endif // USE_SD && !FS_AS_SD

// --------------------------------------------------------- СТАТУСЫ УСТРОЙСТВ В МОДАЛЬНОМ ОКНЕ МЕНЮ --------------------------------------------------
#if STATUS_DEVICE

// статус кнопки
void handle_button_status() {
  DynamicJsonDocument doc(256);

#if USE_BUTTON
  if (buttonEnabled) {
    doc["button_status"] = button_type ? "СЕНСОРНАЯ" : "МЕХАНИЧЕСКАЯ";
  } else {
    doc["button_status"] = "ВЫКЛЮЧЕНА";
  }
  doc["enabled"] = buttonEnabled;
#else
  doc["button_status"] = "ОТКЛЮЧЕНО В ПРОШИВКЕ";
  doc["enabled"] = false;
#endif

  String response;
  serializeJson(doc, response);
  HTTP.send(200, "application/json; charset=utf-8", response);
}

// статус IR, RF
void handle_ir_rf_status() {
  DynamicJsonDocument doc(512);
  String ir_status, rf_status;

#if USE_IR_RECEIVER
  if (irEnabled) {
    ir_status = (millis() - lastIRtime < 5000) ? "OK" : "ОЖИДАЕТ КОМАНДУ";
    String currentRemote = irManager.getCurrentRemoteName();
    if (currentRemote.length() > 0) {
      doc["ir_remote"] = currentRemote;
      doc["ir_remote_id"] = irManager.getCurrentRemoteId();
    } else {
      doc["ir_remote"] = "Не выбран";
      doc["ir_remote_id"] = "";
    }
  } else {
    ir_status = "ВЫКЛЮЧЕН";
    doc["ir_remote"] = "—";
    doc["ir_remote_id"] = "";
  }
#else
  ir_status = "ОТКЛЮЧЕНО В ПРОШИВКЕ";
  doc["ir_remote"] = "—";
  doc["ir_remote_id"] = "";
#endif

#if USE_RF_RECEIVER
  if (rfEnabled) {
    rf_status = rfReceiver.available() ? "OK" : "ОЖИДАЕТ КОМАНДУ";
    if (rf_status == "OK") rfReceiver.resetAvailable();
  } else {
    rf_status = "ВЫКЛЮЧЕН";
  }
#else
  rf_status = "ОТКЛЮЧЕНО В ПРОШИВКЕ";
#endif

  doc["ir"] = ir_status;
  doc["rf"] = rf_status;
  doc["ir_enabled"] = irEnabled;
  doc["rf_enabled"] = rfEnabled;
  String response;
  serializeJson(doc, response);
  HTTP.send(200, "application/json; charset=utf-8", response);
}

// статус дисплея tm1637
void handle_tm1637_status() {
  DynamicJsonDocument doc(256);
  String status;
  bool connected = false;

#if USE_TM1637
  connected = true;
  status = tm1637Enabled ? "ВКЛЮЧЕН" : "ВЫКЛЮЧЕН";
#else
  status = "ОТКЛЮЧЕНО В ПРОШИВКЕ";
#endif

  doc["connected"] = connected;
  doc["status"] = status;
  doc["enabled"] = tm1637Enabled;
  String response;
  serializeJson(doc, response);
  HTTP.send(200, "application/json; charset=utf-8", response);
}

// статус дисплея st7789
void handle_st7789_status() {
  DynamicJsonDocument doc(256);
  String status;
  bool connected = false;

#if USE_ST7789
  connected = true;
  status = st7789Enabled ? "ВКЛЮЧЕН" : "ВЫКЛЮЧЕН";
#else
  status = "ОТКЛЮЧЕНО В ПРОШИВКЕ";
#endif

  doc["connected"] = connected;
  doc["status"] = status;
  doc["enabled"] = st7789Enabled;
  String response;
  serializeJson(doc, response);
  HTTP.send(200, "application/json; charset=utf-8", response);
}

// статус мп3 плеера
void handle_mp3_status() {
  DynamicJsonDocument doc(512);
  String status;
  bool card_ok = false;

#if USE_MP3_PLAYER
  card_ok = (mp3_player_connect == 4) ? mp3_card_ok : false;

  if (!mp3Enabled) {
    status = "ВЫКЛЮЧЕН";
  } else if (mp3_player_connect == 0) {
    status = "НЕТ СВЯЗИ";
  } else if (mp3_player_connect != 4) {
    status = "ИНИЦИАЛИЗАЦИЯ...";
  } else if (dfPlayerIsOriginal && !mp3_card_ok) {
    status = "КАРТА НЕИСПРАВНА";
  } else if (eff_sound_on && !mp3_stop && !pause_on) {
    status = "ИГРАЕТ";
  } else if (eff_sound_on && pause_on) {
    status = "ПАУЗА";
  } else if (eff_sound_on && mp3_stop) {
    status = "ВЫКЛЮЧЕН";
  } else {
    status = "ГОТОВ";
  }

  doc["connect"] = mp3_player_connect;
  doc["folder"] = CurrentFolder;
  doc["volume"] = eff_volume;
  doc["sound_on"] = eff_sound_on;
  doc["stop"] = mp3_stop;
  doc["pause"] = pause_on;
  doc["enabled"] = mp3Enabled;
#else
  status = "ОТКЛЮЧЕНО В ПРОШИВКЕ";
  card_ok = false;
  doc["connect"] = 0;
  doc["folder"] = 0;
  doc["volume"] = 0;
  doc["sound_on"] = 0;
  doc["stop"] = 1;
  doc["pause"] = 1;
  doc["enabled"] = false;
#endif

  doc["mp3_status"] = status;
  doc["card_ok"] = card_ok;

  String response;
  serializeJson(doc, response);
  HTTP.send(200, "application/json; charset=utf-8", response);
}

// статус погоды
void handle_show_weather() {
#if USE_WEATHER
  int enabled = HTTP.arg("show_weather").toInt();
  inClockWeatherMode = (enabled == 1);
  jsonWrite(configWeather, "show_weather", enabled);
  saveConfig();
  if (inClockWeatherMode && Wifi::instance().isConnected()) {
    Weather::instance().forceUpdate();
  }
  HTTP.send(200, "text/plain", "OK");
#endif
}

// статус OTA
uint8_t otaProgress = 0;
String  otaStatus = "ГОТОВ";
void handle_ota_status() {
  DynamicJsonDocument doc(256);
  String status = "ГОТОВ";

#if USE_OTA
  if (Update.isRunning()) {
    status = (otaProgress == 0) ? "ЗАГРУЗКА..." : "ЗАПИСЬ... " + String(otaProgress) + "%";
  }
#else
  status = "ОТКЛЮЧЕНО В ПРОШИВКЕ";
#endif

  doc["status"] = status;
  doc["progress"] = otaProgress;
  doc["enabled"] = !!USE_OTA;

  String resp;
  serializeJson(doc, resp);
  HTTP.send(200, "application/json; charset=utf-8", resp);
}

// статус MQTT
void handle_mqtt_status() {
  DynamicJsonDocument doc(256);
  String status;
  bool connected = false;

#if USE_MQTT
  connected = Mqtt::instance().isConnected();
  status = connected ? "ПОДКЛЮЧЕНО" : "НЕ ПОДКЛЮЧЕНО";
#else
  status = "ОТКЛЮЧЕНО В ПРОШИВКЕ";
#endif

  doc["status"] = status;
  doc["enabled"] = !!USE_MQTT;
  doc["connected"] = connected;

  String resp;
  serializeJson(doc, resp);
  HTTP.send(200, "application/json; charset=utf-8", resp);
}

// статус режим "Мультилампа"
void handle_multilamp_status() {
  DynamicJsonDocument doc(512);

#if USE_MULTILAMP
  doc["enabled"] = true;
  doc["lamp1"] = ml1;
  doc["lamp2"] = ml2;
  doc["lamp3"] = ml3;
  doc["lamp4"] = ml4;
  doc["lamp5"] = ml5;
  doc["host1"] = String(Host1);
  doc["host2"] = String(Host2);
  doc["host3"] = String(Host3);
  doc["host4"] = String(Host4);
  doc["host5"] = String(Host5);
#if USE_MP3_PLAYER
  doc["send_sound"] = send_sound;
  doc["send_volume"] = send_eff_volume;
#endif
#else
  doc["enabled"] = false;
  doc["status"] = "ОТКЛЮЧЕНО В ПРОШИВКЕ";
#endif

  String resp;
  serializeJson(doc, resp);
  HTTP.send(200, "application/json; charset=utf-8", resp);
}

// статус SD
void handle_sd_status() {
  DynamicJsonDocument doc(512);
  String sd_status;

#if USE_SD
#if FS_AS_SD
  sd_status = "ЭМУЛЯЦИЯ В LITTLEFS";
#else
  sd_status = sd_card_present ? "ОК" : "КАРТА НЕ НАЙДЕНА";
#endif
  doc["fs_status"] = LittleFS.begin() ? "ОК" : "ОШИБКА";
#else
  sd_status = "ОТКЛЮЧЕНО В ПРОШИВКЕ";
  doc["fs_status"] = "—";
#endif

  doc["sd_status"] = sd_status;
  String response;
  serializeJson(doc, response);
  HTTP.send(200, "application/json; charset=utf-8", response);
}

#endif // STATUS_DEVICE

// -------------------------------------------------------------------- НАСТРОЙКИ БЕГУЩЕЙ СТРОКИ ------------------------------------------------------
#if LED_PANEL
void ApplyRunningTextSettings() {
  if (!runTextEnabled) {
    textIsRunning = false;
    drawStringThisTick = false;
    runningTextTimer.setInterval(999999999UL);
    runningTextTimer.reset();
    Fill_String = false;
    currentRunningTextIntervalMs = 0;
    return;
  }

  if (IntervalrunText == 0) {
    // непрерывный режим
    textIsRunning = true;
    loadingFlag = true;
    offset = matrixWidth + 8;
    runningTextTimer.setInterval(999999999UL);
    runningTextTimer.reset();
    currentRunningTextIntervalMs = 0;
  } else {
    // периодический режим
    textIsRunning = false;
    uint32_t intervalMs = (uint32_t)IntervalrunText * 60000UL;
    if (currentRunningTextIntervalMs != intervalMs) {
      runningTextTimer.setInterval(intervalMs);
      runningTextTimer.reset();
      currentRunningTextIntervalMs = intervalMs;
    }
  }
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Включить бегущую строку"
void handle_run_text_enabled() {
  String arg = HTTP.arg("value");
  bool newVal = (arg == "1" || arg.toInt() == 1);
  if (newVal == runTextEnabled) return;

  runTextEnabled = newVal;

  if (!runTextEnabled) {
    // полное выключение бегущей строки
    runTextOver = false;// снять галочку и с "поверх эффекта"
    textIsRunning = false;
    drawStringThisTick = false;
    Fill_String = false;
    offset = matrixWidth + 10;
    runningTextTimer.setInterval(TIMER_DISABLED);
    runningTextTimer.reset();

    jsonWrite(configLedPanel, "run_text_over", "0");
  }

  jsonWrite(configLedPanel, "run_text_enabled", newVal ? "1" : "0");
  saveConfig();

  if (runTextEnabled) {
    ApplyRunningTextSettings();
  } else {
    loadingFlag = true;
    needFullRedraw = true;
  }

  HTTP.send(200, "text/plain", "OK");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Бегущая строка поверх эффекта"
void handle_run_text_over() {
  String arg = HTTP.arg("value");
  bool newVal = (arg == "1" || arg == "true" || arg.toInt() == 1);
  if (newVal == runTextOver) return;

  runTextOver = newVal;
  jsonWrite(configLedPanel, "run_text_over", newVal ? "1" : "0");

  if (runTextOver) {
    if (!runTextEnabled) {
      runTextEnabled = true;
      jsonWrite(configLedPanel, "run_text_enabled", "1");
    }
    textIsRunning = true;
    drawStringThisTick = true;
    offset = matrixWidth + 10;
  } else {
    drawStringThisTick = false;
    if (!runTextEnabled) {
      textIsRunning = false;
    }
  }

  saveConfig();
  loadRunningTextSettings();

  loadingFlag = true;
  needFullRedraw = true;

  HTTP.send(200, "text/plain", "OK");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Текст бегущей строки
void handle_run_text ()  {
  jsonWrite(configLedPanel, "run_text", HTTP.arg("run_text"));
  timeout_save_file_changes = millis();
  bitSet (save_file_changes, 0);
  (jsonRead(configLedPanel, "run_text")).toCharArray (TextTicker, (jsonRead(configLedPanel, "run_text")).length() + 1);
  HTTP.send(200, F("text/plain"), F("OK"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP       

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Интервал вывода бегущей строки
void handle_interval_run_text() {
  if (!HTTP.hasArg("interval_run_text")) {
    HTTP.send(400, F("text/plain"), F("Missing parameter: interval_run_text"));
    return;
  }
  String arg = HTTP.arg("interval_run_text");
  int newInterval = arg.toInt();
  newInterval = constrain(newInterval, 0, 60);

  if (newInterval == IntervalrunText) {
    HTTP.send(200, F("text/plain"), F("OK"));
    return;
  }

  IntervalrunText = newInterval;
  jsonWrite(configLedInterval, "interval_run_text", IntervalrunText);
  saveConfig();
  ApplyRunningTextSettings();
  HTTP.send(200, F("text/plain"), F("OK"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Смещение текста по Y
void handle_text_y_offset() {
  if (!HTTP.hasArg("text_y_offset")) {
    HTTP.send(400, "text/plain", "Parameter missing");
    return;
  }

  int value = HTTP.arg("text_y_offset").toInt();
  int maxShift = matrixHeight * 2;
  value = constrain(value, -maxShift, maxShift);

  if (textYOffset != value) {
    textYOffset = value;
    jsonWrite(configLedPanel, "text_y_offset", value);
    saveConfig();
    loadRunningTextSettings();
    loadingFlag = true;
    offset = matrixWidth + 10;
    textIsRunning = true;
    Fill_String = false;
  }

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Скорость бегущей строки
void handle_spt() {
  if (!HTTP.hasArg("spt")) {
    HTTP.send(400);
    return;
  }
  int newSpeed = constrain(HTTP.arg("spt").toInt(), 20, 220);
  if (newSpeed == SpeedRunningText) {
    HTTP.send(200, "application/json", "{\"should_refresh\": true}");
    return;
  }
  SpeedRunningText = newSpeed;
  jsonWrite(configLedPanel, "spt", SpeedRunningText);
  saveConfig();

  if (runTextEnabled) {
    textIsRunning = false;
    drawStringThisTick = false;
    Fill_String = false;
    offset = matrixWidth + 10;
    loadingFlag = true;
    if (IntervalrunText == 0) {
      textIsRunning = true;
      drawStringThisTick = runTextOver;
    } else {
      runningTextTimer.reset();
    }
  }
  HTTP.send(200, "application/json", "{\"should_refresh\": true}");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Оттенок текста бегущей строки
void handle_sct() {
  if (!HTTP.hasArg("sct")) {
    HTTP.send(400);
    return;
  }
  int value = constrain(HTTP.arg("sct").toInt(), 0, 255);

  rainbowText = false;
  jsonWrite(configLedPanel, "rainbow_text", "0");

  if (value >= 253) { // если ползунок сдвинуть на 255, то включится фэйд-режим (смена цвета)
    runTextColorCycle = true;
    autoRunTextHue = false;
    runTextHue = 0;
    ColorRunningText = 0;
    textIsRunning = true;
    offset = matrixWidth + 10;
    loadingFlag = true;
    jsonWrite(configLedPanel, "run_text_cycle", "1");
    jsonWrite(configLedPanel, "run_text_hue", "0");
  } else {
    runTextColorCycle = false;
    autoRunTextHue = false;
    runTextHue = (uint8_t)value;
    ColorRunningText = runTextHue;
    jsonWrite(configLedPanel, "run_text_cycle", "0");
    jsonWrite(configLedPanel, "run_text_hue", String(runTextHue));
  }

  int sctForDisplay = runTextColorCycle ? 0 : runTextHue;
  jsonWrite(configLedPanel, "sct", String(sctForDisplay));
  saveConfig();
  loadingFlag = true;
  scrollTimer = millis() - 1000;

  HTTP.send(200, "application/json", "{\"should_refresh\": true}");
}

// Шрифт бегущей строки
void handle_font_size() {
  if (!HTTP.hasArg("font_size")) {
    HTTP.send(400, "text/plain", "Missing font_size parameter");
    return;
  }

  String arg = HTTP.arg("font_size");
  if (arg.length() == 0) {
    HTTP.send(400, "text/plain", "Empty font_size value");
    return;
  }

  int val = arg.toInt();
  if (val < 0 || val > 2) {
    HTTP.send(400, "text/plain", "Invalid font_size value (must be 0–2)");
    return;
  }

  uint8_t newSize = (uint8_t)val;

  if (newSize == currentFont) {
    HTTP.send(200, "text/plain", "OK (no change)");
    return;
  }

  jsonWrite(configLedPanel, "font_size", newSize);
  saveConfig();

  setFontSize(newSize);

  const char* names[] = {"5×8", "8×13", "10×16"};

  HTTP.send(200, "text/plain", "OK");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}
#endif // LED_PANEL

// --------------------------------------------------------------------------- НАСТРОЙКИ ЧАСОВ --------------------------------------------------------
// Чекбокс "Авто-сдвиг часов"
void handle_auto_move_clock() {
  String arg = HTTP.arg("value");
  bool newVal = (arg == "1" || arg == "true" || arg.toInt() == 1);

  if (autoMoveClockEnabled != newVal) {
    autoMoveClockEnabled = newVal;
    jsonWrite(configLedPanel, "auto_move_clock", newVal ? "1" : "0");
    timeout_save_file_changes = millis();
    bitSet(save_file_changes, 0);

    if (newVal) {
      clockAutoOffsetY = 0;
      loadingFlag = true;
    }
  }

  HTTP.send(200, "text/plain", "OK");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Интервал вывода часов
void handle_print_time() {
  int val = HTTP.arg("print_time").toInt();
  if (val < 1 || val > 60) val = 30;
  PRINT_TIME = HTTP.arg("print_time").toInt();
  jsonWrite(configLedInterval, "print_time", PRINT_TIME);
  saveConfig();
  HTTP.send(200, "text/plain", "OK");
}

// Чекбокс "Часы бегущей строкой"
void handle_run_time_text_enabled() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, "text/plain", "Missing value parameter");
    return;
  }

  String valStr = HTTP.arg("value");
  valStr.toLowerCase();
  bool newVal = (valStr == "1" || valStr == "true" || valStr == "on");
  runTimeTextEnabled = newVal;

  if (runTimeTextEnabled) {
    timeTextJustEnabled = true;
  }

  jsonWrite(configLedPanel, "run_time_text_enabled", runTimeTextEnabled ? "1" : "0");
  saveConfig();
  HTTP.send(200, "text/plain", runTimeTextEnabled ? "1" : "0");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Ведущий ноль"
void handle_clock_leading_zero() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, "text/plain", "Missing value parameter");
    return;
  }
  String valStr = HTTP.arg("value");
  bool newVal = (valStr == "1" || valStr == "true" || valStr == "on");

  if (clockLeadingZero == newVal) {
    HTTP.send(200, "text/plain", clockLeadingZero ? "1" : "0");
    return;
  }

  clockLeadingZero = newVal;
  jsonWrite(configLedPanel, "clock_leading_zero", clockLeadingZero ? "1" : "0");
  saveConfig();
  loadingFlag = true;

  HTTP.send(200, "text/plain", clockLeadingZero ? "1" : "0");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Вертикальные часы"
void handle_clock_vert() {
  String arg = HTTP.arg("value");
  bool newVal = (arg == "1" || arg == "true" || arg.toInt() == 1);

  if (clockIsVertical != newVal) {
    clockIsVertical = newVal;
    jsonWrite(configLedPanel, "clock_vert", newVal ? "1" : "0");
    timeout_save_file_changes = millis();
    bitSet(save_file_changes, 0);
    loadingFlag = true;
    clockAutoOffsetY = 0;
    lastMinute = 255;
  }

  HTTP.send(200, "text/plain", "OK");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP       

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Включить ночные часы"
void handle_night_clock_enabled() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400);
    return;
  }
  uint8_t val = HTTP.arg("value").toInt();
  bool newVal;

  if (val == 2) {
    newVal = !nightClockEnabled;
  } else {
    newVal = (val == 1);
  }

  if (newVal == nightClockEnabled) {
    HTTP.send(200, "text/plain", nightClockEnabled ? "1" : "0");
    return;
  }

  nightClockEnabled = newVal;
  jsonWrite(configLedPanel, "night_clock_enabled", nightClockEnabled ? "1" : "0");
  saveConfig();

  if (nightClockEnabled) {
    ONflag = true;
    jsonWrite(configSetup, "Power", 1);
    currentMode = eff_num_correct[EFF_CLOCK];
    loadingFlag = true;

    uint8_t ui_index = 0;
    for (ui_index = 0; ui_index < MODE_AMOUNT; ui_index++) {
      if (eff_num_correct[ui_index] == currentMode) break;
    }
    jsonWrite(configSetup, "eff_sel", ui_index);

    nightModeBrightness = nightClockBrightness;
    modes[currentMode].Brightness = nightModeBrightness;
    jsonWrite(configSetup, "brightness", nightModeBrightness);
    FastLED.setBrightness(nightModeBrightness);

    effectsTick();
    FastLED.show();
    changePower();
    sendCurrent(udpBuffer);
  } else {
    nightModeBrightness = 0;
    uint8_t normalBrightness = userClockBrightness;
    if (normalBrightness == 0) normalBrightness = 30;
    modes[currentMode].Brightness = normalBrightness;
    jsonWrite(configSetup, "brightness", normalBrightness);
    FastLED.setBrightness(normalBrightness);
    FastLED.show();
    jsonWrite(configSetup, "br", normalBrightness);
  }

  HTTP.send(200, "text/plain", nightClockEnabled ? "1" : "0");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Выбор яркости ночных часов
void handle_night_clock_brightness() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400);
    return;
  }
  uint8_t newVal = HTTP.arg("value").toInt();
  if (newVal < 1 || newVal > 255) {
    newVal = 1;
  }
  if (newVal == nightClockBrightness) {
    HTTP.send(200, "text/plain", String(nightClockBrightness));
    return;
  }
  nightClockBrightness = newVal;
  jsonWrite(configLedPanel, "night_clock_brightness", nightClockBrightness);
  saveConfig();

  if (nightClockEnabled && currentMode == eff_num_correct[EFF_CLOCK]) {
    nightModeBrightness = nightClockBrightness;
    modes[currentMode].Brightness = nightModeBrightness;
    jsonWrite(configSetup, "brightness", nightModeBrightness);
    FastLED.setBrightness(nightModeBrightness);
    FastLED.show();
  }

  HTTP.send(200, "text/plain", String(nightClockBrightness));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

#if LED_PANEL
// Смещение часов по X
void handle_clock_x_offset() {
  if (!HTTP.hasArg("clock_x_offset")) {
    HTTP.send(400, "text/plain", "Parameter missing");
    return;
  }

  int value = HTTP.arg("clock_x_offset").toInt();
  value = constrain(value, -50, 50);

  if (clockXOffset != value) {
    clockXOffset = value;
    jsonWrite(configLedPanel, "clock_x_offset", value);
    saveConfig();
    loadingFlag = true;
    clockNeedRedraw = true;
    needFullRedraw = true;
  }

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Смещение часов по Y
void handle_clock_y_offset() {
  if (!HTTP.hasArg("clock_y_offset")) {
    HTTP.send(400, "text/plain", "Parameter missing");
    return;
  }

  int value = HTTP.arg("clock_y_offset").toInt();
  value = constrain(value, -20, 20);

  if (clockYOffset != value) {
    clockYOffset = value;
    jsonWrite(configLedPanel, "clock_y_offset", value);
    saveConfig();
    loadingFlag = true;
    clockNeedRedraw = true;
    needFullRedraw = true;
  }

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP       

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Оттенок часов
void handle_clock_hue() {
  if (!HTTP.hasArg("clock_hue")) {
    HTTP.send(400);
    return;
  }

  int value = constrain(HTTP.arg("clock_hue").toInt(), 0, 255);

  rainbowClock = false;
  jsonWrite(configLedPanel, "rainbow_clock", "0");

  if (value >= 253) { // если ползунок сдвинуть на 255, то включится фэйд-режим (смена цвета)
    clockColorCycle = true;
    clockHue = 0; // стартовый оттенок (красный)
    jsonWrite(configLedPanel, "clock_cycle", "1");
  } else {
    clockColorCycle = false;
    clockHue = (uint8_t)value;
    jsonWrite(configLedPanel, "clock_cycle", "0");
  }

  jsonWrite(configLedPanel, "clock_hue", String(clockHue));
  saveConfig();
  loadingFlag = true;

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}
#endif // LED_PANEL

// ------------------------------------------------------------------------- НАСТРОЙКИ ПОГОДЫ ---------------------------------------------------------
#if USE_WEATHER
// Интервал вывода погоды
void handle_print_weather() {
  int val = HTTP.arg("print_weather").toInt();
  if (val < 1 || val > 60) val = 30;
  PRINT_WEATHER = HTTP.arg("print_weather").toInt();
  jsonWrite(configLedInterval, "print_weather", PRINT_WEATHER);
  saveConfig();
  HTTP.send(200, "text/plain", "OK");
}

// Чекбокс "Погода бегущей строкой"
void handle_run_weather_text_enabled() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, "text/plain", "Missing value parameter");
    return;
  }

  String valStr = HTTP.arg("value");
  valStr.toLowerCase();
  bool newVal = (valStr == "1" || valStr == "true" || valStr == "on");
  runWeatherTextEnabled = newVal;
  jsonWrite(configLedPanel, "run_weather_text_enabled", runWeatherTextEnabled ? "1" : "0");
  saveConfig();

  HTTP.send(200, "text/plain", runWeatherTextEnabled ? "1" : "0");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}
#endif // USE_WEATHER

#if LED_PANEL
#if USE_WEATHER
// Чекбокс "Включить погоду для отображеня на матрице"
void handle_weather_enabled() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, F("text/plain"), F("Parameter missing"));
    return;
  }
  bool newVal = HTTP.arg("value") == "1";

  if (newVal == weatherEnabled) {
    HTTP.send(200, "text/plain", "OK");
    return;
  }

  bool wasEnabled = weatherEnabled;
  weatherEnabled = newVal;
  jsonWrite(configLedPanel, "weather_enabled", weatherEnabled ? "1" : "0");
  saveConfig();
  loadingFlag = true;
  if (!weatherEnabled && wasEnabled) {
    clearLastWeatherZone();
    lastWeatherLeft = lastWeatherRight = -1;
  }

  weatherNeedRedraw = true;
  needFullRedraw = true;

  HTTP.send(200, "text/plain", "OK");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Оттенок погоды
void handle_weather_hue() {
  if (!HTTP.hasArg("weather_hue")) {
    HTTP.send(400);
    return;
  }

  int value = constrain(HTTP.arg("weather_hue").toInt(), 0, 255);

  rainbowWeather = false;
  jsonWrite(configLedPanel, "rainbow_weather", "0");

  if (value >= 253) { // если ползунок сдвинуть на 255, то включится фэйд-режим (смена цвета)
    weatherColorCycle = true;
    weatherHue = 0; // стартовый оттенок (красный)
    jsonWrite(configLedPanel, "weather_cycle", "1");
  }
  else {
    weatherColorCycle = false;
    weatherHue = (uint8_t)value;
    jsonWrite(configLedPanel, "weather_cycle", "0");
  }

  jsonWrite(configLedPanel, "weather_hue", String(weatherHue));
  saveConfig();
  loadingFlag = true;
  weatherNeedRedraw = true;

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Смещение погоды по X
void handle_weather_x_offset() {
  if (!HTTP.hasArg("weather_x_offset")) {
    HTTP.send(400, "text/plain", "Parameter missing");
    return;
  }

  int value = HTTP.arg("weather_x_offset").toInt();

  int maxOffset = matrixWidth - 20;
  int minOffset = -matrixWidth + 20;
  value = constrain(value, minOffset, maxOffset);

  if (weatherXOffset != value) {
    weatherXOffset = value;
    jsonWrite(configLedPanel, "weather_x_offset", String(value));
    saveConfig();

    loadingFlag = true;
    weatherNeedRedraw = true;
    needFullRedraw = true;
  }

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Смещение погоды по Y
void handle_weather_y_offset() {
  if (!HTTP.hasArg("weather_y_offset")) {
    HTTP.send(400, "text/plain", "Parameter missing");
    return;
  }

  int value = HTTP.arg("weather_y_offset").toInt();

  value = constrain(value, -16, 16);

  if (weatherYOffset != value) {
    weatherYOffset = value;
    jsonWrite(configLedPanel, "weather_y_offset", String(value));
    saveConfig();

    loadingFlag = true;
    weatherNeedRedraw = true;
  }

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

// Чекбокс "Мигание °"
void handle_degree_blink() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400);
    return;
  }
  bool newVal = HTTP.arg("value") == "1";
  if (newVal == degreeSymbolBlinking) {
    HTTP.send(200, "text/plain", degreeSymbolBlinking ? "1" : "0");
    return;
  }
  degreeSymbolBlinking = newVal;
  jsonWrite(configLedPanel, "degree_blink", degreeSymbolBlinking ? "1" : "0");
  saveConfig();
  loadingFlag = true;

  HTTP.send(200, "text/plain", degreeSymbolBlinking ? "1" : "0");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}
#endif // USE_WEATHER

// ------------------------------------------------------------------------ НАСТРОЙКИ ДАТЫ ------------------------------------------------------------
// Чекбокс "Включить дату для отображеня на матрице"
void handle_date_enabled() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, F("text/plain"), F("Parameter missing"));
    return;
  }
  bool newVal = HTTP.arg("value") == "1";
  if (newVal == dateEnabled) {
    HTTP.send(200, "text/plain", "OK");
    return;
  }

  bool wasEnabled = dateEnabled;
  dateEnabled = newVal;
  jsonWrite(configLedPanel, "date_enabled", dateEnabled ? "1" : "0");
  saveConfig();

  if (!dateEnabled && wasEnabled) {
    clearLastDateZone();
    lastDateLeft = lastDateRight = -1;
    dateNeedRedraw = true;
  }

  loadingFlag = true;
  HTTP.send(200, "text/plain", "OK");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Оттенок даты
void handle_date_hue() {
  if (!HTTP.hasArg("date_hue")) {
    HTTP.send(400);
    return;
  }

  int value = constrain(HTTP.arg("date_hue").toInt(), 0, 255);

  rainbowDate = false;
  jsonWrite(configLedPanel, "rainbow_date", "0");

  if (value >= 253) { // если ползунок сдвинуть на 255, то включится фэйд-режим (смена цвета)
    dateColorCycle = true;
    dateHue = 0;
    jsonWrite(configLedPanel, "date_cycle", "1");
  } else {
    dateColorCycle = false;
    dateHue = (uint8_t)value;
    jsonWrite(configLedPanel, "date_cycle", "0");
  }

  jsonWrite(configLedPanel, "date_hue", String(dateHue));
  saveConfig();
  loadingFlag = true;
  dateNeedRedraw = true;

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Смещение даты по X
void handle_date_x_offset() {
  if (!HTTP.hasArg("date_x_offset")) {
    HTTP.send(400, "text/plain", "Parameter missing");
    return;
  }

  int value = HTTP.arg("date_x_offset").toInt();

  int maxOffset = matrixWidth - 10;
  int minOffset = -matrixWidth + 10;
  value = constrain(value, minOffset, maxOffset);

  if (dateXOffset != value) {
    dateXOffset = value;
    jsonWrite(configLedPanel, "date_x_offset", String(value));
    saveConfig();
    loadingFlag = true;
    dateNeedRedraw = true;
    needFullRedraw = true;
  }

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Смещение даты по Y
void handle_date_y_offset() {
  if (!HTTP.hasArg("date_y_offset")) {
    HTTP.send(400, "text/plain", "Parameter missing");
    return;
  }

  int value = HTTP.arg("date_y_offset").toInt();

  value = constrain(value, -16, 16);

  if (dateYOffset != value) {
    dateYOffset = value;
    jsonWrite(configLedPanel, "date_y_offset", String(value));
    saveConfig();

    loadingFlag = true;
#if USE_WEATHER
    weatherNeedRedraw = true;
#endif
  }

  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

// Чекбокс "Показывать 4 цифры года"
void handle_date_full_year() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, "text/plain", "Missing value parameter");
    return;
  }

  String valStr = HTTP.arg("value");
  bool newVal = (valStr == "1" || valStr == "true" || valStr == "on");

  if (showFullYearEnabled == newVal) {
    HTTP.send(200, "text/plain", showFullYearEnabled ? "1" : "0");
    return;
  }

  showFullYearEnabled = newVal;
  jsonWrite(configLedPanel, "date_full_year", showFullYearEnabled ? "1" : "0");
  saveConfig();

  loadingFlag = true;

  HTTP.send(200, "text/plain", showFullYearEnabled ? "1" : "0");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Мигание разделительных точек даты"
void handle_date_separator_blink() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400);
    return;
  }
  bool newVal = HTTP.arg("value") == "1";
  if (newVal == dateSeparatorBlinking) {
    HTTP.send(200, "text/plain", dateSeparatorBlinking ? "1" : "0");
    return;
  }
  dateSeparatorBlinking = newVal;
  jsonWrite(configLedPanel, "date_separator_blink", dateSeparatorBlinking ? "1" : "0");
  saveConfig();
  loadingFlag = true;
  HTTP.send(200, "text/plain", dateSeparatorBlinking ? "1" : "0");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Показывать год"
void handle_date_show_year() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400);
    return;
  }
  bool newVal = HTTP.arg("value") == "1";
  if (newVal == showYearInDate) {
    HTTP.send(200, "text/plain", showYearInDate ? "1" : "0");
    return;
  }
  showYearInDate = newVal;
  jsonWrite(configLedPanel, "date_show_year", showYearInDate ? "1" : "0");
  saveConfig();
  loadingFlag = true;
  HTTP.send(200, "text/plain", showYearInDate ? "1" : "0");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// ----------------------------------------------------------------------- ТАЙМЕРЫ и ИНТЕРВАЛЫ --------------------------------------------------------
#if USE_WEATHER
// Таймер Часы / Погода
void handle_timer_c_w() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, F("text/plain"), F("Missing value"));
    return;
  }

  bool newVal = HTTP.arg("value").toInt() != 0;
  if (newVal == timer_c_w) {
    HTTP.send(200, F("text/plain"), timer_c_w ? F("1") : F("0"));
    return;
  }

  timer_c_d = false;
  timer_d_w = false;
  timer_c_d_w = false;
  timer_clock_fixed = false;
  timer_c_w = newVal;

  jsonWrite(configLedInterval, "timer_c_w", timer_c_w ? "1" : "0");
  jsonWrite(configLedInterval, "timer_c_d", "0");
  jsonWrite(configLedInterval, "timer_d_w", "0");
  jsonWrite(configLedInterval, "timer_c_d_w", "0");
  jsonWrite(configLedInterval, "timer_clock_fixed", "0");

  if (timer_c_w) {
    weatherEnabled = true;
    jsonWrite(configLedPanel, "weather_enabled", "1");
    clockNeedRedraw = true;
    weatherNeedRedraw = true;
  } else {
    weatherEnabled = jsonReadtoInt(configLedPanel, "weather_enabled", 0) == 1;
    clockNeedRedraw = true;
    weatherNeedRedraw = true;
  }

  saveConfig();
  resetTimerState();

  HTTP.send(200, F("text/plain"), timer_c_w ? F("1") : F("0"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Интервал Часы / Погода
void handle_interval_c_w() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, F("text/plain"), F("Missing value"));
    return;
  }
  String valStr = HTTP.arg("value");
  if (valStr.length() == 0) {
    HTTP.send(400, F("text/plain"), F("Empty value"));
    return;
  }
  int newVal = valStr.toInt();
  if (newVal == 0 && valStr != "0") {
    HTTP.send(400, F("text/plain"), F("Invalid number format"));
    return;
  }
  newVal = constrain(newVal, 5, 600);
  if (newVal == interval_c_w) {
    HTTP.send(200, F("text/plain"), String(newVal).c_str());
    return;
  }
  interval_c_w = newVal;
  jsonWrite(configLedInterval, "interval_c_w", String(newVal));
  saveConfig();
  lastSwitchTime = millis();
  lastTimerSwitch = millis();

  if (timer_c_w) {
    clockNeedRedraw = true;
    weatherNeedRedraw = true;
  }

  HTTP.send(200, F("text/plain"), String(newVal).c_str());

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Таймер Дата / Погода
void handle_timer_d_w() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, F("text/plain"), F("Missing value"));
    return;
  }

  bool newVal = HTTP.arg("value").toInt() != 0;
  if (newVal == timer_d_w) {
    HTTP.send(200, F("text/plain"), timer_d_w ? F("1") : F("0"));
    return;
  }

  timer_c_w = false;
  timer_c_d = false;
  timer_c_d_w = false;
  timer_clock_fixed = false;
  timer_d_w = newVal;

  jsonWrite(configLedInterval, "timer_d_w", timer_d_w ? "1" : "0");
  jsonWrite(configLedInterval, "timer_c_w", "0");
  jsonWrite(configLedInterval, "timer_c_d", "0");
  jsonWrite(configLedInterval, "timer_c_d_w", "0");
  jsonWrite(configLedInterval, "timer_clock_fixed", "0");

  if (timer_d_w) {
    dateEnabled = true;
    weatherEnabled = true;
    jsonWrite(configLedPanel, "date_enabled", "1");
    jsonWrite(configLedPanel, "weather_enabled", "1");
    dateNeedRedraw = true;
    weatherNeedRedraw = true;
  } else {
    dateEnabled = jsonReadtoInt(configLedPanel, "date_enabled", 0) == 1;
    weatherEnabled = jsonReadtoInt(configLedPanel, "weather_enabled", 0) == 1;
    dateNeedRedraw = true;
    weatherNeedRedraw = true;
  }

  saveConfig();
  resetTimerState();

  HTTP.send(200, F("text/plain"), timer_d_w ? F("1") : F("0"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Интервал Дата / Погода
void handle_interval_d_w() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, F("text/plain"), F("Missing value"));
    return;
  }
  String valStr = HTTP.arg("value");
  if (valStr.length() == 0) {
    HTTP.send(400, F("text/plain"), F("Empty value"));
    return;
  }
  int newVal = valStr.toInt();
  if (newVal == 0 && valStr != "0") {
    HTTP.send(400, F("text/plain"), F("Invalid number format"));
    return;
  }
  newVal = constrain(newVal, 5, 600);
  if (newVal == interval_d_w) {
    HTTP.send(200, F("text/plain"), String(newVal).c_str());
    return;
  }

  interval_d_w = newVal;
  jsonWrite(configLedInterval, "interval_d_w", String(newVal));
  saveConfig();
  lastSwitchTime = millis();
  lastTimerSwitch = millis();

  if (timer_d_w) {
    dateNeedRedraw = true;
    weatherNeedRedraw = true;
  }

  HTTP.send(200, F("text/plain"), String(newVal).c_str());

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Таймер Часы / Дата / Погода
void handle_timer_c_d_w() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, F("text/plain"), F("Missing value"));
    return;
  }

  bool newVal = HTTP.arg("value").toInt() != 0;
  if (newVal == timer_c_d_w) {
    HTTP.send(200, F("text/plain"), timer_c_d_w ? F("1") : F("0"));
    return;
  }

  timer_c_w = false;
  timer_c_d = false;
  timer_d_w = false;
  timer_clock_fixed = false;
  timer_c_d_w = newVal;

  jsonWrite(configLedInterval, "timer_c_d_w", timer_c_d_w ? "1" : "0");
  jsonWrite(configLedInterval, "timer_c_w", "0");
  jsonWrite(configLedInterval, "timer_c_d", "0");
  jsonWrite(configLedInterval, "timer_d_w", "0");
  jsonWrite(configLedInterval, "timer_clock_fixed", "0");

  if (timer_c_d_w) {
    dateEnabled = true;
    weatherEnabled = true;
    jsonWrite(configLedPanel, "date_enabled", "1");
    jsonWrite(configLedPanel, "weather_enabled", "1");
    clockNeedRedraw = true;
    dateNeedRedraw = true;
    weatherNeedRedraw = true;
  } else {
    dateEnabled = jsonReadtoInt(configLedPanel, "date_enabled", 0) == 1;
    weatherEnabled = jsonReadtoInt(configLedPanel, "weather_enabled", 0) == 1;
    clockNeedRedraw = true;
    dateNeedRedraw = true;
    weatherNeedRedraw = true;
  }

  saveConfig();
  resetTimerState();

  HTTP.send(200, F("text/plain"), timer_c_d_w ? F("1") : F("0"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Интервал Часы / Дата / Погода
void handle_interval_c_d_w() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, F("text/plain"), F("Missing value"));
    return;
  }
  String valStr = HTTP.arg("value");
  if (valStr.length() == 0) {
    HTTP.send(400, F("text/plain"), F("Empty value"));
    return;
  }
  int newVal = valStr.toInt();
  if (newVal == 0 && valStr != "0") {
    HTTP.send(400, F("text/plain"), F("Invalid number format"));
    return;
  }
  newVal = constrain(newVal, 5, 600);
  if (newVal == interval_c_d_w) {
    HTTP.send(200, F("text/plain"), String(newVal).c_str());
    return;
  }
  interval_c_d_w = newVal;
  jsonWrite(configLedInterval, "interval_c_d_w", String(newVal));
  saveConfig();

  lastSwitchTime = millis();
  lastTimerSwitch  = millis();

  if (timer_c_d_w) {
    clockNeedRedraw = true;
    dateNeedRedraw = true;
    weatherNeedRedraw = true;
  }

  HTTP.send(200, F("text/plain"), String(newVal).c_str());

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Таймер Часы + Дата / Погода
void handle_timer_clock_fixed() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, F("text/plain"), F("Missing value"));
    return;
  }

  bool newVal = HTTP.arg("value").toInt() != 0;
  if (newVal == timer_clock_fixed) {
    HTTP.send(200, F("text/plain"), timer_clock_fixed ? F("1") : F("0"));
    return;
  }

  timer_c_d_w = false;
  timer_c_d = false;
  timer_d_w = false;
  timer_c_w = false;
  timer_clock_fixed = newVal;

  jsonWrite(configLedInterval, "timer_clock_fixed", timer_clock_fixed ? "1" : "0");
  jsonWrite(configLedInterval, "timer_c_d_w", "0");
  jsonWrite(configLedInterval, "timer_c_d", "0");
  jsonWrite(configLedInterval, "timer_d_w", "0");
  jsonWrite(configLedInterval, "timer_c_w", "0");

  if (timer_clock_fixed) {
    jsonWrite(configLedPanel, "date_enabled", "1");
    jsonWrite(configLedPanel, "weather_enabled", "1");
    dateEnabled = true;
    weatherEnabled = true;
    clockNeedRedraw = true;
    dateNeedRedraw = true;
    weatherNeedRedraw = true;
    lastClockFixedSwitch = millis();
  } else {
    dateEnabled = jsonReadtoInt(configLedPanel, "date_enabled", 0) == 1;
    weatherEnabled = jsonReadtoInt(configLedPanel, "weather_enabled", 0) == 1;
    clockNeedRedraw = true;
    dateNeedRedraw = true;
    weatherNeedRedraw = true;
  }

  saveConfig();
  resetTimerState();

  HTTP.send(200, F("text/plain"), timer_clock_fixed ? F("1") : F("0"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Интервал Часы + Дата / Погода
void handle_interval_clock_fixed() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, F("text/plain"), F("Missing value"));
    return;
  }
  String valStr = HTTP.arg("value");
  if (valStr.length() == 0) {
    HTTP.send(400, F("text/plain"), F("Empty value"));
    return;
  }
  int newVal = valStr.toInt();
  if (newVal == 0 && valStr != "0") {
    HTTP.send(400, F("text/plain"), F("Invalid number format"));
    return;
  }
  newVal = constrain(newVal, 5, 600);
  if (newVal == interval_clock_fixed) {
    HTTP.send(200, F("text/plain"), String(newVal).c_str());
    return;
  }
  interval_clock_fixed = newVal;
  jsonWrite(configLedInterval, "interval_clock_fixed", String(newVal));
  saveConfig();
  lastClockFixedSwitch = millis();

  if (timer_clock_fixed) {
    dateNeedRedraw = true;
    weatherNeedRedraw = true;
  }

  HTTP.send(200, F("text/plain"), String(newVal).c_str());

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

#endif // USE_WEATHER

// Таймер Часы / Дата
void handle_timer_c_d() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, F("text/plain"), F("Missing value"));
    return;
  }

  bool newVal = HTTP.arg("value").toInt() != 0;
  if (newVal == timer_c_d) {
    HTTP.send(200, F("text/plain"), timer_c_d ? F("1") : F("0"));
    return;
  }

  timer_c_w = false;
  timer_d_w = false;
  timer_c_d_w = false;
  timer_clock_fixed = false;
  timer_c_d = newVal;

  jsonWrite(configLedInterval, "timer_c_d", timer_c_d ? "1" : "0");
  jsonWrite(configLedInterval, "timer_c_w", "0");
  jsonWrite(configLedInterval, "timer_d_w", "0");
  jsonWrite(configLedInterval, "timer_c_d_w", "0");
  jsonWrite(configLedInterval, "timer_clock_fixed", "0");

  if (timer_c_d) {
    dateEnabled = true;
    jsonWrite(configLedPanel, "date_enabled", "1");
    clockNeedRedraw = true;
    dateNeedRedraw = true;
  } else {
    dateEnabled = jsonReadtoInt(configLedPanel, "date_enabled", 0) == 1;
    clockNeedRedraw = true;
    dateNeedRedraw = true;
  }

  saveConfig();
  resetTimerState();

  HTTP.send(200, F("text/plain"), timer_c_d ? F("1") : F("0"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Интервал Часы / Дата
void handle_interval_c_d() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, F("text/plain"), F("Missing value"));
    return;
  }
  String valStr = HTTP.arg("value");
  if (valStr.length() == 0) {
    HTTP.send(400, F("text/plain"), F("Empty value"));
    return;
  }
  int newVal = valStr.toInt();
  if (newVal == 0 && valStr != "0") {
    HTTP.send(400, F("text/plain"), F("Invalid number format"));
    return;
  }
  newVal = constrain(newVal, 5, 600);
  if (newVal == interval_c_d) {
    HTTP.send(200, F("text/plain"), String(newVal).c_str());
    return;
  }
  interval_c_d = newVal;
  jsonWrite(configLedInterval, "interval_c_d", String(newVal));
  saveConfig();
  lastSwitchTime = millis();
  lastTimerSwitch = millis();

  if (timer_c_d) {
    clockNeedRedraw = true;
    dateNeedRedraw = true;
  }

  HTTP.send(200, F("text/plain"), String(newVal).c_str());

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

#endif // LED_PANEL

// ----------------------------------------------------------------------------------------------------------------------------------------------------
void handle_index () {
  bool flg = false;
  if (HTTP.arg("index").toInt()) {
    flg = FileCopy (F("/index/index.setup.gz") , F("/index.json.gz"));
    LittleFS.remove("/effect2.ini");
  }
  if (flg) HTTP.send(200, F("text/plain"), F("OK"));
  else HTTP.send(404, F("text/plain"), "File not found");
}

// ---------------------------------------------------------------
// Пароль на страницу настроек
void handle_PassOn () {
  jsonWrite(configSetup, "PassOn", HTTP.arg("PassOn").toInt());
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
  saveConfig();
}

// ----------------------------------------------------------------------- ЯЗЫК ИНТЕРФЕЙСА ------------------------------------------------------------
void  handle_lang () {
  jsonWrite(configSetup, "lang", HTTP.arg("lang"));
  saveConfig();
  Lang_set();
  HTTP.send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

void Lang_set () {
  String Name = "correct." + jsonRead (configSetup, "lang") + ".json";
  String Correct = readFile(Name, 2048);
  for ( uint8_t n = 0; n < MODE_AMOUNT; n++) {
    eff_num_correct[n] = jsonReadtoInt (Correct, String(n));
    if (eff_num_correct[n] == currentMode) jsonWrite(configSetup, "eff_sel", n);
  }
}

// ----------------------------------------------------------------- СБРОС НАСТРОЕК ПО УМОЛЧАНИЮ ------------------------------------------------------
void handle_reset_to_default() {
  showWarning(CRGB::Red, 500, 250);
  setModeSettings();
  updateSets();

  const char* files[][2] = {
    {"/default/config.json", "/config.json"},
    {"/default/config_cycle.json", "/config_cycle.json"},
    {"/default/sound_list.json", "/sound_list.json"},
    {"/default/config_alarm.json", "/config_alarm.json"},
    {"/default/config_sunset.json", "/config_sunset.json"},
    {"/default/config_schedule.json", "/config_schedule.json"},
    {"/default/config_mp3.json", "/config_mp3.json"},
    {"/default/config_st7789.json", "/config_st7789.json"},
    {"/default/config_multilamp.json", "/config_multilamp.json"},
    {"/default/config_mqtt.json", "/config_mqtt.json"},
    {"/default/config_led_panel.json", "/config_led_panel.json"},
    {"/default/config_button.json", "/config_button.json"},
    {"/default/config_weather.json", "/config_weather.json"},
    {"/default/config_led_matrix.json", "/config_led_matrix.json"},
    {"/default/config_led_interval.json", "/config_led_interval.json"},
    {"/default/config_wifi.json", "/config_wifi.json"}
  };

  for (uint8_t i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (FileCopy(files[i][0], files[i][1])) {
      showWarning(CRGB::Green, 100, 100);
    } else {
      showWarning(CRGB::Red, 100, 100);
    }
    delay(50);
  }

  HTTP.send(200, "text/plain", "OK");
  delay(100);
  ESP.restart();
}

// ---------------------------------------------------------- ВКЛЮЧЕНИЕ/ВЫКЛЮЧЕНИЕ МОДУЛЕЙ ЧЕРЕЗ ВЕБ --------------------------------------------------
// Кнопка
void handle_button_enable() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, "text/plain", "Missing value");
    return;
  }
  bool val = HTTP.arg("value").toInt() != 0;
  if (val != buttonEnabled) {
    buttonEnabled = val;
    jsonWrite(configSetup, "button_enabled", buttonEnabled ? "1" : "0");
    saveConfig();
  }
  HTTP.send(200, "text/plain", buttonEnabled ? "1" : "0");
}

// IR-приёмник
void handle_ir_enable() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, "text/plain", "Missing value");
    return;
  }
  bool val = HTTP.arg("value").toInt() != 0;
  if (val != irEnabled) {
    irEnabled = val;
    jsonWrite(configSetup, "ir_enabled", irEnabled ? "1" : "0");
    saveConfig();
  }
  HTTP.send(200, "text/plain", irEnabled ? "1" : "0");
}

// RF-приёмник
void handle_rf_enable() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, "text/plain", "Missing value");
    return;
  }
  bool val = HTTP.arg("value").toInt() != 0;
  if (val != rfEnabled) {
    rfEnabled = val;
    jsonWrite(configSetup, "rf_enabled", rfEnabled ? "1" : "0");
    saveConfig();
  }
  HTTP.send(200, "text/plain", rfEnabled ? "1" : "0");
}

// TM1637 дисплей
void handle_tm1637_enable() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, "text/plain", "Missing value");
    return;
  }
  bool val = HTTP.arg("value").toInt() != 0;
  if (val != tm1637Enabled) {
    tm1637Enabled = val;
    jsonWrite(configSetup, "tm1637_enabled", tm1637Enabled ? "1" : "0");
    saveConfig();

#if USE_TM1637
    if (tm1637Enabled) {
      display.setBrightness(7);
      display.clear();
      display.display(0, 0, 0, 0);
    } else {
      display.setBrightness(0);
      display.clear();
      display.display(0, 0, 0, 0);
    }
#endif
  }
  HTTP.send(200, "text/plain", tm1637Enabled ? "1" : "0");
}

// ST7789 дисплей
void handle_st7789_enable() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, "text/plain", "Missing value");
    return;
  }
  bool val = HTTP.arg("value").toInt() != 0;
  if (val != st7789Enabled) {
    st7789Enabled = val;
    jsonWrite(configSetup, "st7789_enabled", st7789Enabled ? "1" : "0");
    saveConfig();

#if USE_ST7789
#ifdef TFT_BL
    if (st7789Enabled) {
      pinMode(TFT_BL, OUTPUT);
      digitalWrite(TFT_BL, HIGH);
    } else {
      digitalWrite(TFT_BL, LOW);
    }
#endif
#endif
  }
  HTTP.send(200, "text/plain", st7789Enabled ? "1" : "0");
}

// MP3 плеер
void handle_mp3_enable() {
  if (!HTTP.hasArg("value")) {
    HTTP.send(400, "text/plain", "Missing value");
    return;
  }
  bool val = HTTP.arg("value").toInt() != 0;
  if (val != mp3Enabled) {
    mp3Enabled = val;
    jsonWrite(configSetup, "mp3_enabled", mp3Enabled ? "1" : "0");
    saveConfig();

#if USE_MP3_PLAYER
    if (mp3Enabled) {
      if (mp3_player_connect != 4) {
        mp3_setup();
      }
      if (mp3_stop || pause_on) {
        send_command(0x0E, FEEDBACK, 0, 0);
        mp3_stop = false;
        pause_on = false;
      }
    } else {
      if (mp3_player_connect == 4) {
        send_command(0x0E, FEEDBACK, 0, 0);
        delay(50);
        mp3_stop = true;
        pause_on = true;
      }
    }
#endif
  }
  HTTP.send(200, "text/plain", mp3Enabled ? "1" : "0");
}

// *****************************************************************************************************************************************************

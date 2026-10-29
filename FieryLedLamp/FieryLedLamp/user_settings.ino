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

  HTTP.on("/get_settings", HTTP_GET, [](AsyncWebServerRequest * request) {
    DynamicJsonDocument doc(4096);
    deserializeJson(doc, configSetup);
    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
  });

  HTTP.on("/rename", HTTP_POST, [](AsyncWebServerRequest * request) {
    if (!requireFileManagerAuth(request, true)) return;

    if (!request->hasParam("old") || !request->hasParam("new")) {
      request->send(400, "text/plain", "Missing old or new path");
      return;
    }

    String oldPath = request->getParam("old")->value();
    String newPath = request->getParam("new")->value();

    oldPath.trim();
    newPath.trim();

    if (oldPath.length() == 0 || newPath.length() == 0) {
      request->send(400, "text/plain", "Empty path");
      return;
    }

    if (!oldPath.startsWith("/")) oldPath = "/" + oldPath;
    if (!newPath.startsWith("/")) newPath = "/" + newPath;

    if (isDangerousPath(oldPath) || isDangerousPath(newPath)) {
      request->send(400, "text/plain", "Invalid path");
      return;
    }

    bool isDir = oldPath.endsWith("/");
    String cleanOld = isDir ? oldPath.substring(0, oldPath.length() - 1) : oldPath;
    String cleanNew = newPath.endsWith("/") ? newPath.substring(0, newPath.length() - 1) : newPath;

    auto fsExists = [](const String & p) -> bool {
      return (sdType == 1) ? LittleFS.exists(p) : SD.exists(p);
    };
    auto fsRename = [](const String & a, const String & b) -> bool {
      return (sdType == 1) ? LittleFS.rename(a, b) : SD.rename(a, b);
    };
    auto fsOpen = [](const String & p) -> File {
      return (sdType == 1) ? LittleFS.open(p) : SD.open(p);
    };

    if (!fsExists(cleanOld)) {
      request->send(404, "text/plain", "Source not found");
      return;
    }
    if (fsExists(cleanNew)) {
      request->send(409, "text/plain", "Destination already exists");
      return;
    }

    bool ok = false;

    if (isDir) {
      File f = fsOpen(cleanOld);
      bool oldIsDir = f && f.isDirectory();
      if (f) f.close();

      if (oldIsDir) {
        if (copyDirRecursive(cleanOld, cleanNew)) {
          ok = removeRecursive(cleanOld);
        } else {
          ok = false;
        }
      } else {
        ok = fsRename(cleanOld, cleanNew);
      }
    } else {
      ok = fsRename(cleanOld, cleanNew);
    }

    if (ok) {
      request->send(200, "text/plain", "Renamed");
    } else {
      request->send(500, "text/plain", "Rename failed");
    }
  });

  // ----------------------------------------------------------------------------------------------------------------------------------------------------

  HTTP.on("/PassOn", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_PassOn(request);
  });
  HTTP.on("/Power", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_Power(request);
  });   // Управление питанием
  HTTP.on("/index", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_index(request);
  });   // Переключение на финальную страницу
  HTTP.on("/lang", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_lang(request);
  });     // Язык

  // ------------------------------------------------------------ РЕГУЛИРОВКА НАСТРОЕК ЭФФЕКТОВ ---------------------------------------------------------

  HTTP.on("/all_br", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_all_br(request);
  });                // Общая яркость всех эффектов
  HTTP.on("/br", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_br(request);
  });                        // Яркость эффекта
  HTTP.on("/brm", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_brm(request);
  });                      // Уменьшение яркости (-1)
  HTTP.on("/brp", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_brp(request);
  });                      // Увеличение яркости (+1)
  HTTP.on("/sp", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_sp(request);
  });                        // Скорость эффекта
  HTTP.on("/spm", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_spm(request);
  });                      // Уменьшение скорости (-1)
  HTTP.on("/spp", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_spp(request);
  });                      // Увеличение скорости (+1)
  HTTP.on("/sc", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_sc(request);
  });                        // Масштаб / Цвет эффекта
  HTTP.on("/scm", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_scm(request);
  });                      // Уменьшение масштаба (-1)
  HTTP.on("/scp", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_scp(request);
  });                      // Увеличение масштаба (+1)
  HTTP.on("/auto_bri", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_auto_bri(request);
  });            // Автояркость по времени суток
  HTTP.on("/eff_sel", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_eff_sel(request);
  });              // Выбор эффекта из списка
  HTTP.on("/eff", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_eff(request);
  });                      // Следующий / Предыдущий эффект
  HTTP.on("/eff_all", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_eff_all(request);
  });              // Выбрать все
  HTTP.on("/eff_clr", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_eff_clr(request);
  });              // Сбросить выбор
  HTTP.on("/time_eff", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_time_eff(request);
  });            // Время переключения
  HTTP.on("/eff_save", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_eff_save(request);
  });            // Сохранить настройки эффектов в файл
  HTTP.on("/eff_read", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_eff_read(request);
  });            // Загрузить настройки из файла
  HTTP.on("/def", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_def(request);
  });                      // Настройки текущего эффекта по умолчанию
  HTTP.on("/eff_reset", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_eff_reset(request);
  });          // Сброс настроек всех эффектов по умолчанию
  HTTP.on("/favorit", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_favorit(request);
  });              // Включить / Выключить переход кнопкой только по эффектам из избранного
  HTTP.on("/random_on", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_random(request);
  });             // Случайные настройки эффектов в режиме цикл
  HTTP.on("/effect_always", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_effect_always(request);
  });  // Не возобновлять эффекты после обесточивания лампы

  // -------------------------------------------------------------------------- ЦИКЛ --------------------------------------------------------------------

  HTTP.on("/cycle_on", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_cycle_on(request);
  });          // Вкл/Выкл режим Цикл
  HTTP.on("/rnd_cycle", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_rnd_cycle(request);
  });        // Перемешивать или по порядку
  HTTP.on("/cycle_always", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_cycle_always(request);
  });  // Запускать цикл после включения
  HTTP.on("/cycle_set", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_cycle_set(request);
  });        // Выбор эффектов для цикла

  // ------------------------------------------------------- ВКЛЮЧЕНИЕ/ВЫКЛЮЧЕНИЕ МОДУЛЕЙ В ВЭБ-ИНТЕРФЕЙС -----------------------------------------------

  HTTP.on("/button_enable", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_button_enable(request);
  });
  HTTP.on("/ir_enable", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_ir_enable(request);
  });
  HTTP.on("/rf_enable", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_rf_enable(request);
  });
  HTTP.on("/tm1637_enable", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_tm1637_enable(request);
  });
  HTTP.on("/st7789_enable", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_st7789_enable(request);
  });
  HTTP.on("/mp3_enable", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_mp3_enable(request);
  });
  HTTP.on("/sd_enable", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_sd_enable(request);
  });
  HTTP.on("/sd_type", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_sd_type(request);
  });

  HTTP.on("/get_module_states", HTTP_GET, [](AsyncWebServerRequest * request) {
    DynamicJsonDocument doc(512);
    doc["button_enable"] = buttonEnabled ? "1" : "0";
    doc["ir_enable"] = irEnabled ? "1" : "0";
    doc["rf_enable"] = rfEnabled ? "1" : "0";
    doc["tm1637_enable"] = tm1637Enabled ? "1" : "0";
    doc["st7789_enable"] = st7789Enabled ? "1" : "0";
    doc["mp3_enable"] = mp3Enabled ? "1" : "0";
    doc["radio_enable"] = radioEnabled ? "1" : "0";
    doc["sd_enable"] = sdEnabled ? "1" : "0";
    doc["sd_type"] = String(sdType);
#if MULTI_MATRIX
    doc["multi_matrix_enable"] = multiMatrixEnabled ? "1" : "0";
    doc["multi_matrix"] = "1";
#else
    doc["multi_matrix_enable"] = "0";
    doc["multi_matrix"] = "0";
#endif
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // ----------------------------------------------------------------------------------------------------------------------------------------------------

#if USE_BUTTON
  HTTP.on("/button_type", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_button_type(request);
  });         // Сенсорная / механическая кнопка
  HTTP.on("/save_btn_clicks", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_save_btn_clicks(request);
  }); // Настройки действий по количеству нажатий на кнопку
#endif

#if USE_SUNSET
  HTTP.on("/sunset", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_sunset_http(request);
  });
  HTTP.on("/config_sunset.json", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "application/json", configSunset);
  });
#endif

#if USE_DAWN
  HTTP.on("/alarm", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_alarm_http(request);
  });
  HTTP.on("/config_alarm.json", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "application/json", configAlarm);
  });
#endif

#if USE_SCHEDULE
  HTTP.on("/schedule", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_schedule(request);
  });
  HTTP.on("/config_schedule.json", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "application/json", configSchedule);
  });
#endif

  // -------------------------------------------------------------- УПРАВЛЕНИЕ НЕСКОЛЬКИМИ ЛАМПАМИ ------------------------------------------------------

#if USE_MULTILAMP
  HTTP.on("/multi", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_multiple_lamp(request);
  });
  HTTP.on("/config_multilamp.json", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "application/json", configMultilamp);
  });
#endif

  HTTP.on("/setup_multilamp", HTTP_GET, [](AsyncWebServerRequest * request) {
#ifndef USE_MULTILAMP
    request->send(404, "text/plain", "Управление несколькими лампами отключено в прошивке");
#else
    request->send(LittleFS, "/setup_multilamp.htm", "text/html; charset=utf-8");
#endif
  });

  // --------------------------------------------------------------------- ДИСПЛЕЙ ST7789 ----------------------------------------------------------------

#if USE_ST7789
  HTTP.on("/tft_clock_color", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_tft_clock_color(request);
  });
  HTTP.on("/tft_weather_color", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_tft_weather_color(request);
  });
  HTTP.on("/tft_ticker_on", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_tft_ticker_on(request);
  });
  HTTP.on("/tft_ticker_color", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_tft_ticker_color(request);
  });
  HTTP.on("/tft_ticker_speed", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_tft_ticker_speed(request);
  });
  HTTP.on("/tft_ticker_period", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_tft_ticker_period(request);
  });
  HTTP.on("/tft_ticker_text", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_tft_ticker_text(request);
  });
  HTTP.on("/tft_brightness", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_tft_brightness(request);
  });
  HTTP.on("/tft_auto_brightness", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_tft_auto_brightness(request);
  });
  HTTP.on("/tft_date_color", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_tft_date_color(request);
  });
#endif

  // --------------------------------------------------------------------- ДИСПЛЕЙ TM1637 ----------------------------------------------------------------

#if USE_TM1637
  HTTP.on("/tm1637_brightness", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_tm1637_brightness(request);
  });
  HTTP.on("/tm1637_auto_brightness", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_tm1637_auto_brightness(request);
  });
#endif

  // -------------------------------------------------------------------------- ВРЕМЯ -------------------------------------------------------------------

  HTTP.on("/save_brightness", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_save_brightness(request);
  });
  HTTP.on("/night_time", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_night_time(request);
  });
  HTTP.on("/print_time", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_print_time(request);
  });
  HTTP.on("/clock_vert", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_clock_vert(request);
  });
  HTTP.on("/clock_leading_zero", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_clock_leading_zero(request);
  });
  HTTP.on("/auto_move_clock", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_auto_move_clock(request);
  });
  HTTP.on("/night_clock_enabled", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_night_clock_enabled(request);
  });
  HTTP.on("/night_clock_brightness", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_night_clock_brightness(request);
  });

  HTTP.on("/get_time", HTTP_GET, [](AsyncWebServerRequest * request) {
    DynamicJsonDocument doc(256);

    if (!myTime.isTimeSet()) {
      doc["time"] = "--:--:--";
      doc["city"] = "";
      doc["date"] = "";
      String out;
      serializeJson(doc, out);
      request->send(200, "application/json", out);
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

    if (jsonReadtoInt(configSetup, "geo_tz_enabled") == 1) {
      String geoName = jsonRead(configSetup, "geo_tz_name");
      if (geoName.length() > 0) {
        cityName = geoName;
      }
    }

    if (cityName.length() == 0) {
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
    }

    doc["time"] = String(buf);
    doc["city"] = cityName;
    doc["date"] = dateStr;

    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  // сохранение времени
  HTTP.on("/save_time", HTTP_GET, [](AsyncWebServerRequest * request) {
    bool changed = false;

    String tz = request->getParam("tz")->value();
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
            request->send(400, "text/plain", "INVALID_TZ");
            return;
          }
        }
        changed = true;
      }
    }

    String ntp1_arg = request->getParam("ntp1")->value(); ntp1_arg.trim();
    String ntp2_arg = request->getParam("ntp2")->value(); ntp2_arg.trim();
    String ntp3_arg = request->getParam("ntp3")->value(); ntp3_arg.trim();

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
      request->send(200, "text/plain", "NO_CHANGE");
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

    if (new_ntp3.length() > 0) {
      myTime.setcustomntp(new_ntp3.c_str());
    }

    myTime.tzsetup(current_tz.c_str());

    request->send(200, "text/plain", "OK");
  });

  HTTP.on("/force_time_sync", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (Wifi::instance().isConnected()) {
      myTime.forcesync();
      request->send(200, "text/plain", "SYNC_STARTED");
    } else {
      request->send(503, "text/plain", "Нет подключения к интернету.\nИспользуйте ручную установку времени.");
    }
  });

  // автоопределение часового пояса
  HTTP.on("/auto_tz", HTTP_GET, [](AsyncWebServerRequest * request) {
    jsonWrite(configSetup, "tz", "");
    jsonWrite(configSetup, "use_auto_tz", "1");
    saveConfig();
    g_flagAutoTz = true;
    request->send(200, "text/plain", "AUTO_OK");
  });

  // установка времени с клиента
  HTTP.on("/set_client_time", HTTP_GET, [](AsyncWebServerRequest * request) {
    bool timeChanged = false;
    bool tzChanged = false;

    if (request->hasParam("time")) {
      unsigned long clientTime = request->getParam("time")->value().toInt();
      if (clientTime > 946684800UL) {
        myTime.disable();
        struct timeval tv = { (time_t)clientTime, 0 };
        settimeofday(&tv, nullptr);
        timeChanged = true;
      }
    }

    if (request->hasParam("tz")) {
      String tzStr = urldecode(request->getParam("tz")->value());
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
      request->send(400, "text/plain", "Missing parameters");
      return;
    }

    request->send(200, "text/plain", "OK_TIME_SET");
  });

  // установка часового пояса
  HTTP.on("/tz", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->hasParam("tz")) {
      request->send(400, "text/plain", "Missing tz parameter");
      return;
    }

    String newTz = request->getParam("tz")->value();
    newTz.trim();

    if (newTz.length() == 0 || newTz == "auto") {
      jsonWrite(configSetup, "tz", "");
      jsonWrite(configSetup, "use_auto_tz", "1");
      saveConfig();
      g_flagAutoTz = true;
      request->send(200, "text/plain", "OK_TZ_SET");
      return;
    }

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
        request->send(400, "text/plain", "INVALID_TZ_FORMAT");
        return;
      }
    }

    jsonWrite(configSetup, "tz", posixTz);
    jsonWrite(configSetup, "tz_custom", newTz);
    jsonWrite(configSetup, "tz_is_custom", isPreset ? "0" : "1");
    jsonWrite(configSetup, "use_auto_tz", "0");
    saveConfig();
    myTime.tzsetup(posixTz.c_str());

    request->send(200, "text/plain", "OK_TZ_SET");
  });

  // установка пользовательского часового пояса
  HTTP.on("/set_custom_tz", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->hasParam("tz")) {
      request->send(400, "text/plain", "Missing tz parameter");
      return;
    }

    String customTz = urldecode(request->getParam("tz")->value());
    customTz.trim();

    if (customTz.length() == 0) {
      request->send(400, "text/plain", "Empty timezone");
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
          request->send(400, "text/plain", "Invalid timezone format");
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

    request->send(200, "text/plain", "OK_TZ_SET");
  });

  // установка часового пояса по координатам
  HTTP.on("/set_geo_tz", HTTP_GET, [](AsyncWebServerRequest * request) {
    bool changed = false;

    if (request->hasParam("enabled")) {
      String v = request->getParam("enabled")->value();
      if (jsonRead(configSetup, "geo_tz_enabled") != v) {
        jsonWrite(configSetup, "geo_tz_enabled", v);
        changed = true;
      }
    }
    if (request->hasParam("lat")) {
      String v = request->getParam("lat")->value(); v.replace(",", "."); v.trim();
      if (jsonRead(configSetup, "geo_tz_lat") != v) {
        jsonWrite(configSetup, "geo_tz_lat", v);
        changed = true;
      }
    }
    if (request->hasParam("lon")) {
      String v = request->getParam("lon")->value(); v.replace(",", "."); v.trim();
      if (jsonRead(configSetup, "geo_tz_lon") != v) {
        jsonWrite(configSetup, "geo_tz_lon", v);
        changed = true;
      }
    }

    // применение TZ по координатам (всегда, если включено)
    if (jsonReadtoInt(configSetup, "geo_tz_enabled") == 1) {
      String latS = jsonRead(configSetup, "geo_tz_lat");
      String lonS = jsonRead(configSetup, "geo_tz_lon");
      latS.trim(); lonS.trim();
      if (latS.length() > 0 && lonS.length() > 0) {
        String posixTz = "";
        if (Wifi::instance().isConnected()) {
          posixTz = geoToTimezoneOnline(latS.toFloat(), lonS.toFloat());
        }

        if (posixTz.length() == 0) {
          posixTz = geoToTimezone(latS.toFloat(), lonS.toFloat());
        }

        String savedTz = jsonRead(configSetup, "tz");
        savedTz.trim();

        if (posixTz == "MSK-3" && savedTz.length() > 0 && savedTz != "MSK-3" && savedTz != "auto") {
          posixTz = savedTz;
        } else {
          jsonWrite(configSetup, "tz", posixTz);
          saveConfig();
        }

        myTime.tzsetup(posixTz.c_str());
        return;
      }
    }

    if (changed) {
      saveConfig();
    }

    DynamicJsonDocument doc(256);
    doc["should_refresh"] = "true";
    doc["title"] = jsonRead(configSetup, "geo_tz_name");
    doc["tz"] = jsonRead(configSetup, "tz");
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  // определение города по координатам
  HTTP.on("/resolve_city", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->hasParam("lat") || !request->hasParam("lon")) {
      request->send(400, "text/plain", "Missing lat or lon");
      return;
    }

    float lat = request->getParam("lat")->value().toFloat();
    float lon = request->getParam("lon")->value().toFloat();

    String city = getCityNameFromCoords(lat, lon);
    request->send(200, "text/plain", city);
  });

  // получить текущие настройки geo TZ
  HTTP.on("/get_geo_tz", HTTP_GET, [](AsyncWebServerRequest * request) {
    DynamicJsonDocument doc(512);
    doc["enabled"] = jsonReadtoInt(configSetup, "geo_tz_enabled") == 1;
    doc["lat"] = jsonRead(configSetup, "geo_tz_lat");
    doc["lon"] = jsonRead(configSetup, "geo_tz_lon");
    doc["name"] = jsonRead(configSetup, "geo_tz_name");
    doc["tz"] = jsonRead(configSetup, "tz");
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  HTTP.on("/get_tz_info", HTTP_GET, [](AsyncWebServerRequest * request) {
    String info = getTimezoneInfo();
    request->send(200, "application/json", info);
  });

  // разноцветные часы на матрице (которые EFF_CLOCK)
  HTTP.on("/rainbow_clock", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("value")) {
      String rclVal = request->getParam("value")->value();
      rainbowClock = (rclVal == "1" || rclVal == "true");
      jsonWrite(configLedPanel, "rainbow_clock", rainbowClock ? "1" : "0");
      saveConfig();
      loadingFlag = true;
    }
    request->send(200, "text/plain", rainbowClock ? "1" : "0");
  });

  HTTP.on("/save_geo_tz_param", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->hasParam("key") || !request->hasParam("value")) {
      request->send(400, "text/plain", "Missing key or value");
      return;
    }

    String key = request->getParam("key")->value();
    String value = request->getParam("value")->value();
    if (jsonRead(configSetup, key) != value) {
      jsonWrite(configSetup, key, value);
      saveConfig();
    }
    request->send(200, "text/plain", "OK");
  });

  // --------------------------------------------------------------- ПОГОДА (ЯНДЕКС, OPENWEATHER) --------------------------------------------------------

#if USE_WEATHER
  HTTP.on("/print_weather", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_print_weather(request);
  }); // Интервал вывода погоды
  HTTP.on("/run_weather_text_enabled", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_run_weather_text_enabled(request);
  }); // Включение/выключение погоды бегущей строкой

  HTTP.on("/get_weather", HTTP_GET, [](AsyncWebServerRequest * request) {
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
        String yandexApiKey = jsonRead(configWeather, "yandex_api_key");
        String lat = jsonRead(configWeather, "custom_lat");
        String lon = jsonRead(configWeather, "custom_lon");
        String coordName = jsonRead(configWeather, "custom_coord_name");

        bool useYandexGraphQL = (yandexGeo.length() == 0) && (customYandexGeo.length() == 0) && (lat.length() > 0) && (lon.length() > 0) && (yandexApiKey.length() > 10);

        if (useYandexGraphQL) {
          if (coordName.length() > 0) {
            cityName = coordName;
          } else {
            cityName = "(" + lat + ", " + lon + ")";
          }
        } else {
          if (customYandexGeo.length() > 0) {
            cityName = getCityNameByGeo(customYandexGeo);
            if (cityName.length() == 0) cityName = "Город " + customYandexGeo;
          } else if (yandexGeo.length() > 0) {
            cityName = getCityNameByGeo(yandexGeo);
          } else {
            cityName = "Москва";
          }
        }
      } else { // OpenWeather
        String city = jsonRead(configWeather, "city");
        String customCity = jsonRead(configWeather, "custom_city");
        String coordName = jsonRead(configWeather, "custom_coord_name");
        String lat = jsonRead(configWeather, "custom_lat");
        String lon = jsonRead(configWeather, "custom_lon");

        if (lat.length() > 0 && lon.length() > 0) {
          if (coordName.length() > 0) {
            cityName = coordName;
          } else {
            cityName = "(" + lat + ", " + lon + ")";
          }
        } else if (customCity.length() > 0) {
          cityName = getRussianCityNameForOpenWeather(customCity);
        } else if (city.length() > 0) {
          cityName = getRussianCityNameForOpenWeather(city);
        } else {
          cityName = "Moscow";
        }
      }

      doc["text"] = text;
      doc["temp"] = round(weather.getTemperature());
      doc["condition"] = weather.getCondition();
      doc["show_condition"] = jsonReadtoInt(configWeather, "show_weather_condition", 1) == 1;
      doc["city"] = cityName;
    } else {
      doc["text"] = "—";
      doc["city"] = "";
    }
    doc["init"] = inClockWeatherMode;
    doc["provider"] = weather.isUsingYandex() ? "yandex" : "openweather";
    doc["method"] = weather.isUsingYandex() ? (weather.isUsingYandexGraphQL() ? "graphql" : "sync") : "openweather";

    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  HTTP.on("/save_weather_param", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->hasParam("key") || !request->hasParam("value")) {
      request->send(400, "text/plain", "Missing key or value");
      return;
    }

    String key = request->getParam("key")->value();
    String value = request->getParam("value")->value();
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
      Weather::instance().requestReload();
    }

    if (needForceUpdate && Wifi::instance().isConnected()) {
      Weather::instance().forceUpdate();
    }

    request->send(200, "text/plain", "OK");
  });

  // кнопка "Применить"
  HTTP.on("/save_weather_params", HTTP_GET, [](AsyncWebServerRequest * request) {
    bool configChanged = false;

    String oldLat = jsonRead(configWeather, "custom_lat");
    String oldLon = jsonRead(configWeather, "custom_lon");
    oldLat.trim(); oldLon.trim();

    auto saveIfChanged = [&](const char* key, String value, bool trimVal = false, bool dotComma = false) {
      if (trimVal) value.trim();
      if (dotComma) {
        value.replace(",", ".");
        value.trim();
      }
      String oldValue = jsonRead(configWeather, key);
      if (oldValue != value) {
        jsonWrite(configWeather, key, value);
        configChanged = true;
      }
    };

    if (request->hasParam("weather_source")) saveIfChanged("weather_source", request->getParam("weather_source")->value());
    if (request->hasParam("yandex_geo")) saveIfChanged("yandex_geo", request->getParam("yandex_geo")->value());
    if (request->hasParam("custom_yandex_geo")) saveIfChanged("custom_yandex_geo", request->getParam("custom_yandex_geo")->value(), true);
    if (request->hasParam("city")) saveIfChanged("city", request->getParam("city")->value());
    if (request->hasParam("custom_city")) saveIfChanged("custom_city", request->getParam("custom_city")->value(), true);
    if (request->hasParam("custom_lat")) saveIfChanged("custom_lat", request->getParam("custom_lat")->value(), true, true);
    if (request->hasParam("custom_lon")) saveIfChanged("custom_lon", request->getParam("custom_lon")->value(), true, true);
    if (request->hasParam("yandex_api_key")) saveIfChanged("yandex_api_key", request->getParam("yandex_api_key")->value(), true);

    String latNow = jsonRead(configWeather, "custom_lat");
    String lonNow = jsonRead(configWeather, "custom_lon");
    latNow.trim(); lonNow.trim();

    bool coordsChanged = (oldLat != latNow) || (oldLon != lonNow);

    if (coordsChanged) {
      jsonWrite(configWeather, "custom_coord_name", "");
      configChanged = true;
    } else {
      if (request->hasParam("custom_coord_name")) {
        String value = request->getParam("custom_coord_name")->value();
        value.trim();
        if (jsonRead(configWeather, "custom_coord_name") != value) {
          jsonWrite(configWeather, "custom_coord_name", value);
          configChanged = true;
        }
      }
    }

    String nameNow = jsonRead(configWeather, "custom_coord_name");
    nameNow.trim();

    if (nameNow.length() == 0 && latNow.length() > 0 && lonNow.length() > 0 && Wifi::instance().isConnected()) {
      float latF = latNow.toFloat();
      float lonF = lonNow.toFloat();
      if (latF >= -90.0f && latF <= 90.0f && lonF >= -180.0f && lonF <= 180.0f && !(latF == 0.0f && lonF == 0.0f)) {
        String cityResolved = getCityNameFromCoords(latF, lonF);
        cityResolved.trim();
        if (cityResolved.length() > 0) {
          jsonWrite(configWeather, "custom_coord_name", cityResolved);
          configChanged = true;
        }
      }
    }

    if (configChanged) {
      saveConfig();
      Weather::instance().requestReload();
      if (Wifi::instance().isConnected()) {
        Weather::instance().forceUpdate();
      }
    }

    request->send(200, "text/plain", "OK");
  });

  // текущие настройки
  HTTP.on("/get_weather_settings", HTTP_GET, [](AsyncWebServerRequest * request) {
    DynamicJsonDocument doc(2048);

    String weather_source = jsonRead(configWeather, "weather_source");
    String yandex_geo = jsonRead(configWeather, "yandex_geo");
    String custom_yandex_geo = jsonRead(configWeather, "custom_yandex_geo");
    String city = jsonRead(configWeather, "city");
    String custom_city = jsonRead(configWeather, "custom_city");
    String custom_lat = jsonRead(configWeather, "custom_lat");
    String custom_lon = jsonRead(configWeather, "custom_lon");
    String custom_coord_name = jsonRead(configWeather, "custom_coord_name");
    String yandex_api_key = jsonRead(configWeather, "yandex_api_key");

    doc["yandex_api_key"] = yandex_api_key.length() > 0 ? yandex_api_key : "";
    doc["weather_source"] = weather_source.length() > 0 ? weather_source : "0";
    doc["yandex_geo"] = yandex_geo.length() > 0 ? yandex_geo : "";
    doc["custom_yandex_geo"] = custom_yandex_geo.length() > 0 ? custom_yandex_geo : "";
    doc["city"] = city.length() > 0 ? city : "";
    doc["custom_city"] = custom_city.length() > 0 ? custom_city : "";
    doc["custom_lat"] = custom_lat.length() > 0 ? custom_lat : "";
    doc["custom_lon"] = custom_lon.length() > 0 ? custom_lon : "";
    doc["custom_coord_name"] = custom_coord_name.length() > 0 ? custom_coord_name : "";
    doc["show_condition"] = jsonReadtoInt(configWeather, "show_weather_condition", 1) == 1;

    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

#else
  // заглушки
  // возвращает статус "ОТКЛЮЧЕНО В ПРОШИВКЕ"
  HTTP.on("/get_weather", HTTP_GET, [](AsyncWebServerRequest * request) {
    DynamicJsonDocument doc(128);
    doc["enabled"] = false;
    doc["text"] = "ОТКЛЮЧЕНО В ПРОШИВКЕ";
    doc["temp"] = 0;
    doc["init"] = false;
    doc["city"] = "";
    doc["provider"] = "";
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  HTTP.on("/save_weather_param", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "OK (weather disabled)");
  });

  HTTP.on("/save_weather_params", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "OK (weather disabled)");
  });

  HTTP.on("/get_weather_settings", HTTP_GET, [](AsyncWebServerRequest * request) {
    DynamicJsonDocument doc(128);
    doc["weather_source"] = "0";
    doc["yandex_geo"] = "";
    doc["custom_yandex_geo"] = "";
    doc["city"] = "";
    doc["custom_city"] = "";
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });
#endif

  // ------------------------------------------------------------------------ НАСТРОЙКИ МАТРИЦЫ ---------------------------------------------------------

  HTTP.on("/m_t", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_matrix_tipe(request);
  });
  HTTP.on("/m_o", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_matrix_orientation(request);
  });
#if MULTI_MATRIX
  HTTP.on("/multi_matrix_enable", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_multi_matrix_enable(request);
  });
#endif

  // чип светодиодов, порядок цветов
  HTTP.on("/save_led_settings", HTTP_GET, [](AsyncWebServerRequest * request) {
    String configLED = readFile(F("config_led_matrix.json"), 2048);
    if (configLED == "Failed" || configLED == "Large" || configLED.isEmpty()) {
      configLED = "{}";
    }

    bool changed = false;
    if (request->hasParam("led_chip")) {
      String val = request->getParam("led_chip")->value();
      String oldVal = jsonRead(configLED, "led_chip");
      if (oldVal != val) {
        jsonWrite(configLED, "led_chip", val);
        changed = true;
      }
    }

    if (request->hasParam("color_order")) {
      String val = request->getParam("color_order")->value();
      String oldVal = jsonRead(configLED, "color_order");
      if (oldVal != val) {
        jsonWrite(configLED, "color_order", val);
        changed = true;
      }
    }

    if (changed) {
      writeFile(F("config_led_matrix.json"), configLED);
      request->send(200, "text/plain", "OK_REBOOT");
      pendingRestart = true;
    } else {
      request->send(200, "text/plain", "NO_CHANGE");
    }
  });

  HTTP.on("/save_matrix", HTTP_GET, [](AsyncWebServerRequest * request) {
    bool changed = false;
    bool needReboot = false;
    String configLED = readFile(F("config_led_matrix.json"), 2048);
    if (configLED == "Failed" || configLED == "Large") configLED = "{}";

    // с перезагрузкой
    if (request->hasParam("led_chip")) {
      String val = request->getParam("led_chip")->value();
      String old = jsonRead(configLED, "led_chip");
      if (old != val) {
        jsonWrite(configLED, "led_chip", val);
        changed = true;
        needReboot = true;
      }
    }

    if (request->hasParam("color_order")) {
      String val = request->getParam("color_order")->value();
      String old = jsonRead(configLED, "color_order");
      if (old != val) {
        jsonWrite(configLED, "color_order", val);
        changed = true;
        needReboot = true;
      }
    }

    if (request->hasParam("width")) {
      String val = request->getParam("width")->value();
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

    if (request->hasParam("height")) {
      String val = request->getParam("height")->value();
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
    if (request->hasParam("segMatrix_w")) {
      String val = request->getParam("segMatrix_w")->value();
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

    if (request->hasParam("segMatrix_h")) {
      String val = request->getParam("segMatrix_h")->value();
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
    if (request->hasParam("panel_flip")) {
      String val = request->getParam("panel_flip")->value();
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
    if (request->hasParam("m_t")) {
      String val = request->getParam("m_t")->value();
      String old = jsonRead(configLED, "m_t");
      if (old != val) {
        jsonWrite(configLED, "m_t", val);
        changed = true;
        MatrixType = val.toInt();
      }
    }

    if (request->hasParam("m_o")) {
      String val = request->getParam("m_o")->value();
      String old = jsonRead(configLED, "m_o");
      if (old != val) {
        jsonWrite(configLED, "m_o", val);
        changed = true;
        MatrixOrientation = val.toInt();
      }
    }

    if (request->hasParam("cur_lim")) {
      String val = request->getParam("cur_lim")->value();
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
        request->send(200, "text/plain", "OK_REBOOT");
        pendingRestart = true;
      } else {
        request->send(200, "text/plain", "OK");
      }
    } else {
      request->send(200, "text/plain", "NO_CHANGE");
    }
  });

  // --------------------------------------------------------------------- АНИМАЦИОННЫЕ ЭФФЕКТЫ ---------------------------------------------------------

#if USE_ANIMATIONS
  HTTP.on("/get_anim_list", HTTP_GET, [](AsyncWebServerRequest * request) {
    DynamicJsonDocument doc(1024);
    JsonArray array = doc.to<JsonArray>();

    String list = animations_list;
    if (list.length() == 0) {
      request->send(200, "application/json; charset=utf-8", "[]");
      return;
    }

    int start = 0;
    while (true) {
      int comma = list.indexOf(',', start);
      String item;
      if (comma == -1) {
        item = list.substring(start);
        if (item.length() > 0) {
          item.trim();
          array.add(item);
        }
        break;
      } else {
        item = list.substring(start, comma);
        if (item.length() > 0) {
          item.trim();
          array.add(item);
        }
        start = comma + 1;
      }
    }

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json; charset=utf-8", response);
  });

  HTTP.on("/set_anim", HTTP_GET, [](AsyncWebServerRequest * request) {
#if USE_SD
    if (outFile) {
      outFile.close();
      outAnimationActive = false;
    }
#endif
    outEffectActive = false;
    if (!request->hasParam("anim")) {
      request->send(400, "text/plain", "Missing anim parameter");
      return;
    }

    int idx = request->getParam("anim")->value().toInt();
    if (idx < 0 || idx > max_image_num) {
      request->send(400, "text/plain", "Invalid animation index");
      return;
    }

    uint8_t currentEffSel = jsonReadtoInt(configSetup, "eff_sel");
    if (currentMode != EFF_ANIMATION) {
      lastNonAnimSel = currentEffSel;
    }

    if (idx == 0) {
      specialTextEffectParam = 1;
    } else {
      specialTextEffectParam = idx;
    }

    jsonWrite(configSetup, "anim_sel", idx);

    if (currentMode != EFF_ANIMATION) {
      currentMode = EFF_ANIMATION;
      uint8_t ui_index = 0;
      for (ui_index = 0; ui_index < MODE_AMOUNT; ui_index++) {
        if (eff_num_correct[ui_index] == currentMode) break;
      }
      jsonWrite(configSetup, "eff_sel", ui_index);
    }

    loadingFlag = true;
    saveConfig();

    String response = "{\"anim_sel\":" + String(idx) + ",\"eff_sel\":0}";
    request->send(200, "application/json", response);

#if USE_MULTILAMP
    repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
    if (Wifi::instance().isConnected()) {
      Mqtt::instance().needToPublish = true;
    }
#endif

  });

#endif // USE_ANIMATIONS

  // -------------------------------------------------------------------- ЭФФЕКТЫ С SD формата .out -----------------------------------------------------

#if USE_SD

  HTTP.on("/list_sd", HTTP_GET, [](AsyncWebServerRequest * request) {
    String path = "/";
    if (request->hasParam("dir")) path = request->getParam("dir")->value();

    auto openDir = [](const String & p) -> File {
      if (sdType == 1) return LittleFS.open(p);
      else             return SD.open(p);
    };

    File root = openDir(path);
    if (!root || !root.isDirectory()) {
      if (root) root.close();
      request->send(404, "text/plain", "SD dir not found: " + path);
      return;
    }

    String output = "[";
    File file = root.openNextFile();
    bool first = true;
    while (file) {
      if (!first) output += ",";
      first = false;
      output += "{\"type\":\"";
      output += file.isDirectory() ? "dir" : "file";
      output += "\",\"name\":\"";
      output += file.name();
      output += "\"}";
      file.close();
      file = root.openNextFile();
    }
    root.close();
    output += "]";
    request->send(200, "application/json", output);
  });

  HTTP.on("/list_out_files", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_list_out_files(request);
  });

  HTTP.on("/set_out_effect", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->hasParam("file")) {
      request->send(400, "text/plain", "Missing file parameter");
      return;
    }
    if (!ONflag) {
      request->send(403, "text/plain", "Lamp is off");
      return;
    }

    String fileName = request->getParam("file")->value();
    String sizeFolder = String(matrixWidth) + "x" + String(matrixHeight);
    String path = "/effects/" + sizeFolder + "/" + fileName;

    if (!sdFileExists(path)) {
      request->send(404, "text/plain", "File not found");
      return;
    }

    if (outFile) {
      outFile.close();
    }
    outAnimationActive = false;

    lastOutFileName = fileName;
    currentMode = EFF_SD;

    uint8_t currentEffSel = jsonReadtoInt(configSetup, "eff_sel");
    if (currentMode != EFF_SD) {
      jsonWrite(configSetup, "last_non_sd_eff_sel", currentEffSel);
    }

    jsonWrite(configSetup, "eff_sel", effSdIndex);
    jsonWrite(configSetup, "out_sel", fileName);
    jsonWrite(configSetup, "anim_sel", "0");

    writeFile(F("config.json"), configSetup);

    if (startOutAnimation(fileName)) {
      outAnimationActive = true;
      outEffectActive = true;
      FastLED.show();

      request->send(200, "text/plain", "OK");
    } else {
      request->send(500, "text/plain", "Failed to start");
    }
  });
#else // !USE_SD
  HTTP.on("/set_out_effect", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(404, "text/plain", "SD card not enabled");
  });
#endif // USE_SD

  HTTP.on("/get_eff_sd_index", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(effSdIndex));
  });

  // --------------------------------------------------------------------
  HTTP.on("/debug.log", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!LittleFS.exists("/debug.log")) {
      request->send(404, "text/plain", "Log file not found");
      return;
    }
    request->send(LittleFS, "/debug.log", "text/plain");
  });

  // ------------------------------------------------------------------------- LED ПАНЕЛЬ ---------------------------------------------------------------

  // бегущая строка
#if LED_PANEL
  HTTP.on("/spt", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_spt(request);
  });                                     // Скорость бегущей строки
  HTTP.on("/sct", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_sct(request);
  });                                     // Цвет бегущей строки
  HTTP.on("/font_size", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_font_size(request);
  });                         // Шрифт текста
  HTTP.on("/run_text", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_run_text(request);
  });                           // Текст бегущей строки
  HTTP.on("/run_text_enabled", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_run_text_enabled(request);
  });           // Чекбокс "Включить бегущую строку"
  HTTP.on("/run_text_over", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_run_text_over(request);
  });                 // Чекбокс "Бегущая строка поерх эффекта"
  HTTP.on("/interval_run_text", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_interval_run_text(request);
  });         // Интервал вывода текста бегущей строкой"
  HTTP.on("/run_time_text_enabled", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_run_time_text_enabled(request);
  }); // Включение/выключение часов бегущей строкой

  HTTP.on("/rainbow_text", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("value")) {
      String rtVal = request->getParam("value")->value();
      rainbowText = (rtVal == "1" || rtVal == "true");
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
    request->send(200, "text/plain", rainbowText ? "1" : "0");
  });

  // оттенок текста
  HTTP.on("/run_text_hue", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("value")) {
      int val = request->getParam("value")->value().toInt();
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
    request->send(200, "text/plain", String(runTextHue));
  });

  // если интервал вывода бегущей строки установлен 0, то текст будет бежать непрерывно
  HTTP.on("/run_text_cycle", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("value")) {
      String rtcVal = request->getParam("value")->value();
      runTextColorCycle = (rtcVal == "1");

      jsonWrite(configLedPanel, "run_text_cycle", runTextColorCycle ? "1" : "0");
      saveConfig();
      loadingFlag = true;
    }
    request->send(200, "text/plain", runTextColorCycle ? "1" : "0");
  });

  HTTP.on("/text_y_offset", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_text_y_offset(request);
  }); // Смещение текста по Y (по высоте)
  // часы
  HTTP.on("/clock_x_offset", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_clock_x_offset(request);
  }); // Смещение текста по X (по ширине)
  HTTP.on("/clock_y_offset", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_clock_y_offset(request);
  }); // Смещение текста по Y (по высоте)
  HTTP.on("/clock_hue", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_clock_hue(request);
  }); // Оттенок часов
  // дата
  HTTP.on("/date_enabled", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_date_enabled(request);
  });  // Чекбокс "Включить дату"
  HTTP.on("/date_y_offset", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_date_y_offset(request);
  }); // Смещение даиы по Y (по ширине)
  HTTP.on("/date_x_offset", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_date_x_offset(request);
  }); // Смещение даты по X (по ширине)
  HTTP.on("/date_hue", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_date_hue(request);
  }); // Оттенок даты
  HTTP.on("/date_full_year", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_date_full_year(request);
  }); // Показывать 4 цифры года
  HTTP.on("/date_separator_blink", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_date_separator_blink(request);
  }); // Плавное мигание разделительных точек
  HTTP.on("/date_show_year", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_date_show_year(request);
  }); // Показывать год

  HTTP.on("/rainbow_date", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("value")) {
      String dVal = request->getParam("value")->value();
      rainbowDate = (dVal == "1" || dVal.equalsIgnoreCase("true") || dVal == "on");
      jsonWrite(configLedPanel, "rainbow_date", rainbowDate ? "1" : "0");
      saveConfig();
      dateNeedRedraw = true;
      loadingFlag = true;
    }
    request->send(200, "text/plain", rainbowDate ? "1" : "0");
  });

  // погода
#if USE_WEATHER
  HTTP.on("/weather_enabled", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_weather_enabled(request);
  });           // Чекбокс "Включить погоду"
  HTTP.on("/weather_hue", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_weather_hue(request);
  });                   // Регулировка цвета
  HTTP.on("/weather_x_offset", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_weather_x_offset(request);
  });         // Смещение по X
  HTTP.on("/weather_y_offset", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_weather_y_offset(request);
  });         // Смещение по Y
  HTTP.on("/degree_blink", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_degree_blink(request);
  });                 // Мигание знака градуса (маленький кружок)
  // таймеры и интервалы
  HTTP.on("/timer_c_d_w", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_timer_c_d_w(request);
  });                   // таймер часы/дата/погоды
  HTTP.on("/timer_clock_fixed", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_timer_clock_fixed(request);
  });       // таймер часы + дата/погоды
  HTTP.on("/interval_c_d_w", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_interval_c_d_w(request);
  });             // интервал часы/дата/погоды
  HTTP.on("/interval_clock_fixed", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_interval_clock_fixed(request);
  }); // интервал часы + дата/погоды
  HTTP.on("/timer_d_w", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_timer_d_w(request);
  });                       // таймер дата/погода
  HTTP.on("/interval_d_w", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_interval_d_w(request);
  });                 // интервал дата/погода
  HTTP.on("/timer_c_w", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_timer_c_w(request);
  });                       // таймер часы/погода
  HTTP.on("/interval_c_w", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_interval_c_w(request);
  });                 // интервал часы/погода

  HTTP.on("/rainbow_weather", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("value")) {
      String wVal = request->getParam("value")->value();
      rainbowWeather = (wVal == "1" || wVal == "true");
      jsonWrite(configLedPanel, "rainbow_weather", rainbowWeather ? "1" : "0");
      saveConfig();
      weatherNeedRedraw = true;
      loadingFlag = true;
    }
    request->send(200, "text/plain", rainbowWeather ? "1" : "0");
  });
#endif // USE_WEATHER
  // таймер и интервал часы/дата
  HTTP.on("/timer_c_d", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_timer_c_d(request);
  });
  HTTP.on("/interval_c_d", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_interval_c_d(request);
  });
#endif // LED_PANEL

  // ------------------------------------------------------------------------- МП3 ПЛЕЕР ----------------------------------------------------------------

#if USE_MP3_PLAYER

  HTTP.on("/save_sound_settings", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_save_sound_settings(request);
  });
  HTTP.on("/on_sound", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_on_sound(request);
  });
  HTTP.on("/on_day_adv", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_day_advert_on_sound(request);
  });
  HTTP.on("/on_night_adv", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_night_advert_on_sound(request);
  });
  HTTP.on("/on_alm_adv", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_alarm_advert_sound_on(request);
  });
  HTTP.on("/vol", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_volume(request);
  });
  HTTP.on("/alm_vol", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_alarm_volume(request);
  });
  HTTP.on("/sun_vol", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_sunset_volume(request);
  });
  HTTP.on("/day_vol", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_day_advert_volume(request);
  });
  HTTP.on("/night_vol", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_night_advert_volume(request);
  });
  HTTP.on("/tim_h", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_tim_h(request);
  });
  HTTP.on("/tim_m", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_tim_m(request);
  });
  HTTP.on("/delay", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_delay(request);
  });
  HTTP.on("/on_alm_snd", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_alarm_on_sound(request);
  });
  HTTP.on("/on_sun_snd", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_sunset_on_sound(request);
  });
  HTTP.on("/alm_fold_sel", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_alarm_fold_sel(request);
  });
  HTTP.on("/sun_fold_sel", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_sunset_fold_sel(request);
  });
  HTTP.on("/sound_set", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_sound_set(request);
  });
  HTTP.on("/track_down", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_folder_down(request);
  });
  HTTP.on("/track_up", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_folder_up(request);
  });
  HTTP.on("/fold_sel", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_folder_select(request);
  });
  HTTP.on("/eq", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_equalizer(request);
  });
  HTTP.on("/play_time", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_play_time(request);
  });
  HTTP.on("/play_weather", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_play_weather(request);
  });
  HTTP.on("/weather_speak", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_weather_speak(request);
  });
  HTTP.on("/time_speak", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_time_speak(request);
  });
  HTTP.on("/weather_temp_delay", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_weather_temp_delay(request);
  });
  HTTP.on("/weather_desc_delay", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_weather_desc_delay(request);
  });
  HTTP.on("/weather_day_vol", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_weather_day_volume(request);
  });
  HTTP.on("/weather_night_vol", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_weather_night_volume(request);
  });
  HTTP.on("/day_weather_adv", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_day_weather_adv(request);
  });
  HTTP.on("/day_weather_desc", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_day_weather_desc(request);
  });
  HTTP.on("/night_weather_adv", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_night_weather_adv(request);
  });
  HTTP.on("/night_weather_desc", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_night_weather_desc(request);
  });
  HTTP.on("/show_weather_desc", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_show_weather_desc(request);
  });
  HTTP.on("/time_always", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_time_always(request);
  });
  HTTP.on("/weather_always", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_weather_always(request);
  });

  HTTP.on("/stop_weather_sound", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (mp3_player_connect == 4) {
      mp3SendCmd(0x0E, 0x01, 0, 0);
      mp3_stop = true;
      pause_on = false;
      request->send(200, "text/plain", "OK");
    } else {
      request->send(503, "text/plain", "MP3 player not ready");
    }
  });
#endif // USE_MP3_PLAYER

  HTTP.on("/setup_sound", HTTP_GET, [](AsyncWebServerRequest * request) {
#ifndef USE_MP3_PLAYER
    request->send(404, "text/plain", "Настройки звука отключены в прошивке");
#else
    request->send(LittleFS, "/setup_sound.htm", "text/html; charset=utf-8");
#endif
  });

  // ------------------------------------------------------------------- ИНТЕРНЕТ-РАДИО ---------------------------------------------------------

#if USE_RADIO
  HTTP.on("/radio_enable", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_radio_enable(request);
  });
  HTTP.on("/radio_volume", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_radio_volume(request);
  });
  HTTP.on("/get_radio_stations", HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_get_radio_stations(request);
  });
  HTTP.on("/save_radio_stations", HTTP_POST, [](AsyncWebServerRequest * request) {
    handle_save_radio_stations(request);
  });
  HTTP.on("/radio_map_batch", HTTP_POST, [](AsyncWebServerRequest * request) {
    handle_radio_map_batch(request);
  });
  HTTP.on("/setup_radio", HTTP_GET, [](AsyncWebServerRequest * request) {
#ifndef USE_RADIO
    request->send(404, "text/plain", "Radio disabled");
#else
    request->send(LittleFS, "/setup_radio.json", "application/json");
#endif
  });
#endif

  // ------------------------------------------------------------------- УПРАВЛЕНИЕ ИК ПУЛЬТАМИ ---------------------------------------------------------

#if USE_IR_RECEIVER
  // список всех пультов
  HTTP.on("/api/ir/remotes", HTTP_GET, [](AsyncWebServerRequest * request) {
    String remotes = irManager.getRemotesList();
    request->send(200, "application/json; charset=utf-8", remotes);
  });

  // страница настройки ИК пульта
  HTTP.on("/setup_ir", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/setup_ir.htm", "text/html; charset=utf-8");
  });

  // текущий пульт
  HTTP.on("/api/ir/current", HTTP_GET, [](AsyncWebServerRequest * request) {
    DynamicJsonDocument doc(256);
    doc["id"] = irManager.getCurrentRemoteId();
    doc["name"] = irManager.getCurrentRemoteName();
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json; charset=utf-8", response);
  });

  // установка пульта
  HTTP.on("/api/ir/set", HTTP_POST, [](AsyncWebServerRequest * request) {
    if (!request->hasParam("plain", true)) {
      request->send(400, "application/json; charset=utf-8", "{\"success\":false,\"message\":\"Missing body\"}");
      return;
    }
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, request->getParam("plain", true)->value());
    if (error) {
      request->send(400, "application/json; charset=utf-8", "{\"success\":false,\"message\":\"Invalid JSON\"}");
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
      request->send(200, "application/json; charset=utf-8", resp);
    } else {
      request->send(400, "application/json; charset=utf-8", "{\"success\":false,\"message\":\"Remote not found\"}");
    }
  });

  // обучение пульта
  String learningRemoteName = "";
  DynamicJsonDocument learningDoc(4096);
  bool isLearning = false;

  HTTP.on("/api/ir/learn/save", HTTP_POST, [](AsyncWebServerRequest * request) {
    if (!request->hasParam("plain", true)) {
      request->send(400, "application/json; charset=utf-8", "{\"success\":false,\"message\":\"Missing body\"}");
      return;
    }
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, request->getParam("plain", true)->value());
    if (error) {
      request->send(400, "application/json; charset=utf-8", "{\"success\":false,\"message\":\"Invalid JSON\"}");
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

    String remoteId = "custom_" + String((uint32_t)ESP.getEfuseMac(), HEX);

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
      request->send(500, "application/json; charset=utf-8", "{\"success\":false,\"message\":\"Cannot save file\"}");
      return;
    }
    serializeJson(jsonDoc, file);
    file.close();

    irManager.loadRemote(remoteId);

    request->send(200, "application/json; charset=utf-8", "{\"success\":true,\"id\":\"" + remoteId + "\",\"name\":\"" + remoteName + "\"}");
  });

  // получение последнего ИК кода
  HTTP.on("/api/ir/last_code", HTTP_GET, [](AsyncWebServerRequest * request) {
    DynamicJsonDocument doc(64);
    doc["code"] = String(lastIRCode, HEX);
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json; charset=utf-8", response);
    lastIRCode = 0;
  });

#endif // USE_IR_RECEIVER

  // ----------------------------------------------------------------------- НАСТРОЙКА ШРИФТА -----------------------------------------------------------

  HTTP.on("/static_font", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->hasParam("static_font")) {
      request->send(400, "text/plain", "Missing static_font parameter");
      return;
    }
    String arg = request->getParam("static_font")->value();
    int val = arg.toInt();
    if (val < 0 || val > 3) {
      request->send(400, "text/plain", "Invalid static_font value (must be 0-3)");
      return;
    }
    if (staticFont == val) {
      request->send(200, "text/plain", "OK (no change)");
      return;
    }
    staticFont = val;
    jsonWrite(configLedPanel, "static_font", String(staticFont));
    saveConfig();
    loadingFlag = true;
    clockNeedRedraw = true;
    dateNeedRedraw = true;
#if USE_WEATHER
    weatherNeedRedraw = true;
#endif
    needFullRedraw = true;
    request->send(200, "text/plain", "OK");
  });

  // -------------------------------------------------------------- МОДАЛЬНЫЕ ОКНА МЕНЮ ВЭБ-ИНТЕРФЕЙСА --------------------------------------------------

#if STATUS_DEVICE
  HTTP.on("/sd_status", HTTP_GET, handle_sd_status);
  HTTP.on("/ota_status", HTTP_GET, handle_ota_status);
  HTTP.on("/mqtt_status", HTTP_GET, handle_mqtt_status);
  HTTP.on("/multilamp_status", HTTP_GET, handle_multilamp_status);
  HTTP.on("/mp3_status", HTTP_GET, handle_mp3_status);
#if USE_RADIO
  HTTP.on("/radio_status", HTTP_GET, handle_radio_status);
#endif
  HTTP.on("/tm1637_status", HTTP_GET, handle_tm1637_status);
  HTTP.on("/button_status", HTTP_GET, handle_button_status);
  HTTP.on("/ir_rf_status", HTTP_GET, handle_ir_rf_status);
  HTTP.on("/st7789_status", HTTP_GET, handle_st7789_status);
  HTTP.on("/show_weather", HTTP_GET, handle_show_weather);

  HTTP.on("/version", HTTP_GET, [](AsyncWebServerRequest * request) { // Инфа о прошивке
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
    else wifiModeStr = "WiFi выключен";
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
    request->send(200, "application/json; charset=utf-8", response);
  });

  // IP адрес
  HTTP.on("/wifi_ip", HTTP_GET, [](AsyncWebServerRequest * request) {
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
    request->send(200, "application/json; charset=utf-8", response);
  });

#endif // STATUS_DEVICE

  // ------------------------------------------------------------
#if SOFT_INFO
  // информация о памяти
  HTTP.on("/heap", HTTP_GET, [](AsyncWebServerRequest * request) {
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
    request->send(200, "application/json", response);
  });

  // ----------------------------------------------------------------------
  HTTP.on("/get_version", HTTP_GET, [](AsyncWebServerRequest * request) {
#if USE_UPDATE_CHECK
    StaticJsonDocument<512> doc;
    String resp;

    doc["mod_author"] = "AnCore";
    doc["current_version"] = String(VERSION);
    doc["build_datetime"] = buildDateTimeString();
    doc["folder_url"] = "https://github.com/an-core/FieryLedLamp_mod/tree/main/FieryLedLamp_mod";

    serializeJson(doc, resp);
    request->send(200, "application/json; charset=utf-8", resp);
#endif
  });

  HTTP.on("/check_update", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (updateCache.valid && (millis() - updateCache.timestamp < 600000)) {
      request->send(200, "application/json; charset=utf-8", updateCache.response);
      return;
    }

    if (updateCheckInProgress) {
      DynamicJsonDocument doc(256);
      doc["has_update"] = false;
      doc["message"] = "Проверка обновлений уже выполняется, попробуйте позже";
      String resp;
      serializeJson(doc, resp);
      request->send(200, "application/json; charset=utf-8", resp);
      return;
    }

    if (!updateCheckPending) {
      updateCheckPending = true;
      DynamicJsonDocument doc(256);
      doc["has_update"] = false;
      doc["message"] = "Проверка обновлений запущена, обновите страницу через минуту";
      String resp;
      serializeJson(doc, resp);
      request->send(200, "application/json; charset=utf-8", resp);
      return;
    }

    request->send(200, "application/json; charset=utf-8", "{\"has_update\":false,\"message\":\"Попробуйте позже\"}");
  });

#endif // SOFT_INFO

  // ---------------------------------------------------------------------- СКРЫТИЕ ПУНКТОВ МЕНЮ --------------------------------------------------------

  HTTP.on("/features", HTTP_GET, [](AsyncWebServerRequest * request) {
    DynamicJsonDocument doc(512);
    doc["mqtt"]           = !!USE_MQTT;
    doc["mp3"]            = !!USE_MP3_PLAYER;
    doc["radio"]          = !!USE_RADIO;
    doc["sd"]             = !!USE_SD;
    doc["ota"]            = !!USE_OTA;
    doc["update_check"]   = !!USE_UPDATE_CHECK;
    doc["tm1637"]         = !!USE_TM1637;
    doc["st7789"]         = !!USE_ST7789;
    doc["button"]         = !!USE_BUTTON;
    doc["ir"]             = !!USE_IR_RECEIVER;
    doc["ir_settings"]    = !!USE_IR_RECEIVER;
    doc["rf"]             = !!USE_RF_RECEIVER;
    doc["weather"]        = !!USE_WEATHER;
    doc["multilamp"]      = !!USE_MULTILAMP;
    doc["out_files"]      = (USE_SD && sdType == 0);
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
#if defined(ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ARDUINO_ESP32S3_DEV)
    doc["hardware_menu"] = true;
#else
    doc["hardware_menu"] = false;
#endif

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // ------------------------------------------------------------------------ MQTT ПОДКЛЮЧЕНИЕ ----------------------------------------------------------

#if USE_MQTT
  HTTP.on("/mqtt_set",  HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_mqtt_set(request);
  });
  HTTP.on("/mqtt_on",  HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_mqtt_on(request);
  });
  HTTP.on("/mqtt_prd",  HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_mqtt_period(request);
  });
#endif

  HTTP.on("/setup_mqtt.json.gz", HTTP_GET, [](AsyncWebServerRequest * request) {
#ifndef USE_MQTT
    request->send(404, "text/plain", "MQTT disabled in firmware");
#else
    if (!LittleFS.exists("/setup_mqtt.json.gz")) {
      request->send(404, "text/plain", "File not found");
      return;
    }
    request->send(LittleFS, "/setup_mqtt.json.gz", "application/octet-stream");
#endif
  });

  // ------------------------------------------------------------- ОБНОВЛЕНИЕ ПРОШИВКИ ПО ВОЗДУХУ (OTA) -------------------------------------------------

#if USE_OTA
  // страница OTA ОБНОВЛЕНИЕ
  HTTP.on("/update", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!Wifi::instance().isConnected()) {
      request->send(403, "text/plain; charset=utf-8", "WiFi не подключён");
      return;
    }

    String page = readFile("update.htm", 32768);
    if (page == "Failed" || page.isEmpty()) {
      request->send(500, "text/plain; charset=utf-8", "Ошибка загрузки страницы обновления");
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

    request->send(200, "text/html; charset=utf-8", page);
  });

  // обновление прошивки
  HTTP.on("/update_upload", HTTP_POST, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "OK");
  }, [](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace, U_FLASH)) {
        return;
      }
    }
    if (len) {
      Update.write(data, len);
    }
    if (final) {
      if (Update.end(true)) {
        delay(1000);
        ESP.restart();
      }
    }
  }
         );

  // обновление файловой системы
  HTTP.on("/update_fs", HTTP_POST, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "FS_UPDATE_SUCCESS");
  }, [](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      LittleFS.end();
      delay(200);
      const esp_partition_t* partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);
      uint32_t maxFSSize = partition ? partition->size : (4 * 1024 * 1024);
      if (!Update.begin(maxFSSize, U_SPIFFS)) {
        LittleFS.begin();
        return;
      }
    }
    if (len) {
      Update.write(data, len);
    }
    if (final) {
      if (Update.end(true)) {
        delay(1000);
        ESP.restart();
      } else {
        LittleFS.begin();
      }
    }
  }
         );

  // OTA обновление из Arduino IDE
  HTTP.on("/ota_request", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!Wifi::instance().isConnected()) {
      request->send(403, "text/plain; charset=utf-8", "WiFi не подключён");
      return;
    }

    if (Ota::instance().RequestOtaUpdate()) {
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Не удалось запустить OTA. Возможно обновление уже выполняется?");
    }
  });

#endif // USE_OTA

  // -------------------------------------------------- ПЕРЕКЛЮЧЕНИЕ "ЧАСЫ / ПОГОДА" НА ДИСПЛЕЯХ TM1637 и ST7789 ----------------------------------------

#if (USE_TM1637 || USE_ST7789)
  HTTP.on("/save_display_switch", HTTP_GET, [](AsyncWebServerRequest * request) {
    int interval = request->getParam("value")->value().toInt();
    if (interval >= 5 && interval <= 60) {
      jsonWrite(configSetup, "disp_switch", interval);
      saveConfig();
      DISPLAY_SWITCH_INTERVAL = (uint32_t)interval * 1000UL;
      displaySwitchTimer = millis();
      request->send(200, "text/plain", "OK");
    } else if (interval == 0) {
      jsonWrite(configSetup, "disp_switch", 0);
      saveConfig();
      DISPLAY_SWITCH_INTERVAL = 0;
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Invalid value. Use 0 or 5-60 seconds");
    }
  });

  HTTP.on("/date_switch", HTTP_GET, [](AsyncWebServerRequest * request) {
    int value = request->getParam("value")->value().toInt();
    dateSwitchEnabled = (value != 0);
    jsonWrite(configSetup, "date_switch", dateSwitchEnabled ? 1 : 0);
    saveConfig();
    rebuildActiveModes();
    request->send(200, "text/plain", "OK");
  });

  HTTP.on("/weather_switch", HTTP_GET, [](AsyncWebServerRequest * request) {
    int value = request->getParam("value")->value().toInt();
    weatherSwitchEnabled = (value != 0);
    jsonWrite(configSetup, "weather_switch", weatherSwitchEnabled ? 1 : 0);
    saveConfig();
    rebuildActiveModes();
    request->send(200, "text/plain", "OK");
  });

#endif // (USE_TM1637 || USE_ST7789)

  // ---------------------------------------------------------------------- ПАРОЛЬ ДЛЯ РЕДАКТОРА --------------------------------------------------------

  HTTP.on("/set_local_auth", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->hasParam("value")) {
      request->send(400, "text/plain", "Missing value");
      return;
    }
    String val = request->getParam("value")->value();
    bool newState = (val == "1" || val.equalsIgnoreCase("true") || val.equalsIgnoreCase("on"));

    if (ONflag) changePower();

    jsonWrite(configSetup, "local_auth", newState ? "1" : "0");
    saveConfig();

    if (LittleFS.exists("/auth_local.txt")) LittleFS.remove("/auth_local.txt");

    request->send(200, "text/html", "<html><body>OK, restart...</body></html>");
    pendingRestart = true;
  });

  // ----------------------------------------------------------------------- СИСТЕМНЫЕ ЛОГИ -------------------------------------------------------------

#if DEBUG_ENABLED
  // Чекбокс "Включить получение новых логов"
  HTTP.on("/set_syslog", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->hasParam("value")) {
      request->send(400, "text/plain", "Missing value");
      return;
    }

    bool newState = (request->getParam("value")->value() == "1");

    SystemLog::instance().setEnabled(newState);

    jsonWrite(configSetup, "syslog_enabled", newState ? "1" : "0");
    saveConfig();

    request->send(200, "text/plain", newState ? "1" : "0");
  });

  HTTP.on("/logs", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logs = SystemLog::instance().getAll();
    request->send(200, "text/plain; charset=utf-8", logs);
  });

  HTTP.on("/logs_clear", HTTP_GET, [](AsyncWebServerRequest * request) {
    SystemLog::instance().clear();
    request->send(200, "text/plain", "OK");
  });

  HTTP.on("/logs_page", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/logs.htm", "text/html; charset=utf-8");
  });
#else
  // Заглушки, если логи отключены
  HTTP.on("/set_syslog", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(404, "text/plain", "Logs disabled");
  });
  HTTP.on("/logs", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(404, "text/plain", "Logs disabled");
  });
  HTTP.on("/logs_clear", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(404, "text/plain", "Logs disabled");
  });
  HTTP.on("/logs_page", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(404, "text/plain", "Logs disabled");
  });
#endif // DEBUG_ENABLED

  // --------------------------------------------------------------------- Wi-Fi ПОДКЛЮЧЕНИЕ ------------------------------------------------------------

  HTTP.on("/ssdp",  HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_ssdp(request);
  });
  HTTP.on("/save_wifi",  HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_save_wifi(request);
  });
  HTTP.on("/wifi_reconnect_interval",  HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_wifi_reconnect_interval(request);
  });
  HTTP.on("/wifi_check_interval",  HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_wifi_check_interval(request);
  });
  HTTP.on("/ap_always",  HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_ap_always(request);
  }); // Чекбокс "Показывать точку доступа"
  HTTP.on("/res_to_def",  HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_reset_to_default(request);
  });
  HTTP.on("/reset_to_default",  HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_reset_to_default(request);
  });
  HTTP.on("/s_IP",  HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_use_static_ip(request);
  });
  HTTP.on("/set_ip",  HTTP_GET, [](AsyncWebServerRequest * request) {
    handle_set_static_ip(request);
  });

  HTTP.on("/restart", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "OK");
    pendingRestart = true;
  });

  HTTP.on("/display_ip", HTTP_GET, [](AsyncWebServerRequest * request) { // Чекбокс "Показывать IP адрес при старте"
    int value = request->getParam("value")->value().toInt();
    displayIpAtStart = (value != 0);
    jsonWrite(configSetup, "display_ip", displayIpAtStart ? 1 : 0);
    saveConfig();
    request->send(200, "text/plain", displayIpAtStart ? "1" : "0");
  });


  HTTP.on("/wifi_multi", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("wifi_multi")) {
      int multi = request->getParam("wifi_multi")->value().toInt();
      jsonWrite(configWiFi, "wifi_multi", multi);
      wifi_multi_enabled = multi;
      saveConfig();
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Missing parameter");
    }
  });

  // Точка доступа
  HTTP.on("/ssidap", HTTP_GET, [](AsyncWebServerRequest * request) {
    String newSSID = request->hasParam("ssidAP") ? request->getParam("ssidAP")->value() : "";
    String newPass = request->hasParam("passwordAP") ? request->getParam("passwordAP")->value() : "";
    if (newSSID.length() == 0) newSSID = jsonRead(configWiFi, "ssidAP");
    if (newPass.length() == 0) newPass = jsonRead(configWiFi, "passwordAP");

    jsonWrite(configWiFi, "ssidAP", newSSID);
    jsonWrite(configWiFi, "passwordAP", newPass);
    saveConfig();

    Wifi::instance().setAPSettings(newSSID, newPass);
    Wifi::instance().requestRestartAP();

    request->send(200, "text/plain", "OK");
  });

} // User_settings()

// ========================================================================== ОБРАБОТЧИКИ =============================================================

// Имя лампы
void handle_ssdp(AsyncWebServerRequest *request) {
  if (!request->hasParam("ssdp")) {
    request->send(400, "text/plain", "Missing ssdp");
    return;
  }

  String newName = request->getParam("ssdp")->value();
  newName.trim();

  jsonWrite(configSetup, "ssdp", newName);
  saveConfig();

  LAMP_NAME = newName;
  SSDP.setName(LAMP_NAME.c_str());

  request->send(200, "application/json", "{\"should_refresh\": \"true\"}");
}

// ----------------------------------------------------------------------------- WI-FI ----------------------------------------------------------------

// Показывать точку доступа всегда
void handle_ap_always(AsyncWebServerRequest *request) {
  if (!request->hasParam("ap_always")) {
    request->send(400, "text/plain", "Missing parameter");
    return;
  }
  String apVal = request->getParam("ap_always")->value();
  bool newVal = (apVal == "1" || apVal.toInt() == 1);
  bool oldVal = Always_ap;
  if (newVal == oldVal) {
    request->send(200, "text/plain", newVal ? "1" : "0");
    return;
  }
  Wifi::instance().setAPAlways(newVal);
  Always_ap = newVal;
  request->send(200, "text/plain", newVal ? "1" : "0");
}

void handle_wifi_reconnect_interval(AsyncWebServerRequest *request) {
  if (!request->hasParam("interval")) {
    request->send(400, "text/plain", "Parameter missing");
    return;
  }

  uint32_t interval = request->getParam("interval")->value().toInt();
  interval = constrain(interval, 10, 300);
  Wifi::instance().setReconnectInterval(interval);

  jsonWrite(configWiFi, "wifi_reconnect_interval", String(interval));
  saveConfig();

  request->send(200, "text/plain", "OK");
}

void handle_wifi_check_interval(AsyncWebServerRequest *request) {
  if (!request->hasParam("interval")) {
    request->send(400, "text/plain", "Parameter missing");
    return;
  }

  uint32_t interval = request->getParam("interval")->value().toInt();
  interval = constrain(interval, 1, 30); // минуты
  Wifi::instance().setCheckInterval(interval);

  jsonWrite(configWiFi, "wifi_check_interval", String(interval));
  saveConfig();

  request->send(200, "text/plain", "OK");
}

void handle_save_wifi(AsyncWebServerRequest *request) {
  if (request->hasParam("ssid"))
    jsonWrite(configWiFi, "ssid", request->getParam("ssid")->value());
  if (request->hasParam("password"))
    jsonWrite(configWiFi, "password", request->getParam("password")->value());

  for (int i = 2; i <= 5; i++) {
    String ssidKey = "ssid" + String(i);
    String passKey = "password" + String(i);

    if (request->hasParam(ssidKey)) {
      jsonWrite(configWiFi, ssidKey, request->getParam(ssidKey)->value());
    }
    if (request->hasParam(passKey)) {
      jsonWrite(configWiFi, passKey, request->getParam(passKey)->value());
    }
  }

  if (request->hasParam("wifi_timeout")) {
    int to = request->getParam("wifi_timeout")->value().toInt();
    if (to < 10 || to > 600) to = 30;
    jsonWrite(configWiFi, "wifi_timeout", to);
    Wifi::instance().setWiFiTimeout(to);
  }

  if (request->hasParam("forced_ap_after")) {
    int minutes = request->getParam("forced_ap_after")->value().toInt();
    if (minutes >= 1 && minutes <= 30) {
      jsonWrite(configWiFi, "forced_ap_after", minutes);
      Wifi::instance().setForcedAPTimeout(minutes);
    }
  }

  if (request->hasParam("wifi_reconnect_interval")) {
    int reconnectInterval = request->getParam("wifi_reconnect_interval")->value().toInt();
    reconnectInterval = constrain(reconnectInterval, 30, 300);
    jsonWrite(configWiFi, "wifi_reconnect_interval", reconnectInterval);
    Wifi::instance().setReconnectInterval(reconnectInterval);
  }

  if (request->hasParam("wifi_check_interval")) {
    int checkInterval = request->getParam("wifi_check_interval")->value().toInt();
    checkInterval = constrain(checkInterval, 1, 30);
    jsonWrite(configWiFi, "wifi_check_interval", checkInterval);
    Wifi::instance().setCheckInterval(checkInterval);
  }

  if (request->hasParam("wifi_multi")) {
    int multi = request->getParam("wifi_multi")->value().toInt();
    jsonWrite(configWiFi, "wifi_multi", multi);
  }

  if (request->hasParam("ap_always")) {
    int apAlways = request->getParam("ap_always")->value().toInt();
    Wifi::instance().setAPAlways(apAlways);
    Always_ap = apAlways;
  }

  saveConfig();

  request->send(200, "text/plain", "OK_REBOOT");
  pendingRestart = true;

}

void handle_use_static_ip(AsyncWebServerRequest *request) {
  if (!request->hasParam("s_IP")) {
    request->send(400, "text/plain", "Missing s_IP");
    return;
  }
  bool enable = (request->getParam("s_IP")->value().toInt() == 1);
  jsonWrite(configWiFi, "s_IP", enable ? "1" : "0");
  saveConfig();
  pendingRestart = true;
  request->send(200, "text/plain", "OK_REBOOT");
}

void handle_set_static_ip(AsyncWebServerRequest *request) {
  if (!request->hasParam("ip") || !request->hasParam("gateway") ||
      !request->hasParam("subnet") || !request->hasParam("dns")) {
    request->send(400, "text/plain", "Missing IP parameters");
    return;
  }
  IPAddress ip, gateway, subnet, dns;
  String dnsStr = request->getParam("dns")->value();
  if (!dns.fromString(dnsStr)) {
    dns = IPAddress(8, 8, 8, 8);
  }
  if (ip.fromString(request->getParam("ip")->value()) && gateway.fromString(request->getParam("gateway")->value()) && subnet.fromString(request->getParam("subnet")->value()) && dns.fromString(request->getParam("dns")->value())) {
    Wifi::instance().setStaticIP(ip, gateway, subnet, dns);
    saveConfig();
    pendingRestart = true;
    request->send(200, "text/plain", "OK_REBOOT");
  } else {
    request->send(400, "text/plain", "Invalid IP format");
  }
}

// ----------------------------------------------------------------------- НАСТРОЙКИ КНОПКИ -----------------------------------------------------------

#if USE_BUTTON
void handle_button_type(AsyncWebServerRequest *request) {
  if (!request->hasParam("button_type")) {
    request->send(400, "text/plain", "Missing button_type");
    return;
  }
  String configButton = readFile(F("config_button.json"), 1024);
  if (configButton == "Failed" || configButton == "Large" || configButton.isEmpty()) {
    configButton = "{}";
  }
  button_type = request->getParam("button_type")->value().toInt();
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

  request->send(200, "application/json", "{\"should_refresh\": \"true\"}");
}

void handle_save_btn_clicks(AsyncWebServerRequest *request) {
  const char* required[] = {"btn_click_power", "btn_click_next", "btn_click_prev", "btn_click_action4", "btn_click_ip", "btn_click_time", "btn_click_sound", "btn_click_weather"};
  for (const char* arg : required) {
    if (!request->hasParam(arg)) {
      request->send(400, "text/plain", String("Missing ") + arg);
      return;
    }
  }

  String configButton = readFile(F("config_button.json"), 1024);
  if (configButton == "Failed" || configButton == "Large" || configButton.isEmpty()) {
    configButton = "{}";
  }

  btn_click_power = request->getParam("btn_click_power")->value().toInt();
  btn_click_next = request->getParam("btn_click_next")->value().toInt();
  btn_click_prev = request->getParam("btn_click_prev")->value().toInt();
  btn_click_action4 = request->getParam("btn_click_action4")->value().toInt();
  btn_click_ip = request->getParam("btn_click_ip")->value().toInt();
  btn_click_time = request->getParam("btn_click_time")->value().toInt();
  btn_click_sound = request->getParam("btn_click_sound")->value().toInt();
  btn_click_weather = request->getParam("btn_click_weather")->value().toInt();

  jsonWrite(configButton, "btn_click_power", btn_click_power);
  jsonWrite(configButton, "btn_click_next", btn_click_next);
  jsonWrite(configButton, "btn_click_prev", btn_click_prev);
  jsonWrite(configButton, "btn_click_action4", btn_click_action4);
  jsonWrite(configButton, "btn_click_ip", btn_click_ip);
  jsonWrite(configButton, "btn_click_time", btn_click_time);
  jsonWrite(configButton, "btn_click_sound", btn_click_sound);
  jsonWrite(configButton, "btn_click_weather", btn_click_weather);

  writeFile(F("config_button.json"), configButton);

  request->send(200, "application/json", "{\"should_refresh\": \"true\"}");
}

#endif // USE_BUTTON

// ----------------------------------------------------------------------- НАСТРОЙКИ МП3 ПЛЕЕРА -------------------------------------------------------

#if USE_MP3_PLAYER
// Сохранение настроек
void handle_save_sound_settings(AsyncWebServerRequest *request) {
  uint8_t tmp;
  if (request->hasParam("tim_h")) {
    tmp = request->getParam("tim_h")->value().toInt();
    ADVERT_TIMER_H = 100UL * tmp;
    jsonWrite(configMP3, "tim_h", tmp);
  }
  if (request->hasParam("tim_m")) {
    tmp = request->getParam("tim_m")->value().toInt();
    ADVERT_TIMER_M = 100UL * tmp;
    jsonWrite(configMP3, "tim_m", tmp);
  }
  if (request->hasParam("delay")) {
    tmp = request->getParam("delay")->value().toInt();
    mp3_delay = 10UL * tmp;
    jsonWrite(configMP3, "delay", tmp);
  }
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

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
void handle_on_sound(AsyncWebServerRequest *request) {
  eff_sound_on = request->getParam("on_sound")->value().toInt();
  jsonWrite(configMP3, "on_sound", eff_sound_on);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, ("text/plain"), ("OK"));

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
void handle_day_advert_on_sound(AsyncWebServerRequest *request) {
  day_advert_sound_on = request->getParam("on_day_adv")->value().toInt();
  jsonWrite(configMP3, "on_day_adv", day_advert_sound_on);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, ("text/plain"), ("OK"));

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
void handle_night_advert_on_sound(AsyncWebServerRequest *request) {
  night_advert_sound_on = request->getParam("on_night_adv")->value().toInt();
  jsonWrite(configMP3, "on_night_adv", night_advert_sound_on);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, ("text/plain"), ("OK"));

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
void handle_time_always(AsyncWebServerRequest *request) {
  time_always = request->getParam("time_always")->value().toInt();
  jsonWrite(configMP3, "time_always", time_always);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, ("text/plain"), ("OK"));

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
void handle_weather_always(AsyncWebServerRequest *request) {
  weather_always = request->getParam("weather_always")->value().toInt();
  jsonWrite(configMP3, "weather_always", weather_always);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, ("text/plain"), ("OK"));

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
void handle_day_weather_adv(AsyncWebServerRequest *request) {
  day_weather_temp_on = request->getParam("on_day_wadv")->value().toInt();
  jsonWrite(configMP3, "on_day_wadv", day_weather_temp_on);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, ("text/plain"), ("OK"));

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
void handle_day_weather_desc(AsyncWebServerRequest *request) {
  day_weather_desc_on = request->getParam("on_day_wdesc")->value().toInt();
  jsonWrite(configMP3, "on_day_wdesc", day_weather_desc_on);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, ("text/plain"), ("OK"));

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
void handle_night_weather_adv(AsyncWebServerRequest *request) {
  night_weather_temp_on = request->getParam("on_night_wadv")->value().toInt();
  jsonWrite(configMP3, "on_night_wadv", night_weather_temp_on);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, ("text/plain"), ("OK"));

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
void handle_night_weather_desc(AsyncWebServerRequest *request) {
  night_weather_desc_on = request->getParam("on_night_wdesc")->value().toInt();
  jsonWrite(configMP3, "on_night_wdesc", night_weather_desc_on);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, ("text/plain"), ("OK"));

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
void handle_alarm_advert_sound_on(AsyncWebServerRequest *request) {
  alarm_advert_sound_on = request->getParam("on_alm_adv")->value().toInt();
  jsonWrite(configMP3, "on_alm_adv", alarm_advert_sound_on);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, ("text/plain"), ("OK"));

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
void handle_weather_speak(AsyncWebServerRequest *request) {
  if (request->hasParam("value")) {
    weatherSpeakEnabled = (request->getParam("value")->value() == "1");
    jsonWrite(configMP3, "weather_speak", weatherSpeakEnabled ? "1" : "0");
    writeFile(F("config_mp3.json"), configMP3);
  }
  request->send(200, "text/plain", weatherSpeakEnabled ? "1" : "0");

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
void handle_time_speak(AsyncWebServerRequest *request) {
  if (request->hasParam("value")) {
    timeAnnounceEnabled = (request->getParam("value")->value() == "1");
    jsonWrite(configMP3, "time_speak", timeAnnounceEnabled ? "1" : "0");
    writeFile(F("config_mp3.json"), configMP3);
  }
  request->send(200, "text/plain", timeAnnounceEnabled ? "1" : "0");

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
void handle_alarm_on_sound(AsyncWebServerRequest *request) {
  alarm_sound_on = request->getParam("on_alm_snd")->value().toInt();
  jsonWrite(configMP3, "on_alm_snd", alarm_sound_on);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, ("text/plain"), ("OK"));

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
void handle_sunset_on_sound(AsyncWebServerRequest *request) {
  sunset_sound_on = request->getParam("on_sun_snd")->value().toInt();
  jsonWrite(configMP3, "on_sun_snd", sunset_sound_on);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, ("text/plain"), ("OK"));

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
void handle_show_weather_desc(AsyncWebServerRequest *request) {
  if (request->hasParam("value")) {
    show_weather_desc = request->getParam("value")->value().toInt();
    jsonWrite(configMP3, "show_weather_desc", show_weather_desc);
    writeFile(F("config_mp3.json"), configMP3);
  }
  request->send(200, F("text/plain"), show_weather_desc ? "1" : "0");

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
void handle_volume(AsyncWebServerRequest *request) {
  eff_volume = constrain(request->getParam("vol")->value().toInt(), 0, 30);
  jsonWrite(configMP3, "vol", eff_volume);
  if (mp3Enabled && !dawnflag_sound && !sunsetflag_sound && !isAnnouncing)
    mp3SendCmd(0x06, FEEDBACK, 0, eff_volume);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, "text/plain", String(eff_volume));

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
void handle_alarm_volume(AsyncWebServerRequest *request) {
  alarm_volume = constrain(request->getParam("alm_vol")->value().toInt(), 0, 30);
  jsonWrite(configMP3, "alm_vol", alarm_volume);
  if (mp3Enabled && dawnflag_sound) mp3SendCmd(0x06, FEEDBACK, 0, alarm_volume);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, "text/plain", String(alarm_volume));

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
void handle_sunset_volume(AsyncWebServerRequest *request) {
  sunset_volume = constrain(request->getParam("sun_vol")->value().toInt(), 0, 30);
  jsonWrite(configMP3, "sun_vol", sunset_volume);
  if (mp3Enabled && sunsetflag_sound) mp3SendCmd(0x06, FEEDBACK, 0, sunset_volume);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, "text/plain", String(sunset_volume));

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
void handle_day_advert_volume(AsyncWebServerRequest *request) {
  day_advert_volume = constrain(request->getParam("day_vol")->value().toInt(), 0, 30);
  jsonWrite(configMP3, "day_vol", day_advert_volume);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, "text/plain", String(day_advert_volume));

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
void handle_night_advert_volume(AsyncWebServerRequest *request) {
  night_advert_volume = constrain(request->getParam("night_vol")->value().toInt(), 0, 30);
  jsonWrite(configMP3, "night_vol", night_advert_volume);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, "text/plain", String(night_advert_volume));

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
void handle_tim_h(AsyncWebServerRequest *request) {
  uint8_t tmp = constrain(request->getParam("tim_h")->value().toInt(), 8, 40);
  ADVERT_TIMER_H = 100UL * tmp;
  jsonWrite(configMP3, "tim_h", tmp);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, "text/plain", String(tmp));

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
void handle_tim_m(AsyncWebServerRequest *request) {
  uint8_t tmp = constrain(request->getParam("tim_m")->value().toInt(), 8, 45);
  ADVERT_TIMER_M = 100UL * tmp;
  jsonWrite(configMP3, "tim_m", tmp);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, "text/plain", String(tmp));

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
void handle_delay(AsyncWebServerRequest *request) {
  uint8_t tmp = constrain(request->getParam("delay")->value().toInt(), 3, 23);
  mp3_delay = 10UL * tmp;
  jsonWrite(configMP3, "delay", tmp);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, "text/plain", String(tmp));

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
void handle_weather_day_volume(AsyncWebServerRequest *request) {
  weather_day_volume = constrain(request->getParam("val")->value().toInt(), 0, 30);
  jsonWrite(configMP3, "weather_day_vol", weather_day_volume);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, "text/plain", String(weather_day_volume));

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
void handle_weather_night_volume(AsyncWebServerRequest *request) {
  weather_night_volume = constrain(request->getParam("val")->value().toInt(), 0, 30);
  jsonWrite(configMP3, "weather_night_vol", weather_night_volume);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, "text/plain", String(weather_night_volume));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Пауза после температуры (перед описанием)
void handle_weather_temp_delay(AsyncWebServerRequest *request) {
  uint8_t tmp = constrain(request->getParam("weather_temp_delay")->value().toInt(), 5, 60);
  ADVERT_TIMER_1 = 100UL * tmp;
  jsonWrite(configMP3, "weather_temp_delay", tmp);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, "text/plain", String(tmp));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Пауза после описания (перед завершением озвучки)
void handle_weather_desc_delay(AsyncWebServerRequest *request) {
  uint8_t tmp = constrain(request->getParam("weather_desc_delay")->value().toInt(), 10, 80);
  ADVERT_TIMER_2 = 100UL * tmp; // 1000 - 8000 мс
  jsonWrite(configMP3, "weather_desc_delay", tmp);
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, "text/plain", String(tmp));

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
void handle_play_time(AsyncWebServerRequest *request) {
  if (mp3Enabled && mp3_player_connect == 4 && !isAnnouncing && !alarm_sound_flag && !sunset_sound_flag) {
    Time::instance().forcesync();
    previous_folder = mp3_folder;
    mp3_pending_time_advert = true;
    mp3_pending_time_force = true;
    request->send(200, "text/plain", "OK");
  } else {
    request->send(503, "text/plain", "MP3 not ready, sound off or already speaking");
  }
}

// Озвучка погоды (по кнопке)
void handle_play_weather(AsyncWebServerRequest *request) {
#if USE_WEATHER
  if (mp3Enabled && mp3_player_connect == 4 && !isAnnouncing && !alarm_sound_flag && !sunset_sound_flag) {
    mp3_pending_weather_advert = true;
    mp3_pending_weather_force = true;
    request->send(200, "text/plain", "OK");
  }
  else {
    request->send(503, "text/plain", "MP3 not ready, sound off or already speaking");
  }
#else
  request->send(404, "text/plain", "Weather disabled");
#endif
}

// ----------------------------------------------------------------------
// Папка будильника
void handle_alarm_fold_sel(AsyncWebServerRequest *request) {
  AlarmFolder = request->getParam("alm_fold")->value().toInt();
  jsonWrite(configMP3, "alm_fold", AlarmFolder);
  writeFile(F("config_mp3.json"), configMP3);
  if (mp3Enabled && alarm_sound_flag) {
    mp3_folder = AlarmFolder;
    mp3_folder_last = mp3_folder;
    mp3SendPlayFolder(mp3_folder);
  }
  request->send(200, ("text/plain"), ("OK"));
}

// Папка Заката
void handle_sunset_fold_sel(AsyncWebServerRequest *request) {
  SunsetFolder = request->getParam("sun_fold")->value().toInt();
  jsonWrite(configMP3, "sun_fold", SunsetFolder);
  writeFile(F("config_mp3.json"), configMP3);
  if (mp3Enabled && sunset_sound_flag) {
    mp3_folder = SunsetFolder;
    mp3_folder_last = mp3_folder;
    mp3SendPlayFolder(mp3_folder);
  }
  request->send(200, ("text/plain"), ("OK"));
}

void handle_sound_set(AsyncWebServerRequest *request) {
  char i[4];
  String configSound = readFile(F("sound_list.json"), 2048);
  for (uint8_t k = 0; k < MODE_AMOUNT; k++) {
    itoa(k, i, 10);
    String e = "e" + String(i);
    if (!first_entry)
      jsonWrite(configSound, e, request->getParam(e.c_str())->value().toInt());
    effects_folders[k] = jsonReadtoInt(configSound, e);
    yield();
  }
  if (!first_entry) {
    writeFile(F("sound_list.json"), configSound);
  }
  request->send(200, ("text/plain"), ("OK"));
}

void handle_folder_down(AsyncWebServerRequest *request) {
  CurrentFolder = constrain(CurrentFolder - 1, 0, 99);
  jsonWrite(configMP3, "fold_sel", CurrentFolder);
  writeFile(F("config_mp3.json"), configMP3);
  if (mp3Enabled && !pause_on && !mp3_stop && eff_sound_on && !isAnnouncing) {
    mp3SendCmd(0x17, FEEDBACK, 0, CurrentFolder);
  }
  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
}

void handle_folder_up(AsyncWebServerRequest *request) {
  CurrentFolder = constrain(CurrentFolder + 1, 0, 99);
  jsonWrite(configMP3, "fold_sel", CurrentFolder);
  writeFile(F("config_mp3.json"), configMP3);
  if (mp3Enabled && !pause_on && !mp3_stop && eff_sound_on && !isAnnouncing) {
    mp3SendCmd(0x17, FEEDBACK, 0, CurrentFolder);
  }
  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
}

void handle_folder_select(AsyncWebServerRequest *request) {
  CurrentFolder = request->getParam("fold_sel")->value().toInt();
  jsonWrite(configMP3, "fold_sel", CurrentFolder);
  writeFile(F("config_mp3.json"), configMP3);
  if (mp3Enabled && !pause_on && !mp3_stop && eff_sound_on && !isAnnouncing) {
    mp3SendCmd(0x17, FEEDBACK, 0, CurrentFolder);
  }
  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
}

// ----------------------------------------------------------------------
// Эквалайзер
void handle_equalizer(AsyncWebServerRequest *request) {
  Equalizer = request->getParam("eq")->value().toInt();
  jsonWrite(configMP3, "eq", Equalizer);
  if (mp3Enabled) {
    mp3SendCmd(0x07, FEEDBACK, 0, Equalizer);
  }
  writeFile(F("config_mp3.json"), configMP3);
  request->send(200, ("text/plain"), ("OK"));

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

// ------------------------------------------------------------------------ ИНТЕРНЕТ-РАДИО ------------------------------------------------------------

#if USE_RADIO
void handle_get_radio_stations(AsyncWebServerRequest *request) {
  DynamicJsonDocument doc(8192);
  if (deserializeJson(doc, configRadio) != DeserializationError::Ok) doc.clear();

  JsonObject src = doc.as<JsonObject>();
  DynamicJsonDocument out(8192);
  JsonObject o = out.to<JsonObject>();

  JsonArray stations = o.createNestedArray("stations");
  JsonArray srcStations = src["stations"].as<JsonArray>();

  if (srcStations) {
    for (JsonObject st : srcStations) {
      JsonObject copy = stations.createNestedObject();
      copy["name"] = st["name"].as<String>();
      copy["url"]  = st["url"].as<String>();
    }
  }

  JsonObject map = o.createNestedObject("map");
  JsonObject eff = src["effects"].as<JsonObject>();
  if (!eff) eff = src["map"].as<JsonObject>();

  if (eff) {
    for (JsonPair kv : eff) {
      int v = kv.value().as<int>();
      if (v > 0 && v <= 255) {
        map[kv.key()] = v;
      }
    }
  }

  o["volume"] = src["volume"] | radioVolume;
  o["enabled"] = radioEnabled ? 1 : 0;

  uint8_t currentUiIndex = 0;
  for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
    if (eff_num_correct[i] == currentMode) {
      currentUiIndex = i;
      break;
    }
  }
  o["current_effect"] = currentUiIndex;

  uint8_t currentStationId = 0;
  if (currentMode < MODE_AMOUNT) {
    currentStationId = effects_stations[currentMode];
  }
  o["current_station"] = currentStationId;

  String resp;
  serializeJson(out, resp);
  request->send(200, "application/json; charset=utf-8", resp);
}

void handle_radio_volume(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value");
    return;
  }
  radioSetVolume(constrain(request->getParam("value")->value().toInt(), 0, 21));
  request->send(200, "text/plain", "OK");
}

void handle_save_radio_stations(AsyncWebServerRequest *request) {
  if (!request->hasParam("plain", true)) {
    request->send(400, "text/plain", "Missing body");
    return;
  }
  DynamicJsonDocument req(8192);
  if (deserializeJson(req, request->getParam("plain", true)->value()) != DeserializationError::Ok) {
    request->send(400, "text/plain", "Invalid JSON");
    return;
  }
  JsonArray stations = req["stations"].as<JsonArray>();
  if (!stations) {
    request->send(400, "text/plain", "No stations array");
    return;
  }

  DynamicJsonDocument doc(8192);
  if (deserializeJson(doc, configRadio) != DeserializationError::Ok) doc.clear();

  JsonArray dst = doc.createNestedArray("stations");
  for (JsonObject st : stations) {
    JsonObject o = dst.createNestedObject();
    o["name"] = st["name"].as<String>();
    o["url"]  = st["url"].as<String>();
  }

  JsonObject eff = doc["effects"].as<JsonObject>();
  if (!eff) {
    JsonObject oldMap = doc["map"].as<JsonObject>();
    if (oldMap) {
      eff = doc.createNestedObject("effects");
      for (JsonPair kv : oldMap) eff[kv.key()] = kv.value();
      doc.remove("map");
    } else {
      eff = doc.createNestedObject("effects");
    }
  }

  uint8_t newCount = (uint8_t)stations.size();
  for (JsonPair kv : eff) {
    int stId = kv.value().as<int>();
    if (stId > newCount) {
      eff[kv.key()] = 0;
    }
  }

  if (doc["volume"].isNull()) doc["volume"] = radioVolume;

  configRadio = "";
  serializeJson(doc, configRadio);
  writeFile(F("config_radio.json"), configRadio);

  for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
    effects_stations[i] = eff[String(i)] | 0;
  }

  request->send(200, "text/plain", "OK");
}

void handle_radio_map_batch(AsyncWebServerRequest *request) {
  if (!request->hasParam("plain", true)) {
    request->send(400, "text/plain", "Missing body");
    return;
  }

  DynamicJsonDocument req(8192);
  if (deserializeJson(req, request->getParam("plain", true)->value()) != DeserializationError::Ok) {
    request->send(400, "text/plain", "Invalid JSON");
    return;
  }

  DynamicJsonDocument doc(8192);
  if (deserializeJson(doc, configRadio) != DeserializationError::Ok) doc.clear();

  JsonObject eff = doc["effects"].as<JsonObject>();
  if (!eff) {
    JsonObject oldMap = doc["map"].as<JsonObject>();
    doc.remove("map");
    eff = doc.createNestedObject("effects");
    if (oldMap) {
      for (JsonPair kv : oldMap) eff[kv.key()] = kv.value();
    }
  }
  eff.clear();

  for (JsonPair kv : req.as<JsonObject>()) {
    int effIdRaw = atoi(kv.key().c_str());
    if (effIdRaw < 0 || effIdRaw >= MODE_AMOUNT) continue;
    uint8_t effectId = (uint8_t)effIdRaw;
    int rawStationId = kv.value().as<int>();
    if (rawStationId < 1 || rawStationId > 255) continue;
    eff[String(effectId)] = (uint8_t)rawStationId;
  }

  for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
    effects_stations[i] = eff[String(i)] | 0;
  }

  if (doc["volume"].isNull()) doc["volume"] = radioVolume;
  configRadio = "";
  serializeJson(doc, configRadio);
  writeFile(F("config_radio.json"), configRadio);

  request->send(200, "text/plain", "OK");
}
#endif

// ------------------------------------------------------ РЕГУЛИРОВКА НАСТРОЕК ЭФФЕКТОВ НА ГЛАВНОЙ СТРАНИЦЕ -------------------------------------------

// Общая яркость
void handle_all_br(AsyncWebServerRequest *request) {
  if (!request->hasParam("all_br")) {
    request->send(400, "text/plain", "Missing all_br");
    return;
  }
  jsonWrite(configSetup, "all_br", request->getParam("all_br")->value().toInt());
  uint8_t ALLbri = jsonReadtoInt(configSetup, "all_br");

  for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
    modes[i].Brightness = ALLbri;
  }

  jsonWrite(configSetup, "br", ALLbri);
  SetBrightness(ALLbri);
  saveConfig();
  Eeprom::instance().EepromPut(modes);
  loadingFlag = true;

  request->send(200, "text/plain", String(ALLbri));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
}

// Яркость
void handle_br(AsyncWebServerRequest *request) {
  if (!request->hasParam("br")) {
    request->send(400, "text/plain", "Missing br");
    return;
  }
  if (!ONflag) {
    request->send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  int newBri = constrain(request->getParam("br")->value().toInt(), 1, 255);
  modes[currentMode].Brightness = newBri;
  jsonWrite(configSetup, "br", newBri);
  SetBrightness(newBri);

  saveConfig();
  Eeprom::instance().EepromPut(modes);

  request->send(200, "text/plain", String(newBri));

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
void handle_sp(AsyncWebServerRequest *request) {
  if (!request->hasParam("sp")) {
    request->send(400, "text/plain", "Missing sp");
    return;
  }
  if (!ONflag) {
    request->send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  uint8_t newSpeed = constrain(request->getParam("sp")->value().toInt(), 1, 255);
  modes[currentMode].Speed = newSpeed;
  jsonWrite(configSetup, "sp", newSpeed);
  loadingFlag = true;

  Eeprom::instance().EepromPut(modes);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  request->send(200, "text/plain", String(newSpeed));

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
void handle_sc(AsyncWebServerRequest *request) {
  if (!request->hasParam("sc")) {
    request->send(400, "text/plain", "Missing sc");
    return;
  }
  if (!ONflag) {
    request->send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  uint8_t newScale = constrain(request->getParam("sc")->value().toInt(), 1, 100);
  modes[currentMode].Scale = newScale;
  jsonWrite(configSetup, "sc", newScale);
  loadingFlag = true;

  Eeprom::instance().EepromPut(modes);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  request->send(200, "text/plain", String(newScale));

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
void handle_brm(AsyncWebServerRequest *request) {
  if (!ONflag) {
    request->send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  modes[currentMode].Brightness = constrain(modes[currentMode].Brightness - 1, 1, 255);
  jsonWrite(configSetup, "br", modes[currentMode].Brightness);
  SetBrightness(modes[currentMode].Brightness);

  Eeprom::instance().EepromPut(modes);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

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
void handle_brp(AsyncWebServerRequest *request) {
  if (!ONflag) {
    request->send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  modes[currentMode].Brightness = constrain(modes[currentMode].Brightness + 1, 1, 255);
  jsonWrite(configSetup, "br", modes[currentMode].Brightness);
  SetBrightness(modes[currentMode].Brightness);

  Eeprom::instance().EepromPut(modes);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

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
void handle_spm(AsyncWebServerRequest *request) {
  if (!ONflag) {
    request->send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  modes[currentMode].Speed = constrain(modes[currentMode].Speed - 1, 1, 255);
  jsonWrite(configSetup, "sp", modes[currentMode].Speed);
  loadingFlag = true;

  Eeprom::instance().EepromPut(modes);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

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
void handle_spp(AsyncWebServerRequest *request) {
  if (!ONflag) {
    request->send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  modes[currentMode].Speed = constrain(modes[currentMode].Speed + 1, 1, 255);
  jsonWrite(configSetup, "sp", modes[currentMode].Speed);
  loadingFlag = true;

  Eeprom::instance().EepromPut(modes);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

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
void handle_scm(AsyncWebServerRequest *request) {
  if (!ONflag) {
    request->send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  modes[currentMode].Scale = constrain(modes[currentMode].Scale - 1, 1, 100);
  jsonWrite(configSetup, "sc", modes[currentMode].Scale);
  loadingFlag = true;

  Eeprom::instance().EepromPut(modes);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

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
void handle_scp(AsyncWebServerRequest *request) {
  if (!ONflag) {
    request->send(403, F("application/json"), F("{\"error\": \"Lamp is off\"}"));
    return;
  }
  modes[currentMode].Scale = constrain(modes[currentMode].Scale + 1, 1, 100);
  jsonWrite(configSetup, "sc", modes[currentMode].Scale);
  loadingFlag = true;

  Eeprom::instance().EepromPut(modes);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

void handle_eff_sel(AsyncWebServerRequest *request) {
  systemShuttingDown = false;
#if USE_SD
  if (outFile) {
    outFile.close();
    outAnimationActive = false;
  }
#endif
  outEffectActive = false;
  if (!request->hasParam("eff_sel")) {
    request->send(400, "text/plain", "Missing eff_sel");
    return;
  }
  uint8_t temp = request->getParam("eff_sel")->value().toInt();
  if (temp >= MODE_AMOUNT) {
    request->send(400, "text/plain", "Invalid eff_sel");
    return;
  }
  uint8_t newMode = eff_num_correct[temp];

#if USE_SD
  if (newMode == EFF_SD) {
    uint8_t sdIndex = 255;
    for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
      if (eff_num_correct[i] == EFF_SD) {
        sdIndex = i;
        break;
      }
    }
    if (sdIndex == 255) {
      request->send(500, "text/plain", "EFF_SD not in effect list");
      return;
    }

    if (lastOutFileName.length() > 0 && startOutAnimation(lastOutFileName)) {
      currentMode = EFF_SD;
      jsonWrite(configSetup, "eff_sel", sdIndex);
      jsonWrite(configSetup, "anim_sel", "0");
      loadingFlag = false;
      saveConfig();

      effectsTick();
      FastLED.show();
    }

    request->send(200, "application/json", "{\"should_refresh\": \"true\"}");
    return;
  }
#endif

  jsonWrite(configSetup, "eff_sel", temp);

  specialTextEffectParam = -1;
  jsonWrite(configSetup, "anim_sel", "0");

#if USE_ANIMATIONS
  if (newMode == EFF_ANIMATION) {
    if (request->hasParam("anim")) {
      int animIdx = request->getParam("anim")->value().toInt();
      if (animIdx >= 1 && animIdx <= max_image_num) {
        specialTextEffectParam = animIdx;
        jsonWrite(configSetup, "anim_sel", animIdx);
      } else {
        if (specialTextEffectParam < 1 || specialTextEffectParam > max_image_num) {
          specialTextEffectParam = 1;
        }
      }
    } else {
      if (specialTextEffectParam < 1 || specialTextEffectParam > max_image_num) {
        int saved = jsonReadtoInt(configSetup, "anim_sel");
        specialTextEffectParam = (saved >= 1 && saved <= max_image_num) ? saved : 1;
      }
    }
  }
#endif

  currentMode = newMode;

  SetBrightness(modes[currentMode].Brightness);
  jsonWrite(configSetup, "br", modes[currentMode].Brightness);
  jsonWrite(configSetup, "sp", modes[currentMode].Speed);
  jsonWrite(configSetup, "sc", modes[currentMode].Scale);

  loadingFlag = true;
  effTimer = millis() - 1000;

#if USE_MP3_PLAYER
  if (mp3Enabled && mp3Initialized && !mp3_stop) {
    mp3SendCmd(0x0E, 0, 0, 0);
    mp3_stop = true;
    pause_on = true;
  }
#endif

  effectsTick();
  FastLED.show();

#if USE_RADIO
  if (radioEnabled && ONflag) {
    uint8_t stId = radioGetStationForMode(currentMode);
    if (stId > 0) {
      radioPlayStation(stId);
    } else {
      radioStop();
    }
  } else
#endif

#if USE_MP3_PLAYER
    if (ONflag && eff_sound_on && mp3Enabled && mp3Initialized && !isAnnouncing && !advert_flag && !weather_advert_flag) {
      if (effects_folders[currentMode] != 0) {
        mp3SendPlayFolder(effects_folders[currentMode]);
      }
    }
#endif

  if (random_on && Favorites::instance().FavoritesRunning) {
    selectedSettings = 1U;
  }

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif

#if USE_BLYNK
  updateRemoteBlynkParams();
#endif

  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);

  request->send(200, "application/json", "{\"should_refresh\": \"true\"}");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
}

// ----------------------------------------------------------------------------------------------------------------------------------------------------

void handle_night_time(AsyncWebServerRequest *request) {
  if (!request->hasParam("day_time_hour") || !request->hasParam("day_time_minute") || !request->hasParam("night_time_hour") || !request->hasParam("night_time_minute") || !request->hasParam("day_bright") || !request->hasParam("night_bright")) {
    request->send(400, "text/plain", "Missing night time parameters");
    return;
  }

  String day_hour_str = request->getParam("day_time_hour")->value();
  String day_min_str = request->getParam("day_time_minute")->value();
  String night_hour_str = request->getParam("night_time_hour")->value();
  String night_min_str = request->getParam("night_time_minute")->value();

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

  jsonWrite(configSetup, "day_time_hour",   day_hour_str);
  jsonWrite(configSetup, "day_time_minute", day_min_str);
  jsonWrite(configSetup, "night_time_hour", night_hour_str);
  jsonWrite(configSetup, "night_time_minute", night_min_str);

  uint16_t day_time_minutes   = day_hour * 60 + day_min;
  uint16_t night_time_minutes = night_hour * 60 + night_min;

  jsonWrite(configSetup, "day_time", day_time_minutes);
  jsonWrite(configSetup, "night_time", night_time_minutes);

  uint8_t day_bright = constrain(request->getParam("day_bright")->value().toInt(), 0, 255);
  uint8_t night_bright = constrain(request->getParam("night_bright")->value().toInt(), 0, 255);

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

  request->send(200, ("text/plain"), ("OK"));
}

// ----------------------------------------------------------------------------------------------------------------------------------------------------

// Чекбокс "Включать лампу после обесточивания" (изменённая логика, было наоборот)
void handle_effect_always(AsyncWebServerRequest *request) {
  if (!request->hasParam("effect_always")) {
    request->send(400, "text/plain", "Missing effect_always");
    return;
  }
  int value = request->getParam("effect_always")->value().toInt();
  jsonWrite(configSetup, "effect_always", value);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);
  request->send(200, ("text/plain"), ("OK"));
}

// --------------------------------------------------------------------------- ИЗБРАННОЕ --------------------------------------------------------------

void handle_favorit(AsyncWebServerRequest *request) {
  if (!request->hasParam("favorit")) {
    request->send(400, "text/plain", "Missing favorit");
    return;
  }
  jsonWrite(configSetup, "favorit", request->getParam("favorit")->value().toInt());
  timeout_save_file_changes = millis();
  bitSet (save_file_changes, 0);
  Favorit_only = jsonReadtoInt(configSetup, "favorit");
  request->send(200, ("text/plain"), ("OK"));
}

void handle_random(AsyncWebServerRequest *request) {
  if (!request->hasParam("random_on")) {
    request->send(400, "text/plain", "Missing random_on");
    return;
  }
  jsonWrite(configSetup, "random_on", request->getParam("random_on")->value().toInt());
  timeout_save_file_changes = millis();
  bitSet (save_file_changes, 0);
  random_on = jsonReadtoInt(configSetup, "random_on");
  request->send(200, ("text/plain"), ("OK"));
}

// ----------------------------------------------------------------------- УПРАВЛЕНИЕ ПИТАНИЕМ ---------------------------------------------------------

void handle_Power(AsyncWebServerRequest *request) {
  static uint32_t lastPowerCommandMs = 0;
  uint32_t nowMs = millis();
  if (nowMs - lastPowerCommandMs < 500UL) {
    request->send(200, "application/json", "{\"should_refresh\":\"false\"}");
    return;
  }
  lastPowerCommandMs = nowMs;

#if USE_DAWN
  if (dawnFlag == 1) {
    manualOff = true;
    dawnFlag = 0;
#if USE_TM1637
    if (tm1637Enabled) {
      clockTicker_blink();
    }
#endif // USE_TM1637

    ONflag = false;
    jsonWrite(configSetup, "Power", 0);

    FastLED.clear();
    FastLED.setBrightness(0);
    FastLED.show();

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif
#if defined(ALARM_PIN) && defined(ALARM_LEVEL)
    digitalWrite(ALARM_PIN, !ALARM_LEVEL);
#endif

    saveConfig();
    loadingFlag = false;
    request->send(200, "application/json", "{\"should_refresh\":\"true\"}");
    return;
  }
#endif // USE_DAWN

#if USE_SUNSET
  if (sunsetFlag == 1) {
    manualsOff = true;
    sunsetFlag = 0;

    ONflag = false;
    jsonWrite(configSetup, "Power", 0);
    FastLED.clear();
    FastLED.setBrightness(0);
    FastLED.show();

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif
#if defined(ALARM_PIN) && defined(ALARM_LEVEL)
    digitalWrite(ALARM_PIN, !ALARM_LEVEL);
#endif

    saveConfig();
    loadingFlag = false;
    request->send(200, "application/json", "{\"should_refresh\":\"true\"}");
    return;
  }
#endif

  // в режиме ночных часов после нажатия на кнопку Power лампа просто выключится
  if (ONflag && nightModeBrightness > 0 && currentMode == EFF_CLOCK) {
    nightClockEnabled = false;
    jsonWrite(configLedPanel, "night_clock_enabled", "0");
    nightModeBrightness = 0;

    // выкл лампы
    ONflag = false;
    jsonWrite(configSetup, "Power", ONflag);

    FastLED.setBrightness(0);
    FastLED.clear();
    FastLED.show();
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif

    saveConfig();

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif

    systemShuttingDown = false;
    justPoweredOn = false;
    loadingFlag = false;

    request->send(200, "application/json", "{\"should_refresh\":\"true\"}");

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
  uint8_t requestedState = request->getParam("Power")->value().toInt();
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

  if (!ONflag && leds != nullptr) {
    FastLED.setBrightness(0);
    FastLED.clear();
    FastLED.show();
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif
  }

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

  request->send(200, "application/json", "{\"should_refresh\":\"true\"}");

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

// ----------------------------------------------------------------------------- РАССВЕТ ---------------------------------------------------------------

#if USE_DAWN
bool firstStartAlarm = true;
static void applyAlarmSettings(bool fromRequest, AsyncWebServerRequest *request) {
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

    String hoursStr, minutesStr;
    uint8_t state = 0;

    if (fromRequest && request) {
      hoursStr = request->hasParam(key_h) ? request->getParam(key_h)->value() : "";
      minutesStr = request->hasParam(key_m) ? request->getParam(key_m)->value() : "";
      state = request->hasParam(key_a) ? request->getParam(key_a)->value().toInt() : 0;
    }

    if (hoursStr.length() == 0) {
      hoursStr = jsonRead(configAlarm, key_h);
      if (hoursStr.length() == 0) hoursStr = "07";
    }
    if (minutesStr.length() == 0) {
      minutesStr = jsonRead(configAlarm, key_m);
      if (minutesStr.length() == 0) minutesStr = "00";
    }
    if (!fromRequest) {
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

  uint8_t dawnModeNew = 0, dawnTimeoutNew = 0, dawnBrightNew = 0;

  if (fromRequest && request) {
    dawnModeNew = request->hasParam("t") ? request->getParam("t")->value().toInt() : 0;
    dawnTimeoutNew = request->hasParam("after") ? request->getParam("after")->value().toInt() : 0;
    dawnBrightNew = request->hasParam("a_br") ? request->getParam("a_br")->value().toInt()  : 0;
  }

  if (dawnModeNew == 0) dawnModeNew = jsonReadtoInt(configAlarm, "t");
  if (dawnTimeoutNew == 0) dawnTimeoutNew = jsonReadtoInt(configAlarm, "after");
  if (dawnBrightNew  == 0) dawnBrightNew = jsonReadtoInt(configAlarm, "a_br");

  if (jsonReadtoInt(configAlarm, "t") != dawnModeNew)    {
    jsonWrite(configAlarm, "t", dawnModeNew);
    saveNeeded = true;
  }
  if (jsonReadtoInt(configAlarm, "after") != dawnTimeoutNew) {
    jsonWrite(configAlarm, "after", dawnTimeoutNew);
    saveNeeded = true;
  }
  if (jsonReadtoInt(configAlarm, "a_br") != dawnBrightNew)  {
    jsonWrite(configAlarm, "a_br", dawnBrightNew);
    saveNeeded = true;
  }

  dawnMode = dawnModeNew - 1;
  DAWN_TIMEOUT = dawnTimeoutNew;
  DAWN_BRIGHT = dawnBrightNew;

  if (saveNeeded) {
    save_file_changes |= SAVE_ALARMS_BIT;
    timeout_save_file_changes = millis();
  }
}

void handle_alarm() {
  applyAlarmSettings(false, nullptr);
}

void handle_alarm_http(AsyncWebServerRequest *request) {
  applyAlarmSettings(true, request);
  request->send(200, "application/json", "{\"should_refresh\":\"true\"}");
}

#endif // USE_DAWN

// ------------------------------------------------------------------------------ ЗАКАТ ---------------------------------------------------------------

#if USE_SUNSET
bool firstStartSunset = true;
void saveSunsetConfig(const String & data) {
  writeFile(F("config_sunset.json"), data);
}

static void applySunsetSettings(bool fromRequest, AsyncWebServerRequest *request) {
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

    String hoursStr, minutesStr;
    uint8_t state = 0;

    if (fromRequest && request) {
      hoursStr = request->hasParam(key_h) ? request->getParam(key_h)->value() : "";
      minutesStr = request->hasParam(key_m) ? request->getParam(key_m)->value() : "";
      state = request->hasParam(key_a) ? request->getParam(key_a)->value().toInt() : 0;
    }

    if (hoursStr.length() == 0)   {
      hoursStr = jsonRead(configSunset, key_h);
      if (hoursStr.length() == 0) hoursStr = "21";
    }
    if (minutesStr.length() == 0) {
      minutesStr = jsonRead(configSunset, key_m);
      if (minutesStr.length() == 0) minutesStr = "00";
    }
    if (!fromRequest) state = jsonReadtoInt(configSunset, key_a);

    hoursStr   = zeroPad(hoursStr, 2);
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
    sunsets[k].Time  = hoursStr.toInt() * 60 + minutesStr.toInt();
    yield();
  }

  uint8_t sunsetModeNew = 0, sunsetBrightNew = 0;

  if (fromRequest && request) {
    sunsetModeNew = request->hasParam("t") ? request->getParam("t")->value().toInt() : 0;
    sunsetBrightNew = request->hasParam("s_br") ? request->getParam("s_br")->value().toInt() : 0;
  }

  if (sunsetModeNew == 0) sunsetModeNew = jsonReadtoInt(configSunset, "t");
  if (sunsetBrightNew == 0) sunsetBrightNew = jsonReadtoInt(configSunset, "s_br");

  if (jsonReadtoInt(configSunset, "t") != sunsetModeNew)   {
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
}

void handle_sunset() {
  applySunsetSettings(false, nullptr);
}

void handle_sunset_http(AsyncWebServerRequest *request) {
  applySunsetSettings(true, request);
  request->send(200, "application/json", "{\"should_refresh\":\"true\"}");
}

#endif // USE_SUNSET

// ----------------------------------------------------------------------------- РАСПИСАНИЕ ------------------------------------------------------------

#if USE_SCHEDULE
void handle_schedule(AsyncWebServerRequest *request) {
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
  String enableVal = request->hasParam("schedule_enabled") ? request->getParam("schedule_enabled")->value() : oldEnable;
  if (enableVal != "1") enableVal = "0";
  jsonWrite(configSchedule, "schedule_enabled", enableVal);
  if (enableVal != oldEnable) changed = true;

  uint8_t timerIndex = 0;

  for (uint8_t slot = 1; slot <= 6; slot++) {
    String aKey = "a" + String(slot);
    String hKey = "h" + String(slot);
    String mKey = "m" + String(slot);
    String effKey = (slot == 3) ? "eff3" : "";

    String actionStr = request->hasParam(aKey) ? request->getParam(aKey)->value() : jsonRead(configSchedule, aKey);
    if (actionStr == "" || actionStr == "null") actionStr = "0";
    uint8_t action = actionStr.toInt();

    String hStr = request->hasParam(hKey) ? request->getParam(hKey)->value() : jsonRead(configSchedule, hKey);
    if (hStr == "" || hStr == "null" || hStr == "0") hStr = "00";
    String mStr = request->hasParam(mKey) ? request->getParam(mKey)->value() : jsonRead(configSchedule, mKey);
    if (mStr == "" || mStr == "null" || mStr == "0") mStr = "00";

    uint8_t h = constrain(hStr.toInt(), 0, 23);
    uint8_t m = constrain(mStr.toInt(), 0, 59);
    char buf[3];
    sprintf(buf, "%02d", h); String hFormatted = buf;
    sprintf(buf, "%02d", m); String mFormatted = buf;

    uint8_t effect = 255;

    if (slot == 3 && action == 3) {
      String effStr = request->hasParam(effKey) ? request->getParam(effKey)->value() : jsonRead(configSchedule, effKey);
      if (effStr != "" && effStr != "null") {
        effect = constrain(effStr.toInt(), 0, MODE_AMOUNT - 1);
      }
    }

    bool slotChanged = false;
    if (request->hasParam(aKey)) {
      jsonWrite(configSchedule, aKey, actionStr);
      slotChanged = true;
    }
    if (request->hasParam(hKey)) {
      jsonWrite(configSchedule, hKey, hFormatted);
      slotChanged = true;
    }
    if (request->hasParam(mKey)) {
      jsonWrite(configSchedule, mKey, mFormatted);
      slotChanged = true;
    }
    if (slot == 3 && request->hasParam(effKey)) {
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
  request->send(200, "application/json", respStr);
}
#endif // USE_SCHEDULE

// --------------------------------------------------------------- УПРАВЛЕНИЕ НЕСКОЛЬКИМИ ЛАМПАМИ ------------------------------------------------------

#if USE_MULTILAMP
void handle_multiple_lamp(AsyncWebServerRequest *request) {

  if (request->hasParam("ml1"))
    jsonWrite(configMultilamp, "ml1", request->getParam("ml1")->value().toInt());
  if (request->hasParam("ml2"))
    jsonWrite(configMultilamp, "ml2", request->getParam("ml2")->value().toInt());
  if (request->hasParam("ml3"))
    jsonWrite(configMultilamp, "ml3", request->getParam("ml3")->value().toInt());
  if (request->hasParam("ml4"))
    jsonWrite(configMultilamp, "ml4", request->getParam("ml4")->value().toInt());
  if (request->hasParam("ml5"))
    jsonWrite(configMultilamp, "ml5", request->getParam("ml5")->value().toInt());

  if (request->hasParam("host1"))
    jsonWrite(configMultilamp, "host1", request->getParam("host1")->value());
  if (request->hasParam("host2"))
    jsonWrite(configMultilamp, "host2", request->getParam("host2")->value());
  if (request->hasParam("host3"))
    jsonWrite(configMultilamp, "host3", request->getParam("host3")->value());
  if (request->hasParam("host4"))
    jsonWrite(configMultilamp, "host4", request->getParam("host4")->value());
  if (request->hasParam("host5"))
    jsonWrite(configMultilamp, "host5", request->getParam("host5")->value());

  if (request->hasParam("comment1"))
    jsonWrite(configMultilamp, "comment1", request->getParam("comment1")->value());
  if (request->hasParam("comment2"))
    jsonWrite(configMultilamp, "comment2", request->getParam("comment2")->value());
  if (request->hasParam("comment3"))
    jsonWrite(configMultilamp, "comment3", request->getParam("comment3")->value());
  if (request->hasParam("comment4"))
    jsonWrite(configMultilamp, "comment4", request->getParam("comment4")->value());
  if (request->hasParam("comment5"))
    jsonWrite(configMultilamp, "comment5", request->getParam("comment5")->value());

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
  if (request->hasParam("s_s")) {
    send_sound = request->getParam("s_s")->value().toInt();
    jsonWrite(configMP3, "s_s", send_sound);
  }
  if (request->hasParam("s_e_v")) {
    send_eff_volume = request->getParam("s_e_v")->value().toInt();
  }
  if (!send_sound) {
    send_eff_volume = 0;
  }
  jsonWrite(configMP3, "s_e_v", send_eff_volume);
  writeFile(F("config_mp3.json"), configMP3);
#endif // USE_MP3_PLAYER

  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

void multilamp_get(AsyncWebServerRequest *request) {
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
    }
    if (ml2) {
      Udp.beginPacket(Host2, localPort);
      Udp.print(outputBuffer);
      Udp.endPacket();
    }
    if ( ml3 ) {
      Udp.beginPacket(Host3, localPort);
      Udp.print(outputBuffer);
      Udp.endPacket();
    }
    if ( ml4 ) {
      Udp.beginPacket(Host4, localPort);
      Udp.print(outputBuffer);
      Udp.endPacket();
    }
    if ( ml5 ) {
      Udp.beginPacket(Host5, localPort);
      Udp.print(outputBuffer);
      Udp.endPacket();
    }
    outputBuffer[0] = '\0';
  }
}

#endif // USE_MULTILAMP

// ------------------------------------------------------------------------ РЕЖИМ ЦИКЛ -----------------------------------------------------------------

// Включение/выключение режима Цикл
void handle_cycle_on(AsyncWebServerRequest *request) {
  if (!request->hasParam("cycle_on")) {
    request->send(400, "text/plain", "Missing cycle_on");
    return;
  }
  if (currentMode != EFF_SD) {
#if USE_SD
    if (outFile) {
      outFile.close();
      outAnimationActive = false;
    }
#endif
    outEffectActive = false;
  }
  uint8_t tmp;
  tmp = request->getParam("cycle_on")->value().toInt();
  if (tmp == 2) tmp = (jsonReadtoInt(configSetup, "cycle_on") == 0) ? 1 : 0;

  if (ONflag && tmp) {
    jsonWrite(configSetup, "cycle_on", 1);
    Favorites::instance().FavoritesRunning = 1;
    Eeprom::instance().EepromPut(modes);
  } else {
    Favorites::instance().FavoritesRunning = 0;
    Favorites::instance().nextModeAt = 0;
    jsonWrite(configSetup, "cycle_on", 0);
  }

  saveConfig();
  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// время переключения цикла
void handle_time_eff (AsyncWebServerRequest *request) {
  if (!request->hasParam("time_eff") || !request->hasParam("disp")) {
    request->send(400, "text/plain", "Missing time_eff or disp");
    return;
  }
  jsonWrite(configSetup, "time_eff", request->getParam("time_eff")->value().toInt());
  Favorites::instance().Interval = jsonReadtoInt(configSetup, "time_eff");
  jsonWrite(configSetup, "disp", request->getParam("disp")->value().toInt());

  Favorites::instance().Dispersion = jsonReadtoInt(configSetup, "disp");
  timeout_save_file_changes = millis();
  bitSet (save_file_changes, 0);
  request->send(200, ("text/plain"), ("OK"));
}

// перемешать выбранные или по порядку
void handle_rnd_cycle (AsyncWebServerRequest *request) {
  if (!request->hasParam("rnd_cycle")) {
    request->send(400, "text/plain", "Missing rnd_cycle");
    return;
  }
  jsonWrite(configSetup, "rnd_cycle", request->getParam("rnd_cycle")->value().toInt());
  Favorites::instance().rndCycle = jsonReadtoInt(configSetup, "rnd_cycle");
  timeout_save_file_changes = millis();
  bitSet (save_file_changes, 0);
  request->send(200, ("text/plain"), ("OK"));
}

// Запускать режим цикл после выкл/вкл лампы или нет
void handle_cycle_always (AsyncWebServerRequest *request) {
  if (!request->hasParam("cycle_always")) {
    request->send(400, "text/plain", "Missing cycle_always");
    return;
  }
  jsonWrite(configSetup, "cycle_always", request->getParam("cycle_always")->value().toInt());
  Favorites::instance().UseSavedFavoritesRunning = jsonReadtoInt(configSetup, "cycle_always");

  if (!ONflag && !Favorites::instance().UseSavedFavoritesRunning)   {
    Favorites::instance().FavoritesRunning = 0;
    jsonWrite(configSetup, "cycle_on", 0);
  }

  timeout_save_file_changes = millis();
  bitSet (save_file_changes, 0);
  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

// Выбор эффектов для Цикла
void handle_cycle_set (AsyncWebServerRequest *request) {
  char i[4];
  String configCycle = readFile(F("config_cycle.json"), 2048);

  for (uint8_t k = 0; k < MODE_AMOUNT; k++) {
    itoa ((k), i, 10);
    String e = "e" + String (i) ;
    if (!first_entry)
      jsonWrite(configCycle, e, request->getParam(e.c_str())->value().toInt());
    Favorites::instance().FavoriteModes[k] = jsonReadtoInt(configCycle, e);
    yield();
  }

  if (!first_entry) {
    writeFile(F("config_cycle.json"), configCycle );
  }

  request->send(200, ("text/plain"), ("OK"));
}

// сохранение выбранных эффектов в файл
void cycle_get (AsyncWebServerRequest *request) {
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

// следующий/предыдущий эффект
void handle_eff(AsyncWebServerRequest *request) {
  if (currentMode != EFF_SD) {
#if USE_SD
    if (outFile) {
      outFile.close();
      outAnimationActive = false;
    }
#endif
    outEffectActive = false;
  }

  int temp = jsonReadtoInt(configSetup, "eff_sel");
  if (!request->hasParam("eff")) {
    request->send(400, "text/plain", "Missing eff");
    return;
  }
  bool next = request->getParam("eff")->value().toInt();

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

  effTimer = millis() - 1000;

#if USE_MP3_PLAYER
  if (mp3Enabled && mp3Initialized && !mp3_stop) {
    mp3SendCmd(0x0E, 0, 0, 0);
    mp3_stop = true;
    pause_on = true;
  }
#endif

  effectsTick();
  FastLED.show();

#if USE_RADIO
  if (radioEnabled && ONflag) {
    uint8_t stId = radioGetStationForMode(currentMode);
    if (stId > 0) {
      radioPlayStation(stId);
    } else {
      radioStop();
    }
  } else
#endif

#if USE_MP3_PLAYER
    if (ONflag && eff_sound_on && mp3Enabled && mp3Initialized && !isAnnouncing && !advert_flag && !weather_advert_flag) {
      if (effects_folders[currentMode] != 0) {
        mp3SendPlayFolder(effects_folders[currentMode]);
      }
    }
#endif

  if (random_on && Favorites::instance().FavoritesRunning) selectedSettings = 1U;

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif

#if USE_BLYNK
  updateRemoteBlynkParams();
#endif

  request->send(200, "application/json", "{\"should_refresh\": \"true\"}");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
}

// Выбрать все эффекты
void handle_eff_all (AsyncWebServerRequest *request) {
  char i[4];
  String configCycle = readFile(F("config_cycle.json"), 2048);

  for (uint8_t k = 0; k < MODE_AMOUNT; k++) {
    itoa ((k), i, 10);
    String e = "e" + String (i) ;
    jsonWrite(configCycle, e, 1U);
    yield();
  }
  writeFile(F("config_cycle.json"), configCycle );
  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

// Очистить все эффекты
void handle_eff_clr (AsyncWebServerRequest *request) {
  char i[4];
  String configCycle = readFile(F("config_cycle.json"), 2048);

  for (uint8_t k = 0; k < MODE_AMOUNT; k++) {
    itoa ((k), i, 10);
    String e = "e" + String (i) ;
    jsonWrite(configCycle, e, 0U);
    yield();
  }
  writeFile(F("config_cycle.json"), configCycle );
  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

// --------------------------------------------------------- СОХРАНЕНИЕ, СБРОС И ЗАГРУЗКА НАСТРОЕК ЭФФЕКТОВ --------------------------------------------

// Сброс настроек текущего эффекта по умолчанию
void handle_def(AsyncWebServerRequest *request) {
  outEffectActive = false;
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
  Weather::instance().forceUpdate();
#endif
  resetTimerState();
  manualOverride = true;
  manualOverrideUntil = millis() + 300000UL;

  showWarning(CRGB::Blue, 2000U, 500U);

  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
}

// Сброс настроек всех эффектов по умолчанию
void handle_eff_reset(AsyncWebServerRequest *request) {
#if USE_SD
  if (outFile) {
    outFile.close();
    outAnimationActive = false;
  }
#endif
  outEffectActive = false;
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
  Weather::instance().forceUpdate();
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

  request->send(200, ("text/plain"), ("OK"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
}

// Сохранить настройки эффектов
void handle_eff_save(AsyncWebServerRequest *request) {
  if (!ONflag) {
    request->send(403, "text/plain", "Lamp is off");
    return;
  }

  if (!LittleFS.begin(false) && !LittleFS.exists("/")) {
    request->send(500, "text/plain", "FS mount failed");
    return;
  }

  File file = LittleFS.open("/effect.ini", "w");
  if (!file) {
    request->send(500, "text/plain", "Cannot create file");
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
  request->send(200, "text/plain", "OK");
}

// Загрузить настройки эффектов из файла
void handle_eff_read(AsyncWebServerRequest *request) {
  if (!LittleFS.begin(false)) {
    request->send(500, "text/plain", "FS mount failed");
    return;
  }

  File file = LittleFS.open("/effect.ini", "r");
  if (!file) {
    request->send(404, "text/plain", "File not found");
    return;
  }

  uint16_t file_size = file.size();
  if (file_size < 3 || file_size > MODE_AMOUNT * 3 || (file_size % 3) != 0) {
    file.close();
    request->send(400, "text/plain", "Invalid file size");
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

  request->send(200, "text/plain", "OK");

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// -------------------------------------------------- АВТОЯРКОСТЬ ЭФФЕКТОВ (в зависимости от времени суток) --------------------------------------------

void handle_auto_bri (AsyncWebServerRequest *request) {
  if (!request->hasParam("auto_bri")) {
    request->send(400, "text/plain", "Missing auto_bri");
    return;
  }
  AutoBrightness = request->getParam("auto_bri")->value().toInt();
  jsonWrite(configSetup, "auto_bri", AutoBrightness);

#if USE_DAWN
  if (ONflag && !dawnFlag) {
#else
  if (ONflag) {
#endif
    SetBrightness(modes[currentMode].Brightness);
  }
  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

// ------------------------------------------------------------------ НАСТРОЙКИ ДИСПЛЕЯ ST7789 ---------------------------------------------------------

#if USE_ST7789
void handle_tft_clock_color(AsyncWebServerRequest *request) {
  if (!request->hasParam("tft_clock_color")) {
    request->send(400, "text/plain", "Missing tft_clock_color");
    return;
  }
  int val = request->getParam("tft_clock_color")->value().toInt();
  jsonWrite(configST7789, "tft_clock_color", val);
  saveConfig();
  tft_clock_color = val;
  loadingFlag = true;
  request->send(200, ("text/plain"), ("OK"));
}

void handle_tft_date_color(AsyncWebServerRequest *request) {
  if (request->hasParam("tft_date_color")) {
    uint8_t val = request->getParam("tft_date_color")->value().toInt();
    if (val <= 20) {
      tft_date_color = val;
      jsonWrite(configST7789, "tft_date_color", tft_date_color);
      saveConfig();
      loadingFlag = true;
    }
  }
  request->send(200, ("text/plain"), ("OK"));
}

void handle_tft_weather_color(AsyncWebServerRequest *request) {
  if (!request->hasParam("tft_weather_color")) {
    request->send(400, "text/plain", "Missing tft_weather_color");
    return;
  }
  int val = request->getParam("tft_weather_color")->value().toInt();
  jsonWrite(configST7789, "tft_weather_color", val);
  saveConfig();
  tft_weather_color = val;
  loadingFlag = true;
  request->send(200, ("text/plain"), ("OK"));
}

void handle_tft_ticker_on(AsyncWebServerRequest *request) {
  if (!request->hasParam("tft_ticker_on")) {
    request->send(400, "text/plain", "Missing tft_ticker_on");
    return;
  }
  bool val = (request->getParam("tft_ticker_on")->value().toInt() == 1);
  jsonWrite(configST7789, "tft_ticker_on", val ? "1" : "0");
  saveConfig();
  tft_ticker_on = val;
  if (st7789Enabled) {
    tftTickerStop();
    tftTickerNextStart = millis() + tftTickerPeriodMs();
  }
  request->send(200, ("text/plain"), ("OK"));
}

void handle_tft_ticker_color(AsyncWebServerRequest *request) {
  if (!request->hasParam("tft_ticker_color")) {
    request->send(400, "text/plain", "Missing tft_ticker_color");
    return;
  }
  int val = request->getParam("tft_ticker_color")->value().toInt();
  jsonWrite(configST7789, "tft_ticker_color", val);
  saveConfig();
  tft_ticker_color = val;
  if (st7789Enabled) {
    tftTickerStop();
    tftTickerNextStart = millis() + tftTickerPeriodMs();
  }
  request->send(200, ("text/plain"), ("OK"));
}

void handle_tft_ticker_speed(AsyncWebServerRequest *request) {
  if (!request->hasParam("tft_ticker_speed")) {
    request->send(400, "text/plain", "Missing tft_ticker_speed");
    return;
  }
  uint16_t val = constrain(request->getParam("tft_ticker_speed")->value().toInt(), 50, 500);
  jsonWrite(configST7789, "tft_ticker_speed", val);
  saveConfig();
  tft_ticker_speed = val;
  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

void handle_tft_ticker_period(AsyncWebServerRequest *request) {
  if (!request->hasParam("tft_ticker_period")) {
    request->send(400, "text/plain", "Missing tft_ticker_period");
    return;
  }
  uint16_t val = constrain(request->getParam("tft_ticker_period")->value().toInt(), 0, 3600);
  jsonWrite(configST7789, "tft_ticker_period", val);
  saveConfig();
  tft_ticker_period = val;
  if (st7789Enabled) {
    tftTickerStop();
    tftTickerNextStart = millis() + tftTickerPeriodMs();
  }
  request->send(200, ("text/plain"), ("OK"));
}

void handle_tft_ticker_text(AsyncWebServerRequest *request) {
  if (!request->hasParam("tft_ticker_text")) {
    request->send(400, "text/plain", "Missing tft_ticker_text");
    return;
  }
  String s = request->getParam("tft_ticker_text")->value();
  if (s.length() > 120) s.remove(120);
  jsonWrite(configST7789, "tft_ticker_text", s);
  saveConfig();
  s.toCharArray(TFTTickerText, sizeof(TFTTickerText));
  if (st7789Enabled) {
    tftTickerStop();
    tftTickerNextStart = millis() + tftTickerPeriodMs();
  }
  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

void handle_tft_brightness(AsyncWebServerRequest *request) {
  if (!request->hasParam("tft_brightness")) {
    request->send(400, "text/plain", "Missing parameter");
    return;
  }
  uint8_t brightness = constrain(request->getParam("tft_brightness")->value().toInt(), 0, 255);
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
  request->send(200, "text/plain", "OK");
}

void handle_tft_auto_brightness(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value");
    return;
  }
  bool enable = (request->getParam("value")->value() == "1");
  if (enable != tft_auto_brightness) {
    tft_auto_brightness = enable;
    jsonWrite(configST7789, "tft_auto_brightness", enable ? "1" : "0");
    saveConfig();
    if (st7789Enabled) {
      TFT_SetAutoBrightness(enable);
    }
  }
  request->send(200, "text/plain", "OK");
}

#endif // USE_ST7789
// --------------------------------
void handle_save_brightness(AsyncWebServerRequest *request) {
  if (!request->hasParam("day_bright") || !request->hasParam("night_bright")) {
    request->send(400, "text/plain", "Missing brightness parameters");
    return;
  }
  uint8_t day_bright = constrain(request->getParam("day_bright")->value().toInt(), 0, 255);
  uint8_t night_bright = constrain(request->getParam("night_bright")->value().toInt(), 0, 255);
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

  request->send(200, ("text/plain"), ("OK"));
}

// ----------------------------------------------------------------------- НАСТРОЙКИ TM1637 ------------------------------------------------------------

#if USE_TM1637
void handle_tm1637_brightness(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value");
    return;
  }
  uint8_t brightness = constrain(request->getParam("value")->value().toInt(), 0, 255);
  if (brightness != userTm1637Brightness) {
    userTm1637Brightness = brightness;
    jsonWrite(configSetup, "tm1637_brightness", String(brightness));
    if (userTm1637AutoBrightness) {
      userTm1637AutoBrightness = false;
      jsonWrite(configSetup, "tm1637_auto_brightness", "0");
    }
    saveConfig();
    if (tm1637Enabled) {
      clockTicker_blink();
    }
  }
  request->send(200, "text/plain", String(brightness));
}

void handle_tm1637_auto_brightness(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value");
    return;
  }
  bool enable = (request->getParam("value")->value() == "1");
  if (enable != userTm1637AutoBrightness) {
    userTm1637AutoBrightness = enable;
    jsonWrite(configSetup, "tm1637_auto_brightness", enable ? "1" : "0");
    saveConfig();
    if (tm1637Enabled) {
      clockTicker_blink();
    }
  }
  request->send(200, "text/plain", enable ? "1" : "0");
}
#endif

// ---------------------------------------------------------------------- НАСТРОЙКИ МАТРИЦЫ ------------------------------------------------------------

// Включение/выключение использования поддержки нескольких матриц (рантайм)
#if MULTI_MATRIX
void handle_multi_matrix_enable(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value");
    return;
  }

  String mmVal = request->getParam("value")->value();
  bool newVal = (mmVal == "1" || mmVal.equalsIgnoreCase("true"));

  if (newVal == multiMatrixEnabled) {
    request->send(200, "text/plain", "NO_CHANGE");
    return;
  }

  String configLED = readFile(F("config_led_matrix.json"), 8192);
  if (configLED == "Failed" || configLED == "Large") configLED = "{}";

  jsonWrite(configLED, "multi_matrix", newVal ? 1 : 0);
  writeFile(F("config_led_matrix.json"), configLED);

  multiMatrixEnabled = newVal;

#if MATRIX_LOG
  SYSLOG.add("multiMatrixEnabled: %d (restart)", multiMatrixEnabled);
#endif

  request->send(200, "text/plain", "OK_REBOOT");
  pendingRestart = true;
}
#endif

// -----------------------------------

void handle_matrix_tipe(AsyncWebServerRequest *request) {
  if (!request->hasParam("m_t")) {
    request->send(400, "text/plain", "Missing m_t");
    return;
  }
  String configLED = readFile(F("config_led_matrix.json"), 8192);
  if (configLED == "Failed" || configLED == "Large") {
    configLED = "{}";
  }

  int newType = request->getParam("m_t")->value().toInt();
  if (newType != 0 && newType != 1) {
    newType = 0;
  }

  String oldType = jsonRead(configLED, "m_t");
  if (oldType.toInt() != newType) {
    jsonWrite(configLED, "m_t", newType);
    writeFile(F("config_led_matrix.json"), configLED);
    MatrixType = newType;
    request->send(200, "text/plain", "OK");
  } else {
    request->send(200, "text/plain", "NO_CHANGE");
  }
}

void handle_matrix_orientation(AsyncWebServerRequest *request) {
  if (!request->hasParam("m_o")) {
    request->send(400, "text/plain", "Missing m_o");
    return;
  }
  String configLED = readFile(F("config_led_matrix.json"), 8192);
  if (configLED == "Failed" || configLED == "Large") configLED = "{}";

  int newOri = request->getParam("m_o")->value().toInt();
  if (newOri < 0 || newOri > 7) newOri = 0;

  String oldOri = jsonRead(configLED, "m_o");
  if (oldOri.toInt() != newOri) {
    jsonWrite(configLED, "m_o", newOri);
    writeFile(F("config_led_matrix.json"), configLED);
    MatrixOrientation = newOri;
    loadingFlag = true;
    needFullRedraw = true;
    request->send(200, "text/plain", "OK");
  } else {
    request->send(200, "text/plain", "NO_CHANGE");
  }
}

// -------------------------------------------------------------------------- НАСТРОЙКИ MQTT -----------------------------------------------------------

#if USE_MQTT
void handle_mqtt_set(AsyncWebServerRequest *request) {
  String mq_ip = request->hasParam("mq_ip") ? request->getParam("mq_ip")->value() : "";
  uint16_t mq_port = request->hasParam("mq_port") ? request->getParam("mq_port")->value().toInt() : 1883;
  String mq_user = request->hasParam("mq_user") ? request->getParam("mq_user")->value() : "";
  String mq_pass = request->hasParam("mq_pass") ? request->getParam("mq_pass")->value() : "";
  String mq_topic = request->hasParam("topic") ? request->getParam("topic")->value() : "";

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
  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

void handle_mqtt_on(AsyncWebServerRequest *request) {
  if (!request->hasParam("mq_on")) {
    request->send(400, "text/plain", "Missing mq_on");
    return;
  }
  MqttOn = request->getParam("mq_on")->value().toInt();
  jsonWrite(configMQTT, "mq_on", MqttOn);
  writeFile(F("config_mqtt.json"), configMQTT);
  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

void handle_mqtt_period(AsyncWebServerRequest *request) {
  if (!request->hasParam("mq_prd")) {
    request->send(400, "text/plain", "Missing mq_prd");
    return;
  }
  MqttPeriod = request->getParam("mq_prd")->value().toInt();
  if (MqttPeriod < 10) MqttPeriod = 10;
  if (MqttPeriod > 3600) MqttPeriod = 3600;

  jsonWrite(configMQTT, "mq_prd", MqttPeriod);
  writeFile(F("config_mqtt.json"), configMQTT);
  request->send(200, F("application/json"), F("{\"should_refresh\": \"true\"}"));
}

#endif // USE_MQTT

// -------------------------------------------------------------------- ЧТЕНИЕ ФАЙЛОВ ФОЛРМАТА .OUT ---------------------------------------------------

#if USE_SD
void handle_list_out_files(AsyncWebServerRequest *request) {
  if (!sdEnabled) {
    request->send(200, "application/json", "[]");
    return;
  }

  String fileList = "[";
  bool first = true;
  File root;

  String sizeFolder = String(matrixWidth) + "x" + String(matrixHeight);
  String effectsPath = "/effects/" + sizeFolder;

  auto openDir = [](const String & path) -> File {
    if (sdType == 1) return LittleFS.open(path);
    else return SD.open(path);
  };

  root = openDir(effectsPath);
  if (!root || !root.isDirectory()) {
    if (root) root.close();
    root = openDir("/effects/");
    if (!root || !root.isDirectory()) {
      if (root) root.close();
      request->send(200, "application/json", "[]");
      return;
    }
  }

  while (true) {
    File file = root.openNextFile();
    if (!file) break;
    String name = file.name();
    int lastSlash = name.lastIndexOf('/');
    if (lastSlash >= 0) name = name.substring(lastSlash + 1);
    if (name.endsWith(".out")) {
      if (!first) fileList += ",";
      fileList += "\"" + name + "\"";
      first = false;
    }
    file.close();
  }
  root.close();
  fileList += "]";
  request->send(200, "application/json", fileList);
}
#endif // USE_SD

// --------------------------------------------------------- СТАТУСЫ УСТРОЙСТВ В МОДАЛЬНОМ ОКНЕ МЕНЮ --------------------------------------------------

#if STATUS_DEVICE
// статус кнопки
void handle_button_status(AsyncWebServerRequest *request) {
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
  request->send(200, "application/json; charset=utf-8", response);
}

// статус IR, RF
void handle_ir_rf_status(AsyncWebServerRequest *request) {
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
  request->send(200, "application/json; charset=utf-8", response);
}

// статус дисплея tm1637
void handle_tm1637_status(AsyncWebServerRequest *request) {
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
  request->send(200, "application/json; charset=utf-8", response);
}

// статус дисплея st7789
void handle_st7789_status(AsyncWebServerRequest *request) {
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
  request->send(200, "application/json; charset=utf-8", response);
}

// статус мп3 плеера
void handle_mp3_status(AsyncWebServerRequest *request) {
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
  request->send(200, "application/json; charset=utf-8", response);
}

// статус Радио
#if USE_RADIO
void handle_radio_status(AsyncWebServerRequest *request) {
  DynamicJsonDocument doc(256);
  doc["enabled"] = radioEnabled;
  doc["playing"] = radioPlaying;
  doc["paused"] = !radioPlaying && radioInited && currentStation > 0;
  doc["ready"] = radioInited;
  doc["station"] = currentStation;
  doc["volume"] = radioVolume;
  doc["station_name"] = (currentStation > 0) ? getStationName(currentStation - 1) : "";

  String resp;
  serializeJson(doc, resp);
  request->send(200, "application/json; charset=utf-8", resp);
}
#endif

// статус SD
void handle_sd_status(AsyncWebServerRequest *request) {
  DynamicJsonDocument doc(512);
  String sd_status;

#if USE_SD
  if (!sdEnabled) {
    sd_status = "ВЫКЛЮЧЕНО";
    doc["fs_status"] = "—";
  } else if (sdType == 1) {
    sd_status = "ЭМУЛЯЦИЯ В LITTLEFS";
    doc["fs_status"] = LittleFS.begin() ? "ОК" : "ОШИБКА";
  } else {
    sd_status = sd_card_present ? "ОК" : "КАРТА НЕ НАЙДЕНА";
    doc["fs_status"] = sd_card_present ? "ОК" : "ОШИБКА";
  }
#else
  sd_status = "ОТКЛЮЧЕНО В ПРОШИВКЕ";
#endif

  doc["sd_status"] = sd_status;
  doc["sd_enabled"] = sdEnabled;
  doc["sd_type"] = sdType;
  doc["card_present"] = sd_card_present;
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json; charset=utf-8", response);
}

// статус погоды
void handle_show_weather(AsyncWebServerRequest *request) {
#if USE_WEATHER
  if (!request->hasParam("show_weather")) {
    request->send(400, "text/plain", "Missing show_weather");
    return;
  }
  int enabled = request->getParam("show_weather")->value().toInt();
  inClockWeatherMode = (enabled == 1);
  jsonWrite(configWeather, "show_weather", enabled);
  saveConfig();
  if (inClockWeatherMode && Wifi::instance().isConnected()) {
    Weather::instance().forceUpdate();
  }
  request->send(200, "text/plain", "OK");
#else
  request->send(404, "text/plain", "Weather disabled");
#endif
}

// статус OTA
uint8_t otaProgress = 0;
String  otaStatus = "ГОТОВ";
void handle_ota_status(AsyncWebServerRequest *request) {
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
  request->send(200, "application/json; charset=utf-8", resp);
}

// статус MQTT
void handle_mqtt_status(AsyncWebServerRequest *request) {
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
  request->send(200, "application/json; charset=utf-8", resp);
}

// статус режим "Мультилампа"
void handle_multilamp_status(AsyncWebServerRequest *request) {
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
  request->send(200, "application/json; charset=utf-8", resp);
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
void handle_run_text_enabled(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value");
    return;
  }
  String arg = request->getParam("value")->value();
  bool newVal = (arg == "1" || arg.toInt() == 1);
  if (newVal == runTextEnabled) {
    request->send(200, "text/plain", "OK");
    return;
  }

  runTextEnabled = newVal;

  if (!runTextEnabled) {
    // полное выключение бегущей строки
    runTextOver = false; // снять галочку и с "поверх эффекта"
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

  request->send(200, "text/plain", "OK");

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
void handle_run_text_over(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value");
    return;
  }
  String arg = request->getParam("value")->value();
  bool newVal = (arg == "1" || arg == "true" || arg.toInt() == 1);
  if (newVal == runTextOver) {
    request->send(200, "text/plain", "OK");
    return;
  }

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

  request->send(200, "text/plain", "OK");

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
void handle_run_text (AsyncWebServerRequest *request)  {
  if (!request->hasParam("run_text")) {
    request->send(400, "text/plain", "Missing run_text");
    return;
  }
  jsonWrite(configLedPanel, "run_text", request->getParam("run_text")->value());
  timeout_save_file_changes = millis();
  bitSet (save_file_changes, 0);
  (jsonRead(configLedPanel, "run_text")).toCharArray (TextTicker, (jsonRead(configLedPanel, "run_text")).length() + 1);
  request->send(200, ("text/plain"), ("OK"));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Интервал вывода бегущей строки
void handle_interval_run_text(AsyncWebServerRequest *request) {
  if (!request->hasParam("interval_run_text")) {
    request->send(400, F("text/plain"), F("Missing parameter: interval_run_text"));
    return;
  }
  String arg = request->getParam("interval_run_text")->value();
  int newInterval = arg.toInt();
  newInterval = constrain(newInterval, 0, 60);

  if (newInterval == IntervalrunText) {
    request->send(200, ("text/plain"), ("OK"));
    return;
  }

  IntervalrunText = newInterval;
  jsonWrite(configLedInterval, "interval_run_text", IntervalrunText);
  saveConfig();
  ApplyRunningTextSettings();
  request->send(200, ("text/plain"), ("OK"));

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
void handle_text_y_offset(AsyncWebServerRequest *request) {
  if (!request->hasParam("text_y_offset")) {
    request->send(400, "text/plain", "Parameter missing");
    return;
  }

  int value = request->getParam("text_y_offset")->value().toInt();
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

  request->send(200, "text/plain", String(value));

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
void handle_spt(AsyncWebServerRequest *request) {
  if (!request->hasParam("spt")) {
    request->send(400);
    return;
  }
  int newSpeed = constrain(request->getParam("spt")->value().toInt(), 20, 220);
  if (newSpeed == SpeedRunningText) {
    request->send(200, "text/plain", String(newSpeed));
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
  request->send(200, "text/plain", String(newSpeed));

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
void handle_sct(AsyncWebServerRequest *request) {
  if (!request->hasParam("sct")) {
    request->send(400);
    return;
  }
  int value = constrain(request->getParam("sct")->value().toInt(), 0, 255);

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

  request->send(200, "text/plain", String(sctForDisplay));
}

// Шрифт бегущей строки
void handle_font_size(AsyncWebServerRequest *request) {
  if (!request->hasParam("font_size")) {
    request->send(400, "text/plain", "Missing font_size parameter");
    return;
  }

  String arg = request->getParam("font_size")->value();
  if (arg.length() == 0) {
    request->send(400, "text/plain", "Empty font_size value");
    return;
  }

  int val = arg.toInt();
  if (val < 0 || val > 2) {
    request->send(400, "text/plain", "Invalid font_size value (must be 0–2)");
    return;
  }

  uint8_t newSize = (uint8_t)val;

  if (newSize == currentFont) {
    request->send(200, "text/plain", "OK (no change)");
    return;
  }

  jsonWrite(configLedPanel, "font_size", newSize);
  saveConfig();

  setFontSize(newSize);

  request->send(200, "text/plain", "OK");

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
void handle_auto_move_clock(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value");
    return;
  }
  String arg = request->getParam("value")->value();
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

  request->send(200, "text/plain", "OK");

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
void handle_print_time(AsyncWebServerRequest *request) {
  if (!request->hasParam("print_time")) {
    request->send(400, "text/plain", "Missing print_time");
    return;
  }
  int val = request->getParam("print_time")->value().toInt();
  if (val < 1 || val > 60) val = 30;
  PRINT_TIME = request->getParam("print_time")->value().toInt();
  jsonWrite(configLedInterval, "print_time", PRINT_TIME);
  saveConfig();
  request->send(200, "text/plain", "OK");
}

// Чекбокс "Часы бегущей строкой"
void handle_run_time_text_enabled(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value parameter");
    return;
  }

  String valStr = request->getParam("value")->value();
  valStr.toLowerCase();
  bool newVal = (valStr == "1" || valStr == "true" || valStr == "on");
  runTimeTextEnabled = newVal;

  if (runTimeTextEnabled) {
    timeTextJustEnabled = true;
  }

  jsonWrite(configLedPanel, "run_time_text_enabled", runTimeTextEnabled ? "1" : "0");
  saveConfig();
  request->send(200, "text/plain", runTimeTextEnabled ? "1" : "0");

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
void handle_clock_leading_zero(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value parameter");
    return;
  }
  String valStr = request->getParam("value")->value();
  bool newVal = (valStr == "1" || valStr == "true" || valStr == "on");

  if (clockLeadingZero == newVal) {
    request->send(200, "text/plain", clockLeadingZero ? "1" : "0");
    return;
  }

  clockLeadingZero = newVal;
  jsonWrite(configLedPanel, "clock_leading_zero", clockLeadingZero ? "1" : "0");
  saveConfig();
  loadingFlag = true;

  request->send(200, "text/plain", clockLeadingZero ? "1" : "0");

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
void handle_clock_vert(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value");
    return;
  }
  String arg = request->getParam("value")->value();
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

  request->send(200, "text/plain", "OK");

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Чекбокс "Включить ночные часы"
void handle_night_clock_enabled(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400);
    return;
  }
  uint8_t val = request->getParam("value")->value().toInt();
  bool newVal;

  if (val == 2) {
    newVal = !nightClockEnabled;
  } else {
    newVal = (val == 1);
  }

  if (newVal == nightClockEnabled) {
    request->send(200, "text/plain", nightClockEnabled ? "1" : "0");
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
    jsonWrite(configSetup, "br", nightModeBrightness);
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
    jsonWrite(configSetup, "br", normalBrightness);
    FastLED.setBrightness(normalBrightness);
    FastLED.show();
    jsonWrite(configSetup, "br", normalBrightness);
  }

  request->send(200, "text/plain", nightClockEnabled ? "1" : "0");

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
void handle_night_clock_brightness(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400);
    return;
  }
  uint8_t newVal = request->getParam("value")->value().toInt();
  if (newVal < 1 || newVal > 255) {
    newVal = 1;
  }
  if (newVal == nightClockBrightness) {
    request->send(200, "text/plain", String(nightClockBrightness));
    return;
  }
  nightClockBrightness = newVal;
  jsonWrite(configLedPanel, "night_clock_brightness", nightClockBrightness);
  saveConfig();

  if (nightClockEnabled && currentMode == eff_num_correct[EFF_CLOCK]) {
    nightModeBrightness = nightClockBrightness;
    modes[currentMode].Brightness = nightModeBrightness;
    jsonWrite(configSetup, "br", nightModeBrightness);
    FastLED.setBrightness(nightModeBrightness);
    FastLED.show();
  }

  request->send(200, "text/plain", String(nightClockBrightness));

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
void handle_clock_x_offset(AsyncWebServerRequest *request) {
  if (!request->hasParam("clock_x_offset")) {
    request->send(400, "text/plain", "Parameter missing");
    return;
  }

  int value = request->getParam("clock_x_offset")->value().toInt();
  value = constrain(value, -50, 50);

  if (clockXOffset != value) {
    clockXOffset = value;
    jsonWrite(configLedPanel, "clock_x_offset", value);
    saveConfig();
    loadingFlag = true;
    clockNeedRedraw = true;
    needFullRedraw = true;
  }

  request->send(200, "text/plain", String(value));

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
void handle_clock_y_offset(AsyncWebServerRequest *request) {
  if (!request->hasParam("clock_y_offset")) {
    request->send(400, "text/plain", "Parameter missing");
    return;
  }

  int value = request->getParam("clock_y_offset")->value().toInt();
  value = constrain(value, -20, 20);

  if (clockYOffset != value) {
    clockYOffset = value;
    jsonWrite(configLedPanel, "clock_y_offset", value);
    saveConfig();
    loadingFlag = true;
    clockNeedRedraw = true;
    needFullRedraw = true;
  }

  request->send(200, "text/plain", String(value));

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

// Оттенок часов
void handle_clock_hue(AsyncWebServerRequest *request) {
  if (!request->hasParam("clock_hue")) {
    request->send(400);
    return;
  }

  int value = constrain(request->getParam("clock_hue")->value().toInt(), 0, 255);

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

  request->send(200, "text/plain", String(value));

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
void handle_print_weather(AsyncWebServerRequest *request) {
  if (!request->hasParam("print_weather")) {
    request->send(400, "text/plain", "Missing print_weather");
    return;
  }
  int val = request->getParam("print_weather")->value().toInt();
  if (val < 1 || val > 60) val = 30;
  PRINT_WEATHER = request->getParam("print_weather")->value().toInt();
  jsonWrite(configLedInterval, "print_weather", PRINT_WEATHER);
  saveConfig();
  request->send(200, "text/plain", "OK");
}

// Чекбокс "Погода бегущей строкой"
void handle_run_weather_text_enabled(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value parameter");
    return;
  }

  String valStr = request->getParam("value")->value();
  valStr.toLowerCase();
  bool newVal = (valStr == "1" || valStr == "true" || valStr == "on");
  runWeatherTextEnabled = newVal;
  jsonWrite(configLedPanel, "run_weather_text_enabled", runWeatherTextEnabled ? "1" : "0");
  saveConfig();

  request->send(200, "text/plain", runWeatherTextEnabled ? "1" : "0");

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
void handle_weather_enabled(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, F("text/plain"), F("Parameter missing"));
    return;
  }
  bool newVal = request->getParam("value")->value() == "1";

  if (newVal == weatherEnabled) {
    request->send(200, "text/plain", "OK");
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

  request->send(200, "text/plain", "OK");

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
void handle_weather_hue(AsyncWebServerRequest *request) {
  if (!request->hasParam("weather_hue")) {
    request->send(400);
    return;
  }

  int value = constrain(request->getParam("weather_hue")->value().toInt(), 0, 255);

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

  request->send(200, "text/plain", String(weatherHue));

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
void handle_weather_x_offset(AsyncWebServerRequest *request) {
  if (!request->hasParam("weather_x_offset")) {
    request->send(400, "text/plain", "Parameter missing");
    return;
  }

  int value = request->getParam("weather_x_offset")->value().toInt();

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

  request->send(200, "text/plain", String(value));

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
void handle_weather_y_offset(AsyncWebServerRequest *request) {
  if (!request->hasParam("weather_y_offset")) {
    request->send(400, "text/plain", "Parameter missing");
    return;
  }

  int value = request->getParam("weather_y_offset")->value().toInt();

  value = constrain(value, -16, 16);

  if (weatherYOffset != value) {
    weatherYOffset = value;
    jsonWrite(configLedPanel, "weather_y_offset", String(value));
    saveConfig();

    loadingFlag = true;
    weatherNeedRedraw = true;
  }

  request->send(200, "text/plain", String(value));
}

// Чекбокс "Мигание °"
void handle_degree_blink(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400);
    return;
  }
  bool newVal = request->getParam("value")->value() == "1";
  if (newVal == degreeSymbolBlinking) {
    request->send(200, "text/plain", degreeSymbolBlinking ? "1" : "0");
    return;
  }
  degreeSymbolBlinking = newVal;
  jsonWrite(configLedPanel, "degree_blink", degreeSymbolBlinking ? "1" : "0");
  saveConfig();
  loadingFlag = true;

  request->send(200, "text/plain", degreeSymbolBlinking ? "1" : "0");

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
void handle_date_enabled(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, F("text/plain"), F("Parameter missing"));
    return;
  }
  bool newVal = request->getParam("value")->value() == "1";
  if (newVal == dateEnabled) {
    request->send(200, "text/plain", "OK");
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
  request->send(200, "text/plain", "OK");

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
void handle_date_hue(AsyncWebServerRequest *request) {
  if (!request->hasParam("date_hue")) {
    request->send(400);
    return;
  }

  int value = constrain(request->getParam("date_hue")->value().toInt(), 0, 255);

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

  request->send(200, "text/plain", String(dateHue));

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
void handle_date_x_offset(AsyncWebServerRequest *request) {
  if (!request->hasParam("date_x_offset")) {
    request->send(400, "text/plain", "Parameter missing");
    return;
  }

  int value = request->getParam("date_x_offset")->value().toInt();

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

  request->send(200, "text/plain", String(value));

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
void handle_date_y_offset(AsyncWebServerRequest *request) {
  if (!request->hasParam("date_y_offset")) {
    request->send(400, "text/plain", "Parameter missing");
    return;
  }

  int value = request->getParam("date_y_offset")->value().toInt();

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

  request->send(200, "text/plain", String(value));
}

// Чекбокс "Показывать 4 цифры года"
void handle_date_full_year(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value parameter");
    return;
  }

  String valStr = request->getParam("value")->value();
  bool newVal = (valStr == "1" || valStr == "true" || valStr == "on");

  if (showFullYearEnabled == newVal) {
    request->send(200, "text/plain", showFullYearEnabled ? "1" : "0");
    return;
  }

  showFullYearEnabled = newVal;
  jsonWrite(configLedPanel, "date_full_year", showFullYearEnabled ? "1" : "0");
  saveConfig();

  loadingFlag = true;

  request->send(200, "text/plain", showFullYearEnabled ? "1" : "0");

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
void handle_date_separator_blink(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400);
    return;
  }
  bool newVal = request->getParam("value")->value() == "1";
  if (newVal == dateSeparatorBlinking) {
    request->send(200, "text/plain", dateSeparatorBlinking ? "1" : "0");
    return;
  }
  dateSeparatorBlinking = newVal;
  jsonWrite(configLedPanel, "date_separator_blink", dateSeparatorBlinking ? "1" : "0");
  saveConfig();
  loadingFlag = true;
  request->send(200, "text/plain", dateSeparatorBlinking ? "1" : "0");

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
void handle_date_show_year(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400);
    return;
  }
  bool newVal = request->getParam("value")->value() == "1";
  if (newVal == showYearInDate) {
    request->send(200, "text/plain", showYearInDate ? "1" : "0");
    return;
  }
  showYearInDate = newVal;
  jsonWrite(configLedPanel, "date_show_year", showYearInDate ? "1" : "0");
  saveConfig();
  loadingFlag = true;
  request->send(200, "text/plain", showYearInDate ? "1" : "0");

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
void handle_timer_c_w(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, F("text/plain"), F("Missing value"));
    return;
  }

  bool newVal = request->getParam("value")->value().toInt() != 0;
  if (newVal == timer_c_w) {
    request->send(200, F("text/plain"), timer_c_w ? F("1") : F("0"));
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

  request->send(200, F("text/plain"), timer_c_w ? F("1") : F("0"));

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
void handle_interval_c_w(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, F("text/plain"), F("Missing value"));
    return;
  }
  String valStr = request->getParam("value")->value();
  if (valStr.length() == 0) {
    request->send(400, F("text/plain"), F("Empty value"));
    return;
  }
  int newVal = valStr.toInt();
  if (newVal == 0 && valStr != "0") {
    request->send(400, F("text/plain"), F("Invalid number format"));
    return;
  }
  newVal = constrain(newVal, 5, 600);
  if (newVal == interval_c_w) {
    request->send(200, F("text/plain"), String(newVal).c_str());
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

  request->send(200, F("text/plain"), String(newVal).c_str());

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
void handle_timer_d_w(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, F("text/plain"), F("Missing value"));
    return;
  }

  bool newVal = request->getParam("value")->value().toInt() != 0;
  if (newVal == timer_d_w) {
    request->send(200, F("text/plain"), timer_d_w ? F("1") : F("0"));
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

  request->send(200, F("text/plain"), timer_d_w ? F("1") : F("0"));

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
void handle_interval_d_w(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, F("text/plain"), F("Missing value"));
    return;
  }
  String valStr = request->getParam("value")->value();
  if (valStr.length() == 0) {
    request->send(400, F("text/plain"), F("Empty value"));
    return;
  }
  int newVal = valStr.toInt();
  if (newVal == 0 && valStr != "0") {
    request->send(400, F("text/plain"), F("Invalid number format"));
    return;
  }
  newVal = constrain(newVal, 5, 600);
  if (newVal == interval_d_w) {
    request->send(200, F("text/plain"), String(newVal).c_str());
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

  request->send(200, F("text/plain"), String(newVal).c_str());

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
void handle_timer_c_d_w(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, F("text/plain"), F("Missing value"));
    return;
  }

  bool newVal = request->getParam("value")->value().toInt() != 0;
  if (newVal == timer_c_d_w) {
    request->send(200, F("text/plain"), timer_c_d_w ? F("1") : F("0"));
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

  request->send(200, F("text/plain"), timer_c_d_w ? F("1") : F("0"));

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
void handle_interval_c_d_w(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, F("text/plain"), F("Missing value"));
    return;
  }
  String valStr = request->getParam("value")->value();
  if (valStr.length() == 0) {
    request->send(400, F("text/plain"), F("Empty value"));
    return;
  }
  int newVal = valStr.toInt();
  if (newVal == 0 && valStr != "0") {
    request->send(400, F("text/plain"), F("Invalid number format"));
    return;
  }
  newVal = constrain(newVal, 5, 600);
  if (newVal == interval_c_d_w) {
    request->send(200, F("text/plain"), String(newVal).c_str());
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

  request->send(200, F("text/plain"), String(newVal).c_str());

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
void handle_timer_clock_fixed(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, F("text/plain"), F("Missing value"));
    return;
  }

  bool newVal = request->getParam("value")->value().toInt() != 0;
  if (newVal == timer_clock_fixed) {
    request->send(200, F("text/plain"), timer_clock_fixed ? F("1") : F("0"));
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

  request->send(200, F("text/plain"), timer_clock_fixed ? F("1") : F("0"));

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
void handle_interval_clock_fixed(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, F("text/plain"), F("Missing value"));
    return;
  }
  String valStr = request->getParam("value")->value();
  if (valStr.length() == 0) {
    request->send(400, F("text/plain"), F("Empty value"));
    return;
  }
  int newVal = valStr.toInt();
  if (newVal == 0 && valStr != "0") {
    request->send(400, F("text/plain"), F("Invalid number format"));
    return;
  }
  newVal = constrain(newVal, 5, 600);
  if (newVal == interval_clock_fixed) {
    request->send(200, F("text/plain"), String(newVal).c_str());
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

  request->send(200, F("text/plain"), String(newVal).c_str());

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
void handle_timer_c_d(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, F("text/plain"), F("Missing value"));
    return;
  }

  bool newVal = request->getParam("value")->value().toInt() != 0;
  if (newVal == timer_c_d) {
    request->send(200, F("text/plain"), timer_c_d ? F("1") : F("0"));
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

  request->send(200, F("text/plain"), timer_c_d ? F("1") : F("0"));

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
void handle_interval_c_d(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, F("text/plain"), F("Missing value"));
    return;
  }
  String valStr = request->getParam("value")->value();
  if (valStr.length() == 0) {
    request->send(400, F("text/plain"), F("Empty value"));
    return;
  }
  int newVal = valStr.toInt();
  if (newVal == 0 && valStr != "0") {
    request->send(400, F("text/plain"), F("Invalid number format"));
    return;
  }
  newVal = constrain(newVal, 5, 600);
  if (newVal == interval_c_d) {
    request->send(200, F("text/plain"), String(newVal).c_str());
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

  request->send(200, F("text/plain"), String(newVal).c_str());

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

void handle_index (AsyncWebServerRequest *request) {
  bool flg = false;
  if (request->hasParam("index") && request->getParam("index")->value().toInt()) {
    flg = FileCopy (F("/index/index.setup.gz") , F("/index.json.gz"));
    LittleFS.remove("/effect2.ini");
  }
  if (flg)  request->send(200, "text/plain", "OK");
  else      request->send(404, "text/plain", "File not found");
}

// ---------------------------------------------------------------

// Пароль на страницу настроек
void handle_PassOn (AsyncWebServerRequest *request) {
  if (!request->hasParam("PassOn")) {
    request->send(400, "text/plain", "Missing PassOn");
    return;
  }
  jsonWrite(configSetup, "PassOn", request->getParam("PassOn")->value().toInt());
  request->send(200, "application/json", "{\"should_refresh\": \"true\"}");
  saveConfig();
}

// ----------------------------------------------------------------------- ЯЗЫК ИНТЕРФЕЙСА ------------------------------------------------------------

void handle_lang (AsyncWebServerRequest *request) {
  if (!request->hasParam("lang")) {
    request->send(400, "text/plain", "Missing lang");
    return;
  }
  jsonWrite(configSetup, "lang", request->getParam("lang")->value());
  saveConfig();
  Lang_set();
  request->send(200, "application/json", "{\"should_refresh\": \"true\"}");
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

void handle_reset_to_default(AsyncWebServerRequest *request) {
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
    {"/default/config_radio.json", "/config_radio.json"},
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
  }

  request->send(200, "text/plain", "OK");
  pendingRestart = true;
}

// ---------------------------------------------------------- ВКЛЮЧЕНИЕ/ВЫКЛЮЧЕНИЕ МОДУЛЕЙ ЧЕРЕЗ ВЕБ --------------------------------------------------

// Кнопка
void handle_button_enable(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value");
    return;
  }
  bool val = request->getParam("value")->value().toInt() != 0;
  if (val != buttonEnabled) {
    buttonEnabled = val;
    jsonWrite(configSetup, "button_enabled", buttonEnabled ? "1" : "0");
    saveConfig();
  }
  request->send(200, "text/plain", buttonEnabled ? "1" : "0");
}

// IR-приёмник
void handle_ir_enable(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value");
    return;
  }
  bool val = request->getParam("value")->value().toInt() != 0;
  if (val != irEnabled) {
    irEnabled = val;
    jsonWrite(configSetup, "ir_enabled", irEnabled ? "1" : "0");
    saveConfig();
  }
  request->send(200, "text/plain", irEnabled ? "1" : "0");
}

// RF-приёмник
void handle_rf_enable(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value");
    return;
  }
  bool val = request->getParam("value")->value().toInt() != 0;
  if (val != rfEnabled) {
    rfEnabled = val;
    jsonWrite(configSetup, "rf_enabled", rfEnabled ? "1" : "0");
    saveConfig();
  }
  request->send(200, "text/plain", rfEnabled ? "1" : "0");
}

// TM1637 дисплей
void handle_tm1637_enable(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value");
    return;
  }
  bool val = request->getParam("value")->value().toInt() != 0;
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
  request->send(200, "text/plain", tm1637Enabled ? "1" : "0");
}

// ST7789 дисплей
void handle_st7789_enable(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value");
    return;
  }
  bool val = request->getParam("value")->value().toInt() != 0;
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
  request->send(200, "text/plain", st7789Enabled ? "1" : "0");
}

// MP3 плеер
void handle_mp3_enable(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value");
    return;
  }
  bool val = request->getParam("value")->value().toInt() != 0;

  if (val == mp3Enabled) {
    request->send(200, "text/plain", mp3Enabled ? "1" : "0");
    return;
  }

  if (val) {
    setAudioSource(AUDIO_SOURCE_MP3);
  } else {
    disableAudioSource(AUDIO_SOURCE_MP3);
  }

  request->send(200, "text/plain", mp3Enabled ? "1" : "0");
}

// SD карта
void handle_sd_enable(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400, "text/plain", "Missing value");
    return;
  }
  bool val = request->getParam("value")->value().toInt() != 0;

#if USE_SD
  if (val != sdEnabled) {
    sdEnabled = val;
    jsonWrite(configSetup, "sd_enabled", sdEnabled ? "1" : "0");
    saveConfig();

    if (!sdEnabled) {
      if (outFile) {
        outFile.close();
        outAnimationActive = false;
      }
      outEffectActive = false;
      if (currentMode == EFF_SD) {
        currentMode = EFF_RAINBOW_VER;
        for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
          if (eff_num_correct[i] == currentMode) {
            jsonWrite(configSetup, "eff_sel", i);
            break;
          }
        }
        saveConfig();
        loadingFlag = true;
      }
    } else {
      initSD();
    }
  }
  request->send(200, "text/plain", sdEnabled ? "1" : "0");
#else
  request->send(200, "text/plain", "Not supported");
#endif
}

void handle_sd_type(AsyncWebServerRequest *request) {
  if (!request->hasParam("sd_type")) {
    request->send(400, "text/plain", "Missing sd_type");
    return;
  }
  uint8_t val = constrain(request->getParam("sd_type")->value().toInt(), 0, 1);

#if USE_SD && (defined(ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ARDUINO_ESP32S3_DEV))
  if (val == sdType) {
    request->send(200, "text/plain", String(sdType));
    return;
  }

  if (outFile) {
    outFile.close();
    outAnimationActive = false;
  }
  outEffectActive = false;

  if (currentMode == EFF_SD) {
    currentMode = EFF_RAINBOW_VER;
    for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
      if (eff_num_correct[i] == currentMode) {
        jsonWrite(configSetup, "eff_sel", i);
        break;
      }
    }
  }

  sdType = val;
  jsonWrite(configSetup, "sd_type", sdType);
  saveConfig();

  request->send(200, "text/plain", "OK_REBOOT");
  pendingRestart = true;
#else
  request->send(200, "text/plain", "Not supported on this build");
#endif
}

// Радио
#if USE_RADIO
void handle_radio_enable(AsyncWebServerRequest *request) {
  if (!request->hasParam("value")) {
    request->send(400);
    return;
  }
  bool val = request->getParam("value")->value().toInt() != 0;

  if (val == radioEnabled) {
    request->send(200, "text/plain", radioEnabled ? "1" : "0");
    return;
  }

  if (val) {
    setAudioSource(AUDIO_SOURCE_RADIO);
  } else {
    disableAudioSource(AUDIO_SOURCE_RADIO);
  }

  request->send(200, "text/plain", radioEnabled ? "1" : "0");
}
#endif

// *****************************************************************************************************************************************************

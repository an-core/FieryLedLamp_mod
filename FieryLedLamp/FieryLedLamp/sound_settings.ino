// ************************************************************************* sound_settings.ino *********************************************************
#include "Constants.h"
#include "SystemLog.h"
#include "Extern.h"
#include "Prototypes.h"
#include "Time.h"
// -----------------------------

#if USE_MP3_PLAYER
// -------------------
#if USE_WEATHER
uint16_t weatherTempTrack(int8_t temp) {
  temp = constrain(temp, WEATHER_ADV_MIN_TEMP, WEATHER_ADV_MAX_TEMP);
  return (uint16_t)(WEATHER_ADV_FIRST_FILE + temp - WEATHER_ADV_MIN_TEMP);
}

// ----------------------------------------------------------------------------------
void weatherAdvertAddTrack(uint16_t track) {
  if (weather_advert_count >= (sizeof(weather_advert_tracks) / sizeof(weather_advert_tracks[0]))) return;
  for (uint8_t i = 0; i < weather_advert_count; i++) {
    if (weather_advert_tracks[i] == track) return;
  }
  weather_advert_tracks[weather_advert_count++] = track;
}

// ----------------------------------------------------------------------------------
uint16_t weatherAdvertDescTrackByText(String s) {
  s.trim();
  s.toLowerCase();
  s.replace("ё", "е");

  if (s == "ясно") return 300;
  if (s == "малооблачно" || s == "переменная облачность") return 301;
  if (s == "облачно" || s == "облачно с прояснениями") return 302;
  if (s == "пасмурно") return 303;
  if (s == "дождь" || s == "идет дождь") return 304;
  if (s == "снег" || s == "идет снег") return 305;
  if (s == "туман") return 306;
  if (s == "гроза") return 307;
  if (s == "град") return 308;
  if (s == "смог" || s == "дымка") return 309;
  if (s == "пыльная мгла") return 310;
  if (s == "холодно") return 311;
  if (s == "прохладно") return 312;
  if (s == "комфортно") return 313;
  if (s == "тепло") return 314;
  if (s == "жарко") return 315;
  if (s == "очень жарко") return 316;
  if (s == "морозно") return 317;
  if (s == "мороз") return 318;
  if (s == "сильный мороз") return 319;
  if (s == "экстремальный мороз") return 320;
  if (s == "морось") return 321;
  if (s == "слабый дождь" || s == "небольшой дождь") return 322;
  if (s == "умеренный дождь") return 323;
  if (s == "сильный дождь") return 324;
  if (s == "затяжной сильный дождь" || s == "продолжительный сильный дождь") return 325;
  if (s == "ливень") return 326;
  if (s == "мокрый снег" || s == "дождь со снегом") return 327;
  if (s == "небольшой снег" || s == "слабый снег") return 328;
  if (s == "снегопад") return 329;
  if (s == "дождь, гроза" || s == "дождь с грозой") return 330;
  if (s == "гроза, град" || s == "гроза с градом") return 331;
  if (s == "малооблачно, слабый дождь" || s == "малооблачно, небольшой дождь") return 332;
  if (s == "малооблачно, дождь") return 333;
  if (s == "пасмурно, слабый дождь" || s == "значительная облачность, небольшой дождь") return 334;
  if (s == "пасмурно, сильный дождь" || s == "значительная облачность, сильный дождь") return 335;
  if (s == "пасмурно, сильный дождь, гроза" || s == "сильный дождь с грозой") return 336;
  if (s == "облачно, слабый дождь" || s == "облачно, небольшой дождь") return 337;
  if (s == "облачно, дождь") return 338;
  if (s == "пасмурно, мокрый снег" || s == "пасмурно, дождь со снегом") return 339;
  if (s == "малооблачно, небольшой снег" || s == "малооблачно, слабый снег") return 340;
  if (s == "малооблачно, снег") return 341;
  if (s == "пасмурно, небольшой снег" || s == "пасмурно, слабый снег") return 342;
  if (s == "пасмурно, снегопад") return 343;
  if (s == "облачно, небольшой снег" || s == "облачно, слабый снег") return 344;
  if (s == "облачно, снег") return 345;
  if (s == "пыльная буря") return 346;
  if (s == "песчаная мгла") return 347;
  if (s == "вулканический пепел") return 348;
  if (s == "шторм") return 349;
  if (s == "метель") return 350;
  if (s == "сильная метель") return 351;
  return 0;
}

// ----------------------------------------------------------------------------------
void weatherAdvertAddDescTracks(const String& desc) {
  if (desc.length() == 0) {
#if WEATHER_LOG
    SYSLOG.add("Пустое описание погоды");
#endif
    return;
  }

  String s = desc;
  s.trim();
  s.toLowerCase();
  s.replace("ё", "е");
  s.replace("–", "-");
  s.replace("—", "-");

#if WEATHER_LOG
  SYSLOG.add("Оригинальное описание: '%s'", desc.c_str());
  SYSLOG.add("Обработанное: '%s'", s.c_str());
#endif

  uint16_t exactTrack = weatherAdvertDescTrackByText(s);
  if (exactTrack) {
    weatherAdvertAddTrack(exactTrack);
    return;
  } if (s.indexOf("гроз") >= 0 || s.indexOf("thunder") >= 0) {
    weatherAdvertAddTrack(307);
  } if (s.indexOf("град") >= 0 || s.indexOf("hail") >= 0) {
    weatherAdvertAddTrack(308);
  } if (s.indexOf("ливень") >= 0 || s.indexOf("showers") >= 0) {
    weatherAdvertAddTrack(326);
  } else if (s.indexOf("сильн") >= 0 && (s.indexOf("дожд") >= 0 || s.indexOf("rain") >= 0)) {
    weatherAdvertAddTrack(324);
  } else if (s.indexOf("умерен") >= 0 && (s.indexOf("дожд") >= 0)) {
    weatherAdvertAddTrack(323);
  } else if ((s.indexOf("небольш") >= 0 || s.indexOf("слаб") >= 0 || s.indexOf("light") >= 0) && (s.indexOf("дожд") >= 0 || s.indexOf("rain") >= 0)) {
    weatherAdvertAddTrack(322);
  } else if (s.indexOf("дожд") >= 0 || s.indexOf("rain") >= 0) {
    weatherAdvertAddTrack(304);
  } if (s.indexOf("снегопад") >= 0) {
    weatherAdvertAddTrack(329);
  } else if (s.indexOf("мокрый снег") >= 0 || s.indexOf("wet snow") >= 0) {
    weatherAdvertAddTrack(327);
  } else if ((s.indexOf("небольш") >= 0 || s.indexOf("слаб") >= 0) && s.indexOf("снег") >= 0) {
    weatherAdvertAddTrack(328);
  } else if (s.indexOf("снег") >= 0) {
    weatherAdvertAddTrack(305);
  } if (s.indexOf("метел") >= 0 || s.indexOf("snowstorm") >= 0) {
    weatherAdvertAddTrack((s.indexOf("сильн") >= 0) ? 351 : 350);
  } if (s.indexOf("экстремальн") >= 0 || s.indexOf("strong frost") >= 0) {
    weatherAdvertAddTrack(320);
  } else if (s.indexOf("сильн") >= 0 && s.indexOf("мороз") >= 0) {
    weatherAdvertAddTrack(319);
  } else if (s.indexOf("мороз") >= 0) {
    weatherAdvertAddTrack(318);
  } else if (s.indexOf("очень жарко") >= 0) {
    weatherAdvertAddTrack(316);
  } else if (s.indexOf("жарко") >= 0) {
    weatherAdvertAddTrack(315);
  } if (s.indexOf("пасмурно") >= 0 || s.indexOf("overcast") >= 0) {
    weatherAdvertAddTrack(303);
  } else if (s.indexOf("облачно") >= 0) {
    weatherAdvertAddTrack(302);
  } else if (s.indexOf("малооблачно") >= 0 || s.indexOf("partly cloudy") >= 0) {
    weatherAdvertAddTrack(301);
  } else if (s.indexOf("ясно") >= 0 || s.indexOf("clear") >= 0) {
    weatherAdvertAddTrack(300);
  }
}

// ----------------------------------------------------------------------------------
void playWeatherAdvertTrack(uint16_t track) {
  send_command(0x13, FEEDBACK, (uint8_t)(track >> 8), (uint8_t)(track & 0xFF));
}

// ----------------------------------------------------------------------------------
uint16_t weatherAdvertWaitTime() {
  if (weather_advert_index <= 1) return WEATHER_TEMP_TIMER;
  return WEATHER_DESC_TIMER;
}

// ----------------------------------------------------------------------------------
void start_weather_temp_ADVERT(int8_t temp, bool speakDescription) {
  if (mp3_player_connect != 4 || advert_flag || weather_advert_flag || isAnnouncing) return;
  time_t t = Time::instance().now();
  struct tm tm;
  localtime_r(&t, &tm);
  bool isDay = !((tm.tm_hour >= (NIGHT_HOURS_START / 60)) || (tm.tm_hour < (NIGHT_HOURS_STOP / 60)));
  temp = constrain(temp, WEATHER_ADV_MIN_TEMP, WEATHER_ADV_MAX_TEMP);
  weather_advert_count = 0;
  weatherAdvertAddTrack(weatherTempTrack(temp));
  if (show_weather_desc && speakDescription) weatherAdvertAddDescTracks(Weather::instance().getCondition());
  weather_advert_index = 0;
  weather_advert_state = 1;
  weather_advert_flag = true;
  weather_advert_timer = 0;
}

// ----------------------------------------------------------------------------------
void play_weather(bool force) {
  if (mp3_player_connect != 4) return;
  if (isAnnouncing || advert_flag || weather_advert_flag) return;
  // определение дня/ночи
  time_t t = Time::instance().now();
  struct tm tm;
  localtime_r(&t, &tm);
  bool isDay = (tm.tm_hour >= (NIGHT_HOURS_STOP / 60)) && (tm.tm_hour < (NIGHT_HOURS_START / 60));

  if (!force) {
    if (isDay && !day_weather_temp_on) return;
    if (!isDay && !night_weather_temp_on) return;
  }

  // проверка описания
  bool needDescription = show_weather_desc;
  if (!force) {
    if (isDay && !day_weather_desc_on) needDescription = false;
    if (!isDay && !night_weather_desc_on) needDescription = false;
  }
  if (eff_sound_on && !mp3_stop) { // пауза перед озвучкой (+ запоминание трека)
    saved_mp3_track = send_command(0x4C, 1, 0, 0);
    delay(50);
    saved_mp3_folder = mp3_folder;
    wasPlayingBeforeAnnounce = true;
    send_command(0x0E, FEEDBACK, 0, 0); // пауза
    pause_on = true;
    delay(120);
  } else {
    wasPlayingBeforeAnnounce = false;
    saved_mp3_folder = 0;
    saved_mp3_track = 0;
  }

  weather_advert_count = 0;
  weatherAdvertAddTrack(weatherTempTrack(Weather::instance().getTemperature()));
  if (needDescription) {
    weatherAdvertAddDescTracks(Weather::instance().getCondition());
  }
  if (weather_advert_count == 0) return;

  isAnnouncing = true;
  weather_advert_flag = true;
  advert_flag = true;

#if MP3_LOG
  SYSLOG.add("Запущена погода (%d треков, force=%d)", weather_advert_count, force);
#endif

  uint8_t vol = isDay ? weather_day_volume : weather_night_volume;
  send_command(0x06, FEEDBACK, 0, vol);
  delay(mp3_delay);

  send_command(0x1A, FEEDBACK, 0, 1); // mute on
  delay(mp3_delay);

  for (uint8_t i = 0; i < weather_advert_count; i++) {
    playWeatherAdvertTrack(weather_advert_tracks[i]);
    delay(mp3_delay);
    if (i == 0) {
      send_command(0x1A, FEEDBACK, 0, 0); // mute off
      delay(80);
    }
    if (i < weather_advert_count - 1) {
      delay(ADVERT_TIMER_1);
    }
  }

  delay(ADVERT_TIMER_2 + 400);
  mp3_restore_after_announce(true);

  weather_advert_flag = false;
  advert_flag = false;
  advert_hour = false;
  if (force) manual_weather_request = false;

#if MP3_LOG
  SYSLOG.add("Погода завершена");
#endif
}
// -------------------
#endif // USE_WEATHER

// ----------------------------------------------------------------------------------
void play_time_ADVERT(bool force) {
  if (mp3_player_connect != 4) return;

  if (advert_flag
#if USE_WEATHER
      || weather_advert_flag
#endif
#if USE_DAWN
      || alarm_sound_flag
#endif
#if USE_SUNSET
      || sunset_sound_flag
#endif
      || isAnnouncing) {
    return;
  } if (!force) {
    time_t t = Time::instance().now();
    struct tm tm;
    localtime_r(&t, &tm);
    bool isDay = (tm.tm_hour >= (NIGHT_HOURS_STOP / 60)) && (tm.tm_hour < (NIGHT_HOURS_START / 60));
    if (isDay && !day_advert_sound_on) return;
    if (!isDay && !night_advert_sound_on) return;
  } if (eff_sound_on && !mp3_stop) {  // пауза перед озвучкой (+ запоминание трека)
    saved_mp3_track = send_command(0x4C, 1, 0, 0);
    delay(50);
    saved_mp3_folder = mp3_folder;
    wasPlayingBeforeAnnounce = true;
    send_command(0x0E, FEEDBACK, 0, 0);
    pause_on = true;
    delay(120);
  } else {
    wasPlayingBeforeAnnounce = false;
    saved_mp3_folder = 0;
    saved_mp3_track = 0;
  }

  time_t now = time(nullptr);
  struct tm *tm_info = localtime(&now);
  int pt_h = tm_info->tm_hour;
  if (pt_h == 0) pt_h = 24;
  int pt_m = tm_info->tm_min;

  isAnnouncing = true;
  advert_flag = true;

#if MP3_LOG
  if (force) {
    SYSLOG.add("Ручное время: %02d:%02d", pt_h, pt_m);
  } else {
    SYSLOG.add("Автоматический режим: %02d:%02d", pt_h, pt_m);
  }
#endif

  time_t t = Time::instance().now();
  struct tm tm;
  localtime_r(&t, &tm);
  bool isDay = (tm.tm_hour >= (NIGHT_HOURS_STOP / 60)) && (tm.tm_hour < (NIGHT_HOURS_START / 60));
  uint8_t advert_volume = isDay ? day_advert_volume : night_advert_volume;
  send_command(0x06, FEEDBACK, 0, advert_volume);
  delay(mp3_delay);
  send_command(0x1A, FEEDBACK, 0, 1); // mute on
  delay(mp3_delay);
  send_command(0x13, FEEDBACK, 0, pt_h);
  delay(mp3_delay);
  send_command(0x1A, FEEDBACK, 0, 0);
  delay(ADVERT_TIMER_H);
  send_command(0x13, FEEDBACK, 0, pt_m + 100);
  delay(ADVERT_TIMER_M);
  delay(350);
  mp3_restore_after_announce(true);
  advert_flag = false;
  advert_hour = false;
  if (force) manual_time_request = false;
}

// ----------------------------------------------------------------------------------
void mp3_clear_runtime_flags() {
  first_entry = 0;
  advert_flag = false;
  advert_hour = false;
  set_mp3_play_now = false;
  mp3_stop = true;
  pause_on = true;
#if USE_WEATHER
  weather_advert_flag = false;
  weather_advert_state = 0;
  weather_advert_index = 0;
  weather_advert_count = 0;
#endif
#if USE_DAWN
  alarm_sound_flag = false;
  dawnflag_sound = 0;
#endif
#if USE_SUNSET
  sunset_sound_flag = false;
  sunsetflag_sound = 0;
#endif
}

// ----------------------------------------------------------------------------------
void mp3_send_command_nowait(uint8_t cmd, uint8_t feedback, uint8_t dat1, uint8_t dat2) {
  uint8_t mp3_send_buf[8] = {0x7E, 0xFF, 0x06, cmd, feedback, dat1, dat2, 0xEF};
  mp3.write(mp3_send_buf, sizeof(mp3_send_buf));
}

// ----------------------------------------------------------------------------------
void mp3_periodic_check() {
#ifdef CHECK_MP3_CONNECTION
  static bool checkWaiting = false;
  static uint8_t checkPos = 0;
  static uint8_t checkFails = 0;
  static uint32_t checkStarted = 0;
  uint32_t now = millis();
  if (advert_flag
#if USE_WEATHER
      || weather_advert_flag
#endif
#if USE_DAWN
      || alarm_sound_flag
#endif
#if USE_SUNSET
      || sunset_sound_flag
#endif
      || first_entry) {
    checkWaiting = false;
    checkPos = 0;
    return;
  } if (!checkWaiting) {
    if (now - mp3_check_timer < MP3_CHECK_INTERVAL) return;
    mp3_check_timer = now;
    while (mp3.available()) mp3.read();
    mp3_send_command_nowait(0x06, 1, 0, eff_volume);
    checkStarted = now;
    checkPos = 0;
    checkWaiting = true;
    return;
  } while (mp3.available()) {
    int b = mp3.read();
    if (b < 0) break;
    if (checkPos == 0 && b != 0x7E) continue;
    mp3_receive_buf[checkPos++] = (uint8_t)b;
    if (checkPos >= sizeof(mp3_receive_buf)) {
      bool ok = (mp3_receive_buf[0] == 0x7E && mp3_receive_buf[2] == 0x06 && mp3_receive_buf[9] == 0xEF && mp3_receive_buf[3] != 0x40);
      checkWaiting = false;
      checkPos = 0;
      if (ok) {
        checkFails = 0;
      } else if (++checkFails >= MP3_CHECK_MAX_FAILS) {
#if MP3_LOG
        SYSLOG.add("\nMP3 плеер пропал или не отвечает\n");
#endif
        mp3_clear_runtime_flags();
        mp3_player_connect = 0;
        mp3_timer = millis();
        checkFails = 0;
      } return;
    }
  }

  if (now - checkStarted > MP3_CHECK_TIMEOUT) {
    checkWaiting = false;
    checkPos = 0;
    if (++checkFails >= MP3_CHECK_MAX_FAILS) {
#if MP3_LOG
      SYSLOG.add("\nMP3 плеер пропал или не отвечает\n");
#endif
      mp3_clear_runtime_flags();
      mp3_player_connect = 0;
      mp3_timer = millis();
      checkFails = 0;
    }
  }
#endif // CHECK_MP3_CONNECTION
}

// ----------------------------------------------------------------------------------
void play_sound() {
  if (isAnnouncing) return;
  if (!mp3_folder) {
    delay(mp3_delay);
    send_command(0x0E, FEEDBACK, 0, 0);
    mp3_stop = true;
    pause_on = true;
    CurrentFolder = mp3_folder;
    CurrentFolder_last = CurrentFolder;
    jsonWrite(configMP3, "fold_sel", CurrentFolder);
    writeFile(F("config_mp3.json"), configMP3);
    return;
  }

  delay(mp3_delay);
  CurrentFolder = mp3_folder;
  if (!alarm_sound_flag && !sunset_sound_flag && !isAnnouncing) {
    send_command(0x0E, FEEDBACK, 0, 0); // пауза
    delay(mp3_delay);
  }

  send_command(0x17, FEEDBACK, 0, CurrentFolder);
  delay(mp3_delay);
  mp3_stop = false;
  pause_on = false;
  set_mp3_play_now = true;
  CurrentFolder_last = CurrentFolder;
  jsonWrite(configMP3, "fold_sel", CurrentFolder);
  writeFile(F("config_mp3.json"), configMP3);
#if MP3_LOG
  SYSLOG.add("Воспроизведение: папка %d начата", CurrentFolder);
#endif
}

// ----------------------------------------------------------------------------------
void mp3_restore_after_announce(bool restoreEffect) {
  if (mp3_player_connect != 4) return;

  send_command(0x1A, FEEDBACK, 0, 1); // mute on
  delay(60);

  if (restoreEffect && eff_sound_on && wasPlayingBeforeAnnounce) {
    // просто снимается пауза
    send_command(0x0D, FEEDBACK, 0, 0);
    delay(100);
    send_command(0x06, FEEDBACK, 0, eff_volume);
    delay(50);
    mp3_stop = false;
    pause_on = false;
    set_mp3_play_now = true;
#if MP3_LOG
    SYSLOG.add("Мелодия продолжена с прерванного места");
#endif
  } else {
    // полная остановка
    send_command(0x0E, FEEDBACK, 0, 0);
    delay(50);
    send_command(0x16, FEEDBACK, 0, 0);
    delay(50);
    mp3_stop = true;
    pause_on = true;
    set_mp3_play_now = false;
#if MP3_LOG
    SYSLOG.add("Плеер остановлен после объявления");
#endif
  }

  send_command(0x1A, FEEDBACK, 0, 0); // mute off
  delay(50);
  wasPlayingBeforeAnnounce = false;
  saved_mp3_folder = 0;
  saved_mp3_track = 0;
  isAnnouncing = false;
}

// ----------------------------------------------------------------------------------
void mp3_setup() {
    static uint8_t initStep = 0;
    static int16_t tmp = -1;
    static uint32_t lastAttempt = 0;

    if (mp3Initialized) return;
    if (millis() - lastAttempt < 300) return;
    lastAttempt = millis();

    if (initStep == 0) {
        initStep = 1;
        return;
    }

    if (initStep == 1) {
        int16_t ack = send_command(0x06, 1, 0, 0);
        delay(mp3_delay);

#if MP3_LOG
        char buf[64];
        sprintf(buf, "MP3 setup: ack=%d", ack);
        SYSLOG.add(buf);
#endif

        if (ack == -1) {
#if MP3_LOG
            SYSLOG.add("Нет ответа, проверьте подключение");
#endif
            mp3_player_connect = 0;
            initStep = 0; // попытка в следующем вызове
            return;
        }

        initStep = 2;
        return;
    }

    if (initStep == 2) {
#ifdef DF_PLAYER_IS_ORIGINAL
        send_command(0x09, FEEDBACK, 0, 2);
        delay(MP3_DELAY);
        tmp = 2;
#else
        tmp = 2;
#endif
        initStep = 3;
        return;
    }

    if (initStep == 3) {
        send_command(0x07, FEEDBACK, 0, Equalizer);
        delay(mp3_delay);
        send_command(0x06, FEEDBACK, 0, eff_volume);
        delay(mp3_delay);

        if (tmp == 1 || tmp == 2 || tmp == 3) {
            mp3_player_connect = 4;
#if MP3_LOG
            char buf[64];
            sprintf(buf, "Подключен, носитель %s", (tmp == 2 ? "SD" : "Flash"));
            SYSLOG.add(buf);
#endif
        } else {
            mp3_player_connect = 5;
#if MP3_LOG
            SYSLOG.add("Подключен, но носитель не найден");
#endif
        }

        mp3Initialized = true;
        initStep = 255;
    }
} // void mp3_setup()

// ----------------------------------------------------------------------------------
void mp3_loop() {
  if (isAnnouncing) return;
  static uint32_t lastCheck = 0;
  if (millis() - lastCheck > 5000) {
    lastCheck = millis();
    if (advert_flag || weather_advert_flag) {
      send_command(0x16, 0, 0, 0);
    }
  }
  // ----------------
 handleDawnMp3();
 handleSunsetMp3();

  // ----------------
  if (ONflag && eff_sound_on) {
    set_mp3_play_now = true;
  } else {
    set_mp3_play_now = false;
  } if (!isAnnouncing && !advert_flag && !weather_advert_flag) { // возобновление мелодии
    if (set_mp3_play_now && (mp3_stop || pause_on)) {
      if (mp3_folder != 0) {
        play_sound();
      }
    } else if (!set_mp3_play_now && !mp3_stop) {
      send_command(0x0E, FEEDBACK, 0, 0);
      mp3_stop = true;
      pause_on = true;
    }
  }
} // void mp3_loop()

// ----------------------------------------------------------------------------------
int16_t send_command(int8_t cmd, uint8_t feedback, uint8_t dat1, uint8_t dat2) {
  uint8_t mp3_send_buf[8] = {0x7E, 0xFF, 0x06, cmd, feedback, dat1, dat2, 0xEF};
  for (uint8_t i = 0; i < 8; i++) {
    mp3.write(mp3_send_buf[i]);
    delay(3);
  }

#if MP3_LOG
  char logBuf[128];
  char *ptr = logBuf;
  ptr += sprintf(ptr, "\nmp3_sending: ");
  for (uint8_t i = 0; i < 8; i++) {
    ptr += sprintf(ptr, "%02X ", mp3_send_buf[i]);
  }
  SYSLOG.add(logBuf);
#endif

  if (!feedback && (cmd < 0x30)) {
    return 0xFF00;
  } else if ( feedback && (cmd < 0x30)) {
    return read_command (MP3_READ_TIMEOUT);
  } else if (feedback && (cmd >= 0x30)) {
    if (read_command (MP3_READ_TIMEOUT) == -1) return -1;
    if (read_command (MP3_READ_TIMEOUT) == -1) return -1;
    return (((int16_t)mp3_receive_buf[5]) << 8) + mp3_receive_buf[6];
  } else if (!feedback && (cmd >= 0x30)) {
    if (read_command (MP3_READ_TIMEOUT) == -1) return -1;
    return (((int16_t)mp3_receive_buf[5]) << 8) + mp3_receive_buf[6];
  } return 0xEF00;
}

// --------------------------------------------------------
int16_t read_command (uint32_t mp3_read_timeout) {
  int tmp;
  uint32_t tmr = millis();
  while (true) {
    tmp = mp3.read();
    if (tmp == 0x7E) break;
    if (millis() - tmr > mp3_read_timeout) return -1;
    delay(1);
  }

  mp3_receive_buf[0] = (uint8_t)tmp;

  for (uint8_t i = 1; i < 10; i++) {
    uint32_t btmr = millis();
    while (!mp3.available()) {
      if (millis() - btmr > mp3_read_timeout) return -1;
      delay(1);
    }
    tmp = mp3.read();
    if (tmp < 0) return -1;
    mp3_receive_buf[i] = (uint8_t)tmp;
  } if (mp3_receive_buf[2] == 6 && mp3_receive_buf[9] == 0xEF && mp3_receive_buf[3] != 0x40) {
    return (((int16_t)mp3_receive_buf[5]) << 8) + mp3_receive_buf[6];
  } return -1;
}

// ------------------------
#endif // USE_MP3_PLAYER

// ******************************************************************************************************************************************************

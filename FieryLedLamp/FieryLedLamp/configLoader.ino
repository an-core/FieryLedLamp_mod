// *********************************************************************** configLoader.ino ************************************************************
#include "Constants.h"
#include "Prototypes.h"
#include "Extern.h"
// ---------------------

// ==================================================================== ЗАГРУЗКА КОНФИГОВ =============================================================
void loadAllConfigs() {
  configSetup = safeReadFile("config.json", 2048);
  configLedPanel = safeReadFile("config_led_panel.json", 2048);
  configLedInterval = safeReadFile("config_led_interval.json", 2048);
  configLED = safeReadFile("config_led_matrix.json", 2048);
  configWiFi = safeReadFile("config_wifi.json", 2048);
  configCycle = safeReadFile("config_cycle.json", 2048);

#if USE_BUTTON
  configButton = safeReadFile("config_button.json", 1024);
#endif

#if USE_MQTT
  configMQTT = safeReadFile("config_mqtt.json", 512);
#endif

#if USE_WEATHER
  configWeather = safeReadFile("config_weather.json", 2048);
#endif

#if USE_MP3_PLAYER
  configMP3 = safeReadFile("config_mp3.json", 2048);
  soundList = readFile("sound_list.json", 4024);
  if (soundList.isEmpty() || soundList == "Failed" || soundList == "Large") {
    soundList = "{}";
  }
#endif

#if USE_ST7789
  configST7789 = safeReadFile("config_st7789.json", 1024);
#endif

#if USE_MULTILAMP
  configMultilamp = safeReadFile("config_multilamp.json", 512);
#endif

#if USE_DAWN
  configAlarm = safeReadFile("config_alarm.json", 1024);
#endif

#if USE_SUNSET
  configSunset = safeReadFile("config_sunset.json", 512);
#endif

#if USE_SCHEDULE
  configSchedule = safeReadFile("config_schedule.json", 1024);
#endif

} // loadAllConfigs()

// ===================================================================== БАЗОВЫЕ НАСТРОЙКИ ============================================================
void loadBasicSettings() {
  bool needSave = false;
  LAMP_NAME = jsonRead(configSetup, "ssdp");
  if (LAMP_NAME.length() == 0 || LAMP_NAME == "null") {
    LAMP_NAME = "FieryLedLamp";
    jsonWrite(configSetup, "ssdp", LAMP_NAME);
    needSave = true;
  }

  AP_NAME = jsonRead(configWiFi, "ssidAP");
  if (AP_NAME.length() == 0 || AP_NAME == "null") {
    AP_NAME = "FieryLedLamp";
    jsonWrite(configWiFi, "ssidAP", AP_NAME);
    needSave = true;
  }

  AP_PASS = jsonRead(configWiFi, "passwordAP");
  if (AP_PASS.length() == 0 || AP_PASS == "null") {
    AP_PASS = "";
    jsonWrite(configWiFi, "passwordAP", AP_PASS);
    needSave = true;
  }

  if (needSave) {
    configChanged = true;
  }

  Favorites::instance().rndCycle = jsonReadtoInt(configCycle, "rnd_cycle");
  AutoBrightness = jsonReadtoInt(configSetup, "auto_bri");

  String day_hour_str = jsonRead(configSetup, "day_time_hour");
  String day_min_str = jsonRead(configSetup, "day_time_minute");
  String night_hour_str = jsonRead(configSetup, "night_time_hour");
  String night_min_str = jsonRead(configSetup, "night_time_minute");
  auto fixTwoDigits = [&](String & str, const char* key) {
    if (!str.isEmpty()) {
      int val = str.toInt();
      char buf[3];
      snprintf(buf, sizeof(buf), "%02d", val);
      String newStr = buf;
      if (newStr != str) {
        str = newStr;
        jsonWrite(configSetup, key, str);
        needSave = true;
      }
    }
  };

  fixTwoDigits(day_hour_str, "day_time_hour");
  fixTwoDigits(day_min_str, "day_time_minute");
  fixTwoDigits(night_hour_str, "night_time_hour");
  fixTwoDigits(night_min_str, "night_time_minute");

  if (day_hour_str.isEmpty() || day_min_str.isEmpty() || night_hour_str.isEmpty() || night_min_str.isEmpty()) {
    int day_minutes = jsonReadtoInt(configSetup, "day_time");
    int night_minutes = jsonReadtoInt(configSetup, "night_time");
    if (day_minutes < 24) day_minutes *= 60;
    if (night_minutes < 24) night_minutes *= 60;
    int day_h = day_minutes / 60;
    int day_m = day_minutes % 60;
    int night_h = night_minutes / 60;
    int night_m = night_minutes % 60;
    char buf[3];
    snprintf(buf, sizeof(buf), "%02d", day_h); day_hour_str = buf;
    snprintf(buf, sizeof(buf), "%02d", day_m); day_min_str = buf;
    snprintf(buf, sizeof(buf), "%02d", night_h); night_hour_str = buf;
    snprintf(buf, sizeof(buf), "%02d", night_m); night_min_str = buf;
    jsonWrite(configSetup, "day_time_hour", day_hour_str);
    jsonWrite(configSetup, "day_time_minute", day_min_str);
    jsonWrite(configSetup, "night_time_hour", night_hour_str);
    jsonWrite(configSetup, "night_time_minute", night_min_str);
    jsonWrite(configSetup, "day_time", day_minutes);
    jsonWrite(configSetup, "night_time", night_minutes);
    needSave = true;
  }

  int day_hour = day_hour_str.toInt();
  int day_min = day_min_str.toInt();
  int night_hour = night_hour_str.toInt();
  int night_min = night_min_str.toInt();

  day_hour = constrain(day_hour, 0, 23);
  day_min = constrain(day_min, 0, 59);
  night_hour = constrain(night_hour, 0, 23);
  night_min = constrain(night_min, 0, 59);

  uint16_t day_minutes = day_hour * 60 + day_min;
  uint16_t night_minutes = night_hour * 60 + night_min;

  if (jsonReadtoInt(configSetup, "day_time") != day_minutes || jsonReadtoInt(configSetup, "night_time") != night_minutes) {
    jsonWrite(configSetup, "day_time", day_minutes);
    jsonWrite(configSetup, "night_time", night_minutes);
    needSave = true;
  }

  if (needSave) {
    configChanged = true;
  }

  NIGHT_HOURS_START = night_minutes;
  NIGHT_HOURS_STOP = day_minutes;

  NIGHT_HOURS_BRIGHTNESS = jsonReadtoInt(configSetup, "night_bright");
  DAY_HOURS_BRIGHTNESS = jsonReadtoInt(configSetup, "day_bright");

  TURN_ON_AFTER_SHUTDOWN = jsonReadtoInt(configSetup, "effect_always");

  if (TURN_ON_AFTER_SHUTDOWN) {
    ONflag = jsonReadtoInt(configSetup, "Power");
  } else {
    ONflag = false;
    jsonWrite(configSetup, "Power", ONflag);
  }
} // void loadBasicSettings()

// ======================================================================== ДИСПЛЕЙ ST7789 ============================================================
void loadST7789Settings() {
#if USE_ST7789
  if (jsonRead(configST7789, "tft_clock_color") == "") {
    jsonWrite(configST7789, "tft_clock_color", "0");
    configChanged = true;
  }
  if (jsonRead(configST7789, "tft_weather_color") == "") {
    jsonWrite(configST7789, "tft_weather_color", "1");
    configChanged = true;
  }
  if (jsonRead(configST7789, "tft_ticker_on") == "") {
    jsonWrite(configST7789, "tft_ticker_on", "0");
    configChanged = true;
  }
  if (jsonRead(configST7789, "tft_ticker_color") == "") {
    jsonWrite(configST7789, "tft_ticker_color", "0");
    configChanged = true;
  }
  if (jsonRead(configST7789, "tft_ticker_speed") == "") {
    jsonWrite(configST7789, "tft_ticker_speed", "146");
    configChanged = true;
  }
  if (jsonRead(configST7789, "tft_ticker_period") == "") {
    jsonWrite(configST7789, "tft_ticker_period", "60");
    configChanged = true;
  }
  if (jsonRead(configST7789, "tft_ticker_text") == "") {
    jsonWrite(configST7789, "tft_ticker_text", "Привет");
    configChanged = true;
  }
  if (jsonRead(configST7789, "tft_brightness") == "") {
    jsonWrite(configST7789, "tft_brightness", "255");
    configChanged = true;
  }
  if (jsonRead(configST7789, "tft_auto_brightness") == "") {
    jsonWrite(configST7789, "tft_auto_brightness", "1");
    configChanged = true;
  }
  if (jsonRead(configST7789, "tft_date_color") == "") {
    jsonWrite(configST7789, "tft_date_color", "0");
    configChanged = true;
  }

  int tftDayBright = jsonReadtoInt(configST7789, "day_bright", 255);
  int tftNightBright = jsonReadtoInt(configST7789, "night_bright", 50);
  TFT_DAY_BRIGHTNESS = constrain(tftDayBright, 0, 255);
  TFT_NIGHT_BRIGHTNESS = constrain(tftNightBright, 0, 255);

  tft_clock_color = jsonReadtoInt(configST7789, "tft_clock_color", 0);
  tft_weather_color = jsonReadtoInt(configST7789, "tft_weather_color", 1);
  tft_ticker_on = jsonReadtoInt(configST7789, "tft_ticker_on", 0) != 0;
  tft_ticker_color = jsonReadtoInt(configST7789, "tft_ticker_color", 0);
  tft_ticker_speed = jsonReadtoInt(configST7789, "tft_ticker_speed", 146);
  tft_ticker_period = jsonReadtoInt(configST7789, "tft_ticker_period", 60);
  tft_brightness = jsonReadtoInt(configST7789, "tft_brightness", 255);
  tft_auto_brightness = (jsonReadtoInt(configST7789, "tft_auto_brightness", 1) != 0);
  tft_date_color = jsonReadtoInt(configST7789, "tft_date_color", 0);

  String tickerText = jsonRead(configST7789, "tft_ticker_text");
  if (tickerText.length() > 0 && tickerText.length() < 128) {
    strcpy(TFTTickerText, tickerText.c_str());
  } else {
    strcpy(TFTTickerText, "Привет!");
  }
#endif // USE_ST7789
}

// ======================================= ОБЩАЯ НАСТРОЙКА ПЕРЕКЛЮЧЕНИЯ ЧАСЫ/ПОГОДА/ДАТА (ДЛЯ ДИСПЛЕЕВ TM1637/ST7789) ==================================
void loadDisplaySwitchSettings() {
#if (USE_TM1637 || USE_ST7789)
  uint32_t tmp = jsonReadtoInt(configSetup, "disp_switch");
  if (tmp < 5) tmp = 10;
  DISPLAY_SWITCH_INTERVAL = tmp * 1000UL;
  displaySwitchTimer = millis();
#endif
}

// =========================================================================== КНОПКА =================================================================
void loadButtonSettings() {
#if USE_BUTTON
  bool buttonChanged = false;

  if (jsonRead(configButton, "button_type") == "") {
    jsonWrite(configButton, "button_type", String(BUTTON_TYPE));
    buttonChanged = true;
  }
  if (jsonRead(configButton, "btn_click_power") == "") {
    jsonWrite(configButton, "btn_click_power", String(BUTTON_ACTION_POWER));
    buttonChanged = true;
  }
  if (jsonRead(configButton, "btn_click_next") == "") {
    jsonWrite(configButton, "btn_click_next", String(BUTTON_ACTION_NEXT));
    buttonChanged = true;
  }
  if (jsonRead(configButton, "btn_click_prev") == "") {
    jsonWrite(configButton, "btn_click_prev", String(BUTTON_ACTION_PREV));
    buttonChanged = true;
  }
  if (jsonRead(configButton, "btn_click_action4") == "") {
    jsonWrite(configButton, "btn_click_action4", String(BUTTON_ACTION_ACTION4));
    buttonChanged = true;
  }
  if (jsonRead(configButton, "btn_click_ip") == "") {
    jsonWrite(configButton, "btn_click_ip", String(BUTTON_ACTION_IP));
    buttonChanged = true;
  }
  if (jsonRead(configButton, "btn_click_time") == "") {
    jsonWrite(configButton, "btn_click_time", String(BUTTON_ACTION_TIME));
    buttonChanged = true;
  }
  if (jsonRead(configButton, "btn_click_sound") == "") {
    jsonWrite(configButton, "btn_click_sound", String(BUTTON_ACTION_SOUND));
    buttonChanged = true;
  }
  if (jsonRead(configButton, "btn_click_weather") == "") {
    jsonWrite(configButton, "btn_click_weather", String(BUTTON_ACTION_WEATHER));
    buttonChanged = true;
  }
  if (buttonChanged) {
    configChanged = true;
  }

  button_type = jsonReadtoInt(configButton, "button_type", BUTTON_TYPE);

#if defined(BUTTON_LOCK_ON_START) && BUTTON_LOCK_ON_START
  if (buttonEnabled) {
    bool pinState = digitalRead(BTN_PIN);
    if (button_type == 1) {           // сенсорная
      if (pinState == HIGH) buttonEnabled = false;
    } else {                          // механическая
      if (pinState == LOW) buttonEnabled = false;
    }
  }
#endif

  btn_click_power = jsonReadtoInt(configButton, "btn_click_power", BUTTON_ACTION_POWER);
  btn_click_next = jsonReadtoInt(configButton, "btn_click_next", BUTTON_ACTION_NEXT);
  btn_click_prev = jsonReadtoInt(configButton, "btn_click_prev", BUTTON_ACTION_PREV);
  btn_click_action4 = jsonReadtoInt(configButton, "btn_click_action4", BUTTON_ACTION_ACTION4);
  btn_click_ip = jsonReadtoInt(configButton, "btn_click_ip", BUTTON_ACTION_IP);
  btn_click_time = jsonReadtoInt(configButton, "btn_click_time", BUTTON_ACTION_TIME);
  btn_click_sound = jsonReadtoInt(configButton, "btn_click_sound", BUTTON_ACTION_SOUND);
  btn_click_weather = jsonReadtoInt(configButton, "btn_click_weather", BUTTON_ACTION_WEATHER);
#endif // USE_BUTTON
}

// ============================================================================ МП3 ПЛЕЕР =============================================================
void loadMP3Settings() {
#if USE_MP3_PLAYER
  eff_volume = constrain(jsonReadtoInt(configMP3, "vol", 10), 0, 30);
  alarm_volume = constrain(jsonReadtoInt(configMP3, "alm_vol", 15), 0, 30);
  sunset_volume = constrain(jsonReadtoInt(configMP3, "sun_vol", 10), 0, 30);

  day_advert_volume = constrain(jsonReadtoInt(configMP3, "day_vol", 12), 0, 30);
  night_advert_volume = constrain(jsonReadtoInt(configMP3, "night_vol", 5), 0, 30);

  weather_day_volume = constrain(jsonReadtoInt(configMP3, "weather_day_vol", 12), 0, 30);
  weather_night_volume = constrain(jsonReadtoInt(configMP3, "weather_night_vol", 5), 0, 30);

  eff_sound_on = jsonReadtoInt(configMP3, "on_sound", 1);
  alarm_sound_on = jsonReadtoInt(configMP3, "on_alm_snd", 1);
  sunset_sound_on = jsonReadtoInt(configMP3, "on_sun_snd", 0);

  day_advert_sound_on = jsonReadtoInt(configMP3, "on_day_adv", 1);
  night_advert_sound_on = jsonReadtoInt(configMP3, "on_night_adv", 1);
  alarm_advert_sound_on = jsonReadtoInt(configMP3, "on_alm_adv", 1);
  day_weather_temp_on = jsonReadtoInt(configMP3, "on_day_wadv", 1);
  day_weather_desc_on = jsonReadtoInt(configMP3, "on_day_wdesc", 1);
  night_weather_temp_on = jsonReadtoInt(configMP3, "on_night_wadv", 1);
  night_weather_desc_on = jsonReadtoInt(configMP3, "on_night_wdesc", 1);

  weatherSpeakEnabled = jsonReadtoInt(configMP3, "weather_speak", 1);
  timeAnnounceEnabled = jsonReadtoInt(configMP3, "time_speak", 1);

  AlarmFolder = constrain(jsonReadtoInt(configMP3, "alm_fold", 99), 1, 99);
  SunsetFolder = constrain(jsonReadtoInt(configMP3, "sun_fold", 99), 1, 99);

  CurrentFolder = constrain(jsonReadtoInt(configMP3, "fold_sel", 2), 0, 99);
  mp3_folder = CurrentFolder;
  CurrentFolder_last = CurrentFolder;

  ADVERT_TIMER_H = 100UL * constrain(jsonReadtoInt(configMP3, "tim_h", 16), 10, 40);
  ADVERT_TIMER_M = 100UL * constrain(jsonReadtoInt(configMP3, "tim_m", 20), 12, 45);

  mp3_delay = 10UL * constrain(jsonReadtoInt(configMP3, "delay", 10), 3, 30);

  ADVERT_TIMER_1 = 100UL * constrain(jsonReadtoInt(configMP3, "weather_temp_delay", 8), 5, 60);
  ADVERT_TIMER_2 = 100UL * constrain(jsonReadtoInt(configMP3, "weather_desc_delay", 15), 8, 120);

  Equalizer = constrain(jsonReadtoInt(configMP3, "eq", 0), 0, 5);

  show_weather_desc = jsonReadtoInt(configMP3, "show_weather_desc", 1);
  time_always = jsonReadtoInt(configMP3, "time_always", 0);
  weather_always = jsonReadtoInt(configMP3, "weather_always", 0);

  if (!isAnnouncing && !dawnflag_sound && !sunsetflag_sound) {
    send_command(0x06, FEEDBACK, 0, eff_volume);
  }
#if MP3_LOG
  SYSLOG.add("MP3 плеер: загрузка настроек успешна");
#endif
#endif
}

// ========================================================== СИНХРОННОЕ УПРАВЛЕНИЕ ЛАМПАМИ (до 5 штук) ===============================================
#if USE_MULTILAMP
void loadMultilampSettings() {
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
#endif

// ============================================================================= MQTT =================================================================
void loadMQTTSettings() {
#if USE_MQTT
  bool mqttChanged = false;

  if (configMQTT.isEmpty() || configMQTT == "Failed" || configMQTT == "Large") {
    configMQTT = "{}";
    mqttChanged = true;
  }

  if (jsonRead(configMQTT, "mq_ip") == "") {
    jsonWrite(configMQTT, "mq_ip", "192.168.1.100");
    mqttChanged = true;
  }
  if (jsonRead(configMQTT, "mq_port") == "") {
    jsonWrite(configMQTT, "mq_port", 1883);
    mqttChanged = true;
  }
  if (jsonRead(configMQTT, "mq_user") == "") {
    jsonWrite(configMQTT, "mq_user", "");
    mqttChanged = true;
  }
  if (jsonRead(configMQTT, "mq_pass") == "") {
    jsonWrite(configMQTT, "mq_pass", "");
    mqttChanged = true;
  }
  if (jsonRead(configMQTT, "topic") == "") {
    jsonWrite(configMQTT, "topic", "esp32");
    mqttChanged = true;
  }
  if (jsonRead(configMQTT, "mq_on") == "") {
    jsonWrite(configMQTT, "mq_on", 0);
    mqttChanged = true;
  }
  if (jsonRead(configMQTT, "mq_prd") == "") {
    jsonWrite(configMQTT, "mq_prd", 60);
    mqttChanged = true;
  }

  if (mqttChanged) {
    //writeFile(F("config_mqtt.json"), configMQTT);
    configChanged = true;
  }

  String mq_ip_str = jsonRead(configMQTT, "mq_ip");
  String mq_user = jsonRead(configMQTT, "mq_user");
  String mq_pass = jsonRead(configMQTT, "mq_pass");
  String mq_topic = jsonRead(configMQTT, "topic");
  uint16_t mq_port = jsonReadtoInt(configMQTT, "mq_port", 1883);
  MqttOn = jsonReadtoInt(configMQTT, "mq_on", 0);
  MqttPeriod = jsonReadtoInt(configMQTT, "mq_prd", 60);

  if (mq_port < 1 || mq_port > 65535) mq_port = 1883;
  if (MqttPeriod < 10) MqttPeriod = 60;
  if (MqttPeriod > 3600) MqttPeriod = 3600;
  if (!MqttServer.fromString(mq_ip_str)) {
    mq_ip_str.toCharArray(MqttHost, sizeof(MqttHost) - 1);
    MqttHost[sizeof(MqttHost) - 1] = '\0';
    mqttIPaddr = false;
  } else {
    mqttIPaddr = true;
  }

  mq_user.toCharArray(MqttUser, sizeof(MqttUser) - 1);
  MqttUser[sizeof(MqttUser) - 1] = '\0';

  mq_pass.toCharArray(MqttPassword, sizeof(MqttPassword) - 1);
  MqttPassword[sizeof(MqttPassword) - 1] = '\0';

  mq_topic.toCharArray(TopicBase, sizeof(TopicBase) - 1);
  TopicBase[sizeof(TopicBase) - 1] = '\0';
#if MQTT_LOG
  SYSLOG.add("MQTT настройки загружены: Сервер=%s, Порт=%u, User=%s, Topic=%s, Вкл=%s, Период=%u", mqttIPaddr ? MqttServer.toString().c_str() : MqttHost, mq_port, MqttUser[0] ? MqttUser : "<пусто>", TopicBase, MqttOn ? "да" : "нет",  MqttPeriod);
#endif
#endif // USE_MQTT
}

// =========================================================================== РАССВЕТ ================================================================
#if USE_DAWN
void loadAlarmSettings() {
  if (configAlarm.isEmpty() || configAlarm == "Failed" || configAlarm == "Large") {
    configAlarm = "{}";
  }

  for (uint8_t i = 0; i < 7; i++) {
    char idx[2];
    itoa(i + 1, idx, 10);

    String key_a = "a" + String(idx);
    String key_h = "h" + String(idx);
    String key_m = "m" + String(idx);

    uint8_t state = jsonReadtoInt(configAlarm, key_a, 0);
    String hoursStr = jsonRead(configAlarm, key_h);
    String minutesStr = jsonRead(configAlarm, key_m);

    if (hoursStr.length() == 0 || hoursStr == "null") hoursStr = "07";
    if (minutesStr.length() == 0 || minutesStr == "null") minutesStr = "00";

    uint8_t hours = hoursStr.toInt();
    uint8_t minutes = minutesStr.toInt();
    if (hours > 23) hours = 7;
    if (minutes > 59) minutes = 0;

    alarms[i].State = state;
    alarms[i].Time = hours * 60 + minutes;
  }

  // Режим "Рассвет", таймаут (после будильника), яркость
  dawnMode = jsonReadtoInt(configAlarm, "t", 1) - 1;
  DAWN_TIMEOUT = jsonReadtoInt(configAlarm, "after", 10);
  DAWN_BRIGHT = jsonReadtoInt(configAlarm, "a_br", 100);

  if (dawnMode > 2) dawnMode = 0;
  if (DAWN_TIMEOUT > 120) DAWN_TIMEOUT = 10;
  if (DAWN_BRIGHT < 1 || DAWN_BRIGHT > 255) DAWN_BRIGHT = 100;
}
#endif // USE_DAWN

// =========================================================================== ЗАКАТ ==================================================================
#if USE_SUNSET
void loadSunsetSettings() {
  if (configSunset.isEmpty() || configSunset == "Failed" || configSunset == "Large") {
    configSunset = "{}";
  }

  for (uint8_t i = 0; i < 7; i++) {
    char idx[2];
    itoa(i + 1, idx, 10);

    String key_a = "a" + String(idx);
    String key_h = "h" + String(idx);
    String key_m = "m" + String(idx);

    uint8_t state = jsonReadtoInt(configSunset, key_a, 0);
    String hoursStr = jsonRead(configSunset, key_h);
    String minutesStr = jsonRead(configSunset, key_m);

    if (hoursStr.length() == 0 || hoursStr == "null") hoursStr = "21";
    if (minutesStr.length() == 0 || minutesStr == "null") minutesStr = "00";

    uint8_t hours = hoursStr.toInt();
    uint8_t minutes = minutesStr.toInt();
    if (hours > 23) hours = 21;
    if (minutes > 59) minutes = 0;

    sunsets[i].State = state;
    sunsets[i].Time = hours * 60 + minutes;
  }

  sunsetMode = jsonReadtoInt(configSunset, "t", 1) - 1;
  SUNSET_BRIGHT = jsonReadtoInt(configSunset, "s_br", 100);

  if (sunsetMode > 2) sunsetMode = 0;
  if (SUNSET_BRIGHT < 1 || SUNSET_BRIGHT > 255) SUNSET_BRIGHT = 100;
}
#endif // USE_SUNSET

// =========================================================================== РАСПИСАНИЕ =============================================================
#if USE_SCHEDULE
void loadScheduleSettings() {
  if (configSchedule.isEmpty() || configSchedule == "Failed" || configSchedule == "Large") {
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
  if (enableStr != "1") {
    return;
  }

  uint8_t timerIndex = 0;

  for (uint8_t slot = 1; slot <= 6 && timerIndex < MAX_SCHEDULE_ENTRIES; slot++) {
    String aKey = "a" + String(slot);
    String hKey = "h" + String(slot);
    String mKey = "m" + String(slot);
    String effKey = (slot == 3) ? "eff3" : "";

    uint8_t action = jsonReadtoInt(configSchedule, aKey, 0);
    if (action == 0) continue;

    String hStr = jsonRead(configSchedule, hKey);
    String mStr = jsonRead(configSchedule, mKey);
    if (hStr == "" || hStr == "null") hStr = "00";
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

    if (slot == 1 || slot == 2) {
      // Режимы 1 и 2 - Вкл/Выкл
      schedule[timerIndex].Action = action;
      schedule[timerIndex].EffectNum = 255;
    } else if (slot == 3) {
      // Эффект
      if (action == 3) {
        schedule[timerIndex].Action = 3;
        schedule[timerIndex].EffectNum = effectNum;
      } else {
        schedule[timerIndex].Action = action;
        schedule[timerIndex].EffectNum = 255;
      }
    } else if (slot == 4) {
      // Цикл
      schedule[timerIndex].Action = action;
      schedule[timerIndex].EffectNum = 255;
    } else if (slot == 5) {
      // Часы
      schedule[timerIndex].Action = 4;
      schedule[timerIndex].EffectNum = EFF_CLOCK;
    } else if (slot == 6) {
      // Ночные часы
      schedule[timerIndex].Action = 7;
      schedule[timerIndex].EffectNum = EFF_CLOCK;
    }

    timerIndex++;
  }
}
#endif // USE_SCHEDULE

// ============================================================================= ЯРКОСТЬ ==============================================================
void loadBrightness() {
  DynamicJsonDocument doc(4096);
  deserializeJson(doc, configSetup);
  JsonObject configObj = doc.as<JsonObject>();

  if (!configObj.containsKey("br")) {
    jsonWrite(configSetup, "br", String(BRIGHTNESS));
    configChanged = true;
  }
  int savedBrightness = jsonReadtoInt(configSetup, "br");
  if (savedBrightness < 1 || savedBrightness > 255) {
    savedBrightness = BRIGHTNESS;
    jsonWrite(configSetup, "br", String(savedBrightness));
    configChanged = true;
  }
}

// ---------------------------------------
//дефолтная яркость Рассвета и Заката
void loadDawnSunsetSettings() {
#if USE_DAWN
  DAWN_BRIGHT = jsonReadtoInt(configSetup, "dawnBright");
  if (DAWN_BRIGHT == 0 || DAWN_BRIGHT > 255) DAWN_BRIGHT = 100;
#endif // USE_DAWN

#if USE_SUNSET
  SUNSET_BRIGHT = jsonReadtoInt(configSetup, "sunsetBright");
  if (SUNSET_BRIGHT == 0 || SUNSET_BRIGHT > 255) SUNSET_BRIGHT = 100;
#endif // USE_SUNSET
}

// ----------------------------------------
// пользовательская яркость часов
void loadUserBrightness() {
  userClockBrightness = jsonReadtoInt(configSetup, "br");
  if (userClockBrightness < 2 || userClockBrightness > 255) {
    userClockBrightness = 30;
  }
  nightModeBrightness = 0;
#if GENERAL_LOG
  SYSLOG.add("Загружена пользовательская яркость для часов: %u", userClockBrightness);
#endif
}
// ============================================================================ МАТРИЦА ===============================================================
void loadMatrixAndInitLEDs() {
  bool localChanged = false;

  if (jsonRead(configLED, "width") == "") {
    jsonWrite(configLED, "width", String(WIDTH));
    localChanged = true;
  }
  if (jsonRead(configLED, "height") == "") {
    jsonWrite(configLED, "height", String(HEIGHT));
    localChanged = true;
  }
  if (jsonRead(configLED, "m_t") == "") {
    jsonWrite(configLED, "m_t", String(MATRIX_TYPE));
    localChanged = true;
  }
  if (jsonRead(configLED, "m_o") == "") {
    jsonWrite(configLED, "m_o", String(MATRIX_ORIENTATION));
    localChanged = true;
  }
  if (jsonRead(configLED, "cur_lim") == "") {
    jsonWrite(configLED, "cur_lim", String(CURRENT_LIMIT));
    localChanged = true;
  }

#if MULTI_MATRIX
  if (jsonRead(configLED, "panel_flip") == "") {
    jsonWrite(configLED, "panel_flip", "0");
    localChanged = true;
  }
  if (jsonRead(configLED, "segMatrix_w") == "") {
    jsonWrite(configLED, "segMatrix_w", String(SEG_MATRIX_W));
    localChanged = true;
  }
  if (jsonRead(configLED, "segMatrix_h") == "") {
    jsonWrite(configLED, "segMatrix_h", String(SEG_MATRIX_H));
    localChanged = true;
  }
#endif

  if (localChanged) {
    configChanged = true;
  }

  MatrixType = jsonReadtoInt(configLED, "m_t", MATRIX_TYPE);
  MatrixOrientation = jsonReadtoInt(configLED, "m_o", MATRIX_ORIENTATION);
  current_limit = jsonReadtoInt(configLED, "cur_lim", CURRENT_LIMIT);

  uint16_t newSegWidth = jsonReadtoInt(configLED, "width", WIDTH);
  uint16_t newSegHeight = jsonReadtoInt(configLED, "height", HEIGHT);
  uint8_t newMatrixType = jsonReadtoInt(configLED, "m_t", MATRIX_TYPE);
  uint8_t newMatrixOrientation = jsonReadtoInt(configLED, "m_o", MATRIX_ORIENTATION);
  uint16_t newCurrentLimit = jsonReadtoInt(configLED, "cur_lim", CURRENT_LIMIT);

  if (newSegWidth < 1 || newSegWidth > MAX_MATRIX_WIDTH) newSegWidth = WIDTH;
  if (newSegHeight < 1 || newSegHeight > MAX_MATRIX_HEIGHT) newSegHeight = HEIGHT;
  if (newMatrixType > 1) newMatrixType = MATRIX_TYPE;
  if (newMatrixOrientation > 7) newMatrixOrientation = MATRIX_ORIENTATION;
  if (newCurrentLimit > CURRENT_LIMIT) newCurrentLimit = CURRENT_LIMIT;

#if MULTI_MATRIX
  uint8_t newSegW = jsonReadtoInt(configLED, "segMatrix_w", SEG_MATRIX_W);
  uint8_t newSegH = jsonReadtoInt(configLED, "segMatrix_h", SEG_MATRIX_H);
  if (newSegW < 1) newSegW = 1;
  if (newSegH < 1) newSegH = 1;
  bool newPanelFlip = (jsonReadtoInt(configLED, "panel_flip", 0) == 1);
  panelFlip = newPanelFlip;
#else
  const uint8_t newSegW = 1, newSegH = 1;
#endif

  uint16_t newMatrixWidth = newSegWidth * newSegW;
  uint16_t newMatrixHeight = newSegHeight * newSegH;
  if (newMatrixWidth > MAX_MATRIX_WIDTH) newMatrixWidth = MAX_MATRIX_WIDTH;
  if (newMatrixHeight > MAX_MATRIX_HEIGHT) newMatrixHeight = MAX_MATRIX_HEIGHT;
  uint16_t newUsedLeds = newMatrixWidth * newMatrixHeight * SEGMENTS;
  if (newUsedLeds > MAX_LEDS) newUsedLeds = MAX_LEDS;

  uint16_t oldWidth = matrixWidth;
  uint16_t oldHeight = matrixHeight;
  bool buffersExist = (leds != nullptr);

  if (newMatrixWidth != oldWidth || newMatrixHeight != oldHeight) {
    if (buffersExist) {
      if (reconfigureMatrix(newMatrixWidth, newMatrixHeight, newUsedLeds, oldWidth, oldHeight)) {
        segWidth = newSegWidth;
        segHeight = newSegHeight;
        MatrixType = newMatrixType;
        MatrixOrientation = newMatrixOrientation;
        current_limit = newCurrentLimit;
#if MULTI_MATRIX
        panelFlip = newPanelFlip;
        segMatrix_w = newSegW;
        segMatrix_h = newSegH;
#endif
        matrixWidth = newMatrixWidth;
        matrixHeight = newMatrixHeight;
        usedLeds = newUsedLeds;

        if (localChanged) {
          saveConfig();
          localChanged = false;
        }
#if MATRIX_LOG
        SYSLOG.add("Матрица изменена, перезагрузка...");
#endif
        ESP.restart();
      } else {
#if MATRIX_LOG
        SYSLOG.add("Не удалось переконфигурировать матрицу, будут задействованы старые настройки");
#endif
        return;
      }
    } else {
      if (!reconfigureMatrix(newMatrixWidth, newMatrixHeight, newUsedLeds, oldWidth, oldHeight)) {
#if MATRIX_LOG
        SYSLOG.add("Не удалось выделить буферы для матрицы");
#endif
        return;
      }
      segWidth = newSegWidth;
      segHeight = newSegHeight;
      MatrixType = newMatrixType;
      MatrixOrientation = newMatrixOrientation;
      current_limit = newCurrentLimit;
#if MULTI_MATRIX
      panelFlip = newPanelFlip;
      segMatrix_w = newSegW;
      segMatrix_h = newSegH;
#endif
      matrixWidth = newMatrixWidth;
      matrixHeight = newMatrixHeight;
      usedLeds = newUsedLeds;

      if (localChanged) {
        saveConfig();
        localChanged = false;
      }
    }
  } else {
    if (!buffersExist) {
      if (!reconfigureMatrix(newMatrixWidth, newMatrixHeight, newUsedLeds, oldWidth, oldHeight)) {
#if MATRIX_LOG
        SYSLOG.add("Не удалось выделить буферы для матрицы");
#endif
        return;
      }
      segWidth = newSegWidth;
      segHeight = newSegHeight;
      MatrixType = newMatrixType;
      MatrixOrientation = newMatrixOrientation;
      current_limit = newCurrentLimit;
#if MULTI_MATRIX
      panelFlip = newPanelFlip;
      segMatrix_w = newSegW;
      segMatrix_h = newSegH;
#endif
      matrixWidth = newMatrixWidth;
      matrixHeight = newMatrixHeight;
      usedLeds = newUsedLeds;
      if (localChanged) {
        saveConfig();
        localChanged = false;
      }
    } else {
      segWidth = newSegWidth;
      segHeight = newSegHeight;
      MatrixType = newMatrixType;
      MatrixOrientation = newMatrixOrientation;
      current_limit = newCurrentLimit;
#if MULTI_MATRIX
      panelFlip = newPanelFlip;
      segMatrix_w = newSegW;
      segMatrix_h = newSegH;
#endif
    }
  }

  // ----------------------- Инициализация FastLED ---------------------------
  {
    bool ledConfigChanged = false;
    if (jsonRead(configLED, "led_chip") == "") {
      jsonWrite(configLED, "led_chip", String(LED_CHIP));
      ledConfigChanged = true;
    }
    if (jsonRead(configLED, "color_order") == "") {
      jsonWrite(configLED, "color_order", String(COLOR_ORDER));
      ledConfigChanged = true;
    }
    if (ledConfigChanged) {
      configChanged = true;
      localChanged = true;
    }

    uint8_t ledChipType = jsonReadtoInt(configLED, "led_chip", LED_CHIP);
    uint8_t webColorOrder = jsonReadtoInt(configLED, "color_order", COLOR_ORDER);
    uint8_t colorOrder = GRB;
    switch (webColorOrder) {
      case 0: colorOrder = GRB; break;
      case 1: colorOrder = RGB; break;
      case 2: colorOrder = BRG; break;
      case 3: colorOrder = RBG; break;
      case 4: colorOrder = GBR; break;
      case 5: colorOrder = BGR; break;
    }

    if (ledChipType == 1) { // APA102
      switch (colorOrder) {
        case GRB: FastLED.addLeds<APA102, LED_PIN, CLK_PIN, GRB>(leds, usedLeds); break;
        case RGB: FastLED.addLeds<APA102, LED_PIN, CLK_PIN, RGB>(leds, usedLeds); break;
        case BRG: FastLED.addLeds<APA102, LED_PIN, CLK_PIN, BRG>(leds, usedLeds); break;
        case RBG: FastLED.addLeds<APA102, LED_PIN, CLK_PIN, RBG>(leds, usedLeds); break;
        case GBR: FastLED.addLeds<APA102, LED_PIN, CLK_PIN, GBR>(leds, usedLeds); break;
        case BGR: FastLED.addLeds<APA102, LED_PIN, CLK_PIN, BGR>(leds, usedLeds); break;
        default:  FastLED.addLeds<APA102, LED_PIN, CLK_PIN, GRB>(leds, usedLeds); break;
      }
    } else { // WS2812B
      switch (colorOrder) {
        case GRB: FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, usedLeds); break;
        case RGB: FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, usedLeds); break;
        case BRG: FastLED.addLeds<WS2812B, LED_PIN, BRG>(leds, usedLeds); break;
        case RBG: FastLED.addLeds<WS2812B, LED_PIN, RBG>(leds, usedLeds); break;
        case GBR: FastLED.addLeds<WS2812B, LED_PIN, GBR>(leds, usedLeds); break;
        case BGR: FastLED.addLeds<WS2812B, LED_PIN, BGR>(leds, usedLeds); break;
        default:  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, usedLeds); break;
      }
    }

    FastLED.setCorrection(TypicalLEDStrip);
    uint8_t savedBrightness = jsonReadtoInt(configSetup, "br");
    if (savedBrightness < 1 || savedBrightness > 255) savedBrightness = BRIGHTNESS;

    if (current_limit > 0) {
      FastLED.setMaxPowerInVoltsAndMilliamps(5, current_limit);
#if MATRIX_LOG
      SYSLOG.add("Лимит тока: %d мА", current_limit);
#endif
    }

    for (int i = 0; i < 3; i++) {
      FastLED.clear();
      FastLED.show();
      delay(50);
      FastLED.setBrightness(savedBrightness);
      FastLED.show();
      delay(50);
    }
    FastLED.clear();
    FastLED.setBrightness(savedBrightness);
    FastLED.show();

  }

  printFreeHeap("После выделения матрицы");
}

// ============================================================ НАСТРОЙКИ ОТОБРАЖЕНИЯ ЧАСОВ НА МАТРИЦЕ ================================================
void loadClockSettings() {
#if LED_PANEL
  if (jsonRead(configLedPanel, "clock_hue").isEmpty()) {
    jsonWrite(configLedPanel, "clock_hue", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "clock_cycle").isEmpty()) {
    jsonWrite(configLedPanel, "clock_cycle", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "clock_x_offset").isEmpty()) {
    jsonWrite(configLedPanel, "clock_x_offset", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "clock_y_offset").isEmpty()) {
    jsonWrite(configLedPanel, "clock_y_offset", "0");
    configChanged = true;
  }

  clockHue = jsonReadtoInt(configLedPanel, "clock_hue", 0);
  clockColorCycle = jsonReadtoInt(configLedPanel, "clock_cycle", 0) == 1;
  clockXOffset = jsonReadtoInt(configLedPanel, "clock_x_offset", 0);
  clockYOffset = jsonReadtoInt(configLedPanel, "clock_y_offset", 0);
#endif // LED_PANEL

  if (jsonRead(configLedPanel, "rainbow_clock").isEmpty()) {
    jsonWrite(configLedPanel, "rainbow_clock", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "auto_move_clock").isEmpty()) {
    jsonWrite(configLedPanel, "auto_move_clock", "1");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "clock_leading_zero").isEmpty()) {
    jsonWrite(configLedPanel, "clock_leading_zero", "1");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "clock_vert").isEmpty()) {
    jsonWrite(configLedPanel, "clock_vert", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "night_clock_enabled").isEmpty()) {
    jsonWrite(configLedPanel, "night_clock_enabled", "0");
    configChanged = true;
  }
  nightClockEnabled = false;
  jsonWrite(configLedPanel, "night_clock_enabled", "0");
  if (jsonRead(configLedPanel, "night_clock_brightness").isEmpty()) {
    jsonWrite(configLedPanel, "night_clock_brightness", "1");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "night_clock_hue").isEmpty()) {
    jsonWrite(configLedPanel, "night_clock_hue", "0");
    configChanged = true;
  }

  rainbowClock = jsonReadtoInt(configLedPanel, "rainbow_clock", 0) == 1;
  autoMoveClockEnabled = jsonReadtoInt(configLedPanel, "auto_move_clock", 1) == 1;
  clockLeadingZero = jsonReadtoInt(configLedPanel, "clock_leading_zero", 1) == 1;
  clockIsVertical = jsonReadtoInt(configLedPanel, "clock_vert", 0) == 1;
  nightClockBrightness = jsonReadtoInt(configLedPanel, "night_clock_brightness", 1);
  if (nightClockBrightness < 1 || nightClockBrightness > 255) {
    nightClockBrightness = 1;
  }
  nightClockHue = jsonReadtoInt(configLedPanel, "night_clock_hue", 0);
  nightModeBrightness = 0;
  configChanged = true;
}

// ======================================================== НАСТРОЙКИ ОТОБРАЖЕНИЯ ДАТЫ НА МАТРИЦЕ =====================================================
void loadDateSettings() {
#if LED_PANEL
  if (jsonRead(configLedPanel, "date_hue").isEmpty()) {
    jsonWrite(configLedPanel, "date_hue", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "rainbow_date").isEmpty()) {
    jsonWrite(configLedPanel, "rainbow_date", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "date_cycle").isEmpty()) {
    jsonWrite(configLedPanel, "date_cycle", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "date_enabled").isEmpty()) {
    jsonWrite(configLedPanel, "date_enabled", "1");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "date_x_offset").isEmpty()) {
    jsonWrite(configLedPanel, "date_x_offset", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "date_y_offset").isEmpty()) {
    jsonWrite(configLedPanel, "date_y_offset", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "date_show_year").isEmpty()) {
    jsonWrite(configLedPanel, "date_show_year", "1");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "date_full_year").isEmpty()) {
    jsonWrite(configLedPanel, "date_full_year", "1");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "date_separator_blink").isEmpty()) {
    jsonWrite(configLedPanel, "date_separator_blink", "1");
    configChanged = true;
  }

  dateHue = jsonReadtoInt(configLedPanel, "date_hue", 0);
  rainbowDate = jsonReadtoInt(configLedPanel, "rainbow_date", 0) == 1;
  dateColorCycle = jsonReadtoInt(configLedPanel, "date_cycle", 0) == 1;
  dateEnabled = jsonReadtoInt(configLedPanel, "date_enabled", 1) == 1;
  dateXOffset = jsonReadtoInt(configLedPanel, "date_x_offset", 0);
  dateYOffset = jsonReadtoInt(configLedPanel, "date_y_offset", 0);
  showYearInDate = jsonReadtoInt(configLedPanel, "date_show_year", 1) == 1;
  showFullYearEnabled = jsonReadtoInt(configLedPanel, "date_full_year", 1) == 1;
  dateSeparatorBlinking = jsonReadtoInt(configLedPanel, "date_separator_blink", 1) == 1;
#endif
}

// =========================================================== НАСТРОЙКИ ОТОБРАЖЕНИЯ ПОГОДЫ НА МАТРИЦЕ ================================================
void loadWeatherSettings() {
#if LED_PANEL && USE_WEATHER
  if (jsonRead(configLedPanel, "weather_cycle").isEmpty()) {
    jsonWrite(configLedPanel, "weather_cycle", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "weather_hue").isEmpty()) {
    jsonWrite(configLedPanel, "weather_hue", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "rainbow_weather").isEmpty()) {
    jsonWrite(configLedPanel, "rainbow_weather", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "weather_enabled").isEmpty()) {
    jsonWrite(configLedPanel, "weather_enabled", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "weather_x_offset").isEmpty()) {
    jsonWrite(configLedPanel, "weather_x_offset", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "weather_y_offset").isEmpty()) {
    jsonWrite(configLedPanel, "weather_y_offset", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "degree_blink").isEmpty()) {
    jsonWrite(configLedPanel, "degree_blink", "1");
    configChanged = true;
  }

  weatherHue = jsonReadtoInt(configLedPanel, "weather_hue", 0);
  rainbowWeather = jsonReadtoInt(configLedPanel, "rainbow_weather", 0) == 1;
  weatherColorCycle = jsonReadtoInt(configLedPanel, "weather_cycle", 0) == 1;
  weatherEnabled = jsonReadtoInt(configLedPanel, "weather_enabled", 0) == 1;
  weatherXOffset = jsonReadtoInt(configLedPanel, "weather_x_offset", 0);
  weatherYOffset = jsonReadtoInt(configLedPanel, "weather_y_offset", 0);
  degreeSymbolBlinking = jsonReadtoInt(configLedPanel, "degree_blink", 1) == 1;
#endif
}

// ===================================== НАСТРОЙКИ ТАЙМЕРОВ и ИНТЕРВАЛОВ ПЕРЕКЛЮЧЕНИЯ НА МАТРИЦЕ ЧАСОВ, ДАТЫ, ПОГОДЫ ==================================
// c - clock (часы), d - date (дата), w - weather (погода)
// ---------------------------------------------------------

void loadTimerSettings() {
#if LED_PANEL
  // Таймер: Часы / Погода
  if (jsonRead(configLedInterval, "timer_c_w").isEmpty()) {
    jsonWrite(configLedInterval, "timer_c_w", "0");
    configChanged = true;
  }
  if (jsonRead(configLedInterval, "interval_c_w").isEmpty()) {
    jsonWrite(configLedInterval, "interval_c_w", "10");
    configChanged = true;
  }
  // Таймер: Дата / Погода
  if (jsonRead(configLedInterval, "timer_d_w").isEmpty()) {
    jsonWrite(configLedInterval, "timer_d_w", "0");
    configChanged = true;
  }
  if (jsonRead(configLedInterval, "interval_d_w").isEmpty()) {
    jsonWrite(configLedInterval, "interval_d_w", "10");
    configChanged = true;
  }
  // Таймер: Часы / Дата / Погода
  if (jsonRead(configLedInterval, "timer_c_d_w").isEmpty()) {
    jsonWrite(configLedInterval, "timer_c_d_w", "0");
    configChanged = true;
  }
  if (jsonRead(configLedInterval, "interval_c_d_w").isEmpty()) {
    jsonWrite(configLedInterval, "interval_c_d_w", "10");
    configChanged = true;
  }
  // Таймер: Часы / Дата
  if (jsonRead(configLedInterval, "timer_c_d").isEmpty()) {
    jsonWrite(configLedInterval, "timer_c_d", "0");
    configChanged = true;
  }
  if (jsonRead(configLedInterval, "interval_c_d").isEmpty()) {
    jsonWrite(configLedInterval, "interval_c_d", "10");
    configChanged = true;
  }
  // Таймер: Часы + Дата / Погода (часы всегда, дата/погода чередуются)
  if (jsonRead(configLedInterval, "timer_clock_fixed").isEmpty()) {
    jsonWrite(configLedInterval, "timer_clock_fixed", "0");
    configChanged = true;
  }
  if (jsonRead(configLedInterval, "interval_clock_fixed").isEmpty()) {
    jsonWrite(configLedInterval, "interval_clock_fixed", "30");
    configChanged = true;
  }
  timer_clock_fixed = (jsonReadtoInt(configLedInterval, "timer_clock_fixed", 0) == 1);
  interval_clock_fixed = constrain(jsonReadtoInt(configLedInterval, "interval_clock_fixed", 10), 5, 600);
  timer_c_w = (jsonReadtoInt(configLedInterval, "timer_c_w", 0) == 1);
  interval_c_w = constrain(jsonReadtoInt(configLedInterval, "interval_c_w", 10), 5, 600);
  timer_d_w = (jsonReadtoInt(configLedInterval, "timer_d_w", 0) == 1);
  interval_d_w = constrain(jsonReadtoInt(configLedInterval, "interval_d_w", 10), 5, 600);
  timer_c_d_w = (jsonReadtoInt(configLedInterval, "timer_c_d_w", 0) == 1);
  interval_c_d_w = constrain(jsonReadtoInt(configLedInterval, "interval_c_d_w", 10), 5, 600);
  timer_c_d = (jsonReadtoInt(configLedInterval, "timer_c_d", 0) == 1);
  interval_c_d = constrain(jsonReadtoInt(configLedInterval, "interval_c_d", 10), 5, 600);

#if USE_WEATHER
  // Взаимоисключаемость таймеров (активен может быть только один)
  if (timer_clock_fixed) {
    timer_c_w = timer_c_d = timer_d_w = timer_c_d_w = false;
    jsonWrite(configLedInterval, "timer_c_w", "0");
    jsonWrite(configLedInterval, "timer_c_d", "0");
    jsonWrite(configLedInterval, "timer_d_w", "0");
    jsonWrite(configLedInterval, "timer_c_d_w", "0");
    configChanged = true;
  }
  else if (timer_c_d_w) {
    timer_c_w = timer_c_d = timer_d_w = false;
    jsonWrite(configLedInterval, "timer_c_w", "0");
    jsonWrite(configLedInterval, "timer_c_d", "0");
    jsonWrite(configLedInterval, "timer_d_w", "0");
    configChanged = true;
  }
  else if (timer_c_w) {
    timer_c_d = timer_d_w = timer_c_d_w = false;
    jsonWrite(configLedInterval, "timer_c_d", "0");
    jsonWrite(configLedInterval, "timer_d_w", "0");
    jsonWrite(configLedInterval, "timer_c_d_w", "0");
    configChanged = true;
  }
  else if (timer_c_d) {
    timer_c_w = timer_d_w = timer_c_d_w = false;
    jsonWrite(configLedInterval, "timer_c_w", "0");
    jsonWrite(configLedInterval, "timer_d_w", "0");
    jsonWrite(configLedInterval, "timer_c_d_w", "0");
    configChanged = true;
  }
  else if (timer_d_w) {
    timer_c_w = timer_c_d = timer_c_d_w = false;
    jsonWrite(configLedInterval, "timer_c_w", "0");
    jsonWrite(configLedInterval, "timer_c_d", "0");
    jsonWrite(configLedInterval, "timer_c_d_w", "0");
    configChanged = true;
  }
#endif // USE_WEATHER
#endif // LED_PANEL
}

// ============================================= НАСТРОЙКИ ИНТЕРВАЛОВ ВЫВОДА БЕГУЩЕЙ СТРОКОЙ ЧАСОВ и ПОГОДЫ ===========================================
void loadClockAndWeatherIntervals() {
  //bool configChanged = false;

  // Интервал вывода времени (минуты)
  int printTimeTmp = jsonReadtoInt(configLedInterval, "print_time", 30);
  if (printTimeTmp < 1 || printTimeTmp > 60) {
    printTimeTmp = 30;
    jsonWrite(configLedInterval, "print_time", 30);
    configChanged = true;
  }
  PRINT_TIME = printTimeTmp;
  // -----------------------------------------------------------------
#if USE_WEATHER
  // Интервал вывода погоды (минуты)
  int printWeatherTmp = jsonReadtoInt(configLedInterval, "print_weather", 10);
  if (printWeatherTmp < 1 || printWeatherTmp > 60) {
    printWeatherTmp = 10;
    jsonWrite(configLedInterval, "print_weather", 10);
    configChanged = true;
  }
  PRINT_WEATHER = printWeatherTmp;
#endif
}

// ==================================================================== НАСТРОЙКИ БЕГУЩЕЙ СТРОКИ ======================================================
void loadRunningTextSettings() {
  if (jsonRead(configLedPanel, "spt").isEmpty()) {
    jsonWrite(configLedPanel, "spt", "118");
    configChanged = true;
  }
  if (!jsonRead(configLedPanel, "sct").isEmpty() && jsonRead(configLedPanel, "run_text_hue").isEmpty()) {
    int oldHue = constrain(jsonReadtoInt(configLedPanel, "sct", 0), 0, 255);
    jsonWrite(configLedPanel, "run_text_hue", String(oldHue));
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "run_text").isEmpty()) {
    jsonWrite(configLedPanel, "run_text", "Привет");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "run_text_enabled").isEmpty()) {
    jsonWrite(configLedPanel, "run_text_enabled", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "run_text_over").isEmpty()) {
    jsonWrite(configLedPanel, "run_text_over", "0");
    configChanged = true;
  }
  if (jsonRead(configLedInterval, "interval_run_text").isEmpty()) {
    jsonWrite(configLedInterval, "interval_run_text", "1");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "rainbow_text").isEmpty()) {
    jsonWrite(configLedPanel, "rainbow_text", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "run_text_hue").isEmpty()) {
    jsonWrite(configLedPanel, "run_text_hue", "0");
    configChanged = true;
  }
  if (jsonRead(configLedPanel, "run_text_cycle").isEmpty()) {
    jsonWrite(configLedPanel, "run_text_cycle", "0");
    configChanged = true;
  }

  SpeedRunningText = jsonReadtoInt(configLedPanel, "spt", 80);
  if (SpeedRunningText < 20 || SpeedRunningText > 220) {
    SpeedRunningText = 80;
    jsonWrite(configLedPanel, "spt", SpeedRunningText);
    configChanged = true;          // вместо saveConfig()
  }

  ColorTextFon = jsonReadtoInt(configLedPanel, "ctf", 0);
  runTextHue = jsonReadtoInt(configLedPanel, "run_text_hue", 0);
  runTextColorCycle = (jsonReadtoInt(configLedPanel, "run_text_cycle", 0) == 1);
  rainbowText = (jsonReadtoInt(configLedPanel, "rainbow_text", 0) == 1);
  autoRunTextHue = runTextColorCycle;

  int sctForDisplay = runTextColorCycle ? 255 : runTextHue;
  if (jsonReadtoInt(configLedPanel, "sct", 0) != sctForDisplay) {
    jsonWrite(configLedPanel, "sct", String(sctForDisplay));
    configChanged = true;
  }
  // -----------------------------------------------------------------
  // Цвет
  autoRunTextHue = runTextColorCycle;
  if (rainbowText) {
    runTextColorCycle = false;
    autoRunTextHue = false;
    jsonWrite(configLedPanel, "run_text_cycle", "0");
    configChanged = true;
  }

  if (runTextColorCycle) {
    ColorRunningText = 0;
  } else {
    ColorRunningText = runTextHue;
  }

  runTextStr = jsonRead(configLedPanel, "run_text");
  runTextStr.trim();
  runTextStr.toCharArray(TextTicker, sizeof(TextTicker) - 1);
  TextTicker[sizeof(TextTicker) - 1] = '\0';
  runTextEnabled = jsonReadtoInt(configLedPanel, "run_text_enabled", 0) == 1;
  runTextOver = jsonReadtoInt(configLedPanel, "run_text_over", 0) == 1;

  if (jsonRead(configLedPanel, "text_y_offset").isEmpty()) {
    jsonWrite(configLedPanel, "text_y_offset", "0");
    configChanged = true;
  }
  textYOffset = jsonReadtoInt(configLedPanel, "text_y_offset", 0);
  // -----------------------------------------------------------------
  // Интервалы
  IntervalrunText = constrain(jsonReadtoInt(configLedInterval, "interval_run_text", 1), 0, 60);
  if (!runTextEnabled) {
    runningTextTimer.setInterval(TIMER_DISABLED);
    textIsRunning = false;
  } else if (IntervalrunText == 0) {
    runningTextTimer.setInterval(TIMER_DISABLED);
    textIsRunning = true;
    loadingFlag = true;
  } else {
    runningTextTimer.setInterval((uint32_t)IntervalrunText * 60000UL);
    textIsRunning = false;
  }
  runningTextTimer.reset();
  // -----------------------------------------------------------------
  // Часы бегущей строкой
  String runTimeTextVal = jsonRead(configLedPanel, "run_time_text_enabled");
  if (runTimeTextVal.length() == 0 || runTimeTextVal == "null") {
    jsonWrite(configLedPanel, "run_time_text_enabled", "1");
    configChanged = true;
    runTimeTextVal = "1";
  }
  runTimeTextEnabled = (runTimeTextVal == "1" || runTimeTextVal == "true");
  // -----------------------------------------------------------------
  // Погода бегущей строкой
#if USE_WEATHER
  String runWeatherTextVal = jsonRead(configLedPanel, "run_weather_text_enabled");
  if (runWeatherTextVal.length() == 0 || runWeatherTextVal == "null") {
    jsonWrite(configLedPanel, "run_weather_text_enabled", "1");
    configChanged = true;
    runWeatherTextVal = "1";
  }
  runWeatherTextEnabled = (runWeatherTextVal == "1" || runWeatherTextVal == "true");
#endif
}

// ======================================================================== НАСТРОЙКИ ШРИФТА ==========================================================
void loadFontSettings() {
#if LED_PANEL
  String fontStr = jsonRead(configLedPanel, "font_size");
  int fontId = FONT_SIZE;
  if (fontStr.isEmpty() || fontStr == "null") {
    jsonWrite(configLedPanel, "font_size", String(fontId));
    configChanged = true;
  } else {
    int val = fontStr.toInt();
    if (val >= 0 && val <= 2) fontId = val;
    else jsonWrite(configLedPanel, "font_size", String(FONT_SIZE));
  }
  setFontSize(fontId);
#endif
}

void loadStaticFontSettings() {
#if LED_PANEL
  String fontStr = jsonRead(configLedPanel, "static_font");
  int fontId = STATIC_FONT;   // значение по умолчанию из Constants.h
  if (!fontStr.isEmpty() && fontStr != "null") {
    int val = fontStr.toInt();
    if (val >= 0 && val <= 3) {
      fontId = val;
    } else {
      jsonWrite(configLedPanel, "static_font", String(fontId));
      configChanged = true;
    }
  } else {
    jsonWrite(configLedPanel, "static_font", String(fontId));
    configChanged = true;
  }
  staticFont = fontId;
#endif
}

// ================================================================= НАСТРОЙКИ ЭФФЕКТОВ и ЦИКЛОВ ======================================================
void loadEffectsAndCycleSettings() {
  // Корректировки эффектов
  {
    String Name = F("correct.");
    Name += jsonRead(configSetup, "lang");
    Name += F(".json");
    String Correct = readFile(Name, 2048);
    for (uint8_t n = 0; n < MODE_AMOUNT; n++) {
      eff_num_correct[n] = jsonReadtoInt(Correct, String(n));
    }
  }

#ifdef USE_SHUFFLE_FAVORITES
  for (uint8_t i = 0; i < MODE_AMOUNT; i++)
    shuffleFavoriteModes[i] = i;
#endif

  Eeprom::instance().InitEepromSettings(modes, &restoreSettings);

  Favorites::instance().FavoritesRunning = jsonReadtoInt(configSetup, "cycle_on");
  Favorites::instance().Interval = jsonReadtoInt(configSetup, "time_eff");
  Favorites::instance().Dispersion = jsonReadtoInt(configSetup, "disp");
  Favorites::instance().UseSavedFavoritesRunning = jsonReadtoInt(configSetup, "cycle_always");

  currentMode = eff_num_correct[jsonReadtoInt(configSetup, "eff_sel")];
  modes[currentMode].Brightness = jsonReadtoInt(configSetup, "br");
  modes[currentMode].Speed = jsonReadtoInt(configSetup, "sp");
  modes[currentMode].Scale = jsonReadtoInt(configSetup, "sc");

  SetBrightness(modes[currentMode].Brightness);
  FastLED.setBrightness(modes[currentMode].Brightness);
  FastLED.show();

  if (configCycle.length() > 0 && configCycle != "{}") {
    for (int i = 0; i < MODE_AMOUNT; i++) {
      String key = "e" + String(i);
      int val = jsonReadtoInt(configCycle, key);
      Favorites::instance().FavoriteModes[i] = (val == 1) ? 1 : 0;
    }
  } else {
    for (int i = 0; i < MODE_AMOUNT; i++) {
      Favorites::instance().FavoriteModes[i] = 1;
    }
    String defaultCycle = "{";
    for (int i = 0; i < MODE_AMOUNT; i++) {
      defaultCycle += "\"e" + String(i) + "\":" + String(Favorites::instance().FavoriteModes[i]) + ((i < MODE_AMOUNT - 1) ? "," : "");
    }
    defaultCycle += "}";
    configCycle = defaultCycle;
    configChanged = true;
  }
}

// ============================================================= ЗАГРУЗКА ДЕФОЛТНЫХ НАСТРОЕК ЭФФЕКТОВ =================================================
void loadDefaultEffectSettings() {
  for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
    modes[i].Brightness = pgm_read_byte(&defaultSettings[i][0]);
    modes[i].Speed = pgm_read_byte(&defaultSettings[i][1]);
    modes[i].Scale = pgm_read_byte(&defaultSettings[i][2]);
  }
}

// ================================================================== ЗАГРУЗКА НАСТРОЕК ЭФФЕКТОВ ======================================================
void loadEffectSettings() {
  static bool initialized = false;
  if (!initialized) {
    initialized = true;
    if (modes[0].Brightness == 0) {
      loadDefaultEffectSettings();
    }
  }

  if (currentMode >= MODE_AMOUNT) {
    currentMode = EFF_CLOCK;
  }

  loadBrightnessForMode(currentMode);
}

// ==================================================================== ЗАГРУЗКА МОДУЛЕЙ ==============================================================
void loadModuleSettings() {
#if USE_BUTTON
#if defined(ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ARDUINO_ESP32S3_DEV)
  buttonEnabled = jsonReadtoInt(configSetup, "button_enabled", 0) == 1;
#else
  buttonEnabled = true;
#endif
#endif

#if USE_IR_RECEIVER
#if defined(ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ARDUINO_ESP32S3_DEV)
  irEnabled = jsonReadtoInt(configSetup, "ir_enabled", 0) == 1;
#else
  irEnabled = true;
#endif
#endif

#if USE_RF_RECEIVER
#if defined(ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ARDUINO_ESP32S3_DEV)
  rfEnabled = jsonReadtoInt(configSetup, "rf_enabled", 0) == 1;
#else
  rfEnabled = true;
#endif
#endif

#if USE_TM1637
#if defined(ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ARDUINO_ESP32S3_DEV)
  tm1637Enabled = jsonReadtoInt(configSetup, "tm1637_enabled", 0) == 1;
#else
  tm1637Enabled = true;
#endif
#endif

#if USE_ST7789
#if defined(ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ARDUINO_ESP32S3_DEV)
  st7789Enabled = jsonReadtoInt(configSetup, "st7789_enabled", 0) == 1;
#else
  st7789Enabled = true;
#endif
#endif

#if USE_MP3_PLAYER
#if defined(ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ARDUINO_ESP32S3_DEV)
  mp3Enabled = jsonReadtoInt(configSetup, "mp3_enabled", 0) == 1;
#else
  mp3Enabled = true;
#endif
#endif
}

// =================================================================== ЗАГРУЗКА ВСЕХ НАСТРОЕК =========================================================
void loadAllSettings() {
  loadBasicSettings();            // Базовые настройки (LAMP_NAME, AP_NAME, AP_PASS, time_always, ночные/дневные режимы и т.д.)
  loadLocalAuthSettings();        // local_auth (установка пароля для редактора)
  loadBrightness();               // Яркость
  loadUserBrightness();           // Пользовательская яркость
  loadRunningTextSettings();      // Бегущая строка
  loadClockSettings();            // Часы на матрице
  initNightClockSettings();       // Ночные часы
  loadDateSettings();             // Дата на матрице
  loadWeatherSettings();          // Погода на матрице
  loadTimerSettings();            // Таймеры отображения на матрице часы/дата/погода
  loadModuleSettings();
  loadFontSettings();             // Шрифт текста бегущей строки
  loadStaticFontSettings();       // загрузка шрифта для статики (часы, дата, погода)
  loadDawnSunsetSettings();       // Яркость рассвета и заката
#if USE_ST7789
  loadST7789Settings();           // Дисплей ST7789
#endif
#if USE_WEATHER
  Weather::instance().loadSettings(); // Погода (OpenWeather, Яндекс)
#endif
  loadDisplaySwitchSettings();    // Общее переключение "Часы / Погода / Дата" на дисплеях TM1637 / ST7789
  loadClockAndWeatherIntervals(); // Интервал вывода бегущей строкой часов и погоды
#if USE_MP3_PLAYER
  loadMP3Settings();              // MP3 плеер
#endif
#if USE_BUTTON
  loadButtonSettings();           // Кнопка
#endif
#if USE_MQTT
  loadMQTTSettings();             // MQTT
#endif
#if USE_MULTILAMP
  loadMultilampSettings();        // Настройки управления несколькими лампами
#endif
#if USE_DAWN
  loadAlarmSettings();            // Будильник Рассвет
#endif
#if USE_SUNSET
  loadSunsetSettings();           // Режим Закат
#endif
#if USE_SCHEDULE
  loadScheduleSettings();         // Расписание лампы
#endif
  loadEffectsAndCycleSettings();  // Корректировки эффектов, циклы, избранное
  loadEffectSettings();           // Загрузка настроек эффектов

  if (configChanged) {
    saveConfig();
    configChanged = false;
  }
}

// ******************************************************************************************************************************************************

// ***************************************************************************** button.ino *************************************************************
#include "Time.h"
// --------------------------

#if USE_BUTTON
bool brightDirection = true;               // true — увеличение, false — уменьшение
static bool startButtonHolding = false;    // кнопка удерживается для изменения яркости/скорости/масштаба лампы кнопкой
static bool Button_Holding = false;

void buttonTick() {
  if (!buttonEnabled) {
    touch.tick();
    if (touch.isStep() && touch.getHoldClicks() == 14U) {
#if BUTTON_LOG
      SYSLOG.add("\n*** Reset to Default ***");
#endif
      showWarning(CRGB::Red, 500, 250U);
      setModeSettings();
      updateSets();
      if (FileCopy(F("/default/config.json"), F("/config.json"))) {
        showWarning(CRGB::Green, 500, 250U);
        ESP.restart();
      } else {
        showWarning(CRGB::Red, 500, 250U);
      }
    }
    return;
  }

  touch.tick();
  uint8_t clickCount = touch.hasClicks() ? touch.getClicks() : 0U;
  // --------------------------------------------------------------------------------

// 1 клик — Вкл/Выкл (или обработка рассвета/заката)
if (clickCount == btn_click_power) {
    // Если кнопка отключена через веб – игнорируем
    #if USE_BUTTON
    if (!buttonEnabled) return;
    #endif

    #if USE_DAWN || USE_SUNSET
    if (dawnFlag == 1 || sunsetFlag == 1) {
        if (dawnFlag == 1) {
            manualOff = true;
            dawnFlag = 0;
            #if USE_MP3_PLAYER
            if (mp3Enabled && alarm_sound_flag) {
                send_command(0x0E, 0, 0, 0);
                mp3_stop = true;
                alarm_sound_flag = false;
            }
            #endif
        } else if (sunsetFlag == 1) {
            manualOff = true;
            sunsetFlag = 0;
            #if USE_MP3_PLAYER
            if (mp3Enabled && sunset_sound_flag) {
                sunset_sound_flag = false;
            }
            #endif
        }
        #if USE_TM1637
        if (tm1637Enabled) {
            clockTicker_blink();
        }
        #endif
        FastLED.clear();
        FastLED.setBrightness(0);
        FastLED.show();
        yield();
        ONflag = true;
        jsonWrite(configSetup, "Power", ONflag);
        saveConfig();
        loadingFlag = true;
        changePower();
        return;
    }
    #endif // USE_DAWN || USE_SUNSET

    ONflag = !ONflag;
    jsonWrite(configSetup, "Power", ONflag);
    saveConfig();

    changePower();

    if (!ONflag) {
        timeout_save_file_changes = millis() - SAVE_FILE_DELAY_TIMEOUT;
        if (!Favorites::instance().FavoritesRunning) {
            Eeprom::instance().EepromPut(modes);
        }
        save_file_changes = 7;
        Save_File_Changes();
    } else {
        Eeprom::instance().EepromGet(modes);
        timeout_save_file_changes = millis();
        bitSet(save_file_changes, 0);
    }
    loadingFlag = true;

    #if USE_MQTT
    if (Wifi::instance().isConnected()) {
        Mqtt::instance().needToPublish = true;
    }
    #endif
    #if USE_BLYNK
    updateRemoteBlynkParams();
    #endif
    #if USE_MULTILAMP
    if (ONflag) repeat_multiple_lamp_control = true;
    else multiple_lamp_control();
    #endif
}

  // --------------------------------------------------------------------------------
  // 2 клика — Следующий эффект
  if (clickCount == btn_click_next && ONflag) {
    uint8_t temp = jsonReadtoInt(configSetup, "eff_sel");
    uint8_t lastMode = currentMode;

    if (Favorit_only) {
      do {
        if (++temp >= MODE_AMOUNT) temp = 0;
        currentMode = eff_num_correct[temp];
      }
      while (Favorites::instance().FavoriteModes[currentMode] == 0 && currentMode != lastMode);
    } else if (++temp >= MODE_AMOUNT) temp = 0;

    currentMode = eff_num_correct[temp];

    jsonWrite(configSetup, "eff_sel", temp);
    jsonWrite(configSetup, "br", modes[currentMode].Brightness);
    jsonWrite(configSetup, "sp", modes[currentMode].Speed);
    jsonWrite(configSetup, "sc", modes[currentMode].Scale);

#if USE_MP3_PLAYER
    mp3_folder = pgm_read_byte(&default_effects_folders[currentMode]);
#endif

    SetBrightness(modes[currentMode].Brightness);
    saveConfig();
    loadingFlag = true;
#if USE_MQTT
    if (Wifi::instance().isConnected()) {
      Mqtt::instance().needToPublish = true;
    }
#endif
#if USE_BLYNK
    updateRemoteBlynkParams();
#endif
#if USE_MULTILAMP
    repeat_multiple_lamp_control = true;
#endif
  }

  // --------------------------------------------------------------------------------
  // 3 клика — Предыдущий эффект
  if (clickCount == btn_click_prev && ONflag) {
    uint8_t temp = jsonReadtoInt(configSetup, "eff_sel");
    uint8_t lastMode = currentMode;

    if (Favorit_only) {
      do {
        if (--temp >= MODE_AMOUNT) temp = MODE_AMOUNT - 1;
        currentMode = eff_num_correct[temp];
      }
      while (Favorites::instance().FavoriteModes[currentMode] == 0 && currentMode != lastMode);
    } else if (--temp >= MODE_AMOUNT) temp = MODE_AMOUNT - 1;

    currentMode = eff_num_correct[temp];

    jsonWrite(configSetup, "eff_sel", temp);
    jsonWrite(configSetup, "br", modes[currentMode].Brightness);
    jsonWrite(configSetup, "sp", modes[currentMode].Speed);
    jsonWrite(configSetup, "sc", modes[currentMode].Scale);

#if USE_MP3_PLAYER
    mp3_folder = pgm_read_byte(&default_effects_folders[currentMode]);
#endif

    SetBrightness(modes[currentMode].Brightness);
    saveConfig();
    loadingFlag = true;

#if USE_MQTT
    if (Wifi::instance().isConnected()) {
      Mqtt::instance().needToPublish = true;
    }
#endif
#if USE_BLYNK
    updateRemoteBlynkParams();
#endif
#if USE_MULTILAMP
    repeat_multiple_lamp_control = true;
#endif
  }

  // --------------------------------------------------------------------------------
  // 4 клика — OTA
  if (clickCount == btn_click_action4) {
#if USE_OTA
    if (Ota::instance().RequestOtaUpdate()) {
      showWarning(CRGB::Yellow, 2000, 500);
      ONflag = true;
      jsonWrite(configSetup, "Power", ONflag);
      currentMode = EFF_WHITE_COLOR;
      jsonWrite(configSetup, "eff_sel", currentMode);
      jsonWrite(configSetup, "br", 255);
      saveConfig();
      changePower();
    }
#endif
  }

  // --------------------------------------------------------------------------------
  // 5 кликов — Показ IP
  if (clickCount == btn_click_ip) {
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
#endif
    loadingFlag = true;
    auto& wifi = Wifi::instance();
    String textToShow = wifi.isConnected() ? wifi.localIP().toString() : "AP: " + wifi.apIP().toString();
    while (!fillString(textToShow.c_str(), CRGB::White, false)) {
      delay(1);
    }
#if USE_ST7789
    if (st7789Enabled) {
        TFT_HideIP();
    }
#endif
    loadingFlag = true;
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, ONflag ? MOSFET_LEVEL : !MOSFET_LEVEL);
#endif
  }

  // --------------------------------------------------------------------------------
  // Озвучка времени
  if (clickCount == btn_click_time) {
#if USE_MP3_PLAYER
    if (mp3Enabled && mp3_player_connect == 4) {
      play_time_ADVERT(true);
    } else
#endif
      if (myTime.isTimeSet()) {
        printTime(true);
      } else {
        fillString("NO TIME", CRGB::Red, true);
      }
  }

  // --------------------------------------------------------------------------------
  // Озвучка погоды
  if (clickCount == btn_click_weather) {
#if USE_WEATHER
#if USE_MP3_PLAYER
    if (mp3Enabled && mp3_player_connect == 4) {
      play_weather(true);
    } else
#endif
    {
      uint8_t oldPrintWeather = PRINT_WEATHER;
      PRINT_WEATHER = 1;
      printWeather();
      PRINT_WEATHER = oldPrintWeather;
    }
#endif
  }

  // --------------------------------------------------------------------------------
  // Вкл/Выкл звук
  if (clickCount == btn_click_sound) {
#if USE_MP3_PLAYER
    if (mp3Enabled && mp3_player_connect == 4) {
      eff_sound_on = eff_sound_on ? 0 : eff_volume;
      showWarning(eff_sound_on ? CRGB::Blue : CRGB::Yellow, 1000, 250U);
      jsonWrite(configSetup, "on_sound", eff_sound_on > 0 ? 1 : 0);
      saveConfig();
#if USE_MULTILAMP
      repeat_multiple_lamp_control = true;
#endif
    } else {
      showWarning(CRGB::Red, 1000, 250U);
    }
#endif
  }

  // --------------------------------------------------------------------------------
  // кнопка только начала удерживаться
  if (touch.isHolded()) {
    brightDirection = !brightDirection;
    startButtonHolding = true;
  }

  // кнопка нажата и удерживается
  if (touch.isStep()) {
    if (ONflag && !Button_Holding) {
      int8_t but = touch.getHoldClicks();
      switch (but) {
        // изменение яркости
        case 0U: {
            uint8_t delta = modes[currentMode].Brightness < 10U ? 1U : 5U;
            modes[currentMode].Brightness = constrain(brightDirection ? modes[currentMode].Brightness + delta : modes[currentMode].Brightness - delta, 1, 255);
            jsonWrite(configSetup, "br", modes[currentMode].Brightness);
            SetBrightness(modes[currentMode].Brightness);
            saveConfig();
#if USE_TM1637
            DisplayFlag = 3;
            Display_Timer(modes[currentMode].Brightness);
#endif
#if USE_ST7789
            DisplayFlag = 3;
            TFT_Display_Timer(modes[currentMode].Brightness);
#endif
#if USE_MULTILAMP
            repeat_multiple_lamp_control = true;
#endif
            break;
          }
        // изменение скорости
        case 1U: {
            modes[currentMode].Speed = constrain(brightDirection ? modes[currentMode].Speed + 1 : modes[currentMode].Speed - 1, 1, 255);
            jsonWrite(configSetup, "sp", modes[currentMode].Speed);
            saveConfig();
            loadingFlag = true;
#if USE_TM1637
            DisplayFlag = 3;
            Display_Timer(modes[currentMode].Speed);
#endif
#if USE_ST7789
            DisplayFlag = 3;
            TFT_Display_Timer(modes[currentMode].Speed);
#endif
#if USE_MULTILAMP
            repeat_multiple_lamp_control = true;
#endif
            break;
          }
        // изменение масштаба
        case 2U: {
            modes[currentMode].Scale = constrain(brightDirection ? modes[currentMode].Scale + 1 : modes[currentMode].Scale - 1, 1, 100);
            jsonWrite(configSetup, "sc", modes[currentMode].Scale);
            saveConfig();
            loadingFlag = true;
#if USE_TM1637
            DisplayFlag = 3;
            Display_Timer(modes[currentMode].Scale);
#endif
#if USE_ST7789
            DisplayFlag = 3;
            TFT_Display_Timer(modes[currentMode].Scale);
#endif
#if USE_MULTILAMP
            repeat_multiple_lamp_control = true;
#endif
            break;
          }
        // сброс config.json
        case 14U: {
            showWarning(CRGB::Red, 500, 250U);

            setModeSettings();
            updateSets();
            if (FileCopy(F("/default/config.json"), F("/config.json"))) {
              showWarning(CRGB::Green, 2500, 250U);
              ESP.restart();
            } else {
              showWarning(CRGB::Red, 2500, 250U);
            } break;
          }

        case 19U: {
            showWarning(CRGB::Red, 500, 250U);
            setModeSettings();
            updateSets();

            // конфиги
            if (FileCopy(F("/default/config.json"), F("/config.json"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);

            if (FileCopy(F("/default/config_cycle.json"), F("/config_cycle.json"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);

            if (FileCopy(F("/default/config_button.json"), F("/config_button.json"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);

            if (FileCopy(F("/default/config_led_panel.json"), F("/config_led_panel.json"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);

            if (FileCopy(F("/default/config_led_interval.json"), F("/config_led_interval.json"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);

            if (FileCopy(F("/default/config_led_matrix.json"), F("/config_led_matrix.json"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);

            if (FileCopy(F("/default/config_wifi.json"), F("/config_wifi.json"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);

            if (FileCopy(F("/default/config_st7789.json"), F("/config_st7789.json"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);

#if USE_MP3_PLAYER
            if (FileCopy(F("/default/sound_list.json"), F("/sound_list.json"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);

            if (FileCopy(F("/default/config_mp3.json"), F("/config_mp3.json"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);
#endif

#if USE_DAWN
            if (FileCopy(F("/default/config_alarm.json"), F("/config_alarm.json"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);
#endif

#if USE_SUNSET
            if (FileCopy(F("/default/config_sunset.json"), F("/config_sunset.json"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);
#endif

#if USE_SCHEDULE
            if (FileCopy(F("/default/config_schedule.json"), F("/config_schedule.json"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);
#endif

#if USE_WEATHER
            if (FileCopy(F("/default/config_weather.json"), F("/config_weather.json"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);
#endif

#if USE_MULTILAMP
            if (FileCopy(F("/default/config_multilamp.json"), F("/config_multilamp.json"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);
#endif

#if USE_MQTT
            if (FileCopy(F("/default/config_mqtt.json"), F("/config_mqtt.json"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);
#endif

            // Веб-интерфейс
            if (FileCopy(F("/default/index.json.gz"), F("/index.json.gz"))) showWarning(CRGB::Green, 500, 250U);
            else showWarning(CRGB::Red, 500, 250U);

            ESP.restart();
            break;
        } default:
          break;
      }
    } else {
      if (!Button_Holding) {
        int8_t but = touch.getHoldClicks();
        switch (but) {
          // белый свет при выключенной лампе
          case 0U: {
              Button_Holding = true;
              currentMode = EFF_WHITE_COLOR;
#if USE_MP3_PLAYER
              mp3_folder = pgm_read_byte(&default_effects_folders[currentMode]);
#endif
              for (uint8_t n = 0; n < MODE_AMOUNT; n++) {
                if (eff_num_correct[n] == currentMode) {
                  jsonWrite(configSetup, "eff_sel", n);
                  break;
                }
              }
              jsonWrite(configSetup, "br", modes[currentMode].Brightness);
              jsonWrite(configSetup, "sp", modes[currentMode].Speed);
              jsonWrite(configSetup, "sc", modes[currentMode].Scale);
              saveConfig();
              ONflag = true;
              jsonWrite(configSetup, "Power", ONflag);
              changePower();
#if USE_BLYNK
              updateRemoteBlynkParams();
#endif
              break;
            }
          case 14U: {
              showWarning(CRGB::Red, 500, 250U);

              if (FileCopy(F("/default/config.json"), F("/config.json"))) {

                showWarning(CRGB::Green, 2500, 250U);
                ESP.restart();
              } else {
                showWarning(CRGB::Red, 2500, 250U);
              } break;
            }
          case 19U: {
              showWarning(CRGB::Red, 500, 250U);
              setModeSettings();
              updateSets();

              // конфиги
              if (FileCopy(F("/default/config.json"), F("/config.json"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);

              if (FileCopy(F("/default/config_cycle.json"), F("/config_cycle.json"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);

              if (FileCopy(F("/default/config_button.json"), F("/config_button.json"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);

              if (FileCopy(F("/default/config_led_panel.json"), F("/config_led_panel.json"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);

              if (FileCopy(F("/default/config_led_interval.json"), F("/config_led_interval.json"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);

              if (FileCopy(F("/default/config_led_matrix.json"), F("/config_led_matrix.json"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);

              if (FileCopy(F("/default/config_wifi.json"), F("/config_wifi.json"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);

              if (FileCopy(F("/default/config_st7789.json"), F("/config_st7789.json"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);

#if USE_WEATHER
              if (FileCopy(F("/default/config_weather.json"), F("/config_weather.json"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);
#endif

#if USE_MP3_PLAYER
              if (FileCopy(F("/default/sound_list.json"), F("/sound_list.json"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);

              if (FileCopy(F("/default/config_mp3.json"), F("/config_mp3.json"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);
#endif

#if USE_DAWN
              if (FileCopy(F("/default/config_alarm.json"), F("/config_alarm.json"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);
#endif

#if USE_SUNSET
              if (FileCopy(F("/default/config_sunset.json"), F("/config_sunset.json"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);
#endif

#if USE_SCHEDULE
              if (FileCopy(F("/default/config_schedule.json"), F("/config_schedule.json"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);
#endif

#if USE_MULTILAMP
              if (FileCopy(F("/default/config_multilamp.json"), F("/config_multilamp.json"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);
#endif

#if USE_MQTT
              if (FileCopy(F("/default/config_mqtt.json"), F("/config_mqtt.json"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);
#endif

              // Веб
              if (FileCopy(F("/default/index.json.gz"), F("/index.json.gz"))) showWarning(CRGB::Green, 500, 250U);
              else showWarning(CRGB::Red, 500, 250U);

              ESP.restart();
              break;
            }
        }
      }
    }
  }

  // кнопка отпущена после удерживания
  if (ONflag && !touch.isHold() && startButtonHolding) {
    startButtonHolding = false;
    Button_Holding = false;
    loadingFlag = true;
#if USE_MQTT
    if (Wifi::instance().isConnected()) {
      Mqtt::instance().needToPublish = true;
    }
#endif
#if USE_BLYNK
    updateRemoteBlynkParams();
#endif
  }
}

#endif // USE_BUTTON

// *****************************************************************************************************************************************************

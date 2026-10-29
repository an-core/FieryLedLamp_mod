// ************************************************************************* ФУНКЦИИ IR, RF *************************************************************

#if (USE_IR_RECEIVER || USE_RF_RECEIVER)

// Вкл / Выкл лампы
void IR_Power() {
#if USE_DAWN
  if (dawnFlag == 1) {
    manualOff = true;
    dawnFlag = 0;
#if USE_MP3_PLAYER
    if (mp3Enabled && alarm_sound_flag) {
      mp3SendCmd(0x0E, 0, 0, 0);
      mp3_stop = true;
      alarm_sound_flag = false;
    }
#endif
#if USE_TM1637
    if (tm1637Enabled) {
      DisplayFlag = 0;
      clockTicker_blink();
    }
#endif

    ONflag = false;
    jsonWrite(configSetup, "Power", 0);

    FastLED.clear();
    FastLED.setBrightness(0);
    FastLED.show();
    yield();

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif
#if defined(ALARM_PIN) && defined(ALARM_LEVEL)
    digitalWrite(ALARM_PIN, !ALARM_LEVEL);
#endif

    saveConfig();
    loadingFlag = false;
    return;
  }
#endif

#if USE_SUNSET
  if (sunsetFlag == 1) {
    manualsOff = true;
    sunsetFlag = 0;
#if USE_MP3_PLAYER
    if (mp3Enabled && sunset_sound_flag) {
      mp3SendCmd(0x0E, 0, 0, 0);
      mp3_stop = true;
      sunset_sound_flag = false;
    }
#endif
#if USE_TM1637
    if (tm1637Enabled) {
      clockTicker_blink();
    }
#endif

    ONflag = false;
    jsonWrite(configSetup, "Power", 0);

    FastLED.clear();
    FastLED.setBrightness(0);
    FastLED.show();
    yield();

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif
#if defined(ALARM_PIN) && defined(ALARM_LEVEL)
    digitalWrite(ALARM_PIN, !ALARM_LEVEL);
#endif

    saveConfig();
    loadingFlag = false;
    return;
  }
#endif

  // обычное переключение Power
  ONflag = !ONflag;
  jsonWrite(configSetup, "Power", ONflag);

  if (!ONflag && leds != nullptr) {
    FastLED.setBrightness(0);
    FastLED.clear();
    FastLED.show();
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif
  }

  changePower();
  saveConfig();

  if (!ONflag) {
    timeout_save_file_changes = millis() - SAVE_FILE_DELAY_TIMEOUT;
    if (!Favorites::instance().FavoritesRunning) Eeprom::instance().EepromPut(modes);
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
  if (ONflag) {
    repeat_multiple_lamp_control = true;
  } else {
    multiple_lamp_control();
  }
#endif
}

// Вкл / Выкл звука
void Mute() {
#if USE_MP3_PLAYER
  if (mp3Enabled && mp3_player_connect >= 4) {
    if (eff_sound_on) {
      eff_sound_on = 0;
      advert_flag = false;
      weather_advert_flag = false;
      isAnnouncing = false;
      announceSessionActive = false;

      mp3SendCmd(0x0E, FEEDBACK, 0, 0);
      mp3_stop = true;
      pause_on = true;
      set_mp3_play_now = false;
    } else {
      eff_sound_on = eff_volume;
      if (ONflag) {
        uint8_t newFolder = effects_folders[currentMode];
        if (newFolder != 0) {
          mp3_folder = newFolder;
          mp3_folder_last = newFolder;
          mp3SendCmd(0x06, FEEDBACK, 0, eff_volume);
          mp3SendCmd(0x17, FEEDBACK, 0, newFolder);
          mp3_stop = false;
          pause_on = false;
          set_mp3_play_now = true;
        }
      }
    }
  } else {
    showWarning(CRGB::Red, 1000, 250U);
  }
  jsonWrite(configMP3, "on_sound", eff_sound_on > 0 ? 1 : 0);
  timeout_save_file_changes = millis();
  bitSet(save_file_changes, 0);
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#endif
}

// Следующий эффект, предыдущий эффект
void Prev_Next_eff(bool direction) {
  if (!ONflag) return;

  uint8_t temp = jsonReadtoInt(configSetup, "eff_sel");
  if (temp >= MODE_AMOUNT) temp = 0;

  auto step = [&](int dir) {
    int t = temp;
    uint8_t lastMode = currentMode;
    do {
      t += dir;
      if (t < 0) t = MODE_AMOUNT - 1;
      if (t >= MODE_AMOUNT) t = 0;
      currentMode = eff_num_correct[t];
    } while (Favorit_only
             && Favorites::instance().FavoriteModes[currentMode] == 0
             && currentMode != lastMode);
    temp = (uint8_t)t;
    // если в избранном никого нет — всё равно переключимся на следующий
    if (Favorit_only && currentMode == lastMode) {
      t += dir;
      if (t < 0) t = MODE_AMOUNT - 1;
      if (t >= MODE_AMOUNT) t = 0;
      currentMode = eff_num_correct[t];
      temp = (uint8_t)t;
    }
  };

  step(direction ? +1 : -1);

  jsonWrite(configSetup, "eff_sel", temp);
  jsonWrite(configSetup, "br", modes[currentMode].Brightness);
  jsonWrite(configSetup, "sp", modes[currentMode].Speed);
  jsonWrite(configSetup, "sc", modes[currentMode].Scale);
  SetBrightness(modes[currentMode].Brightness);
  loadingFlag = true;

#if USE_RADIO
  if (radioEnabled) {
    uint8_t stId = radioGetStationForMode(currentMode);
    if (stId > 0) radioPlayStation(stId);
    else radioStop();
  }
#endif

#if USE_TM1637
  if (tm1637Enabled) {
    DisplayFlag = 0;
    Display_Timer();
  }
#endif
#if USE_ST7789
  if (st7789Enabled) {
    DisplayFlag = 0;
    TFT_Display_Timer(0);
  }
#endif

  effectsTick();
  FastLED.show();

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
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP
}

void Cycle_on_off() {
  if (ONflag) {
    uint8_t tmp;
    jsonReadtoInt(configSetup, "cycle_on") == 0 ? tmp = 1 : tmp = 0;
    jsonWrite(configSetup, "cycle_on", tmp);
    saveConfig();
    Favorites::instance().FavoritesRunning = tmp;
    if (tmp) {
      showWarning(CRGB::Blue, 500, 250U); // мигание синим цветом 0.5 секунды
      Eeprom::instance().EepromPut(modes);
    } else {
      showWarning(CRGB::Red, 500, 250U); // мигание красным цветом 0.5 секунды
      Eeprom::instance().EepromGet(modes);
    }
#if USE_MQTT
    if (Wifi::instance().isConnected()) {
      Mqtt::instance().needToPublish = true;
    }
#endif
  }
}

void Bright_Up_Down(bool direction) {
  uint8_t delta = IR_Data_Ready == 1 ? 1U : 4U;
  modes[currentMode].Brightness = constrain(direction ? modes[currentMode].Brightness + delta : modes[currentMode].Brightness - delta, 1, 255);
  jsonWrite(configSetup, "br", modes[currentMode].Brightness);
  SetBrightness(modes[currentMode].Brightness);
#if USE_TM1637
  if (tm1637Enabled) {
    DisplayFlag = 3;
    Display_Timer(modes[currentMode].Brightness);
  }
#endif
#if USE_ST7789
  if (st7789Enabled) {
    DisplayFlag = 3;
    TFT_Display_Timer(modes[currentMode].Brightness);
  }
#endif

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

void Speed_Up_Down(bool direction) {
  uint8_t delta = IR_Data_Ready == 1 ? 1U : 4U;
  modes[currentMode].Speed = constrain(direction ? modes[currentMode].Speed + delta : modes[currentMode].Speed - delta, 1, 255);
  jsonWrite(configSetup, "sp", modes[currentMode].Speed);
  loadingFlag = true; // без перезапуска эффекта ничего и не увидишь
#if USE_TM1637
  if (tm1637Enabled) {
    DisplayFlag = 3;
    Display_Timer(modes[currentMode].Speed);
  }
#endif
#if USE_ST7789
  if (st7789Enabled) {
    DisplayFlag = 3;
    TFT_Display_Timer(modes[currentMode].Speed);
  }
#endif

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

void Scale_Up_Down(bool direction) {
  uint8_t delta = IR_Data_Ready == 1 ? 1U : 2U;
  modes[currentMode].Scale = constrain(direction ? modes[currentMode].Scale + delta : modes[currentMode].Scale - delta, 1, 100);
  jsonWrite(configSetup, "sc", modes[currentMode].Scale);
  loadingFlag = true; // без перезапуска эффекта ничего и не увидишь
#if USE_TM1637
  if (tm1637Enabled) {
    DisplayFlag = 3;
    Display_Timer(modes[currentMode].Scale);
  }
#endif
#if USE_ST7789
  if (st7789Enabled) {
    DisplayFlag = 3;
    TFT_Display_Timer(modes[currentMode].Scale);
  }
#endif

#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP
#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}

void Volum_Up_Down (bool direction) {
#if USE_MP3_PLAYER
  if (mp3Enabled) {
    eff_volume = constrain(direction ? eff_volume + 1 : eff_volume - 1, 1, 30);
    jsonWrite(configMP3, "vol", eff_volume);
    if (!dawnflag_sound) mp3SendCmd(0x06, FEEDBACK, 0, eff_volume); // Громкость
#if USE_TM1637
    if (tm1637Enabled) {
      DisplayFlag = 3;
      Display_Timer(eff_volume);
    }
#endif
#if USE_ST7789
    if (st7789Enabled) {
      DisplayFlag = 3;
      TFT_Display_Timer(eff_volume);
    }
#endif
#if USE_MULTILAMP
    repeat_multiple_lamp_control = true;
#endif
  }
#endif
}

void Print_IP() {
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
  digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
#endif

  loadingFlag = true;

  auto& wifi = Wifi::instance();
  String textToShow;

  if (wifi.isConnected()) {
    textToShow = wifi.localIP().toString();
  } else {
    IPAddress apIP = wifi.apIP();
    if (apIP == IPAddress(0, 0, 0, 0)) {
      apIP = IPAddress(192, 168, 4, 1);
    }
    textToShow = "AP: " + apIP.toString();
  }

#if USE_ST7789
  if (st7789Enabled) {
    TFT_ShowIP(textToShow.c_str());
  }
#endif

  FastLED.clear();
  FastLED.show();

  bool oldRunTextOver = runTextOver;
  runTextOver = false;

  uint8_t savedBrightness = FastLED.getBrightness();
  FastLED.setBrightness(20);

  bool textEnded = false;
  while (!textEnded) {
    textEnded = fillString(textToShow.c_str(), CRGB::White, false);
    FastLED.show();
    parseUDP();
#if USE_RADIO
    radioLoop();
#endif
    delay(1);
    yield();
  }

  FastLED.setBrightness(savedBrightness);
  FastLED.clear();
  FastLED.show();
  runTextOver = oldRunTextOver;
  textIsRunning = false;

#if USE_ST7789
  if (st7789Enabled) {
    TFT_HideIP();
  }
#endif

  loadingFlag = true;

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
#if USE_DAWN
  digitalWrite(MOSFET_PIN, ONflag || (dawnFlag == 1 && !manualOff) ? MOSFET_LEVEL : !MOSFET_LEVEL);
#else
  digitalWrite(MOSFET_PIN, ONflag ? MOSFET_LEVEL : !MOSFET_LEVEL);
#endif
#endif
}

void Folder_Next_Prev(bool direction) {
#if USE_MP3_PLAYER
  if (mp3Enabled) {
    jsonWrite(configMP3, "fold_sel", CurrentFolder);
    writeFile(F("config_mp3.json"), configMP3);
    if (!pause_on && !mp3_stop && eff_sound_on) {
      mp3SendCmd(0x17, FEEDBACK, 0, CurrentFolder);
    }
  }

#if USE_TM1637
  if (tm1637Enabled) {
    DisplayFlag = 0;
    Display_Timer();
  }
#endif
#if USE_ST7789
  if (st7789Enabled) {
    DisplayFlag = 0;
    TFT_Display_Timer(0);
  }
#endif
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP
#endif  // USE_MP3_PLAYER
}

void Current_Eff_Rnd_Def(bool direction) {
  if (direction) {
    selectedSettings = 1U;
    updateSets();
#if USE_MULTILAMP
    repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP
  } else {
    setModeSettings();
    updateSets();
#if USE_MULTILAMP
    repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP
  }
  if (direction) showWarning(CRGB::Blue, 500, 250U); // мигание синим цветом 0.5 секунды
  else showWarning(CRGB::Red, 500, 250U); // мигание красным цветом 0.5 секунды
}

void IR_Equalizer() {
#if USE_MP3_PLAYER
  if (mp3Enabled) {
    Equalizer++;
    if (Equalizer > 5) Equalizer = 0;
    jsonWrite(configMP3, "eq", Equalizer);
    writeFile(F("config_mp3.json"), configMP3);
    mp3SendCmd(0x07, FEEDBACK, 0, Equalizer);
    timeout_save_file_changes = millis();
    bitSet (save_file_changes, 0);
#if USE_TM1637
    if (tm1637Enabled) {
      DisplayFlag = 3;
      Display_Timer(Equalizer);
    }
#endif
#if USE_ST7789
    if (st7789Enabled) {
      DisplayFlag = 3;
      TFT_Display_Timer(Equalizer);
    }
#endif
  }
#endif  // USE_MP3_PLAYER
}

void Favorit_Add_Del(bool direction) {
  String configCycle = readFile(F("config_cycle.json"), 2048);
  String e = "e" + String (currentMode);
  jsonWrite(configCycle, e, direction ? 1 : 0);
  Favorites::instance().FavoriteModes[currentMode] = (direction ? 1 : 0);
  timeout_save_file_changes = millis();
  bitSet (save_file_changes, 2);
  if (direction) showWarning(CRGB::Blue, 500, 250U); // мигание синим цветом 0.5 секунды
  else showWarning(CRGB::Red, 500, 250U); // мигание красным цветом 0.5 секунды
}

void Digit_Handle(uint8_t digit) {
  if (!Enter_Digit_1) {
    // первая цифра
    Enter_Digit_1 = 1;
    IR_Dgit_Enter_Timer = millis();
    Enter_Number = digit;
#if USE_TM1637
    if (tm1637Enabled) {
      DisplayFlag = 3;
      Display_Timer(digit);
    }
#endif
#if USE_ST7789
    if (st7789Enabled) {
      DisplayFlag = 3;
      TFT_Display_Timer(Enter_Number);
    }
#endif
  } else {
    // вторая цифра
    Enter_Digit_1 = 0;
    Enter_Number = Enter_Number * 10 + digit;

    if (Enter_Number >= MODE_AMOUNT) {
      Enter_Number = digit;
    }

    if (Enter_Number < MODE_AMOUNT) {
      currentMode = eff_num_correct[Enter_Number];

#if USE_TM1637
      if (tm1637Enabled) {
        DisplayFlag = 3;
        Display_Timer(Enter_Number);
      }
#endif
      jsonWrite(configSetup, "eff_sel", Enter_Number);
      jsonWrite(configSetup, "br", modes[currentMode].Brightness);
      jsonWrite(configSetup, "sp", modes[currentMode].Speed);
      jsonWrite(configSetup, "sc", modes[currentMode].Scale);
      SetBrightness(modes[currentMode].Brightness);
      loadingFlag = true;

      if (random_on && Favorites::instance().FavoritesRunning) selectedSettings = 1U;

#if USE_MP3_PLAYER
      if (ONflag && mp3Enabled && mp3_player_connect >= 4 && eff_sound_on) {
        uint8_t newFolder = effects_folders[currentMode];
        if (mp3_folder != newFolder) {
          mp3_folder = newFolder;
          mp3_folder_last = newFolder;
          mp3SendPlayFolder(newFolder);
        }
      }
#endif

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
#endif  // USE_MULTILAMP
    } else {
      showWarning(CRGB::Red, 500, 250U);
    }
  }
}
#endif // USE_IR_RECEIVER || USE_RF_RECEIVER

// ******************************************************************************************************************************************************

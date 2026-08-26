// ************************************************************************* ФУНКЦИИ IR, RF *************************************************************

#if (USE_IR_RECEIVER || USE_RF_RECEIVER)

// Вкл / Выкл лампы
void IR_Power() {
  if (dawnFlag == 1) {
#if USE_MP3_PLAYER
    if (mp3Enabled && alarm_sound_flag) {
      send_command(0x0E, 0, 0, 0);
      mp3_stop = true;
      alarm_sound_flag = false;
    } else
#endif // USE_MP3_PLAYER
    {
      manualOff = true;
      dawnFlag = 2;
#if USE_TM1637
      if (tm1637Enabled) {
        clockTicker_blink();
      }
#endif
      SetBrightness(modes[currentMode].Brightness);
      changePower();
    }
    return;
  } else {
    ONflag = !ONflag;
    jsonWrite(configSetup, "Power", ONflag);
    saveConfig();
    changePower(); // сначала надо выключать матрицу
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
    multiple_lamp_control ();
  }
#endif  // USE_MULTILAMP
}

// Вкл / Выкл звука
void Mute() {
#if USE_MP3_PLAYER
  if (mp3Enabled && mp3_player_connect == 4) {
    if (eff_sound_on) {
      eff_sound_on = 0;
#if IR_LOG
      SYSLOG.add("Звук выключен");
#endif
    } else {
      eff_sound_on = eff_volume;
#if IR_LOG
      SYSLOG.add("Звук включен");
#endif
    }
  } else  {
    showWarning(CRGB::Red, 1000, 250U);
#if IR_LOG
    SYSLOG.add("mp3 плеер не подключен");
#endif
  }
  jsonWrite(configSetup, "on_sound", eff_sound_on > 0 ? 1 : 0);
  timeout_save_file_changes = millis();
  bitSet (save_file_changes, 0);
#if USE_MULTILAMP
  repeat_multiple_lamp_control = true;
#endif
#endif // USE_MP3_PLAYER
}

// Следующий эффект, предыдущий эффект
void Prev_Next_eff(bool direction) {
  if (ONflag) {
    uint8_t temp = jsonReadtoInt(configSetup, "eff_sel");
    if (direction) {
      if (Favorit_only) {
        uint8_t lastMode = currentMode;
        do {
          if (++temp >= MODE_AMOUNT) temp = 0;
          currentMode = eff_num_correct[temp];
        } while (Favorites::instance().FavoriteModes[currentMode] == 0 && currentMode != lastMode);
        if (currentMode == lastMode) // если ни один режим не добавлен в избранное, всё равно куда-нибудь переключимся
          if (++temp >= MODE_AMOUNT) temp = 0;
        currentMode = eff_num_correct[temp];
      } else if (++temp >= MODE_AMOUNT) temp = 0;
    } else {
      if (Favorit_only) {
        uint8_t lastMode = currentMode;
        do {
          if (--temp >= MODE_AMOUNT) temp = MODE_AMOUNT - 1;
          currentMode = eff_num_correct[temp];
        } while (Favorites::instance().FavoriteModes[currentMode] == 0 && currentMode != lastMode);
        if (currentMode == lastMode) // если ни один режим не добавлен в избранное, всё равно куда-нибудь переключимся
          if (--temp >= MODE_AMOUNT) temp = MODE_AMOUNT - 1;
        currentMode = eff_num_correct[temp];
      } else if (--temp >= MODE_AMOUNT) temp = MODE_AMOUNT - 1;
    }
    currentMode = eff_num_correct[temp];
    jsonWrite(configSetup, "eff_sel", temp);
    jsonWrite(configSetup, "br", modes[currentMode].Brightness);
    jsonWrite(configSetup, "sp", modes[currentMode].Speed);
    jsonWrite(configSetup, "sc", modes[currentMode].Scale);
    SetBrightness(modes[currentMode].Brightness);
    loadingFlag = true;

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
}

void Cycle_on_off() {
  if (ONflag) {
    uint8_t tmp;
    jsonReadtoInt(configSetup, "cycle_on") == 0 ? tmp = 1 : tmp = 0;
    jsonWrite(configSetup, "cycle_on", tmp);
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
    jsonWrite(configSetup, "vol", eff_volume);
    if (!dawnflag_sound) send_command(6, FEEDBACK, 0, eff_volume); // Громкость

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
  CRGB textColor = CRGB::White;

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
  TFT_ShowIP(wifi.localIP().toString().c_str());
  }
#endif
  while (!fillString(textToShow.c_str(), textColor, false)) {
    delay(1);
    yield();
  }
#if USE_ST7789
  if (st7789Enabled) {
  TFT_HideIP();
  }
#endif
  if (ColorTextFon && (!ONflag || (currentMode == EFF_COLOR && modes[currentMode].Scale < 3))) {
    FastLED.clear();
    delay(1);
    FastLED.show();
  }

  loadingFlag = true;

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
  digitalWrite(MOSFET_PIN, ONflag || (dawnFlag == 1 && !manualOff) ? MOSFET_LEVEL : !MOSFET_LEVEL);
#endif
}

void Folder_Next_Prev(bool direction) {
#if USE_MP3_PLAYER
  if (mp3Enabled) {
  if (true) { // (!pause_on && !mp3_stop && eff_sound_on) {
    CurrentFolder = constrain(direction ? CurrentFolder + 1 : CurrentFolder - 1, 0, 99);
    jsonWrite(configSetup, "fold_sel", CurrentFolder);
    if (!pause_on && !mp3_stop && eff_sound_on) {
      send_command(0x17, FEEDBACK, 0, CurrentFolder); // Включить непрерывное воспроизведение указанной папки
      delay(mp3_delay);
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
  }
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
  } if (direction) showWarning(CRGB::Blue, 500, 250U); // мигание синим цветом 0.5 секунды
  else showWarning(CRGB::Red, 500, 250U); // мигание красным цветом 0.5 секунды
}

void IR_Equalizer() {
#if USE_MP3_PLAYER
  if (mp3Enabled) {
  Equalizer++;
  if (Equalizer > 5) Equalizer = 0;
  jsonWrite(configSetup, "eq", Equalizer);
  send_command(0x07, FEEDBACK, 0, Equalizer);
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

void Digit_Handle (uint8_t digit) {
  if (!Enter_Digit_1) {
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
    Enter_Digit_1 = 0;
    Enter_Number = Enter_Number * 10 + digit;
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
}
#endif // USE_IR_RECEIVER || USE_RF_RECEIVER

// ******************************************************************************************************************************************************

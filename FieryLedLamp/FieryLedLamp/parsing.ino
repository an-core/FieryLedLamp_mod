// *************************************************************************** parsing.ino *************************************************************

void parseUDP() {
  int32_t packetSize = Udp.parsePacket();

  if (packetSize) {
    int16_t n = Udp.read(packetBuffer, MAX_UDP_BUFFER_SIZE);
    packetBuffer[n] = '\0';
    strcpy(inputBuffer, packetBuffer);

    auto& wifi = Wifi::instance();
    IPAddress myIP = wifi.isConnected() ? wifi.localIP() : wifi.apIP();
    if (Udp.remoteIP() == myIP) {
      return;
    }

    char reply[MAX_UDP_BUFFER_SIZE];
    reply [0] = '\0';
    processInputBuffer(inputBuffer, reply, true);

#if USE_MQTT
    if (wifi.isConnected()) {
      strcpy(Mqtt::instance().mqttBuffer, reply);
    }
#endif

    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.print(reply);
    Udp.endPacket();
  }
}
// ------------------
void updateSets() {
  loadingFlag = true;

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
}
// ------------------
void processInputBuffer(char *inputBuffer, char *outputBuffer, bool generateOutput) {
  char buff[MAX_UDP_BUFFER_SIZE], *endToken = NULL;
  String BUFF = String(inputBuffer);

  if (!strncmp_P(inputBuffer, PSTR("GET"), 3)) {
    if (inputBuffer[3] == '-') sendCurrent(inputBuffer);
    else NEWsendCurrent(inputBuffer);
  }
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("EFF"), 3)) {
    uint8_t temp;
    uint8_t tmp = 0;
    if ((!strncmp_P(inputBuffer, PSTR("EFF_N"), 5)) || (!strncmp_P(inputBuffer, PSTR("EFF_P"), 5))) {
      if (!strncmp_P(inputBuffer, PSTR("EFF_N"), 5)) tmp = 1;
      temp = jsonReadtoInt(configSetup, "eff_sel");

      if (tmp) {
        if (Favorit_only) {
          uint8_t lastMode = currentMode;
          do {
            if (++temp >= MODE_AMOUNT) temp = 0;
            currentMode = eff_num_correct[temp];
#if USE_MP3_PLAYER
            if (mp3Enabled && mp3_player_connect == 4) {
              uint8_t newFolder = effects_folders[currentMode];
              if (mp3_folder != newFolder) {
                mp3_folder = newFolder;
                if (mp3Enabled) {
                  play_sound();
                }
              }
            }
#endif
          }
          while (Favorites::instance().FavoriteModes[currentMode] == 0 && currentMode != lastMode);
          if (currentMode == lastMode) {
            if (++temp >= MODE_AMOUNT) temp = 0;
            currentMode = eff_num_correct[temp];
#if USE_MP3_PLAYER
            if (mp3Enabled && mp3_player_connect == 4) {
              uint8_t newFolder = effects_folders[currentMode];
              if (mp3_folder != newFolder) {
                mp3_folder = newFolder;
                play_sound();
              }
            }
#endif
          }
        } else {
          if (++temp >= MODE_AMOUNT) temp = 0;
          currentMode = eff_num_correct[temp];
#if USE_MP3_PLAYER
          if (mp3Enabled && mp3_player_connect == 4) {
            uint8_t newFolder = effects_folders[currentMode];
            if (mp3_folder != newFolder) {
              mp3_folder = newFolder;
              play_sound();
            }
          }
#endif
        }
      } else {
        if (Favorit_only) {
          uint8_t lastMode = currentMode;
          do {
            if (temp == 0) temp = MODE_AMOUNT;
            temp--;
            currentMode = eff_num_correct[temp];
#if USE_MP3_PLAYER
            if (mp3Enabled && mp3_player_connect == 4) {
              uint8_t newFolder = effects_folders[currentMode];
              if (mp3_folder != newFolder) {
                mp3_folder = newFolder;
                play_sound();
              }
            }
#endif
          }
          while (Favorites::instance().FavoriteModes[currentMode] == 0 && currentMode != lastMode);
          if (currentMode == lastMode) {
            if (temp == 0) temp = MODE_AMOUNT;
            temp--;
            currentMode = eff_num_correct[temp];
#if USE_MP3_PLAYER
            if (mp3Enabled && mp3_player_connect == 4) {
              uint8_t newFolder = effects_folders[currentMode];
              if (mp3_folder != newFolder) {
                mp3_folder = newFolder;
                play_sound();
              }
            }
#endif
          }
        } else {
          if (temp == 0) temp = MODE_AMOUNT;
          temp--;
          currentMode = eff_num_correct[temp];
#if USE_MP3_PLAYER
          if (mp3Enabled && mp3_player_connect == 4) {
            uint8_t newFolder = effects_folders[currentMode];
            if (mp3_folder != newFolder) {
              mp3_folder = newFolder;
              play_sound();
            }
          }
#endif
        }
      }

      if (currentMode != eff_num_correct[EFF_CLOCK]) {
        nightClockEnabled = false;
        jsonWrite(configLedPanel, "night_clock_enabled", "0");
        nightModeBrightness = 0;
        saveConfig();
      }

      jsonWrite(configSetup, "eff_sel", temp);
      jsonWrite(configSetup, "br", modes[currentMode].Brightness);
      jsonWrite(configSetup, "sp", modes[currentMode].Speed);
      jsonWrite(configSetup, "sc", modes[currentMode].Scale);
      SetBrightness(modes[currentMode].Brightness);
      loadingFlag = true;
      if (random_on && Favorites::instance().FavoritesRunning) selectedSettings = 1U;

      sendCurrent(inputBuffer);

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

    } else {
      memcpy(buff, &inputBuffer[3], strlen(inputBuffer));
      temp = (uint8_t)atoi(buff);
      currentMode = eff_num_correct[temp];

      if (currentMode != eff_num_correct[EFF_CLOCK]) {
        nightClockEnabled = false;
        jsonWrite(configLedPanel, "night_clock_enabled", "0");
        nightModeBrightness = 0;
        saveConfig();
      }

#if USE_MP3_PLAYER
      if (mp3Enabled && mp3_player_connect == 4) {
        uint8_t newFolder = effects_folders[currentMode];
        if (mp3_folder != newFolder) {
          mp3_folder = newFolder;
          play_sound();
        }
      }
#endif
      updateSets();
      jsonWrite(configSetup, "eff_sel", temp);
      jsonWrite(configSetup, "br", modes[currentMode].Brightness);
      jsonWrite(configSetup, "sp", modes[currentMode].Speed);
      jsonWrite(configSetup, "sc", modes[currentMode].Scale);

#if USE_MULTILAMP
      repeat_multiple_lamp_control = true;
#endif

      sendCurrent(inputBuffer);

#if USE_BLYNK_PLUS
      updateRemoteBlynkParams();
#endif

      if (random_on && Favorites::instance().FavoritesRunning) selectedSettings = 1U;
      SetBrightness(modes[currentMode].Brightness);
    }
  }
  // ------------------
#if USE_MP3_PLAYER
  else if (!strncmp_P(inputBuffer, PSTR("VOL"), 3)) {
    memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
    uint8_t eff_sound_on_tmp = (uint8_t)atoi(buff);
    if (eff_sound_on_tmp)  {
      eff_sound_on = eff_volume = constrain( eff_sound_on_tmp, 1, 30 );
    } else if (!eff_sound_on) {
      eff_sound_on = eff_volume;
    } else {
      eff_sound_on = 0;
    }

    if (mp3Enabled) {
      send_command(6, 0, 0, eff_volume);
    }
    jsonWrite(configMP3, "vol", eff_volume);
    jsonWrite(configMP3, "on_sound", eff_sound_on > 0 ? 1 : 0);
    sendVolume(inputBuffer);

#if USE_MQTT
    if (Wifi::instance().isConnected()) {
      Mqtt::instance().needToPublish = true;
    }
#endif

#if USE_BLYNK_PLUS
    updateRemoteBlynkParams();
#endif

#if USE_MULTILAMP
    repeat_multiple_lamp_control = true;
#endif
  }
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("SO_ON"), 5)) {
    eff_sound_on = eff_volume;
    jsonWrite(configMP3, "on_sound", 1);
    timeout_save_file_changes = millis();
    bitSet (save_file_changes, 0);
    sendVolume(inputBuffer);

#if USE_MQTT
    if (Wifi::instance().isConnected()) {
      Mqtt::instance().needToPublish = true;
    }
#endif

#if USE_BLYNK_PLUS
    updateRemoteBlynkParams();
#endif

#if USE_MULTILAMP
    repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP
  }
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("SO_OFF"), 6)) {
    eff_sound_on = 0;
    jsonWrite(configMP3, "on_sound", 0);
    timeout_save_file_changes = millis();
    bitSet (save_file_changes, 0);
    sendVolume(inputBuffer);

#if USE_MQTT
    if (Wifi::instance().isConnected()) {
      Mqtt::instance().needToPublish = true;
    }
#endif

#if USE_BLYNK_PLUS
    updateRemoteBlynkParams();
#endif

#if USE_MULTILAMP
    repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP
  }
#endif  // USE_MP3_PLAYER (стр.206)
  /// ------------------
  else if (!strncmp_P(inputBuffer, PSTR("LANG"), 4)) {
    memcpy(buff, &inputBuffer[4], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
    jsonWrite(configSetup, "lang", buff);
    saveConfig();
    Lang_set();
    NEWsendCurrent(inputBuffer);
  }
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("BRI"), 3)) {
    memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
    modes[currentMode].Brightness = constrain(atoi(buff), 1, 255);
    jsonWrite(configSetup, "br", modes[currentMode].Brightness);

#if USE_MULTILAMP
    repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP

    SetBrightness(modes[currentMode].Brightness);
    sendCurrent(inputBuffer);

#if USE_MQTT
    if (Wifi::instance().isConnected()) {
      Mqtt::instance().needToPublish = true;
    }
#endif

#if USE_BLYNK_PLUS
    updateRemoteBlynkParams();
#endif
  }
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("SPD"), 3)) {
    memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
    modes[currentMode].Speed = atoi(buff);
    jsonWrite(configSetup, "sp", modes[currentMode].Speed);

#if USE_BLYNK_PLUS
    updateRemoteBlynkParams();
#endif

#if USE_MULTILAMP
    repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP

    updateSets();
    sendCurrent(inputBuffer);
  }
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("SCA"), 3)) {
    memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
    modes[currentMode].Scale = atoi(buff);
    jsonWrite(configSetup, "sc", modes[currentMode].Scale);

#if USE_MULTILAMP
    repeat_multiple_lamp_control = true;

#endif  // USE_MULTILAMP
    updateSets();
    sendCurrent(inputBuffer);

#if USE_BLYNK_PLUS
    updateRemoteBlynkParams();
#endif
  }
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("P_ON"), 4)) {
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
      sendCurrent(inputBuffer);
      return;
    }
#endif // USE_DAWN

    // обычное включение лампы (если не был активен рассвет)
    ONflag = true;
    jsonWrite(configSetup, "Power", ONflag);
    Eeprom::instance().EepromGet(modes);
    timeout_save_file_changes = millis();
    bitSet(save_file_changes, 0);
    updateSets();
    changePower();
    loadingFlag = true;
#if USE_MULTILAMP
    repeat_multiple_lamp_control = true;
#endif
    sendCurrent(inputBuffer);
#if USE_BLYNK_PLUS
    updateRemoteBlynkParams();
#endif
  }
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("P_OFF"), 5)) {
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
      sendCurrent(inputBuffer);
      return;
    }
#endif // USE_DAWN

    // обычное выключение лампы
    ONflag = false;
    jsonWrite(configSetup, "Power", ONflag);

    if (!Favorites::instance().FavoritesRunning) {
      Eeprom::instance().EepromPut(modes);
    }

    save_file_changes = 7;
    timeout_save_file_changes = millis() - SAVE_FILE_DELAY_TIMEOUT;

    Save_File_Changes();

    changePower();
    loadingFlag = true;
#if USE_MULTILAMP
    multiple_lamp_control();
#endif
    sendCurrent(inputBuffer);
#if USE_MQTT
    if (Wifi::instance().isConnected()) {
      Mqtt::instance().needToPublish = true;
    }
#endif
#if USE_BLYNK_PLUS
    updateRemoteBlynkParams();
#endif
  }
  // ------------------
#if USE_MULTILAMP
  else if (!strncmp_P(inputBuffer, PSTR("MULTI"), 5)) { // Управление несколькими лампами
    uint8_t valid = 0, i = 0;
    while (inputBuffer[i])   {   // пакет должен иметь вид MULTI,%U,%U,%U,%U,%U соответственно ON/OFF,№эффекта,яркость,скорость,масштаб или + №текущей папки или + озвучування_on/off, гучнисть
      if (inputBuffer[i] == ',') {
        valid++;  // Проверка на правильность пакета (по количеству запятых)
      }
      i++;
    }
    if (valid == 5 || valid == 6 || valid == 8) {   // Если пакет правильный выделяем лексемы,разделённые запятыми, и присваиваем параметрам эффектов
      char *tmp = strtok (inputBuffer, ","); // Первая лексема MULTI пропускается
      tmp = strtok (NULL, ",");
      bool onflg = false;
      if (ONflag != atoi(tmp)) {
        ONflag = atoi( tmp);
        onflg = true;
      }
      tmp = strtok (NULL, ",");
      if (currentMode != atoi(tmp)) {
        if (atoi (tmp) < MODE_AMOUNT) {
          currentMode = atoi (tmp);
          tmp = strtok (NULL, ",");
          modes[currentMode].Brightness = atoi (tmp);
          tmp = strtok (NULL, ",");
          modes[currentMode].Speed = atoi (tmp);
          tmp = strtok (NULL, ",");
          modes[currentMode].Scale = atoi (tmp);
#if USE_MP3_PLAYER
          if (valid == 8) {
            tmp = strtok (NULL, ",");
            eff_sound_on = atoi (tmp);
            tmp = strtok (NULL, ",");
            eff_volume = atoi (tmp);
#if USE_DAWN
            if (mp3Enabled && !dawnFlag && ONflag && eff_sound_on) {
              send_command(6, FEEDBACK, 0, eff_volume); // Меняем громкость
              delay(mp3_delay);
            }
#endif
          }
          if (valid == 8 || valid == 6) {
            tmp = strtok (NULL, ",");
            mp3_folder = effects_folders[currentMode];
            mp3_folder_last = mp3_folder;
            if (mp3Enabled) {
              play_sound();
            }
            if (atoi (tmp) != CurrentFolder) {
              CurrentFolder = atoi (tmp);
#if USE_DAWN
              if (mp3Enabled && !dawnFlag && ONflag && eff_sound_on) {
                send_command(0x17, FEEDBACK, 0, CurrentFolder);
                CurrentFolder_last = CurrentFolder;
                mp3_stop = false;
                delay(mp3_delay);
              }
#endif
            }
          }
#endif // USE_MP3_PLAYER
          loadingFlag = true; // Перезапуск эффекта
          SetBrightness(modes[currentMode].Brightness); //Применение яркости
        } else   {
          currentMode = MODE_AMOUNT - 3;  // Если полученный номер эффекта больше , чем количество эффектов в лампе,включаем последний "адекватный" эффект
          loadingFlag = true; // Перезапуск эффекта
          SetBrightness(modes[currentMode].Brightness); // Применение яркости
        }
      } else   {
        tmp = strtok (NULL, ",");
        if (modes[currentMode].Brightness != atoi(tmp)) {
          modes[currentMode].Brightness = atoi (tmp);
          SetBrightness(modes[currentMode].Brightness); // Применение яркости
        }
        tmp = strtok (NULL, ",");
        if (modes[currentMode].Speed != atoi(tmp)) {
          modes[currentMode].Speed = atoi (tmp);
          loadingFlag = true; // Перезапуск эффекта
        }
        tmp = strtok (NULL, ",");
        if (modes[currentMode].Scale != atoi(tmp)) {
          modes[currentMode].Scale = atoi (tmp);
          loadingFlag = true; // Перезапуск эффекта
        }
#if USE_MP3_PLAYER
        if (valid == 8) {
          tmp = strtok (NULL, ",");
          eff_sound_on = atoi (tmp);
          tmp = strtok (NULL, ",");
          eff_volume = atoi (tmp);
#if USE_DAWN
          if (mp3Enabled && !dawnFlag && ONflag && eff_sound_on) {
            send_command(6, FEEDBACK, 0, eff_volume);
            delay(mp3_delay);
          }
#endif
        }
        if (valid == 8 || valid == 6) {
          tmp = strtok (NULL, ",");
          if (atoi (tmp) != CurrentFolder) {
            CurrentFolder = atoi (tmp);
#if USE_DAWN
            if (mp3Enabled && eff_sound_on && !dawnFlag && ONflag) {
              send_command(0x17, FEEDBACK, 0, CurrentFolder);
              mp3_stop = false;
              CurrentFolder_last = CurrentFolder;
              delay(mp3_delay);
            }
#endif
          }
        }
#endif // USE_MP3_PLAYER
      }
      if (onflg) {
#if USE_MP3_PLAYER
        if (ONflag) mp3_folder = effects_folders[currentMode];
#endif
        changePower();   // Активация состояния ON/OFF
      }
#if GENERAL_LOG
      SYSLOG.add ("Принято MULTI ");
      SYSLOG.add("ONflag: %d", ONflag);
      SYSLOG.add("currentMode: %u", currentMode);
      SYSLOG.add("Brightness: %u", modes[currentMode].Brightness);
      SYSLOG.add("Speed: %u", modes[currentMode].Speed);
      SYSLOG.add("Scale: %u", modes[currentMode].Scale);
#if USE_MP3_PLAYER
      SYSLOG.add("CurrentFolder: %u", CurrentFolder);
      SYSLOG.add("eff_sound_on: %u", eff_sound_on);
      SYSLOG.add("eff_volume: %u", eff_volume);
#endif // USE_MP3_PLAYER
#endif  // GENERAL_LOG

      jsonWrite(configSetup, "br", modes[currentMode].Brightness); //Передаём в веб интерфейс новые параметры
      jsonWrite(configSetup, "sp", modes[currentMode].Speed); //для правильного отображения
      jsonWrite(configSetup, "sc", modes[currentMode].Scale);
      jsonWrite(configSetup, "eff_sel", currentMode);
#if USE_MP3_PLAYER
      jsonWrite(configMP3, "on_sound", eff_sound_on > 0 ? 1 : 0);
      jsonWrite(configMP3, "vol", eff_volume);
      jsonWrite(configMP3, "fold_sel", CurrentFolder);
#endif // USE_MP3_PLAYER

      for ( uint8_t n = 0; n < MODE_AMOUNT; n++) {
        if (eff_num_correct[n] == currentMode)  {
          jsonWrite(configSetup, "eff_sel", n);
          break;
        }
      }

      jsonWrite(configSetup, "Power", ONflag);
    }
    inputBuffer[0] = '\0';
    generateOutput = false;
  }
#endif // USE_MULTILAMP
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("FAV_"), 4)) {
    if (!strncmp_P(inputBuffer, PSTR("FAV_ON"), 6)) {
      if (ONflag) {
        Favorites::instance().FavoritesRunning = 1;
        jsonWrite(configSetup, "cycle_on", 1);
        jsonWrite(configSetup, "eff_sel", currentMode);
        Eeprom::instance().EepromPut(modes);
      } else {
        Favorites::instance().FavoritesRunning = 0;
        Favorites::instance().nextModeAt = 0;
        jsonWrite(configSetup, "cycle_on", 0);
      }
      timeout_save_file_changes = millis();
      bitSet(save_file_changes, 2);
    } else if (!strncmp_P(inputBuffer, PSTR("FAV_OFF"), 7)) {
      Favorites::instance().FavoritesRunning = 0;
      Favorites::instance().nextModeAt = 0;
      jsonWrite(configSetup, "cycle_on", 0);
      Eeprom::instance().EepromGet(modes);
      timeout_save_file_changes = millis();
      bitSet(save_file_changes, 2);
    } else if (!strncmp_P(inputBuffer, PSTR("FAV_SET"), 7)) {
      Favorites::instance().ConfigureFavorites(inputBuffer);
      if (!ONflag) Favorites::instance().FavoritesRunning = 0;
      Favorites::instance().SetStatus(inputBuffer);
      jsonWrite(configSetup, "cycle_on", Favorites::instance().FavoritesRunning);
      jsonWrite(configSetup, "time_eff", Favorites::instance().Interval);
      jsonWrite(configSetup, "disp", Favorites::instance().Dispersion);
      jsonWrite(configSetup, "cycle_always", Favorites::instance().UseSavedFavoritesRunning);

      if (Favorites::instance().FavoritesRunning) {
        jsonWrite(configSetup, "eff_sel", currentMode);
        Eeprom::instance().EepromPut(modes);
      } else {
        Eeprom::instance().EepromGet(modes);
      }
      timeout_save_file_changes = millis();
      bitSet(save_file_changes, 2);
#if USE_MQTT
      if (Wifi::instance().isConnected()) {
        Mqtt::instance().needToPublish = true;
      }
#endif
    } else if (!strncmp_P(inputBuffer, PSTR("FAV_GET"), 7)) {
      Favorites::instance().SetStatus(inputBuffer);
    }
  }
  // ------------------
#if USE_OTA
  else if (!strncmp_P(inputBuffer, PSTR("OTA"), 3)) {
    Ota::instance().RequestOtaUpdate();
    delay(70);
    Ota::instance().RequestOtaUpdate();
    if (Ota::instance().isOtaActive()) {
      currentMode = EFF_MATRIX;
      FastLED.clear();
      delay(1);
      ONflag = true;
      jsonWrite(configSetup, "Power", ONflag);
      jsonWrite(configSetup, "eff_sel", currentMode);
      jsonWrite(configSetup, "br", modes[currentMode].Brightness);
      jsonWrite(configSetup, "sp", modes[currentMode].Speed);
      jsonWrite(configSetup, "sc", modes[currentMode].Scale);
      changePower();
    } else
      showWarning(CRGB::Red, 2000U, 500U);
  }
#endif // USE_OTA
  // ------------------
#if USE_BUTTON
  else if (!strncmp_P(inputBuffer, PSTR("BTN"), 3)) {
    if (strstr_P(inputBuffer, PSTR("ON")) - inputBuffer == 4) {
      buttonEnabled = true;
    } else { // OFF
      buttonEnabled = false;
    }
    jsonWrite(configSetup, "button_enabled", buttonEnabled ? "1" : "0");
    saveConfig();
    sendCurrent(inputBuffer);
  }
#endif // USE_BUTTON
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("GBR"), 3)) { // выставляем общую яркость для всех эффектов без сохранения в EEPROM, если приложение присылает такую строку
    memcpy(buff, &inputBuffer[3], strlen(inputBuffer));   // взять подстроку, состоящую последних символов строки inputBuffer, начиная с символа 4
    uint8_t ALLbri = constrain(atoi(buff), 1, 255);
    for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
      modes[i].Brightness = ALLbri;
    }
    jsonWrite(configSetup, "br", ALLbri);
    FastLED.setBrightness(ALLbri);
    loadingFlag = true;
#if USE_MULTILAMP
    repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP
  }
  // ------------------
#ifdef USE_RANDOM_SETS_IN_APP
  else if (!strncmp_P(inputBuffer, PSTR("RND_"), 4)) { // управление включением случайных настроек
    if (!strncmp_P(inputBuffer, PSTR("RND_0"), 5)) { // вернуть настройки по умолчанию текущему эффекту
      setModeSettings();
      updateSets();
#if USE_MULTILAMP
      repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP
      sendCurrent(inputBuffer);
    } else if (!strncmp_P(inputBuffer, PSTR("RND_1"), 5)) { // выбрать случайные настройки текущему эффекту  // раньше была идея, что будут числа RND_1, RND_2, RND_3 - выбор из предустановленных настроек, но потом всё свелось к единственному варианту случайных настроек
      selectedSettings = 1U;
      updateSets();
#if USE_MULTILAMP
      repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP
    } else if (!strncmp_P(inputBuffer, PSTR("RND_Z"), 5)) { // вернуть настройки по умолчанию всем эффектам
      restoreSettings();
      selectedSettings = 0U;
      updateSets();
#if USE_MULTILAMP
      repeat_multiple_lamp_control = true;
#endif  // USE_MULTILAMP
      sendCurrent(inputBuffer);
#if USE_BLYNK
      updateRemoteBlynkParams();
#endif
    } else if (!strncmp_P(inputBuffer, PSTR("RND_C1"), 5)) { // Включаем случайный выбор эффектов в цикле
      Favorites::instance().rndCycle = 1;
      jsonWrite(configSetup, "rnd_cycle", 1);
      saveConfig();
    } else if (!strncmp_P(inputBuffer, PSTR("RND_C0"), 5)) { // Выключаем случайный выбор эффектов в цикле
      Favorites::instance().rndCycle = 0;
      jsonWrite(configSetup, "rnd_cycle", 0);
      saveConfig();
    } else if (!strncmp_P(inputBuffer, PSTR("RND_ON"), 6)) { // включить выбор случайных настроек в режиме Цикл
      random_on = 1U;
      jsonWrite(configSetup, "random_on", (int)random_on);
      saveConfig();
      showWarning(CRGB::Blue, 1000U, 500U);                    // мигание синим цветом 1 секунду
    } else if (!strncmp_P(inputBuffer, PSTR("RND_OFF"), 7)) { // отключить выбор случайных настроек в режиме Цикл
      random_on = 0U;
      jsonWrite(configSetup, "random_on", (int)random_on);
      saveConfig();
      showWarning(CRGB::Blue, 1000U, 500U);                    // мигание синим цветом 1 секунду
    }
  }
#endif // USE_RANDOM_SETS_IN_APP
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("RUN_"), 4)) {          // Настройка бегущей строки
    if (!strncmp_P(inputBuffer, PSTR("RUN_C"), 5)) {             // Цвет бегущей строки (0-255)
      memcpy(buff, &inputBuffer[5], strlen(inputBuffer));
      ColorRunningText = (uint8_t)atoi(buff);
      jsonWrite(configLedPanel, "sct", ColorRunningText);
      bitSet (save_file_changes, 0);
      timeout_save_file_changes = millis();
    } else if (!strncmp_P(inputBuffer, PSTR("RUN_S"), 5)) {      // скорость бегущей строки
      memcpy(buff, &inputBuffer[5], strlen(inputBuffer));
      SpeedRunningText = (uint8_t)atoi(buff);
      jsonWrite(configLedPanel, "spt", SpeedRunningText);
      bitSet (save_file_changes, 0);
      timeout_save_file_changes = millis();
    } else if (!strncmp_P(inputBuffer, PSTR("RUN_X"), 5)) {
      const char* ptr = inputBuffer + 5;

      while (*ptr == ' ' || *ptr == '=') ptr++;

      if (*ptr == '\0') {
#if GENERAL_LOG
        SYSLOG.add("RUN_X: пустая команда — игнорируем");
#endif
        return;
      }

      char tempText[sizeof(TextTicker)];
      strncpy(tempText, ptr, sizeof(tempText) - 1);
      tempText[sizeof(tempText) - 1] = '\0';

      String newText = String(tempText);
      newText.trim();

      if (newText.length() == 0) {
        return;
      }

      jsonWrite(configLedPanel, "run_text", newText);
      newText.toCharArray(TextTicker, sizeof(TextTicker));
      TextTicker[sizeof(TextTicker) - 1] = '\0';

      bitSet(save_file_changes, 0);
      timeout_save_file_changes = millis();

      loadingFlag = true;
      textIsRunning = true;
    }
  }
  // ------------------
#if USE_DAWN
  else if (!strncmp_P(inputBuffer, PSTR("ALM_"), 4)) { // сокращаем GET и SET для ускорения регулярного цикла
    if (!strncmp_P(inputBuffer, PSTR("ALM_SET"), 7)) {
      uint8_t alarmNum = (char)inputBuffer[7] - '0';
      alarmNum -= 1;
      if (strstr_P(inputBuffer, PSTR("ON")) - inputBuffer == 9) {
        alarms[alarmNum].State = true;
        sendAlarms(inputBuffer);
      } else if (strstr_P(inputBuffer, PSTR("OFF")) - inputBuffer == 9) {
        alarms[alarmNum].State = false;
        sendAlarms(inputBuffer);
      } else {
        memcpy(buff, &inputBuffer[8], strlen(inputBuffer));
        alarms[alarmNum].Time = atoi(buff);
        sendAlarms(inputBuffer);
      }
#if USE_MQTT
      if (Wifi::instance().isConnected()) {
        strcpy(Mqtt::instance().mqttBuffer, inputBuffer);
        Mqtt::instance().needToPublish = true;
      }
#endif
    } else
      sendAlarms(inputBuffer);
  }
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("DAWN"), 4)) {
    memcpy(buff, &inputBuffer[4], strlen(inputBuffer));
    dawnMode = atoi(buff) - 1;
    sendAlarms(inputBuffer);
#if USE_MQTT
    if (Wifi::instance().isConnected()) {
      Mqtt::instance().needToPublish = true;
    }
#endif
  }
#endif  // USE_DAWN
  // ------------------
#if USE_SUNSET
  else if (!strncmp_P(inputBuffer, PSTR("SUN_"), 4)) { // обработка команд заката
    if (!strncmp_P(inputBuffer, PSTR("SUN_SET"), 7)) {
      uint8_t sunsetNum = (char)inputBuffer[7] - '0';
      sunsetNum -= 1;
      if (strstr_P(inputBuffer, PSTR("ON")) - inputBuffer == 9) {
        sunsets[sunsetNum].State = true;
        sendSunsets(inputBuffer);
      } else if (strstr_P(inputBuffer, PSTR("OFF")) - inputBuffer == 9) {
        sunsets[sunsetNum].State = false;
        sendSunsets(inputBuffer);
      } else {
        memcpy(buff, &inputBuffer[8], strlen(inputBuffer));
        sunsets[sunsetNum].Time = atoi(buff);
        sendSunsets(inputBuffer);
      }
#if USE_MQTT
      if (Wifi::instance().isConnected()) {
        strcpy(Mqtt::instance().mqttBuffer, inputBuffer);
        Mqtt::instance().needToPublish = true;
      }
#endif
    } else
      sendSunsets(inputBuffer);
  } // else if (!strncmp_P(inputBuffer, PSTR("SUN_"), 4))
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("SUNSET"), 6)) {
    memcpy(buff, &inputBuffer[6], strlen(inputBuffer));
    sunsetMode = atoi(buff) - 1;
    sendSunsets(inputBuffer);
#if USE_MQTT
    if (Wifi::instance().isConnected()) {
      Mqtt::instance().needToPublish = true;
    }
#endif
  }
#endif  // USE_SUNSET
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("DISCOVER"), 8)) {
    auto& wifi = Wifi::instance();
    IPAddress currentIP = wifi.isConnected() ? wifi.localIP() : wifi.apIP();
    if (currentIP == IPAddress(0, 0, 0, 0)) {
      currentIP = IPAddress(AP_STATIC_IP[0], AP_STATIC_IP[1], AP_STATIC_IP[2], AP_STATIC_IP[3]);
    }

    char lamp_name[LAMP_NAME.length() + 1];
    LAMP_NAME.toCharArray(lamp_name, LAMP_NAME.length() + 1);

    sprintf_P(inputBuffer, PSTR("IP %u.%u.%u.%u:%u:%s STA:%u"), currentIP[0], currentIP[1], currentIP[2], currentIP[3], ESP_UDP_PORT, lamp_name, wifi.isConnected() ? 1 : 0);
  }
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("LIST"), 4)) { // передача списка эффектов
    memcpy(buff, &inputBuffer[4], strlen(inputBuffer));  // взять подстроку, начиная с символа 5
    switch (atoi(buff)) {
      case 1U: {
          EffectList(F("/efflist1"));
          break;
        }
      case 2U: {
          EffectList(F("/efflist2"));
          break;
        }
      case 3U: {
          EffectList(F("/efflist3"));
#if USE_DEFAULT_SETTINGS_RESET
          restoreSettings();
          updateSets();
#if USE_BLYNK_PLUS
          updateRemoteBlynkParams();
#endif
#endif
          break;
        }
    }
  }
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("ssid"), 4)) {
    jsonWrite(configWiFi, "ssid", BUFF.substring(5, BUFF.length()));
    writeFile(F("config_wifi.json"), configWiFi);
  } else if (!strncmp_P(inputBuffer, PSTR("passw"), 5)) {
    jsonWrite(configWiFi, "password", BUFF.substring(6, BUFF.length()));
    writeFile(F("config_wifi.json"), configWiFi);
  } else if (!strncmp_P(inputBuffer, PSTR("timeout"), 7)) {
    jsonWrite(configWiFi, "TimeOut", BUFF.substring(8, BUFF.length()));
    writeFile(F("config_wifi.json"), configWiFi);
  } else if (!strncmp_P(inputBuffer, PSTR("TXT"), 3)) {
    String str = (BUFF.length() > 4) ? BUFF.substring(4, BUFF.length()) : "";
    str.toCharArray(TextTicker, str.length() + 1);
  } else if (!strncmp_P(inputBuffer, PSTR("DRW"), 3)) {
    drawPixelXY((int8_t)getValue(BUFF, ';', 1).toInt(), (int8_t)getValue(BUFF, ';', 2).toInt(), DriwingColor);
    if (leds != nullptr) FastLED.show();
  } else if (!strncmp_P(inputBuffer, PSTR("CLR"), 3)) {
    if (leds != nullptr) {
      FastLED.clear();
      FastLED.show();
    }
  }

  else if (!strncmp_P(inputBuffer, PSTR("COL"), 3)) {
#ifdef USE_OLD_APP_FROM_KOTEYKA
    DriwingColor = CRGB(getValue(BUFF, ';', 1).toInt(), getValue(BUFF, ';', 2).toInt(), getValue(BUFF, ';', 3).toInt());
#else
    DriwingColor = CRGB(getValue(BUFF, ';', 1).toInt(), getValue(BUFF, ';', 3).toInt(), getValue(BUFF, ';', 2).toInt());
#endif
  }
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("DRAWO"), 5)) {
    if (!strncmp_P(inputBuffer, PSTR("DRAWON"), 6)) Painting = 1;
    else Painting = 0;
  }
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("RESET"), 5)) {
    if (!strncmp_P(inputBuffer, PSTR("RESET_ALL"), 9)) {
#if GENERAL_LOG
      SYSLOG.add("\n*** Reset to Default ***");
#endif
      bool allCopiedSuccessfully = true;
      setModeSettings();
      updateSets();

      if (!FileCopy(F("/default/config.json"), F("/config.json"))) allCopiedSuccessfully = false;
      if (!FileCopy(F("/default/config_cycle.json"), F("/config_cycle.json"))) allCopiedSuccessfully = false;
      if (!FileCopy(F("/default/sound_list.json"), F("/sound_list.json"))) allCopiedSuccessfully = false;
      if (!FileCopy(F("/default/config_alarm.json"), F("/config_alarm.json"))) allCopiedSuccessfully = false;
      if (!FileCopy(F("/default/config_sunset.json"), F("/config_sunset.json"))) allCopiedSuccessfully = false;
      if (!FileCopy(F("/default/config_mp3.json"), F("/config_mp3.json"))) allCopiedSuccessfully = false;
      if (!FileCopy(F("/default/config_button.json"), F("/config_button.json"))) allCopiedSuccessfully = false;
      if (!FileCopy(F("/default/config_st7789.json"), F("/config_st7789.json"))) allCopiedSuccessfully = false;
      if (!FileCopy(F("/default/config_led_matrix.json"), F("/config_led_matrix.json"))) allCopiedSuccessfully = false;
      if (!FileCopy(F("/default/config_led_panel.json"), F("/config_led_panel.json"))) allCopiedSuccessfully = false;
      if (!FileCopy(F("/default/config_led_interval.json"), F("/config_led_interval.json"))) allCopiedSuccessfully = false;
      if (!FileCopy(F("/default/config_multilamp.json"), F("/config_multilamp.json"))) allCopiedSuccessfully = false;
      if (!FileCopy(F("/default/config_weather.json"), F("/config_weather.json"))) allCopiedSuccessfully = false;
      if (!FileCopy(F("/default/config_mqtt.json"), F("/config_mqtt.json"))) allCopiedSuccessfully = false;
      if (!FileCopy(F("/default/index.setup.json.gz"), F("/index.setup.json.gz"))) allCopiedSuccessfully = false;
      if (allCopiedSuccessfully) showWarning(CRGB::Green, 2000U, 500U);
      else showWarning(CRGB::Red, 3000U, 500U);
      delay(100);
      ESP.restart();
    } else {
      jsonWrite(configWiFi, "ssid", "");
      jsonWrite(configWiFi, "password", "");
      saveConfig();
      showWarning(CRGB::Blue, 2000U, 500U);
    }
  }
  // ------------------
#if defined(GENERAL_DEBUG) || defined(USE_OLD_IOS_APP)
  else if (!strncmp_P(inputBuffer, PSTR("DEB"), 3)) {
    if (myTime.isTimeSet()) {
      String shortTime = myTime.getFormattedShortTime();
      sprintf_P(inputBuffer, PSTR("OK %s:00"), shortTime.c_str());
    } else {
      strcpy_P(inputBuffer, PSTR("OK --:--"));
    }
  }
#endif

#if USE_MQTT
  else if (!strncmp_P(inputBuffer, PSTR("STATE"), 5)) {
    if (Wifi::instance().isConnected()) Mqtt::instance().needToPublish = true;
  }
#endif
  // ------------------
  else if (!strncmp_P(inputBuffer, PSTR("SETS"), 4)) {
    memcpy(buff, &inputBuffer[4], 1U);
    switch (atoi(buff)) {
      case 1U: {
          memcpy(buff, &inputBuffer[5], strlen(inputBuffer));
          uint8_t eff = getValue(buff, ';', 0).toInt();
          modes[eff].Brightness = getValue(buff, ';', 1).toInt();
          modes[eff].Speed = getValue(buff, ';', 2).toInt();
          modes[eff].Scale = getValue(buff, ';', 3).toInt();
          jsonWrite(configSetup, "br", modes[eff].Brightness);
          jsonWrite(configSetup, "sp", modes[eff].Speed);
          jsonWrite(configSetup, "sc", modes[eff].Scale);
          if (eff == currentMode) {
            updateSets();
#if USE_BLYNK_PLUS
            updateRemoteBlynkParams();
#endif
          }
          break;
        }
      case 2U: {
          String OutString;
          char replyPacket[MAX_UDP_BUFFER_SIZE];
          for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
            OutString = String(i) + ";" + String(modes[i].Brightness) + ";" + String(modes[i].Speed) + ";" + String(modes[i].Scale) + "\n";
            OutString.toCharArray(replyPacket, MAX_UDP_BUFFER_SIZE);
            Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
            Udp.print(replyPacket);
            Udp.endPacket();
          }
#if USE_MQTT
          if (Wifi::instance().isConnected()) Mqtt::instance().needToPublish = true;
#endif
          break;
        }
    }
  }

  else {
    inputBuffer[0] = '\0';
  }
  // ------------------
  if (strlen(inputBuffer) <= 0) {
    return;
  }

  if (generateOutput) {
    strcpy(outputBuffer, inputBuffer);
  }

  inputBuffer[0] = '\0';

} // void processInputBuffer(char *inputBuffer, char *outputBuffer, bool generateOutput)

void sendCurrent(char *outputBuffer) {
  uint8_t n;
  for (n = 0; n < MODE_AMOUNT; n++)
    if (eff_num_correct[n] == currentMode) break;

  sprintf_P(outputBuffer, PSTR("CURR %u %u %u %u %u %u"), n, modes[currentMode].Brightness, modes[currentMode].Speed, modes[currentMode].Scale, ONflag, Wifi::instance().isConnected() ? 1 : 0);
#if USE_BUTTON
  sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, (uint8_t)buttonEnabled);
#else
  sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, 0);
#endif

  char timeBuf[9];
  if (myTime.isTimeSet()) {
    String shortTime = myTime.getFormattedShortTime();
    sprintf_P(timeBuf, PSTR("%s:00"), shortTime.c_str());
  } else {
    strcpy(timeBuf, "--:--");
  }
  sprintf_P(outputBuffer, PSTR("%s %s"), outputBuffer, timeBuf);

#if USE_MP3_PLAYER
  sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, (uint8_t)eff_sound_on);
#endif
}

void NEWsendCurrent(char *outputBuffer) {
  uint8_t n;
  for (n = 0; n < MODE_AMOUNT; n++) {
    if (eff_num_correct[n] == currentMode) break;
  }

  sprintf_P(outputBuffer, PSTR("NEWCURR %u %u %u %u %u %u %u %u %u %u"), n, modes[currentMode].Brightness, modes[currentMode].Speed, modes[currentMode].Scale, ONflag, Wifi::instance().isConnected() ? 1 : 0,

#if USE_BUTTON
            (uint8_t)buttonEnabled,
#else
            0,
#endif
            Favorites::instance().FavoritesRunning, Favorites::instance().rndCycle, random_on);

  char timeBuf[9];
  if (myTime.isTimeSet()) {
    String shortTime = myTime.getFormattedShortTime();
    sprintf_P(timeBuf, PSTR("%s:00"), shortTime.c_str());
  } else {
    strcpy(timeBuf, "--:--");
  }
  sprintf_P(outputBuffer, PSTR("%s %s"), outputBuffer, timeBuf);

  // Язык
  String str = jsonRead(configSetup, "lang");
  char temp[3];
  str.toCharArray(temp, sizeof(temp));
  sprintf_P(outputBuffer, PSTR("%s %s"), outputBuffer, temp);

#if USE_MP3_PLAYER
  sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, (uint8_t)eff_sound_on);
#endif
}

#if USE_DAWN
void sendAlarms(char *outputBuffer) {
  char k[2];
  bool alarm_change = false;
  String configAlarm = readFile(F("config_alarm.json"), 512);
#if GENERAL_LOG
  SYSLOG.add("\nТекущие установки будильника");
  SYSLOG.add(configAlarm.c_str());
#endif
  strcpy_P(outputBuffer, PSTR("ALMS"));

  for (byte i = 0; i < 7; i++) {
    itoa ((i + 1), k, 10);
    k[1] = 0;
    String a = "a" + String (k) ;
    String h = "h" + String (k) ;
    String m = "m" + String (k) ;
    if (alarms[i].State != (jsonReadtoInt(configAlarm, a)) || alarms[i].Time != (jsonReadtoInt(configAlarm, h)) * 60U + (jsonReadtoInt(configAlarm, m)))  {
      alarm_change = true;
      jsonWrite(configAlarm, a, alarms[i].State);
      jsonWrite(configAlarm, h, (alarms[i].Time / 60U));
      jsonWrite(configAlarm, m, (alarms[i].Time % 60U));
    }
    sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, (uint8_t)alarms[i].State);
  }

  for (byte i = 0; i < 7; i++) {
    sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, alarms[i].Time);
  }

  if (dawnMode != (jsonReadtoInt(configAlarm, "t") - 1)) {
    alarm_change = true;
    jsonWrite(configAlarm, "t", (dawnMode + 1));
  }
  sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, dawnMode + 1);
  if (alarm_change) {
    timeout_save_file_changes = millis();
    bitSet (save_file_changes, 1);
  }
  DAWN_TIMEOUT = jsonReadtoInt(configAlarm, "after");
  DAWN_BRIGHT = jsonReadtoInt(configAlarm, "a_br");
}
#endif // USE_DAWN

#if USE_SUNSET
void sendSunsets(char *outputBuffer) {
  char k[2];
  bool sunset_change = false;
  String configSunset = readFile(F("config_sunset.json"), 512);

  strcpy_P(outputBuffer, PSTR("SUNS"));

  for (byte i = 0; i < 7; i++) {
    itoa ((i + 1), k, 10);
    k[1] = 0;
    String a = "a" + String (k) ;
    String h = "h" + String (k) ;
    String m = "m" + String (k) ;
    if (sunsets[i].State != (jsonReadtoInt(configSunset, a)) || sunsets[i].Time != (jsonReadtoInt(configSunset, h)) * 60U + (jsonReadtoInt(configSunset, m))) {
      sunset_change = true;
      jsonWrite(configSunset, a, sunsets[i].State);
      jsonWrite(configSunset, h, (sunsets[i].Time / 60U));
      jsonWrite(configSunset, m, (sunsets[i].Time % 60U));
    }
    sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, (uint8_t)sunsets[i].State);
  }

  for (byte i = 0; i < 7; i++) {
    sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, sunsets[i].Time);
  }

  if (sunsetMode != (jsonReadtoInt(configSunset, "t") - 1)) {
    sunset_change = true;
    jsonWrite(configSunset, "t", (sunsetMode + 1));
  }
  sprintf_P(outputBuffer, PSTR("%s %u"), outputBuffer, sunsetMode + 1);
  if (sunset_change) {
    timeout_save_file_changes = millis();
    bitSet (save_file_changes, 1);
  }
  SUNSET_BRIGHT = jsonReadtoInt(configSunset, "s_br");
}
#endif // USE_SUNSET

String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

#if USE_MP3_PLAYER
void sendVolume(char *outputBuffer) {
  sprintf_P(outputBuffer, PSTR("VOL %u %u"), eff_sound_on, eff_volume);
}
#endif  // USE_MP3_PLAYER

// *****************************************************************************************************************************************************

// ******************************************************************************** blynk.ino ************************************************************

#if USE_BLYNK

BLYNK_CONNECTED() {
  updateRemoteBlynkParams();
}

// кнопка POWER ON / OFF
BLYNK_WRITE(V0) {
  int value = param.asInt();
  if (value == 1)
    processParams("P_ON", "");
  else
    processParams("P_OFF", "");

  updateRemoteBlynkParams();
}

// бегунок яркости от 1 до 255
BLYNK_WRITE(V1) {
  processParams("BRI", param.asString());
}

// бегунок скорости от 1 до 255
BLYNK_WRITE(V2) {
  processParams("SPD", param.asString());
}

// бегунок масштаба от 1 до 100
BLYNK_WRITE(V3) {
  processParams("SCA", param.asString());
}

// выбор эффекта из списка или по номеру
BLYNK_WRITE(V4) {
  int value = param.asInt() - 1;
  processParams("EFF", String(value).c_str());
  updateRemoteBlynkParams();
}

void updatePlayerBlynkParams(bool isRunning) {
  if (isRunning) {
    Blynk.setProperty(V6, "label", String("№ ") + String(currentMode) + String(" (") + String(Favorites::instance().Interval) + String(" сек)"));
    Blynk.virtualWrite(V6, "play");
  } else {
    Blynk.setProperty(V6, "label", String("№ ") + String(currentMode));
    Blynk.virtualWrite(V6, "stop");
  }
}

BLYNK_WRITE(V6) {
  String action = param.asStr();
  uint8_t nextmode;

  if (action == "play") {
    Favorites::instance().FavoritesRunning = 1U;
    Favorites::instance().nextModeAt = 0;
    Favorites::instance().Interval = CYCLE_TIMER;
    Favorites::instance().Dispersion = CYCLE_TIMER_PLUS;
    Favorites::instance().UseSavedFavoritesRunning = CYCLE_DONT_OFF;
    for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
      Favorites::instance().FavoriteModes[i] = (i < CYCLE_1ST_EFFECT || i > CYCLE_LAST_EFFECT) ? 0U : 1U;
    }
    updatePlayerBlynkParams(true);
  }
  else if (action == "stop") {
    Favorites::instance().FavoritesRunning = 0U;
    updatePlayerBlynkParams(false);
  }
  else if (action == "next") {
    nextmode = currentMode + 1U;
    if (nextmode >= MODE_AMOUNT) nextmode = 0U;
    processParams("EFF", String(nextmode).c_str());
    updateRemoteBlynkParams();
  }
  else {
    if (currentMode != 0U)
      nextmode = currentMode - 1U;
    else
      nextmode = MODE_AMOUNT - 1U;
    processParams("EFF", String(nextmode).c_str());
    updateRemoteBlynkParams();
  }
}

BLYNK_WRITE(V7) {
  String action = param.asStr();
  processParams("TXT-", String(action).c_str());
}

void updateRemoteBlynkParams() {
#if USE_BLYNK
  if (Wifi::instance().isConnected()) {
    Blynk.virtualWrite(V0, ONflag ? 1 : 0);
    Blynk.virtualWrite(V1, modes[currentMode].Brightness);
    Blynk.virtualWrite(V2, modes[currentMode].Speed);
    Blynk.virtualWrite(V3, modes[currentMode].Scale);
    Blynk.virtualWrite(V4, currentMode + 1);
    updatePlayerBlynkParams(Favorites::instance().FavoritesRunning);
  }
#endif
}

// кнопка звук ON / OFF
BLYNK_WRITE(V8) {
  int value = param.asInt();
  if (value == 1)
    processParams("SO_ON", "");
  else
    processParams("SO_OFF", "");
  updateRemoteBlynkParams();
}

// бегунок громкости от 1 до 30
BLYNK_WRITE(V9) {
  processParams("VOL", param.asString());
}

void processParams(char *prefix, const char *paramValue) {
  char charBuf[50];
  String value = prefix + String(paramValue);
  value.toCharArray(charBuf, 50);
  processInputBuffer(charBuf, NULL, false);
  // добавляем сброс настроек на значения по умолчанию при выборе всех единичек на всех бегунках
  if (modes[currentMode].Brightness == 1U && modes[currentMode].Speed == 1U && modes[currentMode].Scale == 1U) {
    restoreSettings();
    loadingFlag = true;
#if USE_MQTT
    if (Wifi::instance().isConnected()) {
      Mqtt::instance().needToPublish = true;
    }
#endif
    SetBrightness(modes[currentMode].Brightness);
    updateRemoteBlynkParams();
  }

#if USE_OTA
  else if ((currentMode == MODE_AMOUNT - 1U) && modes[currentMode].Brightness == 255U && modes[currentMode].Speed == 255U && modes[currentMode].Scale == 100U) {
    // включение прошивки по воздуху
    modes[currentMode].Brightness = 10U;
    modes[currentMode].Speed = 99U;
    modes[currentMode].Scale = 38U;
    jsonWrite(configSetup, "br", modes[currentMode].Brightness);
    jsonWrite(configSetup, "sp", modes[currentMode].Speed);
    jsonWrite(configSetup, "sc", modes[currentMode].Scale);
    Ota::instance().RequestOtaUpdate();
    delay(70);
    Ota::instance().RequestOtaUpdate();
    currentMode = EFF_MATRIX;  // принудительное включение режима "Матрица" для индикации перехода в режим обновления по воздуху
    jsonWrite(configSetup, "eff_sel", currentMode);
    FastLED.clear();
    delay(1);
    ONflag = true;
    jsonWrite(configSetup, "Power", ONflag);
    changePower();
    updateRemoteBlynkParams();
  }
#endif // USE_OTA
}

#endif // USE_BLYNK

// ******************************************************************************************************************************************************

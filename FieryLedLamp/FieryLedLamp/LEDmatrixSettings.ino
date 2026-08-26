// ********************************************************************** LEDmatrixSettings.ino *********************************************************
#include <FastLED.h>
#include "Constants.h"
#include "Extern.h"
#include "Prototypes.h"
//----------------------

// ============================================================================= ЯРКОСТЬ ===============================================================
// Применение настроек яркости
void SetBrightness(uint8_t newBrightness) {
  uint8_t finalBrightness = newBrightness;

  if (AutoBrightness && !ONflag) {
    finalBrightness = getBrightnessForPrintTime();
  }
#if USE_DAWN
  else if (dawnFlag == 1) {
    return;
  }
#endif

#if USE_DAWN
  if (AutoBrightness && !dawnFlag && !day_night) {
    finalBrightness = constrain(newBrightness >> AutoBrightness, 1, 100);
  }
#else
  if (AutoBrightness && !day_night) {
    finalBrightness = constrain(newBrightness >> AutoBrightness, 1, 100);
  }
#endif
  FastLED.setBrightness(finalBrightness);

  // Сохранение новой яркости только, если лампа включена (или нет рассвета)
#if USE_DAWN
  if (dawnFlag != 1) {
#endif
    modes[currentMode].Brightness = newBrightness;

    if (ONflag) {
      jsonWrite(configSetup, "br", newBrightness);

      if (nightModeBrightness == 0) {
        userClockBrightness = newBrightness;
      }
    }
#if USE_DAWN
  }
#endif

#if USE_ST7789
  TFT_ApplyBrightnessNow();
#endif
} // void SetBrightness(uint8_t newBrightness)

// Сохранение текущей яркости эффекта
void saveCurrentBrightness() {
  if (!ONflag) return;
  modes[currentMode].Brightness = FastLED.getBrightness();
  jsonWrite(configSetup, "br", modes[currentMode].Brightness);

  if (nightModeBrightness == 0) {
    userClockBrightness = modes[currentMode].Brightness;
  }

  Eeprom::instance().EepromPut(modes);
}

// Загрузка и применение яркость для эффекта
void loadBrightnessForMode(uint8_t mode) {
  if (mode >= MODE_AMOUNT) mode = 0;

  uint8_t jsonBrightness = jsonReadtoInt(configSetup, "br");
  uint8_t eepromBrightness = modes[mode].Brightness;

  if (jsonBrightness >= 1 && jsonBrightness <= 255) {
    if (eepromBrightness != jsonBrightness) {
      modes[mode].Brightness = jsonBrightness;
      if (ONflag) {
        Eeprom::instance().EepromPut(modes);
      }
#if GENERAL_LOG
      SYSLOG.add("Яркость из JSON (%u) сохранена в EEPROM", jsonBrightness);
#endif
    }
  } else {
    if (eepromBrightness == 0) {
      eepromBrightness = pgm_read_byte(&defaultSettings[mode][0]);
      modes[mode].Brightness = eepromBrightness;
      if (ONflag) {
        Eeprom::instance().EepromPut(modes);
      }
    }
    jsonWrite(configSetup, "br", String(eepromBrightness));
    if (ONflag) {
      saveConfig();
    }
  }

  if (nightModeBrightness == 0 && ONflag) {
    userClockBrightness = modes[mode].Brightness;
  }

  SetBrightness(modes[mode].Brightness);
} // void loadBrightnessForMode(uint8_t mode)

// ============================================================================ LED_PANEL ==============================================================
void handleTimerPhases(bool &showClock, bool &showDate, bool &showWeather) {
#if LED_PANEL
  if (timer_clock_fixed) {
    static bool showDateNow = true;

    if (millis() - lastClockFixedSwitch >= (unsigned long)interval_clock_fixed * 1000UL) {
      showDateNow = !showDateNow;
      lastClockFixedSwitch = millis();
      loadingFlag = true;
    }

    showClock = true;
    showDate = showDateNow;
    showWeather = !showDateNow;
    return;
  }

  static uint32_t lastTimerSwitch = 0;
  static uint8_t currentTimerPhase = 0;
  uint16_t intervalSec = 10;
  uint8_t maxPhase = 0;

  if (timer_c_w) {
    intervalSec = interval_c_w;
    maxPhase = 1;
  }
  else if (timer_c_d) {
    intervalSec = interval_c_d;
    maxPhase = 1;
  }
  else if (timer_d_w) {
    intervalSec = interval_d_w;
    maxPhase = 1;
  }
  else if (timer_c_d_w) {
    intervalSec = interval_c_d_w;
    maxPhase = 2;
  }

  if (millis() - lastTimerSwitch >= (unsigned long)intervalSec * 1000UL) {
    currentTimerPhase = (currentTimerPhase + 1) % (maxPhase + 1);
    lastTimerSwitch = millis();
    loadingFlag = true;
  }

  if (timer_c_w) {
    showClock = (currentTimerPhase == 0);
    showWeather = (currentTimerPhase == 1);
  }
  else if (timer_c_d) {
    showClock = (currentTimerPhase == 0);
    showDate = (currentTimerPhase == 1);
  }
  else if (timer_d_w) {
    showDate = (currentTimerPhase == 0);
    showWeather = (currentTimerPhase == 1);
  }
  else if (timer_c_d_w) {
    showClock = (currentTimerPhase == 0);
    showDate  = (currentTimerPhase == 1);
    showWeather = (currentTimerPhase == 2);
  }
#endif
}

// ------------------------------------------
#if LED_PANEL
void clearTextAreaOnly() {
  int y_start = textBaseY + textYOffset - 1;
  int y_end = y_start + LET_HEIGHT + 2;

  for (int y = max(0, y_start); y <= min((int)matrixHeight - 1, y_end); y++) {
    for (int x = 0; x < matrixWidth; x++) {
      // не затирать, если на матрице могут быть дата/погода
      if (y >= lastDateTop && y <= lastDateBottom && lastDateLeft >= 0) continue;
#if USE_WEATHER
      if (y >= lastWeatherTop && y <= lastWeatherBottom && lastWeatherLeft >= 0) continue;
#endif
      leds[XY(x, y)] = CRGB::Black;
    }
  }
}
#endif

// ------------------------------------------
#if LED_PANEL
void finishRunningText() {
  if (systemShuttingDown) return;
  textIsRunning = false;
  offset = matrixWidth + 10;
  Fill_String = false;

  if (IntervalrunText > 0) {
    runningTextTimer.reset();
  } else if (!runTextOver) {
    textIsRunning = true;
    loadingFlag = true;
    offset = matrixWidth + 10;
  }
  loadingFlag = true;
}
#endif

// ------------------------------------------
// сброс таймера (часы, дата, погода)
void resetTimerState() {
  if (systemShuttingDown) return;
#if LED_PANEL
  lastTimerSwitch = millis();
  currentTimerPhase = 0;
  clockNeedRedraw = true;
  loadingFlag = true;
  needFullRedraw = true;
#endif
}

// ------------------------------------------
void led_panel(bool drawStringThisTick) {
#if LED_PANEL
  if (!ONflag) {
    FastLED.clear();
    FastLED.show();
    return;
  }

  static uint32_t lastShow = 0;
  const uint16_t FRAME_MS = 25;
  bool anyTimerActive = (currentMode == EFF_CLOCK) && (timer_c_w || timer_c_d || timer_d_w || timer_c_d_w || timer_clock_fixed);
  // бегущая строка
  if (runTextEnabled && textIsRunning) {
    CRGB textColor = CHSV(ColorRunningText, 255U, 255U);
    if (runTextOver) {
      // поверх эффекта
      if (Painting == 0 && isLampActive() && currentMode != EFF_CLOCK) {
        effectsTick();
      }
      else if (currentMode == EFF_CLOCK && !drawStringThisTick) {
        FastLED.clear();
        bool showClock = true, showDate = false, showWeather = false;
        if (anyTimerActive) handleTimerPhases(showClock, showDate, showWeather);
        else {
          if (dateEnabled) showDate = true;
#if USE_WEATHER
          if (weatherEnabled) showWeather = true;
#endif
        }
        if (showClock) clockRoutine();
        if (showDate) dateRoutine();
#if USE_WEATHER
        if (showWeather) weatherRoutine();
#endif
      }

      clearTextAreaOnly();
      fillString(TextTicker, textColor, true);
    }
    else {
      FastLED.clear();
      fillString(TextTicker, textColor, false);
    }

    if (Fill_String) {
      finishRunningText();
    }
    loadingFlag = true;
  }
  else { // обычный режим (без бегущей строки)
    if (currentMode == EFF_CLOCK && !drawStringThisTick) {
      FastLED.clear();
      bool showClock = true, showDate = false, showWeather = false;
      if (anyTimerActive) {
        handleTimerPhases(showClock, showDate, showWeather);
      }
      else {
        if (dateEnabled) showDate = true;
#if USE_WEATHER
        if (weatherEnabled) showWeather = true;
#endif
      }
      if (showClock) clockRoutine();
      if (showDate) dateRoutine();
#if USE_WEATHER
      if (showWeather) weatherRoutine();
#endif
    }
    else if (ONflag && Painting == 0 && isLampActive()) {
      effectsTick();
    }
  }

  // вывод на матрицу
  bool needShow = loadingFlag || anyTimerActive || (millis() - lastShow >= FRAME_MS);
  if (needShow) {
    FastLED.show();
    lastShow = millis();
    loadingFlag = false;
  }
#endif // LED_PANEL
} // void led_panel(bool drawStringThisTick)

// ====================================================== СМЕНА ЦВЕТА ЧАСОВ, ДАТЫ, ПОГОДЫ, ТЕКСТА (НА МАТРИЦЕ) =========================================
void updateAutoHueModes() {
  static uint32_t prev = 0;
  const uint32_t interval = 80;
  const uint8_t step = 1;
  uint32_t now = millis();

  if (now - prev < interval) return;
  prev = now;

  bool changed = false;

#if LED_PANEL
#if USE_WEATHER
  if (autoWeatherHue && !rainbowWeather) {
    weatherHue = (weatherHue + step) % 256;
    changed = true;
  }
#endif // USE_WEATHER

  if (autoHueDate && !rainbowDate) {
    dateHue = (dateHue + step) % 256;
    changed = true;
  }
  if (autoRunTextHue && !rainbowText) {
    runTextHue = (runTextHue + step) % 256;
    ColorRunningText = runTextHue;
    changed = true;
  }
#endif // LED_PANEL

  if (autoClockHue && !rainbowClock) {
    clockHue = (clockHue + step) % 256;
    changed = true;
  }
  if (changed) {
    loadingFlag = true;
  }
}

// ======================================================================= IP-адрес НА МАТРИЦЕ =========================================================
void showIPOnMatrix() {
#if DISPLAY_IP_AT_START
  static bool ipShown = false;
  if (ipShown) return;

  IPAddress ip = WiFi.localIP();
  if (ip == IPAddress(0, 0, 0, 0)) {
    ip = WiFi.softAPIP();
  }
  String ipToShow = ip.toString();

  ipShown = true;
  bool oldRunTextOver = runTextOver;
  runTextOver = false;
  loadingFlag = true;

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
  digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
#endif

#if USE_ST7789
  TFT_ShowIP(ipToShow.c_str());
#endif

  uint8_t savedBrightness = FastLED.getBrightness();
  FastLED.setBrightness(20);

  bool textEnded = false;
  while (!textEnded) {
#if USE_ST7789
    TFT_LoopTick();
#endif
    textEnded = fillString(ipToShow.c_str(), CRGB(220, 220, 240), false);
    FastLED.show();
    parseUDP();
    HTTP.handleClient();
    delay(1);
    yield();
  }

  FastLED.setBrightness(savedBrightness);
  FastLED.clear();
  FastLED.show();
  textIsRunning = false;
  runTextOver = oldRunTextOver;

#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
#if USE_DAWN
  digitalWrite(MOSFET_PIN, ONflag || (dawnFlag == 1 && !manualOff) ? MOSFET_LEVEL : !MOSFET_LEVEL);
#else
  digitalWrite(MOSFET_PIN, ONflag ? MOSFET_LEVEL : !MOSFET_LEVEL);
#endif
#endif

  loadingFlag = false;
#endif
}

// ================================================================== РАБОТА С ПАМЯТЬЮ (буферы) ========================================================
template<typename T>
void safeDeleteArray(T*& ptr) {
  if (ptr) {
    delete[] ptr;
    ptr = nullptr;
  }
}

// -------------------------------------------------------------------
bool checkAlloc(void* ptr, const char* name) {
  if (!ptr) {
#if MATRIX_LOG
    SYSLOG.add("Ошибка выделения памяти: %s", name);
#endif
    return false;
  }
  return true;
}

// -------------------------------------------------------------------
void printFreeHeap(const char* stage) {
#if MEMORY_LOG
  SYSLOG.add("%s - свободно ОЗУ: %d байт", stage, ESP.getFreeHeap());
#endif
}

// -------------------------------------------------------------------
// освобождение 3D noise с указанием старой ширины матрицы
void freeNoise3D(uint16_t oldWidth) {
  if (!noise3d) return;

  for (uint8_t l = 0; l < NUM_LAYERSMAX; l++) {
    if (noise3d[l]) {
      for (uint16_t i = 0; i < oldWidth; i++) {
        safeDeleteArray(noise3d[l][i]);
      }
      safeDeleteArray(noise3d[l]);
    }
  }
  safeDeleteArray(noise3d);
  noise3d = nullptr;
}

// -------------------------------------------------------------------
// освобождение всех буферов
void freeOldBuffers(uint16_t oldWidth, uint16_t oldHeight) {
  safeDeleteArray(leds);
  safeDeleteArray(effectBuffer);
  safeDeleteArray(ledsbuff);
  safeDeleteArray(line);
  safeDeleteArray(shiftHue);
  safeDeleteArray(shiftValue);
  safeDeleteArray(fireBufferGlobal);

  if (matrixValue) {
    for (uint16_t y = 0; y < oldHeight; y++) safeDeleteArray(matrixValue[y]);
    safeDeleteArray(matrixValue);
    matrixValue = nullptr;
  }

  if (noise) {
    for (uint16_t i = 0; i < prevMaxNoiseDim; i++) safeDeleteArray(noise[i]);
    safeDeleteArray(noise);
    noise = nullptr;
  }
  prevMaxNoiseDim = 0;

  freeNoise3D(oldWidth);
}

// -------------------------------------------------------------------
// выделение всех буферов (под новые размеры)
bool allocateAllBuffers(uint16_t width, uint16_t height, uint16_t used, MatrixBuffers& out) {
  out.leds = new (std::nothrow) CRGB[used];
  if (!out.leds) goto cleanup;
  out.ledsbuff = new (std::nothrow) CRGB[used];
  if (!out.ledsbuff) goto cleanup;
  memset(out.ledsbuff, 0, used * sizeof(CRGB));

  if (used * sizeof(CRGB) <= (size_t)ESP.getFreeHeap() - 8192) {
    out.effectBuffer = new (std::nothrow) CRGB[used];
  }

  out.line = new (std::nothrow) uint8_t[width]();
  if (!out.line) goto cleanup;
  out.shiftHue = new (std::nothrow) uint8_t[height]();
  if (!out.shiftHue) goto cleanup;
  out.shiftValue = new (std::nothrow) uint8_t[height]();
  if (!out.shiftValue) goto cleanup;

  out.matrixValue = new (std::nothrow) uint8_t*[height];
  if (!out.matrixValue) goto cleanup;
  for (uint16_t y = 0; y < height; y++) {
    out.matrixValue[y] = new (std::nothrow) uint8_t[width]();
    if (!out.matrixValue[y]) goto cleanup;
  }

  out.noiseDim = max(width, height);
  out.noise = new (std::nothrow) uint8_t*[out.noiseDim];
  if (!out.noise) goto cleanup;
  for (uint16_t i = 0; i < out.noiseDim; i++) {
    out.noise[i] = new (std::nothrow) uint8_t[out.noiseDim]();
    if (!out.noise[i]) goto cleanup;
  }

  out.noise3d = new (std::nothrow) uint8_t**[NUM_LAYERSMAX];
  if (!out.noise3d) goto cleanup;
  for (uint8_t l = 0; l < NUM_LAYERSMAX; l++) {
    out.noise3d[l] = new (std::nothrow) uint8_t*[width];
    if (!out.noise3d[l]) goto cleanup;
    for (uint16_t i = 0; i < width; i++) {
      out.noise3d[l][i] = new (std::nothrow) uint8_t[height]();
      if (!out.noise3d[l][i]) goto cleanup;
    }
  }
  return true;

cleanup:
  freeAllBuffers(out, width, height, used);
  return false;
}

// -------------------------------------------------------------------
// выделенные буферы
void freeAllBuffers(MatrixBuffers& buf, uint16_t width, uint16_t height, uint16_t used) {
  safeDeleteArray(buf.leds);
  safeDeleteArray(buf.ledsbuff);
  safeDeleteArray(buf.effectBuffer);
  safeDeleteArray(buf.line);
  safeDeleteArray(buf.shiftHue);
  safeDeleteArray(buf.shiftValue);

  if (buf.matrixValue) {
    for (uint16_t y = 0; y < height; y++) safeDeleteArray(buf.matrixValue[y]);
    safeDeleteArray(buf.matrixValue);
  }

  if (buf.noise) {
    for (uint16_t i = 0; i < buf.noiseDim; i++) safeDeleteArray(buf.noise[i]);
    safeDeleteArray(buf.noise);
  }
  buf.noiseDim = 0;

  if (buf.noise3d) {
    for (uint8_t l = 0; l < NUM_LAYERSMAX; l++) {
      if (buf.noise3d[l]) {
        for (uint16_t i = 0; i < width; i++) safeDeleteArray(buf.noise3d[l][i]);
        safeDeleteArray(buf.noise3d[l]);
      }
    }
    safeDeleteArray(buf.noise3d);
  }
}

// -------------------------------------------------------------------
bool reconfigureMatrix(uint16_t newWidth, uint16_t newHeight, uint16_t newUsed, uint16_t oldWidth, uint16_t oldHeight) {
  MatrixBuffers newBuf;
  if (!allocateAllBuffers(newWidth, newHeight, newUsed, newBuf)) {
#if MATRIX_LOG
    SYSLOG.add("Не удалось выделить буферы для новой матрицы (%dx%d)", newWidth, newHeight);
#endif
    return false;
  }

  freeOldBuffers(oldWidth, oldHeight); // освобождение старых буферов

  leds = newBuf.leds;
  ledsbuff = newBuf.ledsbuff;
  effectBuffer = newBuf.effectBuffer;
  line = newBuf.line;
  shiftHue = newBuf.shiftHue;
  shiftValue = newBuf.shiftValue;
  matrixValue = newBuf.matrixValue;
  noise = newBuf.noise;
  prevMaxNoiseDim = newBuf.noiseDim;
  noise3d = newBuf.noise3d;

  newBuf.leds = nullptr;
  newBuf.ledsbuff = nullptr;
  newBuf.effectBuffer = nullptr;
  newBuf.line = nullptr;
  newBuf.shiftHue = nullptr;
  newBuf.shiftValue = nullptr;
  newBuf.matrixValue = nullptr;
  newBuf.noise = nullptr;
  newBuf.noise3d = nullptr;

  return true;
}

// ******************************************************************************************************************************************************

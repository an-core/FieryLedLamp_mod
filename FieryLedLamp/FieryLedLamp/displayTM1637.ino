// *************************************************************************** displayTM1637.ino ********************************************************
#include "Extern.h"
#include "Time.h"
#include "Types.h"
// -------------------------

#if USE_TM1637
// вспомогательная функция для отображения даты (день.месяц)
static void displayDate() {
  time_t t = getCurrentLocalTime();
  struct tm *tm = localtime(&t);
  if (!tm) return;

  uint8_t day = tm->tm_mday;
  uint8_t month = tm->tm_mon + 1;

  uint8_t buf[4];
  buf[0] = display.encodeDigit(day / 10);
  buf[1] = display.encodeDigit(day % 10);
  buf[2] = 0b01000000;  // тире

  if (month < 10) {
    buf[3] = display.encodeDigit(month);
  } else {
    // месяц 10, 11, 12 - не влезает в 4 разряда с тире
    // вариант: показать 25-1 (первая цифра месяца)
    buf[3] = display.encodeDigit(month / 10);  // покажет 25-1 для 10-12
  }

  display.setSegments(buf, 4, 0);
}

static void displayClock() {
  static uint32_t callCount = 0;
  callCount++;
  static uint32_t lastLog = 0;
  if (millis() - lastLog > 1000) {
#if TM1637_LOG
    SYSLOG.add("CLOCK calls/sec: %u", callCount);
#endif
    callCount = 0;
    lastLog = millis();
  }
  clockTicker_blink();
}

static void displayWeather() {
  static uint32_t callCount = 0;
  callCount++;
  static uint32_t lastLog = 0;
  if (millis() - lastLog > 1000) {
#if TM1637_LOG
    SYSLOG.add("WEATHER calls/sec: %u", callCount);
#endif
    callCount = 0;
    lastLog = millis();
  }
  display.resetPoints();
  float temp = Weather::instance().getTemperature();
  uint8_t buf[4];

  if (temp > -50) {
    int8_t t = round(temp);
    bool neg = t < 0;
    int8_t abs_t = abs(t);
    if (abs_t > 99) abs_t = 99;
    uint8_t d10 = abs_t / 10;
    uint8_t d1  = abs_t % 10;

    if (neg) {
      if (abs_t == 0) {
        buf[0] = _empty; buf[1] = display.encodeDigit(0); buf[2] = _deg; buf[3] = _C;
      } else if (abs_t < 10) {
        buf[0] = _dash; buf[1] = display.encodeDigit(d1); buf[2] = _deg; buf[3] = _C;
      } else {
        buf[0] = _dash; buf[1] = display.encodeDigit(d10); buf[2] = display.encodeDigit(d1); buf[3] = _deg;
      }
    } else {
      if (abs_t == 0) {
        buf[0] = _empty; buf[1] = display.encodeDigit(0); buf[2] = _deg; buf[3] = _C;
      } else if (abs_t < 10) {
        buf[0] = _empty; buf[1] = display.encodeDigit(d1); buf[2] = _deg; buf[3] = _C;
      } else {
        buf[0] = display.encodeDigit(d10); buf[1] = display.encodeDigit(d1); buf[2] = _deg; buf[3] = _C;
      }
    }
  } else {
    buf[0] = _empty; buf[1] = _empty; buf[2] = _empty; buf[3] = _E;
  }

  display.setSegments(buf, 4, 0);
}

static DisplayMode activeModes[3];
static uint8_t activeModeCount = 0;
static uint8_t activeModeIndex = 0;

static void rebuildActiveModes() {
  activeModeCount = 0;
  activeModes[activeModeCount++] = DISP_MODE_CLOCK;  // часы всегда
  if (weatherSwitchEnabled) activeModes[activeModeCount++] = DISP_MODE_WEATHER;
  if (dateSwitchEnabled)    activeModes[activeModeCount++] = DISP_MODE_DATE;

  if (activeModeIndex >= activeModeCount) activeModeIndex = 0;
}

void Display_Timer(uint8_t argument) {
  // показ E:xx (номер эффекта)
  if (!tm1637Enabled) return;
  if (DisplayFlag == 0 && LastEffect != currentMode) {
    LastEffect = currentMode;
    DisplayTimer = millis();
    DisplayFlag = 1;
    uint8_t n = 0;
    for (; n < MODE_AMOUNT; n++) {
      if (eff_num_correct[n] == currentMode) break;
    }
    display.point(true);
    if (n < 100) {
      display.displayByte(_E_, _empty, _empty, _empty);
      display.showNumberDecEx(n, 0, true, 2, 2);
    } else {
      display.displayByte(_E_, _empty, _empty, _empty);
      display.showNumberDecEx(n, 0, true, 3, 1);
    }
  }

  if (DisplayFlag == 1 && (millis() - DisplayTimer > 2000)) {
    DisplayFlag = 0;
    display.point(false);
  }

#if USE_MP3_PLAYER && USE_TM1637
  // отображение номера папки только если и MP3, и TM1637 включены в прошивку
  if (mp3Enabled && tm1637Enabled) {
    if (DisplayFlag == 0 && LastCurrentFolder != CurrentFolder) {
      LastCurrentFolder = CurrentFolder;
      DisplayTimer = millis();
      DisplayFlag = 2;
      display.point(true);
      display.displayByte(_F_, _empty, _empty, _empty);
      display.showNumberDecEx(CurrentFolder, 0, true, 2, 2);
    }

    if (DisplayFlag == 2 && (millis() - DisplayTimer > 3000)) {
      DisplayFlag = 0;
      display.point(false);
    }
  } else {
    DisplayFlag = 0;
  }
#endif

  // показ параметра (25 и т.д.)
  if (DisplayFlag == 3) {
    DisplayTimer = millis();
    DisplayFlag = 4;
    display.point(false);
    display.clear();
    display.showNumberDecEx(argument, 0, true, 3, 1);
  }

  if (DisplayFlag == 4 && (millis() - DisplayTimer > 3000)) {
    DisplayFlag = 0;
  }
  // переключение Часы / Погода / Дата
  if (DisplayFlag == 0) {
#if USE_DAWN || USE_SUNSET
    bool blinkActive =
#if USE_DAWN
      (dawnFlag == 1)
#endif
#if USE_DAWN && USE_SUNSET
      ||
#endif
#if USE_SUNSET
      (sunsetFlag == 1)
#endif
      ;

    if (blinkActive) {
      // во время рассвета/заката будут только часы, без переключения на погоду и дату
      displayMode = DISP_MODE_CLOCK;
      displayClock();
    } else
#endif
      if (inClockWeatherMode && DISPLAY_SWITCH_INTERVAL > 0) {
        if (millis() - displaySwitchTimer >= DISPLAY_SWITCH_INTERVAL) {
          displaySwitchTimer = millis();
          activeModeIndex = (activeModeIndex + 1) % activeModeCount;
          displayMode = activeModes[activeModeIndex];
        }

        if (displayMode == DISP_MODE_WEATHER && weatherErrActive) {
          if (millis() - weatherErrTimer >= WEATHER_ERR_TIME) {
            weatherErrActive = false;
            activeModeIndex = 0; // вернуться на часы
            displayMode = activeModes[0];
            displaySwitchTimer = millis();
          }
        }

        switch (displayMode) {
          case DISP_MODE_CLOCK: displayClock(); break;
          case DISP_MODE_WEATHER: displayWeather(); break;
          case DISP_MODE_DATE: displayDate(); break;
        }
      } else {
        displayMode = DISP_MODE_CLOCK;
        displayClock();
      }
  }
} // void Display_Timer(uint8_t argument

// ----------------------------------------------------------------------
void clockTicker_blink() {
  if (!tm1637Enabled) return;
  if (myTime.isTimeSet() && !DisplayFlag) {
    time_t t = getCurrentLocalTime();
    struct tm *ti = localtime(&t);
    uint8_t h = ti->tm_hour;
    uint8_t m = ti->tm_min;

#if USE_DAWN || USE_SUNSET
    bool blinkActive =
#if USE_DAWN
      (dawnFlag == 1)
#endif
#if USE_DAWN && USE_SUNSET
      ||
#endif
#if USE_SUNSET
      (sunsetFlag == 1)
#endif
      ;

    if (blinkActive) {
  display.displayClock(h, m);
  if (millis() - tmr_blink > 100) {
    tmr_blink = millis();
    display.setBrightness((DispBrightness / 51U) > 4 ? 7 : DispBrightness / 51U, DispBrightness);
    if (DispBrightness >= 204) aDirection = false;
    if (DispBrightness < 51U) {
      if (!DispBrightness) DispBrightness = 1;
      aDirection = true;
    }
    if (aDirection) DispBrightness += 51U;
    else DispBrightness -= 51U;
  }
  boolean points[4] = {0, 0, 0, 0};
  points[1] = true;
  display.setSegmentPoints(points);
  return;
}
#endif

    tm1637_brightness();
    display.setBrightness((DispBrightness / 51U) > 4 ? 7 : DispBrightness / 51U, DispBrightness);
    display.displayClock(h, m);
  }
}

void tm1637_brightness() {
  if (NIGHT_HOURS_START >= NIGHT_HOURS_STOP) { // переход через полночь
    if (thisTime >= NIGHT_HOURS_START || thisTime <= NIGHT_HOURS_STOP) {
      DispBrightness = NIGHT_HOURS_BRIGHTNESS ? NIGHT_HOURS_BRIGHTNESS : 0;
    }
    else {
      DispBrightness = DAY_HOURS_BRIGHTNESS ? DAY_HOURS_BRIGHTNESS : 0;
    }
  }
  else {
    if (thisTime >= NIGHT_HOURS_START && thisTime <= NIGHT_HOURS_STOP) {
      DispBrightness = NIGHT_HOURS_BRIGHTNESS ? NIGHT_HOURS_BRIGHTNESS : 0;
    }
    else {
      DispBrightness = DAY_HOURS_BRIGHTNESS ? DAY_HOURS_BRIGHTNESS : 0;
    }
  }
}

#endif // USE_TM1637

// ******************************************************************************************************************************************************

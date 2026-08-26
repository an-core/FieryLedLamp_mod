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

  uint8_t d1 = day / 10;
  uint8_t d2 = day % 10;
  uint8_t m1 = month / 10;
  uint8_t m2 = month % 10;

  uint8_t seg1 = display.encodeDigit(d1);
  uint8_t seg2 = display.encodeDigit(d2);
  uint8_t seg3 = display.encodeDigit(m1);
  uint8_t seg4 = display.encodeDigit(m2);

  seg2 |= 0b10000000;

  display.displayByte(seg1, seg2, seg3, seg4);
}

static void displayClock() {
  clockTicker_blink();
}

static void displayWeather() {
  display.point(false);
  float temp = Weather::instance().getTemperature();
  if (temp > -50) {
    int8_t t = round(temp);
    bool neg = t < 0;
    int8_t abs_t = abs(t);
    if (abs_t > 99) abs_t = 99;
    uint8_t d10 = abs_t / 10;
    uint8_t d1  = abs_t % 10;

    if (neg) {
      if (abs_t == 0) {
        display.displayByte(_empty, display.encodeDigit(0), _deg, _C);
      } else if (abs_t < 10) {
        display.displayByte(_dash, display.encodeDigit(d1), _deg, _C);
      } else {
        display.displayByte(_dash, display.encodeDigit(d10), display.encodeDigit(d1), _deg);
      }
    } else {
      if (abs_t == 0) {
        display.displayByte(_empty, display.encodeDigit(0), _deg, _C);
      } else if (abs_t < 10) {
        display.displayByte(_empty, display.encodeDigit(d1), _deg, _C);
      } else {
        display.displayByte(display.encodeDigit(d10), display.encodeDigit(d1), _deg, _C);
      }
    }
  } else {
    display.displayByte(_empty, _empty, _empty, _E_);
  }
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
  // Отображение номера папки только если и MP3, и TM1637 включены в прошивку
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
    if (inClockWeatherMode) {
      if (millis() - displaySwitchTimer >= DISPLAY_SWITCH_INTERVAL) {
        displaySwitchTimer = millis();
        displayMode = (DisplayMode)((displayMode + 1) % 3);
      }

      if (displayMode == DISP_MODE_WEATHER && weatherErrActive) {
        if (millis() - weatherErrTimer >= WEATHER_ERR_TIME) {
          weatherErrActive = false;
          displayMode = DISP_MODE_CLOCK;
          displaySwitchTimer = millis();
        }
      }

      switch (displayMode) {
        case DISP_MODE_CLOCK:
          displayClock();
          break;
        case DISP_MODE_WEATHER:
          displayWeather();
          break;
        case DISP_MODE_DATE:
          displayDate();
          break;
      }
    } else {
      displayClock(); // если режим переключения выключен (т.е. интервал переключения установлен 0), то всегда часы
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

    if (dawnFlag == 1) { // если рассвет - мигаем дисплеем
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
    } 
    else {
      tm1637_brightness();
      display.setBrightness((DispBrightness / 51U) > 4 ? 7 : DispBrightness / 51U, DispBrightness);
      display.displayClock(h, m);
    }
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

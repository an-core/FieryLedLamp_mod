// *************************************************************************** displayST7789.ino ********************************************************
#if USE_ST7789
#include <TFT_eSPI.h>
#include <TJpg_Decoder.h>
#include "Prototypes.h"
#include "Extern.h"
#include "Time.h"
#include "Types.h"
// -------------------------

static bool tftInited = false;
static uint8_t tftDispBrightness = 255;
static bool tftBreathDir = true;
static uint32_t tftBlinkTmr = 0;
static void tftBrightnessTick();
#if USE_WEATHER
static uint32_t weatherErrBlinkTimer = 0;
static bool weatherErrBlinkState = false;
#endif

static TFT_eSPI tft = TFT_eSPI();

#if USE_WEATHER
const uint32_t WEATHER_ERR_BLINK = 500;       // Интервал мигания погоды при ошибке
#endif

static inline uint16_t tftColorFromId(uint8_t id) {
  switch (id) {
    default:
    case 0: return TFT_WHITE;
    case 1: return TFT_CYAN;
    case 2: return TFT_YELLOW;
    case 3: return TFT_GREEN;
    case 4: return TFT_RED;
    case 5: return TFT_BLUE;
    case 6: return TFT_MAGENTA;
    case 7: return 0xFD20; // оранжевый
    case 8: return TFT_DARKGREEN;
    case 9: return TFT_DARKCYAN;
    case 10: return TFT_MAROON;
    case 11: return TFT_PURPLE;
    case 12: return TFT_OLIVE;
    case 13: return TFT_LIGHTGREY;
    case 14: return TFT_GREENYELLOW;
    case 15: return TFT_PINK;
    case 16: return TFT_BROWN;
    case 17: return TFT_GOLD;
    case 18: return TFT_SILVER;
    case 19: return TFT_SKYBLUE;
    case 20: return TFT_VIOLET;
  }
}

static inline void tftBacklightWrite(uint8_t val) {
  ledcWrite(TFT_BL_CH, val);
}

void TFT_PowerOff() {
  if (tftInited) {
    tft.fillScreen(TFT_BLACK);
  }
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);
  tftBacklightWrite(0);
  tftInited = false;
}

static TFT_View lastView = TFT_VIEW_DASH;
static uint32_t lastDraw = 0;
static int lastMinuteTFT = -1;
static int lastHourTFT = -1;
static int lastTempTFT = 1000;
static bool tftColonState = true;
static bool lastColonTFT  = true;
static uint32_t tftColonTmr   = 0;
static uint16_t lastEffTFT = 0xFFFF;
static uint8_t lastArgTFT = 0xFF;
static DisplayMode lastDisplayMode = DISP_MODE_CLOCK;
static bool lastTimeSynched = false;
static uint8_t lastDisplayFlag = 255;
static bool tftShowIP = false;
static uint32_t tftIPShowTmr = 0;
static char tftIPBuf[24] = {0};

// ----------------------------------------------------------------------------------------
// Бегущая строка
static constexpr uint8_t  TFT_TICKER_FONT = 1;
static constexpr uint8_t  TFT_TICKER_SIZE = 9;
static constexpr int16_t  TFT_TICKER_SPR_H = (int16_t)(8 * TFT_TICKER_SIZE + 2);
static bool tftTickerActive = false;
static uint32_t tftTickerNextStart = 0;
static int16_t tftTickerX = 0;
static int16_t tftTickerW = 1;
static uint32_t tftTickerLastUs = 0;
static uint32_t tftTickerAccPxUs = 0;
static constexpr uint32_t TFT_TICKER_FRAME_US = 100;
static TFT_eSprite tftTickerSpr(&tft);
static bool tftTickerSprReady = false;
static inline uint16_t clampU16(uint16_t v, uint16_t lo, uint16_t hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static inline uint32_t tftTickerPeriodMs() {
  uint16_t p = clampU16((uint16_t)tft_ticker_period, 0, 3600);
  return (uint32_t)p * 1000UL;
}

static inline bool tftTickerHasText() {
  return (TFTTickerText[0] != '\0');
}

static inline bool tftTickerEnabled() {
  return tftInited && tft_ticker_on && tftTickerHasText();
}

static void tftTickerEnsureSprite() {
  if (tftTickerSprReady) return;

  tftTickerSpr.setColorDepth(16);
  tftTickerSprReady = tftTickerSpr.createSprite(tft.width(), TFT_TICKER_SPR_H);
}

static void tftTickerApplyTextStyle() {
  tftTickerSpr.setTextColor(tftColorFromId(tft_ticker_color), TFT_BLACK);
  tftTickerSpr.setTextFont(TFT_TICKER_FONT);
  tftTickerSpr.setTextSize(TFT_TICKER_SIZE);
}

static void tftTickerStart() {
  if (!tftTickerEnabled()) return;
  tftTickerEnsureSprite();
  tftTickerSpr.fillSprite(TFT_BLACK);
  tftTickerApplyTextStyle();
  tftTickerW = tftTickerSpr.textWidth(TFTTickerText);
  if (tftTickerW < 1) tftTickerW = 1;
  tftTickerX = tft.width();
  tftTickerLastUs = micros();
  tftTickerAccPxUs = 0;
  tft.fillScreen(TFT_BLACK);
  tftTickerActive = true;
  lastView = TFT_VIEW_TICKER;
}

static void tftTickerStop() {
  tftTickerActive = false;
  tftTickerNextStart = millis() + tftTickerPeriodMs();
  displayMode = DISP_MODE_CLOCK;
  displaySwitchTimer = millis();
  lastView = (TFT_View) - 1;
}

static void tftTickerDrawFrame() {
  tftTickerSpr.fillSprite(TFT_BLACK);
  tftTickerApplyTextStyle();
  tftTickerSpr.drawString(TFTTickerText, tftTickerX, 0);
  const int16_t y = (tft.height() - TFT_TICKER_SPR_H) / 2;
  tftTickerSpr.pushSprite(0, y);
}

static void tftTickerTick() {
  if (!tftInited) return;

  if (!tftTickerEnabled()) {
    tftTickerActive = false;
    tftTickerNextStart = 0;
    return;
  }

  if (!tftTickerActive) {
    if (tftTickerNextStart == 0) {
      tftTickerNextStart = millis() + tftTickerPeriodMs();
    }
    if ((int32_t)(millis() - tftTickerNextStart) >= 0) {
      tftTickerStart();
    }
    return;
  }

  uint32_t nowUs = micros();
  uint32_t dtUs  = nowUs - tftTickerLastUs;
  if (dtUs < TFT_TICKER_FRAME_US) return;
  tftTickerLastUs = nowUs;

  uint16_t sp = clampU16((uint16_t)tft_ticker_speed, 50, 500);
  tftTickerAccPxUs += (uint32_t)((uint64_t)dtUs * sp);
  int16_t dx = (int16_t)(tftTickerAccPxUs / 1000000UL);
  if (dx < 1) return;
  tftTickerAccPxUs %= 1000000UL;

  tftTickerX -= dx;

  if (tftTickerSprReady) {
    tftTickerDrawFrame();
  }

  if (tftTickerX < -tftTickerW - 1) {
    tftTickerStop();
  }
}

static void tftClear() {
  tft.fillScreen(TFT_BLACK);
}

static void tftDrawDate() {
  time_t currentLocalTime = getCurrentLocalTime();
  struct tm *local = localtime(&currentLocalTime);
  if (!local) return;
  char buf[12];
  snprintf(buf, sizeof(buf), "%02d.%02d", local->tm_mday, local->tm_mon + 1);
  tft.setTextColor(tftColorFromId(tft_date_color), TFT_BLACK);
  tft.setTextFont(7);
  tft.setTextSize(2);
  int w = tft.textWidth(buf);
  int h = tft.fontHeight();
  int x = (tft.width() - w) / 2;
  int y = (tft.height() - h) / 2;
  tft.setCursor(x, y);
  tft.print(buf);
}

static void tftDrawClock(bool colonOn) {
  time_t currentLocalTime = getCurrentLocalTime();
  uint8_t nowHour = Time::instance().hour(currentLocalTime);
  uint8_t nowMinute = Time::instance().minute(currentLocalTime);
  char buf[6];
  snprintf(buf, sizeof(buf), "%02u%c%02u", nowHour, colonOn ? ':' : ' ', nowMinute);
  String s = String(buf);
  tft.setTextColor(tftColorFromId(tft_clock_color), TFT_BLACK);
  tft.setTextFont(7);
  tft.setTextSize(2);
  int w = tft.textWidth(s);
  int h = tft.fontHeight();
  int x = (tft.width()  - w) / 2;
  int y = (tft.height() - h) / 2 - 8;
  tft.setCursor(x, y);
  tft.print(s);
}

static void tftDrawDashes() {
  String s = "--:--";
  tft.setTextColor(tftColorFromId(tft_clock_color), TFT_BLACK);
  tft.setTextFont(7);
  tft.setTextSize(2);
  int w = tft.textWidth(s);
  int h = tft.fontHeight();
  int x = (tft.width()  - w) / 2;
  int y = (tft.height() - h) / 2;
  tft.setCursor(x, y);
  tft.print(s);
}

#if USE_WEATHER
static void tftDrawWeather() {
  float temp = Weather::instance().getTemperature();
  int t = (int)round(temp);
  if (t < -99) t = -99;
  if (t >  99) t =  99;

  String tempStr = String(t);
  const char* unitStr = "°C";
  const int gap = 8;

  tft.setTextFont(7);
  tft.setTextSize(2);
  tft.setTextColor(tftColorFromId(tft_weather_color), TFT_BLACK);

  int tempW = tft.textWidth(tempStr);
  int tempH = tft.fontHeight();

  tft.setTextFont(1);
  tft.setTextSize(6);
  int unitW = tft.textWidth(unitStr);

  int totalWidth = tempW + gap + unitW;

  int x = (tft.width() - totalWidth) / 2;
  if (x < 0) x = 2;

  int y = (tft.height() - tempH) / 2;

  tft.setTextFont(7);
  tft.setTextSize(2);
  tft.setCursor(x, y);
  tft.print(tempStr);

  tft.setTextFont(1);
  tft.setTextSize(6);
  tft.setCursor(x + tempW + gap, y);
  tft.print(unitStr);
}

static void tftDrawWeatherErr(bool blinkOn) {
  if (blinkOn) {
    tft.setTextFont(1);
    tft.setTextSize(6);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(115, 25);
    tft.print("Нет");
    tft.setCursor(65, 90);
    tft.print("данных");
  } else {
    tft.fillRect(0, 10, tft.width(), 150, TFT_BLACK);
  }
}
#endif // USE_WEATHER

static void tftDrawEffect(uint16_t effIndex) {
  tft.setTextFont(1);
  tft.setTextSize(3);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(115, 5);
  tft.print("ЭФФЕКТ");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(6);
  tft.setTextSize(2);
  String s = String((int)effIndex);
  int x = (tft.width() - tft.textWidth(s)) / 2;
  tft.setCursor(x, 55);
  tft.print(s);
}

static void tftDrawValue(uint8_t v) {
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(7);
  tft.setTextSize(2);
  int w = tft.textWidth(String(v));
  int h = tft.fontHeight();
  int x = (tft.width()  - w) / 2;
  int y = (tft.height() - h) / 2;
  tft.setCursor(x, y);
  tft.print(String(v));
}

static void tftDrawIP(const char* ip) {
  tft.setTextFont(1);
  tft.setTextSize(3);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(90, 15);
  tft.print("IP адрес");
  tft.setTextFont(6);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  String ipStr = String(ip);
  int ipX = (tft.width() - tft.textWidth(ipStr)) / 2;
  tft.setCursor(ipX, 85);
  tft.print(ipStr);
}

static bool tftJpgOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap) {
  if (!tftInited) return false;
  tft.pushImage(x, y, w, h, bitmap);
  return true;
}

static bool tftDrawJpgFromFS(const char* path) {
  if (!tftInited || !path || !path[0]) return false;
  File f = LittleFS.open(path, "r");
  if (!f) return false;
  size_t sz = f.size();
  if (!sz) {
    f.close();
    return false;
  }

  uint8_t* jpgBuf = (uint8_t*)malloc(sz);
  if (!jpgBuf) {
    f.close();
    return false;
  }

  size_t rd = f.read(jpgBuf, sz);
  f.close();
  if (rd != sz) {
    free(jpgBuf);
    return false;
  }

  tft.fillScreen(TFT_BLACK);
  JRESULT rc = TJpgDec.drawJpg(0, 0, jpgBuf, sz);
  free(jpgBuf);
  return (rc == JDR_OK);
}

void tftShowStartText() {
  if (tftDrawJpgFromFS("/start.jpg")) return;
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextSize(3);
  tft.setTextColor(TFT_GOLD);
  String s = "Fiery Led Lamp";
  int x = (tft.width()  - tft.textWidth(s)) / 2;
  int y = (tft.height() - tft.fontHeight()) / 2;
  tft.setCursor(x, y);
  tft.print(s);
}

// ----------------------------------------------------------------------------------------

void TFT_Init() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  TJpgDec.setJpgScale(1);
  TJpgDec.setCallback(tftJpgOutput);
  ledcSetup(TFT_BL_CH, TFT_BL_FREQ, TFT_BL_BITS);
  ledcAttachPin(TFT_BL, TFT_BL_CH);
  tftBacklightWrite(255);
  tftDispBrightness = 255;
  tftBreathDir = true;
  tftBlinkTmr = millis();
  lastView = TFT_VIEW_DASH;
  lastDraw = 0;
  lastMinuteTFT = -1;
  lastHourTFT = -1;
  lastTempTFT = 1000;
  lastEffTFT = 0xFFFF;
  lastArgTFT = 0xFF;
  lastDisplayMode = DISP_MODE_CLOCK;
  lastTimeSynched = false;
  lastDisplayFlag = 255;
  tftColonState = true;
  lastColonTFT = true;
  tftColonTmr = millis();
  tftInited = true;
  TFT_SetBrightness(tft_brightness);
  TFT_SetAutoBrightness(tft_auto_brightness);
}

void TFT_ShowIP(const char* ip) {
  if (!tftInited) return;
  if (!ip) ip = "";
  strncpy(tftIPBuf, ip, sizeof(tftIPBuf) - 1);
  tftIPBuf[sizeof(tftIPBuf) - 1] = 0;
  tftShowIP = true;
  tftIPShowTmr = millis();
  tftClear();
  tftDrawIP(tftIPBuf);
  lastView = TFT_VIEW_IP;
}

void TFT_HideIP() {
  if (!tftInited) return;
  tft.fillScreen(TFT_BLACK);
  tftShowIP = false;
  tftTickerNextStart = millis() + tftTickerPeriodMs();
  tftTickerActive = false;
  lastView = TFT_VIEW_DASH;
}

void TFT_Display_Timer(uint8_t argument) {
  if (!tftInited) return;
  static uint16_t tftLastMode = 0xFFFF;
  static uint32_t tftEffectShowTmr = 0;
  static bool tftShowEffect = false;
  static uint16_t tftEffIndex = 0;

  tftTickerTick();
  if (tftTickerActive) return;

  if (tftShowIP) {
    if (millis() - tftIPShowTmr >= 7000) {
      tftShowIP = false;
      TFT_HideIP();
    }
  }
  if (!tftShowEffect && tftLastMode != currentMode) {
    tftLastMode = currentMode;
    uint8_t n;
    for (n = 0; n < MODE_AMOUNT; n++) {
      if (eff_num_correct[n] == currentMode) break;
    }
    tftEffIndex = n;
    tftEffectShowTmr = millis();
    tftShowEffect = true;
  }
  if (tftShowEffect && (millis() - tftEffectShowTmr > 2000)) {
    tftShowEffect = false;
  }

  static uint32_t tftArgShowTmr = 0;
  static bool tftShowArg = false;
  static uint8_t prevArgLocal = 0xFF;

  if (DisplayFlag == 3) {
    const bool argChanged = (argument != prevArgLocal);

    if (argChanged) {
      tftArgShowTmr = millis();
      tftShowArg = true;
      prevArgLocal = argument;
      lastArgTFT = argument;
    }
  }

  if (tftShowArg && (millis() - tftArgShowTmr > 2000)) {
    tftShowArg = false;
  }

  if (millis() - tftColonTmr >= 500) {
    tftColonTmr = millis();
    tftColonState = !tftColonState;
  }

  // Переключение "Часы / Погода / Дата"
  if (!tftShowEffect && !tftShowArg) {
#if (USE_WEATHER == 0)
    displayMode = DISP_MODE_CLOCK;
#else
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
    } else {
      displayMode = DISP_MODE_CLOCK;
    }
#endif
  }

  TFT_View view = TFT_VIEW_DASH;
  bool needBlinkErr = false;

  if (tftShowIP) {
    view = TFT_VIEW_IP;
    tftShowEffect = false;
    tftShowArg = false;
  } else if (!timeSynched) {
    view = TFT_VIEW_DASH;
  } else if (tftShowArg) {
    view = TFT_VIEW_VALUE;
  } else if (tftShowEffect) {
    view = TFT_VIEW_EFFECT;
  } else if (tftTickerActive) {
    view = TFT_VIEW_TICKER;
  } else {
#if USE_WEATHER
    if (inClockWeatherMode) {
      switch (displayMode) {
        case DISP_MODE_CLOCK:
          view = TFT_VIEW_CLOCK;
          break;
        case DISP_MODE_WEATHER:
          if (Weather::instance().getTemperature() > -998.0f) {
            view = TFT_VIEW_WEATHER;
          } else {
            view = TFT_VIEW_WEATHER_ERR;
            if (!weatherErrActive) {
              weatherErrActive = true;
              weatherErrTimer = millis();
              weatherErrBlinkTimer = millis();
              weatherErrBlinkState = true;
            }
            if (millis() - weatherErrBlinkTimer >= WEATHER_ERR_BLINK) {
              weatherErrBlinkTimer = millis();
              weatherErrBlinkState = !weatherErrBlinkState;
            }
            needBlinkErr = weatherErrBlinkState;
          }
          break;
        case DISP_MODE_DATE:
          view = TFT_VIEW_DATE;
          break;
      }
    } else {
      view = TFT_VIEW_CLOCK;
    }
#else
    view = TFT_VIEW_CLOCK;
#endif
  }

  if (view == TFT_VIEW_TICKER) {
    lastView = view;
    return;
  }

  bool changed = false;
  if (view != lastView) changed = true;
  if (DisplayFlag != lastDisplayFlag) changed = true;
  if (lastDisplayMode != displayMode) changed = true;
  if (lastTimeSynched != timeSynched) changed = true;

  int drawHourTFT = lastHourTFT;
  int drawMinuteTFT = lastMinuteTFT;
  if (timeSynched) {
    time_t currentLocalTime = getCurrentLocalTime();
    drawHourTFT = Time::instance().hour(currentLocalTime);
    drawMinuteTFT = Time::instance().minute(currentLocalTime);
  }

  if (view == TFT_VIEW_CLOCK) {
    if (lastMinuteTFT != drawMinuteTFT || lastHourTFT != drawHourTFT) changed = true;
    if (lastColonTFT != tftColonState) changed = true;
  }
#if USE_WEATHER
  if (view == TFT_VIEW_WEATHER) {
    float temp = Weather::instance().getTemperature();
    int t = (int)round(temp);
    if (t != lastTempTFT) changed = true;
  }
  if (view == TFT_VIEW_EFFECT) {
    if (tftEffIndex != lastEffTFT) changed = true;
  }
  if (view == TFT_VIEW_VALUE) {
    if (argument != lastArgTFT) changed = true;
  }
  if (view == TFT_VIEW_WEATHER_ERR) {
    changed = true;
  }
#endif
  if (view == TFT_VIEW_IP) {
    changed = true;
  }

  if (!changed && (millis() - lastDraw) < 250) return;
  lastDraw = millis();

  if (view != lastView) {
    tftClear();
  }

  switch (view) {
    case TFT_VIEW_CLOCK:        tftDrawClock(tftColonState); break;
#if USE_WEATHER
    case TFT_VIEW_WEATHER:      tftDrawWeather(); break;
    case TFT_VIEW_WEATHER_ERR:  tftDrawWeatherErr(needBlinkErr); break;
#endif
    case TFT_VIEW_DATE:         tftDrawDate(); break;
    case TFT_VIEW_EFFECT:       tftDrawEffect(tftEffIndex); break;
    case TFT_VIEW_VALUE:        tftDrawValue(argument); break;
    case TFT_VIEW_IP:           tftDrawIP(tftIPBuf); break;
    default:                    tftDrawDashes(); break;
  }

  lastView = view;
  lastMinuteTFT = last_minute;
  lastHourTFT = hours;
#if USE_WEATHER
  lastTempTFT = (int)round(Weather::instance().getTemperature());
#endif
  lastEffTFT = tftEffIndex;
  lastArgTFT = argument;
  lastDisplayMode = displayMode;
  lastTimeSynched = timeSynched;
  lastDisplayFlag = DisplayFlag;
  lastColonTFT = tftColonState;
}

static inline uint8_t tftGetDayNightTarget() {
  if (!timeSynched) return 255U;
  const uint8_t dayB  = TFT_DAY_BRIGHTNESS;
  const uint8_t nightB = TFT_NIGHT_BRIGHTNESS;

  if (NIGHT_HOURS_START >= NIGHT_HOURS_STOP) {
    if (thisTime >= NIGHT_HOURS_START || thisTime <= NIGHT_HOURS_STOP) return nightB;
    return dayB;
  } else {
    if (thisTime >= NIGHT_HOURS_START && thisTime <= NIGHT_HOURS_STOP) return nightB;
    return dayB;
  }
}

void TFT_ApplyBrightnessNow() {
  if (!tftInited) return;
  tftBlinkTmr = 0;
  tftBrightnessTick();
}

static void tftBrightnessTick() {
  if (!tftInited) return;
  if (!tft_auto_brightness) return;

  const uint8_t target = tftGetDayNightTarget();
#if USE_DAWN || USE_SUNSET
  const bool activeBreath = (dawnFlag == 1 || sunsetFlag == 1);
#else
  const bool activeBreath = false;
#endif

  if (!activeBreath) {
    if (tftDispBrightness != target) {
      tftDispBrightness = target;
      tftBacklightWrite(tftDispBrightness);
    }
    tftBreathDir = true;
    tftBlinkTmr = millis();
    return;
  }

  if (target == 0U) {
    if (tftDispBrightness != 0U) {
      tftDispBrightness = 0U;
      tftBacklightWrite(0U);
    }
    tftBreathDir = true;
    tftBlinkTmr = millis();
    return;
  }

  const uint8_t minB = (target <= 5U) ? 1U : 5U;
  const uint8_t maxB = target;

  if (millis() - tftBlinkTmr < 5) return;
  tftBlinkTmr = millis();

  if (tftBreathDir) {
    if (tftDispBrightness < maxB) tftDispBrightness++;
    if (tftDispBrightness >= maxB) tftBreathDir = false;
  } else {
    if (tftDispBrightness > minB) tftDispBrightness--;
    if (tftDispBrightness <= minB) tftBreathDir = true;
  }

  tftBacklightWrite(tftDispBrightness);
}

void TFT_LoopTick() {
  if (!tftInited) return;
  tftBrightnessTick();
  tftTickerTick();

  static uint32_t tmr = 0;
  if (millis() - tmr > 200) {
    tmr = millis();
    TFT_Display_Timer(lastArgTFT);
  }
}

void TFT_SetBrightness(uint8_t brightness) {
  if (!tftInited) return;
  brightness = constrain(brightness, 0, 255);
  tftBacklightWrite(brightness);
  tftDispBrightness = brightness;
}

void TFT_SetAutoBrightness(bool enable) {
  tft_auto_brightness = enable;
  if (enable) {
    tftDispBrightness = 255;
    tftBrightnessTick();
  }
}

#endif // USE_ST7789

// ******************************************************************************************************************************************************

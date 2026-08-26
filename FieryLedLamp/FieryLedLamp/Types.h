// ***************************************************************************** Types.h ****************************************************************
#pragma once
// --------------
#include <Arduino.h>
#include <functional>
#include <time.h>
#include <WiFi.h>
#include <FastLED.h>
// --------------------

// -------------------------------
enum LedChipType {
  CHIP_WS2812B = 0,
  CHIP_APA102  = 1,
  CHIP_COUNT
};

// -------------------------------
enum ColorOrderType {
  ORDER_GRB = 0,
  ORDER_RGB = 1,
  ORDER_BRG = 2,
  ORDER_RBG = 3,
  ORDER_GBR = 4,
  ORDER_BGR = 5,
  ORDER_COUNT
};

// -------------------------------
#if USE_TM1637 || USE_ST7789
enum DisplayMode : uint8_t {
  DISP_MODE_CLOCK = 0,
  DISP_MODE_WEATHER = 1,
  DISP_MODE_DATE = 2
};
#endif

// -------------------------------
#if USE_ST7789
enum TFT_View : uint8_t {
  TFT_VIEW_CLOCK = 0,
#if USE_WEATHER
  TFT_VIEW_WEATHER = 1,
#endif
  TFT_VIEW_EFFECT = 2,
  TFT_VIEW_VALUE = 3,
#if USE_WEATHER
  TFT_VIEW_WEATHER_ERR = 4,
#endif
  TFT_VIEW_IP = 5,
  TFT_VIEW_DASH = 6,
  TFT_VIEW_TICKER = 7,
  TFT_VIEW_DATE = 8
};
#endif

// -------------------------------
#if USE_BUTTON
enum ButtonActions {
  BUTTON_ACTION_POWER = 1,
  BUTTON_ACTION_NEXT = 2,
  BUTTON_ACTION_PREV = 3,
  BUTTON_ACTION_ACTION4 = 4,
  BUTTON_ACTION_IP = 5,
  BUTTON_ACTION_TIME = 6,
  BUTTON_ACTION_SOUND = 8,
  BUTTON_ACTION_WEATHER = 9
};
#endif

// -------------------------------
#if USE_OTA
enum OtaPhase {
  None = 0,
  InProgress,
  Done
};
#endif

// -------------------------------
#if DEBUG_ENABLED
enum LogLevel : uint8_t {
  LOG_LEVEL_VERBOSE = 0,
  LOG_LEVEL_DEBUG,
  LOG_LEVEL_INFO,
  LOG_LEVEL_WARN,
  LOG_LEVEL_ERROR,
  LOG_LEVEL_NONE
};
#endif

// -------------------------------
struct MatrixBuffers {
  CRGB* leds = nullptr;
  CRGB* ledsbuff = nullptr;
  CRGB* effectBuffer = nullptr;
  uint8_t* line = nullptr;
  uint8_t* shiftHue = nullptr;
  uint8_t* shiftValue = nullptr;
  uint8_t** matrixValue = nullptr;
  uint8_t** noise = nullptr;
  uint16_t noiseDim = 0;
  uint8_t*** noise3d = nullptr;
};

// -------------------------------
struct ModeType {
  uint8_t Brightness = 50U;
  uint8_t Speed = 225U;
  uint8_t Scale = 40U;
};

// -----------------------------------------------------
typedef void (*SendCurrentDelegate)(char *outputBuffer);
typedef void (*ShowWarningDelegate)(CRGB color, uint32_t duration, uint16_t blinkHalfPeriod);

// ----------------------------------------------------------------------------------------
#if USE_DAWN
struct AlarmType {
  bool State = false;
  uint16_t Time = 0U;
};
#endif

// -------------------------------
#if USE_SUNSET
struct SunsetType {
  bool State = false;
  uint16_t Time = 0U;
};
#endif

// -------------------------------
#if USE_SCHEDULE
struct scheduleType {
  uint8_t State;
  uint8_t Day;
  uint16_t Time;
  uint8_t Action;
  uint8_t EffectNum;
};
#endif

// -------------------------------
struct FontDesc {
  uint8_t width;
  uint8_t height;
  uint8_t arrayHeight;
  bool is16bit;
  const void* data;
};

// -------------------------------
#if USE_MP3_PLAYER
struct WeatherData {
  int8_t temp;
  uint8_t humidity;
  uint8_t windSpeed;
  uint8_t windDir;
  uint8_t condition;
};
#endif

// -------------------------------
#if BACKUP_CFG_FILES
struct WifiBackupData {
  uint32_t magic;
  uint8_t version;
  uint8_t pending;
  uint8_t reserved;
  uint16_t timeoutSec;
  char ssid1[33];
  char password1[65];
  uint32_t crc32;
};

struct FlashBackupHeader {
  uint32_t magic;
  uint16_t version;
  uint16_t reserved;
  uint32_t size;
  uint32_t crc32;
};

struct BackupConfigFileInfo {
  const char* fsPath;
  const char* zipName;
  bool required;
};
#endif // BACKUP_CFG_FILES

// -------------------------------
struct UpdateCache {
    String response;
    unsigned long timestamp;
    bool valid;
};

// ******************************************************************************************************************************************************

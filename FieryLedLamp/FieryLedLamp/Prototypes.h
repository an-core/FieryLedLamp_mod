// ***************************************************************************** Prototypes.h ***********************************************************
#pragma once
// --------------

#include <Arduino.h>
#include <FastLED.h>
#include "Constants.h"
#include "Extern.h"
// ---------------------

#if USE_DAWN
void handle_alarm();
void save_alarms();
#endif
// ------------------------------------------------------
#if USE_SUNSET
void handle_sunset();
void save_sunsets();
#endif
// ------------------------------------------------------
#if USE_SCHEDULE
void load_schedule();
#endif
// ------------------------------------------------------
#if USE_MP3_PLAYER
void play_by_name(const char* fullPath);
void sendVolume(char *outputBuffer);
void play_weather(bool manual = false);
void play_condition();
void play_file(uint16_t trackNumber);
uint16_t weatherAdvertDescTrackByText(String s);
uint8_t getWeatherCodeFromCondition(String condition);
int16_t send_command(int8_t cmd, uint8_t feedback, uint8_t dat1, uint8_t dat2);
int16_t read_command (uint32_t mp3_read_timeout);
void mp3_setup();
void play_time_ADVERT(bool force = false);
void play_sound();
void mp3_loop();
void start_weather_temp_ADVERT(int8_t temp, bool speakDescription);
void mp3_restore_after_announce(bool restoreEffect = true);
#endif
// ------------------------------------------------------
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
void setModeSettings(uint8_t Scale = 0U, uint8_t Speed = 0U);
#endif
// ------------------------------------------------------
#if USE_BLYNK
void updatePlayerBlynkParams(bool isRunning);
#endif
// ------------------------------------------------------
#if USE_MULTILAMP
void multiple_lamp_control();
#endif
// ------------------------------------------------------
#if USE_IR_RECEIVER
void IR_Receive_Handle();
void IR_Receive_Button_Handle();
#endif
// ------------------------------------------------------
#if USE_RF_RECEIVER
void RF_Receive_Handle();
void RF_Receive_Button_Handle();
#endif
// ------------------------------------------------------
#if USE_ST7789
void TFT_Init();
void TFT_PowerOff();
void TFT_ShowIP(const char* ip);
void TFT_HideIP();
void TFT_Display_Timer(uint8_t argument);
void TFT_ApplyBrightnessNow();
void TFT_SetAutoBrightness(bool enable);
void TFT_LoopTick();
void TFT_SetBrightness(uint8_t brightness);
void tftShowStartText();
void handle_tft_clock_color();
void handle_tft_weather_color();
void handle_tft_ticker_on();
void handle_tft_ticker_color();
void handle_tft_ticker_speed();
void handle_tft_ticker_period();
void handle_tft_ticker_text();
void handle_tft_brightness();
void handle_tft_status();
#endif
// ------------------------------------------------------
#if BACKUP_CFG_FILES
String getConfigBackupMessage();
bool isConfigBackupPending();
void clearConfigBackupPending();
bool saveConfigBackupToPartition(bool setPendingFlag);
bool ConfigBackupFromPartition();
void BackupInit();
void BackupsIfNeeded();
#endif
// ------------------------------------------------------
#if DEBUG_ENABLED
void loadSystemLogSettings();
#else
#define loadSystemLogSettings() ((void)0)
#endif
// ------------------------------------------------------
time_t getCurrentLocalTime();
String zeroPad(String str, uint8_t len);
uint32_t getRunningTextDelayMs();
inline bool isLampActive();
void resetTimerState();
uint32_t get_Chip_ID(void);
uint32_t getPixColor(uint32_t thisSegm);
uint32_t getPixColorXY(uint8_t x, uint8_t y);
float sqrt3(const float x);
uint16_t XY(uint8_t x, uint8_t y);
boolean fillString(const char* text, CRGB letterColor, boolean itsText);
uint8_t getFont(uint8_t high, uint8_t low, uint8_t row);
void fillAll(CRGB color);
void drawPixelXY(int8_t x, int8_t y, CRGB color);
void freeNoise3D();
void handle_run_text_over_effects();
void setCurrentFont(uint8_t size);
void restoreSettings();
void handle_cycle_set();
void handle_sound_set();
void multilamp_get();
void User_settings();
void SetBrightness(uint8_t brightness);
void getFormattedTime(char* buffer);
void Lang_set();
void Display_Timer(uint8_t argument = 0);
void timeTick();
void Save_File_Changes();
void showWarning(CRGB color, uint32_t duration, uint16_t blinkHalfPeriod);
void clockTicker_blink();
void IR_Power();
void Mute();
void Prev_Next_eff(bool next);
void Cycle_on_off();
void Bright_Up_Down(bool up);
void Speed_Up_Down(bool up);
void Scale_Up_Down(bool up);
void Volum_Up_Down(bool up);
void Print_IP();
void Folder_Next_Prev(bool next);
void Current_Eff_Rnd_Def(bool rnd);
void IR_Equalizer();
void Favorit_Add_Del(bool add);
void Digit_Handle(uint8_t digit);
void printTime(bool onDemand = false);
void printWeather();
void drawLetter(uint8_t high, uint8_t low, int16_t xPos, CRGB letterColor, CRGB bgColor);
void drawSym3x5(uint8_t x, uint8_t y, uint8_t symIdx, CRGB color);
void sendCurrent(char *outputBuffer);
void NEWsendCurrent(char *outputBuffer);
void sendAlarms(char *outputBuffer);
void sendSunsets(char *outputBuffer);
void sendTimer(char *outputBuffer);
bool FileCopy(const String& SourceFile, const String& TargetFile);
// ------------------------------------------------------
// effectsSupport
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
void setModeSettings(uint8_t Scale, uint8_t Speed);
#endif
uint16_t XY(uint8_t x, uint8_t y);
void blurScreen(fract8 blur_amount, CRGB *LEDarray = leds);
void dimAll(uint8_t value, CRGB *LEDarray = leds);
void setNoise(uint16_t x, uint16_t y, uint8_t val);
uint8_t safeGetNoise(uint8_t layer, uint8_t x, uint8_t y);
void safeSetNoise(uint8_t layer, uint8_t x, uint8_t y, uint8_t val);
uint8_t getNoise(uint16_t x, uint16_t y);
uint8_t mapsin8(uint8_t theta, uint8_t lowest = 0, uint8_t highest = 255);
uint8_t mapcos8(uint8_t theta, uint8_t lowest = 0, uint8_t highest = 255);
uint8_t myScale8(uint8_t x);
void DrawLine(int x1, int y1, int x2, int y2, CRGB color);
void DrawLineF(float x1, float y1, float x2, float y2, CRGB color);
CRGB makeDarker(const CRGB& color, fract8 howMuchDarker);
void drawPixelXYF(float x, float y, CRGB color);
void drawCircleF(float x0, float y0, float radius, CRGB color);
void setCurrentPalette();
void fillnoise8();
void fillNoiseLED();
uint32_t colorDimm(uint32_t colorValue, long lenght, long pixel);
void drawScaledImage(uint16_t imgW, uint16_t imgH, uint16_t offsetX);
uint16_t getSizeValue(byte* buffer, byte b);
void readBinFile(String fileName, size_t len);
void gradientDownTop(uint8_t bottom, CHSV bottom_color, uint8_t top, CHSV top_color);
void gradientVertical(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, uint8_t start_color, uint8_t end_color, uint8_t start_br, uint8_t end_br, uint8_t saturate);
void gradientHorizontal(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, uint8_t start_color, uint8_t end_color, uint8_t start_br, uint8_t end_br, uint8_t saturate);
uint8_t validMinMax(float val, uint8_t minV, uint32_t maxV);
void drawRecCHSV(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, CHSV color);
void drawRec(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, uint32_t color);
void espModeState(uint8_t color);

// ******************************************************************************************************************************************************

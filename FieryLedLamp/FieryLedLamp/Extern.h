// ***************************************************************************** Extern.h ***************************************************************
#pragma once
// --------------
#include "Constants.h"
#include "Types.h"
#include "Time.h"
#include "IRManager.h"
// ---------------------

extern bool configChanged;
extern volatile bool systemShuttingDown;
extern volatile bool isPrintingMessage;
extern bool justPoweredOn;
extern uint16_t prevMaxNoiseDim;
extern uint8_t ihue;
extern uint8_t colorLoop;
extern uint32_t noise32_x[2];
extern uint32_t noise32_y[2];
extern uint32_t noise32_z[2];
extern uint32_t scale32_x[2];
extern uint32_t scale32_y[2];
extern uint8_t noisesmooth;
extern int Painting;
extern CRGB DriwingColor;
extern CRGBPalette16 currentPalette;
extern uint8_t*** noise3d;
extern const char* ntp1;
extern const char* ntp2;
extern String LAMP_NAME;
extern bool loadingFlag;
extern uint8_t currentMode;
extern uint16_t usedLeds;
extern uint16_t matrixWidth;
extern uint16_t matrixHeight;
extern String configSetup;
extern String configWiFi;
extern String configLED;
extern String configLedPanel;
extern String configLedInterval;
extern String configCycle;
extern uint8_t T_flag;
extern uint8_t TURN_ON_AFTER_SHUTDOWN;
extern File fsUploadFile;
extern CRGB* leds;
extern uint8_t hue, hue2;
extern uint8_t deltaHue, deltaHue2;
extern uint8_t step;
extern uint8_t pcnt;
extern uint8_t deltaValue;
extern float speedfactor;
extern float emitterX, emitterY;
extern uint8_t centerX, centerY;
extern byte binImage[8192];
extern uint16_t ff_x, ff_y, ff_z;
extern uint16_t speed, scale;
extern uint8_t custom_eff;
extern float trackingObjectPosX[];
extern float trackingObjectPosY[];
extern float trackingObjectSpeedX[];
extern float trackingObjectSpeedY[];
extern float trackingObjectShift[];
extern uint8_t trackingObjectHue[];
extern uint8_t trackingObjectState[];
extern bool trackingObjectIsShift[];
extern uint8_t enlargedObjectNUM;
extern long enlargedObjectTime[];
extern float liquidLampHot[];
extern float liquidLampSpf[];
extern unsigned liquidLampMX[];
extern unsigned liquidLampSC[];
extern unsigned liquidLampTR[];
extern char TextTicker[86];
extern uint8_t currentFont;
extern uint8_t LET_WIDTH;
extern uint8_t LET_HEIGHT;
extern const uint8_t font5x8[][5];
extern const uint16_t font8x13[][8];
extern const uint16_t font10x16[][10];
extern const FontDesc fontTable[3];
extern uint8_t effects_folders[MODE_AMOUNT];
extern uint8_t eff_num_correct[MODE_AMOUNT];
extern ModeType modes[MODE_AMOUNT];
extern bool day_night;
extern volatile bool timeSyncJustOccurred;
extern uint8_t first_entry;
extern boolean fillString(const char* text, CRGB letterColor, boolean itsText);
extern bool scrollTimerForcedReset;
extern bool forceScrollTimerReset;
extern bool showClock;
extern bool nightClockEnabled;
extern uint8_t nightClockBrightness;
extern uint8_t nightClockHue;
extern String autoDetectedTz;
extern bool ONflag;
extern uint8_t userClockBrightness;
extern uint8_t nightModeBrightness;
extern const uint8_t clockFont3x5[10][3];
extern const uint8_t weatherSym3x5[4][3];
extern uint8_t staticFont;
extern void clearLastDateZone();
extern bool buttonEnabled;
extern bool irEnabled;
extern bool rfEnabled;
extern bool tm1637Enabled;
extern bool st7789Enabled;
extern bool mp3Enabled;
// ------------------------------------------------------------------------------
#if LED_PANEL
extern unsigned long lastClockFixedSwitch;
extern bool timer_clock_fixed;
extern uint16_t interval_clock_fixed;
#endif
// ------------------------------------------------------------------------------
#if MULTI_MATRIX
extern bool panelFlip;
extern uint8_t segMatrix_w, segMatrix_h;
extern uint16_t segWidth, segHeight;
extern uint16_t totalWidth, totalHeight;
#endif
// ------------------------------------------------------------------------------
#if USE_BUTTON
extern String configButton;
#endif
// ------------------------------------------------------------------------------
#if USE_MP3_PLAYER
extern uint8_t mp3_init_step;
extern String configMP3;
extern String soundList;
extern bool isAnnouncing;
extern uint8_t mp3_folder;
extern uint8_t mp3_folder_last;
extern uint8_t CurrentFolder;
extern uint8_t CurrentFolder_last;
extern uint8_t mp3_player_connect;
extern uint8_t eff_volume;
extern uint8_t eff_sound_on;
extern uint8_t alarm_volume;
extern uint8_t sunset_volume;
extern uint8_t day_advert_volume;
extern uint8_t night_advert_volume;
extern uint8_t AlarmFolder;
extern uint8_t SunsetFolder;
extern uint8_t Equalizer;
extern uint32_t ADVERT_TIMER_1;
extern uint32_t ADVERT_TIMER_2;
extern uint16_t ADVERT_TIMER_H;
extern uint16_t ADVERT_TIMER_M;
extern uint16_t mp3_delay;
extern bool alarm_sound_on;
extern bool sunset_sound_on;
extern bool day_advert_sound_on;
extern bool night_advert_sound_on;
extern bool alarm_advert_sound_on;
extern bool dawnflag_sound;
extern bool sunsetflag_sound;
extern bool alarm_sound_flag;
extern bool sunset_sound_flag;
extern bool set_mp3_play_now;
extern bool mp3_stop;
extern bool pause_on;
extern uint32_t mp3_timer;
extern uint32_t alarm_timer;
extern uint32_t sunset_timer;
extern uint8_t mp3_receive_buf[10];
extern HardwareSerial mp3;
extern bool timeAnnounceEnabled;
extern uint8_t previous_folder;
extern bool mp3_card_ok;
extern bool dfPlayerIsOriginal;
extern uint16_t NIGHT_HOURS_START;
extern uint16_t NIGHT_HOURS_STOP;
extern bool send_sound;
extern bool send_eff_volume;
extern uint8_t weather_day_volume;
extern uint8_t weather_night_volume;
extern bool advert_flag;
extern bool advert_hour;
extern bool show_weather_desc;
extern uint32_t mp3_check_timer;
extern int day_weather_temp_on;
extern int day_weather_desc_on;
extern int night_weather_temp_on;
extern int night_weather_desc_on;
extern bool time_always;
extern bool weather_always;
#endif // USE_MP3_PLAYER
// ------------------------------------------------------------------------------
#if USE_ST7789
extern String configST7789;
extern uint8_t tft_clock_color;
extern uint8_t tft_weather_color;
extern bool tft_ticker_on;
extern uint8_t tft_ticker_color;
extern uint16_t tft_ticker_speed;
extern uint16_t tft_ticker_period;
extern char TFTTickerText[128];
extern uint8_t tft_date_color;
extern DisplayMode displayMode;
extern uint32_t displaySwitchTimer;
extern uint8_t DisplayFlag;
extern uint8_t last_minute;
extern uint8_t hours;
extern uint32_t thisTime;
extern bool tft_auto_brightness;
extern uint8_t TFT_DAY_BRIGHTNESS;
extern uint8_t TFT_NIGHT_BRIGHTNESS;
#endif
// ------------------------------------------------------------------------------
#if USE_MULTILAMP
extern String configMultilamp;
extern bool repeat_multiple_lamp_control;
#endif
// ------------------------------------------------------------------------------
#if USE_WEATHER
extern bool weatherErrActive;
extern uint32_t weatherErrTimer;
extern bool weatherSpeakEnabled;
extern String configWeather;
extern uint8_t PRINT_WEATHER;
extern bool inClockWeatherMode;
extern bool runWeatherTextEnabled;
extern bool weatherEnabled;
extern bool degreeSymbolBlinking;
extern bool rainbowWeather;
extern bool autoWeatherHue;
extern uint8_t weatherHue;
extern int weatherXOffset;
extern int weatherYOffset;
extern int8_t weatherAutoOffsetY;
extern bool forceWeatherEnabled;
extern bool weatherColorCycle;
extern uint8_t globalDegBrightness;
extern bool weatherNeedRedraw;
extern int weatherPoleX, weatherPoleY;
extern int16_t lastWeatherLeft;
extern int16_t lastWeatherRight;
extern int16_t lastWeatherTop;
extern int16_t lastWeatherBottom;
extern bool weatherTextJustEnabled;
#endif
// ------------------------------------------------------------------------------
#if USE_MQTT
extern uint32_t mqtt_timer;
extern String configMQTT;
extern bool MqttOn;
extern uint16_t MqttPeriod;
extern IPAddress MqttServer;
extern char MqttHost[64];
extern char MqttUser[32];
extern char MqttPassword[32];
extern char TopicBase[64];
extern bool mqttIPaddr;
#endif
// ------------------------------------------------------------------------------
#if USE_IR_RECEIVER
extern uint32_t IR_Code;
extern uint8_t IR_Data_Ready;
extern unsigned long IR_Repeat_Timer;
extern unsigned long lastIRtime;
extern uint32_t IR_Tick_Timer;
extern uint32_t IR_Dgit_Enter_Timer;
extern uint8_t Enter_Digit_1;
extern uint8_t Enter_Number;
extern uint32_t lastIRCode;
extern IRManager irManager;
#endif
// ------------------------------------------------------------------------------
#if USE_DAWN
extern CRGB dawnColor[6];
extern uint8_t dawnCounter;
extern uint16_t dawnPosition;
extern String configAlarm;
extern uint8_t dawnFlag;
extern AlarmType alarms[7];
extern CRGB dawnColor[6];
extern uint8_t dawnCounter;
extern const uint8_t dawnOffsets[];
#endif
// ------------------------------------------------------------------------------
#if USE_SUNSET
extern CRGB sunsetColor[6];
extern uint8_t sunsetCounter;
extern uint16_t sunsetPosition;
extern String configSunset;
extern uint8_t sunsetFlag;
extern SunsetType sunsets[7];
extern CRGB sunsetColor[6];
extern uint8_t sunsetCounter;
extern const uint8_t sunsetOffsets[];
#endif
// -------------------------------------------------------------------------------
#if USE_SCHEDULE
extern String configSchedule;
extern scheduleType schedule[MAX_SCHEDULE_ENTRIES];
extern bool allowScheduleAtStartup;
extern uint32_t scheduleStartupDelay;
#endif
// ------------------------------------------------------------------------------

// ******************************************************************************************************************************************************

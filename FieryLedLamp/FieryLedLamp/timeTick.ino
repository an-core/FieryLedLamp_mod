// ******************************************************************************* timeTick.ino *********************************************************
#include <time.h>
#include "Prototypes.h"
#include "Extern.h"
//--------------------------

void timeTick() {
  static uint32_t secsTimer = 0UL;
  if (millis() - secsTimer < 1000UL) return;
  secsTimer = millis();

  if (!myTime.isTimeSet()) {
#if USE_TM1637
    if (!DisplayFlag) display.displayByte(_dash, _dash, _dash, _dash);
#endif
    return;
  }

  // для ST7789
  if (!timeSynched) {
    timeSynched = true;
  }

  time_t t = getCurrentLocalTime();
  struct tm *ti = localtime(&t);
  uint8_t currentHour = ti->tm_hour;
  uint8_t currentMinute = ti->tm_min;
  uint8_t currentSecond = ti->tm_sec;
  uint32_t currentSeconds = currentHour * 3600UL + currentMinute * 60UL + currentSecond;
  uint16_t currentMinutes = currentHour * 60 + currentMinute;
  uint8_t thisDay = (ti->tm_wday == 0) ? 6 : ti->tm_wday - 1;
  thisTime = currentMinutes;

#if USE_SCHEDULE
  static uint16_t lastExecutedMinute[MAX_SCHEDULE_ENTRIES] = {0};
  static uint16_t lastYearDay = 0xFFFF;
  static time_t lastEpoch = 0;

  uint16_t currentYearDay = ti->tm_yday;
  time_t nowEpoch = t;

  bool needReset = false;

  if (currentYearDay != lastYearDay) {
    needReset = true;
    lastYearDay = currentYearDay;
  }

  if (lastEpoch != 0 && nowEpoch > lastEpoch + 120) {
    needReset = true;
  }

  if (needReset) {
    for (uint8_t i = 0; i < MAX_SCHEDULE_ENTRIES; i++) {
      lastExecutedMinute[i] = 0xFFFF;
    }
  }

  lastEpoch = nowEpoch;
#endif

// ============================================================================= РАССВЕТ ===============================================================
#if USE_DAWN
const uint8_t dawnOffsets[] PROGMEM = {5, 10, 15, 20, 25, 30, 40, 50, 60}; // Длительность рассвета
uint16_t dawnStartMin = alarms[thisDay].Time - pgm_read_byte(&dawnOffsets[dawnMode]);
bool dawnActive = alarms[thisDay].State && currentMinutes >= dawnStartMin && currentMinutes < alarms[thisDay].Time + DAWN_TIMEOUT;
if (dawnActive && !manualOff) {
  uint32_t dawnStartSec = dawnStartMin * 60UL;
  dawnPosition = map(currentSeconds - dawnStartSec, 0, pgm_read_byte(&dawnOffsets[dawnMode]) * 60UL, 0, 255);
  dawnPosition = constrain(dawnPosition, 0, 255);
  for (uint8_t j = 5; j > 0; j--) {
    if (dawnCounter >= j) dawnColor[j] = dawnColor[j - 1];
  }
  dawnColor[0] = CHSV(map(dawnPosition, 0, 255, 10, 35), map(dawnPosition, 0, 255, 255, 170), map(dawnPosition, 0, 255, 2, DAWN_BRIGHT));
  if (dawnCounter < 5) dawnCounter++;
  dawnFlag = 1;
  ONflag = false;
  for (uint16_t i = 0; i < usedLeds; i++) {
    leds[i] = dawnColor[i % 6];
  }
  FastLED.setBrightness(255);
  FastLED.show();
  yield();
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
  digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
#endif
#if defined(ALARM_PIN) && defined(ALARM_LEVEL)
  if (currentMinutes == alarms[thisDay].Time) {
    digitalWrite(ALARM_PIN, manualOff ? !ALARM_LEVEL : ALARM_LEVEL);
  }
#endif
}
else if (dawnFlag == 1) {
  dawnFlag = 0;
  manualOff = false;
  dawnCounter = 0;
  dawnPosition = 0;
  for (uint8_t j = 0; j < 6; j++) dawnColor[j] = CRGB(0, 0, 0);
  FastLED.clear();
  FastLED.setBrightness(0);
  FastLED.show();
  yield();
#if defined(ALARM_PIN) && defined(ALARM_LEVEL)
  digitalWrite(ALARM_PIN, !ALARM_LEVEL);
#endif
}
#endif // USE_DAWN

// ============================================================================= ЗАКАТ =================================================================
#if USE_SUNSET
const uint8_t sunsetOffsets[] PROGMEM = {5, 10, 15, 20, 25, 30, 40, 50, 60}; // Длительность заката
uint16_t sunsetTime = sunsets[thisDay].Time;
uint16_t sunsetDuration = pgm_read_byte(&sunsetOffsets[sunsetMode]);
uint16_t sunsetEndMin = (sunsetTime + sunsetDuration) % 1440;
bool sunsetActive = sunsets[thisDay].State && !manualOff;
bool inSunsetWindow;
if (sunsetEndMin > sunsetTime) {
  inSunsetWindow = currentMinutes >= sunsetTime && currentMinutes < sunsetEndMin;
}
else {
  inSunsetWindow = currentMinutes >= sunsetTime || currentMinutes < sunsetEndMin;
}
if (sunsetActive && inSunsetWindow) {
  uint32_t sunsetStartSec = sunsetTime * 60UL;
  uint32_t elapsedSec;
  if (currentMinutes >= sunsetTime) {
    elapsedSec = currentSeconds - sunsetStartSec;
  } else {
    elapsedSec = currentSeconds + (1440UL * 60UL - sunsetStartSec);
  }
  uint32_t durationSec = sunsetDuration * 60UL;
  sunsetPosition = map(elapsedSec, 0, durationSec, 255, 0);
  sunsetPosition = constrain(sunsetPosition, 0, 255);
  for (uint8_t j = 5; j > 0; j--) {
    if (sunsetCounter >= j) sunsetColor[j] = sunsetColor[j - 1];
  }
  sunsetColor[0] = CHSV(map(sunsetPosition, 255, 0, 30, 0), map(sunsetPosition, 255, 0, 180, 255), map(sunsetPosition, 255, 0, SUNSET_BRIGHT, 5));
  if (sunsetCounter < 5) sunsetCounter++;
  sunsetFlag = 1;
  for (uint16_t i = 0; i < usedLeds; i++) {
    leds[i] = sunsetColor[i % 6];
  }
  FastLED.setBrightness(255);
  FastLED.show();
  yield();
#if USE_MP3_PLAYER
  if (mp3Enabled && sunset_sound_on && !sunset_sound_flag) {
    sunset_sound_flag = true;
    CurrentFolder = SunsetFolder;
    send_command(0x17, FEEDBACK, 0, CurrentFolder);
  }
#endif
}
else if (sunsetFlag == 1) {
  sunsetFlag = 2;
  sunsetCounter = 0;
  for (uint8_t j = 0; j < 6; j++) sunsetColor[j] = CRGB(0, 0, 0);
  changePower();

  loadingFlag = true;

#if USE_MP3_PLAYER
  sunset_sound_flag = false;
#endif

#if defined(ALARM_PIN) && defined(ALARM_LEVEL)
  digitalWrite(ALARM_PIN, !ALARM_LEVEL);
#endif
}
#endif // USE_SUNSET

// ========================================================================= РАСПИСАНИЕ ЛАМПЫ ==========================================================
#if USE_SCHEDULE
static uint32_t lastScheduleCheck = 0;
if (millis() - lastScheduleCheck >= 1000UL) {
  lastScheduleCheck = millis();

  uint16_t currentMinutes = myTime.hour() * 60 + myTime.minute();

  if (manualOverride && millis() < manualOverrideUntil) {
    goto sched_end;
  }

  if (isPrintingMessage) {
    goto sched_end;
  }

  for (uint8_t i = 0; i < MAX_SCHEDULE_ENTRIES; i++) {
    if (schedule[i].State == 0) continue;

    int16_t diff = currentMinutes - schedule[i].Time;
    if (diff < 0) continue;
    if (lastExecutedMinute[i] == schedule[i].Time) continue;

    switch (schedule[i].Action) {
      case 0:
        break;

      case 1:
        if (!ONflag) {
          ONflag = true;
          jsonWrite(configSetup, "Power", 1);
          changePower();
          sendCurrent(udpBuffer);
#if USE_MQTT
          Mqtt::instance().needToPublish = true;
#endif
          timeout_save_file_changes = millis();
          bitSet(save_file_changes, 0);
        } break;

      case 2:
        if (ONflag) {
          ONflag = false;
          jsonWrite(configSetup, "Power", 0);
          changePower();
          sendCurrent(udpBuffer);
#if USE_MQTT
          Mqtt::instance().needToPublish = true;
#endif
          timeout_save_file_changes = millis();
          bitSet(save_file_changes, 0);
        } break;

      case 3: // включить эффект
        {
          if (Favorites::instance().FavoritesRunning == 1) {
            Favorites::instance().FavoritesRunning = 0;
            Favorites::instance().nextModeAt = 0;
            jsonWrite(configSetup, "cycle_on", "0");
            timeout_save_file_changes = millis();
            bitSet(save_file_changes, 0);
#if USE_MQTT
            Mqtt::instance().needToPublish = true;
#endif
          }

          ONflag = true;
          jsonWrite(configSetup, "Power", 1);

          uint8_t newEff = eff_num_correct[schedule[i].EffectNum];

          bool effectChanged = (currentMode != newEff);
          if (effectChanged && newEff < MODE_AMOUNT) {
            currentMode = newEff;
            loadingFlag = true;

            uint8_t ui_index = 0;
            for (ui_index = 0; ui_index < MODE_AMOUNT; ui_index++) {
              if (eff_num_correct[ui_index] == currentMode) break;
            }
            jsonWrite(configSetup, "eff_sel", ui_index);

            timeout_save_file_changes = millis();
            bitSet(save_file_changes, 0);
          }

          effectsTick();
          FastLED.show();
          changePower();
          sendCurrent(udpBuffer);
#if USE_MQTT
          Mqtt::instance().needToPublish = true;
#endif
        } break;

      case 4: { // включить Часы
          // когда включены часы в обычном режиме, галочка на чекбоксе "Включить ночные часы" снимается автоматически
          nightClockEnabled = false;
          jsonWrite(configSetup, "night_clock_enabled", "0");
          timeout_save_file_changes = millis();
          bitSet(save_file_changes, 0);
          if (Favorites::instance().FavoritesRunning == 1) {
            Favorites::instance().FavoritesRunning = 0;
            Favorites::instance().nextModeAt = 0;
            jsonWrite(configSetup, "cycle_on", "0");
            timeout_save_file_changes = millis();
            bitSet(save_file_changes, 0);
#if USE_MQTT
            Mqtt::instance().needToPublish = true;
#endif
          }

          ONflag = true;
          jsonWrite(configSetup, "Power", 1);

          uint8_t newEff = eff_num_correct[EFF_CLOCK];

          if (newEff < MODE_AMOUNT) {
            currentMode = newEff;
            loadingFlag = true;

            uint8_t ui_index = 0;
            for (ui_index = 0; ui_index < MODE_AMOUNT; ui_index++) {
              if (eff_num_correct[ui_index] == currentMode) break;
            }
            jsonWrite(configSetup, "eff_sel", ui_index);

            timeout_save_file_changes = millis();
            bitSet(save_file_changes, 0);
          }

          nightModeBrightness = 0;

          if (userClockBrightness == 0) {
            userClockBrightness = jsonReadtoInt(configSetup, "brightness");
            if (userClockBrightness < 10) userClockBrightness = 30;
          }

          modes[currentMode].Brightness = userClockBrightness;
          jsonWrite(configSetup, "brightness", userClockBrightness);
          timeout_save_file_changes = millis();
          bitSet(save_file_changes, 0);

          FastLED.setBrightness(userClockBrightness);

          effectsTick();
          FastLED.show();
          changePower();
          sendCurrent(udpBuffer);
#if USE_MQTT
          Mqtt::instance().needToPublish = true;
#endif
        } break;

      case 5:
        if (Favorites::instance().FavoritesRunning == 0) {
          Favorites::instance().FavoritesRunning = 1;
          Favorites::instance().nextModeAt = 0;
          if (!ONflag) {
            ONflag = true;
            jsonWrite(configSetup, "Power", 1);
            changePower();
            sendCurrent(udpBuffer);
          }
          jsonWrite(configSetup, "cycle_on", "1");
          timeout_save_file_changes = millis();
          bitSet(save_file_changes, 0);
#if USE_MQTT
          Mqtt::instance().needToPublish = true;
#endif
        } break;

      case 6:
        if (Favorites::instance().FavoritesRunning == 1) {
          Favorites::instance().FavoritesRunning = 0;
          Favorites::instance().nextModeAt = 0;
          jsonWrite(configSetup, "cycle_on", "0");
          timeout_save_file_changes = millis();
          bitSet(save_file_changes, 0);
#if USE_MQTT
          Mqtt::instance().needToPublish = true;
#endif
        } break;

      case 7: { // включить Ночные часы
          // когда включены ночные часы, галочка на чекбоксе "Включить ночные часы" устанавливается автоматически
          nightClockEnabled = true;
          jsonWrite(configSetup, "night_clock_enabled", "1");
          timeout_save_file_changes = millis();
          bitSet(save_file_changes, 0);

          if (Favorites::instance().FavoritesRunning == 1) {
            Favorites::instance().FavoritesRunning = 0;
            Favorites::instance().nextModeAt = 0;
            jsonWrite(configSetup, "cycle_on", "0");
            timeout_save_file_changes = millis();
            bitSet(save_file_changes, 0);
#if USE_MQTT
            Mqtt::instance().needToPublish = true;
#endif
          }

          ONflag = true;
          jsonWrite(configSetup, "Power", 1);

          uint8_t newEff = eff_num_correct[EFF_CLOCK];

          if (newEff < MODE_AMOUNT) {
            currentMode = newEff;
            loadingFlag = true;

            uint8_t ui_index = 0;
            for (ui_index = 0; ui_index < MODE_AMOUNT; ui_index++) {
              if (eff_num_correct[ui_index] == currentMode) break;
            }
            jsonWrite(configSetup, "eff_sel", ui_index);

            timeout_save_file_changes = millis();
            bitSet(save_file_changes, 0);
          }

          if (userClockBrightness == 0) {
            userClockBrightness = jsonReadtoInt(configSetup, "brightness");
            if (userClockBrightness < 10) userClockBrightness = 30;
          }

          nightModeBrightness = nightClockBrightness;
          modes[currentMode].Brightness = nightModeBrightness;
          jsonWrite(configSetup, "brightness", nightModeBrightness);
          timeout_save_file_changes = millis();
          bitSet(save_file_changes, 0);

          FastLED.setBrightness(nightModeBrightness);

          effectsTick();
          FastLED.show();
          changePower();
          sendCurrent(udpBuffer);
#if USE_MQTT
          Mqtt::instance().needToPublish = true;
#endif
        } break;
    }

    lastExecutedMinute[i] = schedule[i].Time;
  }
}
sched_end:;
#endif // USE_SCHEDULE

// -----------------------------------------------------------------------
char timeBuf[9];
sprintf_P(timeBuf, PSTR("%02u:%02u:00"), currentHour, currentMinute);
jsonWrite(configSetup, "time", String(timeBuf));
Save_File_Changes();

// -----------------------------------------------------------------------------------------------------------------------------------------------------
} // void timeTick()

// ******************************************************************************************************************************************************

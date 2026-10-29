// *************************************************************************** radio.ino ****************************************************************
#include "Constants.h"
#include "Extern.h"
#include "Prototypes.h"
#include "Time.h"
#include "Types.h"
// -----------------------

#if USE_RADIO
// -----------------

#include <Audio.h> // в прошивке используется модифициованная версия библиотеки ESP32-audioI2S-3.0.12 (

// ------------------------------------------

Audio audio;

// -----------------------------------------------------------------------------------------

void radioInit() {
  if (radioInited) return;
  audio.setPinout(I2S_BCLK, I2S_LRCK, I2S_DOUT);
  audio.setVolume(radioVolume);
  audio.setConnectionTimeout(5000, 5000);
  radioInited = true;
}

// -----------------------------------------------------------------------------------------

// чтение URL станции
String getStationUrl(uint8_t idx) {
  if (configRadio.isEmpty()) return "";
  DynamicJsonDocument doc(4096);
  if (deserializeJson(doc, configRadio) != DeserializationError::Ok) return "";
  JsonArray stations = doc["stations"].as<JsonArray>();
  if (!stations || idx >= stations.size()) return "";
  return stations[idx]["url"].as<String>();
}

// -----------------------------------------------------------------------------------------

String getStationName(uint8_t idx) {
  if (configRadio.isEmpty()) return "";
  DynamicJsonDocument doc(4096);
  if (deserializeJson(doc, configRadio) != DeserializationError::Ok) return "";
  JsonArray stations = doc["stations"].as<JsonArray>();
  if (!stations || idx >= stations.size()) return "";
  return stations[idx]["name"].as<String>();
}

// -----------------------------------------------------------------------------------------

uint8_t getStationCount() {
  if (configRadio.isEmpty()) return 0;
  DynamicJsonDocument doc(4096);
  if (deserializeJson(doc, configRadio) != DeserializationError::Ok) return 0;
  JsonArray stations = doc["stations"].as<JsonArray>();
  return stations ? stations.size() : 0;
}

// -----------------------------------------------------------------------------------------

void radioPlayStation(uint8_t stationId) {
  if (radioPlaying && currentStation == stationId) return;
  if (!radioEnabled) return;
  if (stationId == 0) {
#if RADIO_LOG
    SYSLOG.add("radio: станция не назначена для эффекта %u", currentMode);
#endif
    return;
  }

  uint8_t idx0 = stationId - 1;
  String url = getStationUrl(idx0);
  if (url.length() == 0) {
#if RADIO_LOG
    SYSLOG.add("radio: станция %u не найдена (url пуст)", stationId);
#endif
    return;
  }

  radioInit();
  if (radioPlaying) {
    audio.stopSong();
    delay(50);
  }
  audio.connecttohost(url.c_str());
  radioPlaying = true;
  currentStation = stationId;

#if RADIO_LOG
  SYSLOG.add("radio: play station %u (%s)", stationId, getStationName(idx0).c_str());
#endif
}

// -----------------------------------------------------------------------------------------

// вспомогательная функция
uint8_t radioGetStationForMode(uint8_t modeCode) {
  for (uint8_t uiIdx = 0; uiIdx < MODE_AMOUNT; uiIdx++) {
    if (eff_num_correct[uiIdx] == modeCode) {
      return effects_stations[uiIdx];
    }
  }
  return 0;
}

// -----------------------------------------------------------------------------------------

void radioStop() {
  if (!radioInited) return;
  audio.stopSong();
  radioPlaying = false;
  currentStation = 0;
#if RADIO_LOG
  SYSLOG.add("radio: stop");
#endif
}

// -----------------------------------------------------------------------------------------

void radioSetVolume(uint8_t vol) {
  vol = constrain(vol, 0, 21);
  radioVolume = vol;
  if (radioInited) audio.setVolume(radioVolume);
  jsonWrite(configRadio, "volume", radioVolume);
  writeFile(F("config_radio.json"), configRadio);
}

// -----------------------------------------------------------------------------------------

void radioLoop() {
  if (!radioEnabled) return;
  audio.loop();
}

// -----------------
#endif // USE_RADIO

// ******************************************************************************************************************************************************

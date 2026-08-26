// ************************************************************************** effectsNoise.ino **********************************************************
#include "Extern.h"
#include "Prototypes.h"
// ---------------------

// =========================================================================== БЕЗУМИЕ =================================================================
void madnessNoiseRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t tmp = random8(9U);
      setModeSettings(30U + tmp * tmp, 20U + random8(41U));
    }
#endif
    loadingFlag = false;
    scale = modes[currentMode].Scale;
    speed = modes[currentMode].Speed;
  }
  scale = modes[currentMode].Scale;
  speed = modes[currentMode].Speed;

  fillnoise8();

  for (uint16_t i = 0; i < matrixWidth; i++) {
    for (uint16_t j = 0; j < matrixHeight; j++) {
      uint8_t hue = noise[j][i];
      uint8_t val = noise[i][j];
      drawPixelXY(i, j, CHSV(hue, 255, val));
    }
  }
}

// =========================================================================== ЗЕБРА ===================================================================
void zebraNoiseRoutine() {
  if (loadingFlag)
  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(12U + random8(16U), 1U + random8(9U));
    }
#endif
    loadingFlag = false;
    fill_solid(currentPalette, 16, CRGB(0, 0, 0));
    currentPalette[0] = currentPalette[4] = currentPalette[8] = currentPalette[12] = CRGB::White;
    noiseX = noiseY = noiseZ = random16();
    ihue = 0;
  }
  scale = modes[currentMode].Scale;
  speed = modes[currentMode].Speed;
  colorLoop = 1;
  fillNoiseLED();
}

// ============================================================================== ЛЕС ==================================================================
void forestNoiseRoutine() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(70U + random8(31U), 2U + random8(24U));
    }
#endif
    loadingFlag = false;
    currentPalette = ForestColors_p;
    colorLoop = 0;
    noiseX = noiseY = noiseZ = random16();
    ihue = 0;
  }
  scale = modes[currentMode].Scale;
  speed = modes[currentMode].Speed;
  fillNoiseLED();
}

// ======================================================================== ОКЕАН ======================================================================
void oceanNoiseRoutine() {
  if (loadingFlag)
  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(6U + random8(25U), 4U + random8(8U));
    }
#endif
    loadingFlag = false;
    currentPalette = OceanColors_p;
    scale = modes[currentMode].Scale;
    speed = modes[currentMode].Speed;
    colorLoop = 0;
    noiseX = noiseY = noiseZ = random16();
    ihue = 0;
  }
  scale = modes[currentMode].Scale;
  speed = modes[currentMode].Speed;
  fillNoiseLED();
}

// ========================================================================== ПЛАЗМА ===================================================================
void plasmaNoiseRoutine() {
  if (loadingFlag)
  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t tmp = random8(10U);
      setModeSettings(20U + tmp * tmp, 1U + random8(27U));
    }
#endif
    loadingFlag = false;
    currentPalette = PartyColors_p;
    scale = modes[currentMode].Scale;
    speed = modes[currentMode].Speed;
    colorLoop = 1;
    noiseX = noiseY = noiseZ = random16();
    ihue = 0;
  }
  scale = modes[currentMode].Scale;
  speed = modes[currentMode].Speed;
  fillNoiseLED();
}

// ============================================================================= ОБЛАКА ================================================================
void cloudsNoiseRoutine() {
  if (loadingFlag)
  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(15U + random8(36U), 1U + random8(10U));
    }
#endif
    loadingFlag = false;
    currentPalette = CloudColors_p;
    scale = modes[currentMode].Scale;
    speed = modes[currentMode].Speed;
    colorLoop = 0;
    noiseX = noiseY = noiseZ = random16();
    ihue = 0;
  }
  scale = modes[currentMode].Scale;
  speed = modes[currentMode].Speed;
  fillNoiseLED();
}

// ======================================================================= ЛАВА ========================================================================
void lavaNoiseRoutine() {
  if (loadingFlag)
  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t tmp = random8(9U);
      setModeSettings(10U + tmp * tmp, 5U + random8(16U));
    }
#endif
    loadingFlag = false;
    currentPalette = LavaColors_p;
    scale = modes[currentMode].Scale;
    speed = modes[currentMode].Speed;
    colorLoop = 0;
    noiseX = noiseY = noiseZ = random16();
    ihue = 0;
  }
  scale = modes[currentMode].Scale;
  speed = modes[currentMode].Speed;
  fillNoiseLED();
}

// ========================================================================== РАДУГА ===================================================================
void rainbowNoiseRoutine()
{
  if (loadingFlag)
  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      uint8_t tmp = random8(10U);
      setModeSettings(20U + tmp * tmp, 1U + random8(23U));
    }
#endif
    loadingFlag = false;
    currentPalette = RainbowColors_p;
    colorLoop = 1;
    noiseX = noiseY = noiseZ = random16();
    ihue = 0;
    scale = modes[currentMode].Scale;
    speed = modes[currentMode].Speed;
  }
  scale = modes[currentMode].Scale;
  speed = modes[currentMode].Speed;
  fillNoiseLED();
}
// -----------------------------------------------------------
void rainbowStripeNoiseRoutine() {
  if (loadingFlag)
  {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(8U + random8(17U), 1U + random8(9U));
    }
#endif
    loadingFlag = false;
    currentPalette = RainbowStripeColors_p;
    colorLoop = 1;
    noiseX = noiseY = noiseZ = random16();
    ihue = 0;
    scale = modes[currentMode].Scale;
    speed = modes[currentMode].Speed;
  }
  scale = modes[currentMode].Scale;
  speed = modes[currentMode].Speed;
  fillNoiseLED();
}

// ******************************************************************************************************************************************************

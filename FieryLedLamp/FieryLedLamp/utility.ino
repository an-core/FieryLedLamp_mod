// **************************************************************************** utility.ino *************************************************************
#include "Prototypes.h"
// -----------------------

void fillAll(CRGB color) {
  fill_solid(leds, usedLeds, color);
}

void drawPixelXY(int8_t x, int8_t y, CRGB color) {
  if (x < 0 || x >= matrixWidth || y < 0 || y >= matrixHeight) return;
  uint32_t thisPixel = XY((uint8_t)x, (uint8_t)y) * SEGMENTS;
  for (uint8_t i = 0; i < SEGMENTS; i++) {
    leds[thisPixel + i] = color;
  }
}

uint32_t getPixColor(uint32_t thisSegm) {
  if (SEGMENTS == 0) return 0;
  uint32_t thisPixel = thisSegm * SEGMENTS;
  if (thisPixel + SEGMENTS > usedLeds) return 0;
  return ((uint32_t)leds[thisPixel].r << 16) | ((uint32_t)leds[thisPixel].g << 8) | (uint32_t)leds[thisPixel].b;
}

uint32_t getPixColorXY(uint8_t x, uint8_t y) {
  return getPixColor(XY(x, y));
}

// вспомогательная функция для одной физической матрицы
static uint16_t XY_single(uint8_t x, uint8_t y) {
  uint8_t THIS_X, THIS_Y;
  uint16_t _WIDTH = segWidth;
  switch (MatrixOrientation) {
    case 0: THIS_X = x; THIS_Y = y; break;
    case 1: _WIDTH = segHeight; THIS_X = y; THIS_Y = x; break;
    case 2: THIS_X = x; THIS_Y = (segHeight - y - 1); break;
    case 3: _WIDTH = segHeight; THIS_X = (segHeight - y - 1); THIS_Y = x; break;
    case 4: THIS_X = (segWidth - x - 1); THIS_Y = (segHeight - y - 1); break;
    case 5: _WIDTH = segHeight; THIS_X = (segHeight - y - 1); THIS_Y = (segWidth - x - 1); break;
    case 6: THIS_X = (segWidth - x - 1); THIS_Y = y; break;
    case 7: _WIDTH = segHeight; THIS_X = y; THIS_Y = (segWidth - x - 1); break;
    default: THIS_X = x; THIS_Y = y; break;
  }
  uint16_t idx;
  if (!(THIS_Y & 0x01) || MATRIX_TYPE)
    idx = THIS_Y * _WIDTH + THIS_X;
  else
    idx = THIS_Y * _WIDTH + _WIDTH - THIS_X - 1;
  return idx;
}

uint16_t XY(uint8_t x, uint8_t y) {
  #if MULTI_MATRIX
  // переворот панели
  if (panelFlip) {
    x = matrixWidth - 1 - x;
    y = matrixHeight - 1 - y;
  }

  if (x >= matrixWidth || y >= matrixHeight) return 0;
  if (segWidth == 0 || segHeight == 0) {
    return 0;
  }
  uint8_t tileX = x / segWidth;
  uint8_t tileY = y / segHeight;
  uint8_t localX = x % segWidth;
  uint8_t localY = y % segHeight;
  uint16_t localIdx = XY_single(localX, localY);
  uint16_t tileNumber = tileY * segMatrix_w + tileX;
  uint16_t pixelsPerTile = segWidth * segHeight;
  uint16_t globalPixel = tileNumber * pixelsPerTile + localIdx;
  if (globalPixel >= usedLeds / SEGMENTS) return 0;
  return globalPixel;
#else
  // когда одна матрица
  if (x >= segWidth || y >= segHeight) return 0;
  return XY_single(x, y);
#endif
}

// Для совместимости со старыми эффектами
uint16_t getPixelNumber(uint8_t x, uint8_t y) {
  return XY(x, y);
}

void restoreSettings() {
  for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
    modes[i].Brightness = pgm_read_byte(&defaultSettings[i][0]);
    modes[i].Speed = pgm_read_byte(&defaultSettings[i][1]);
    modes[i].Scale = pgm_read_byte(&defaultSettings[i][2]);
  }
}

// Квадратный корень
float sqrt3(const float x) {
  union {
    int i;
    float x;
  } u;
  u.x = x;
  u.i = (1 << 29) + (u.i >> 1) - (1 << 22);
  return u.x;
}

uint8_t SpeedFactor(uint8_t spd) {
  // Быстрее и точнее без float
  uint16_t result = (uint16_t)spd * usedLeds / 1024U;
  return (uint8_t)result;
}

// *******************************************************************************************************************************************************

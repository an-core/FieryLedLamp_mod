// ************************************************************************ effectsSupport.ino **********************************************************
#include "Prototypes.h"
#include "Extern.h"
#include "Constants.h"
#include "SystemLog.h"
#include <algorithm>
#include <cmath>
// -----------------------

// =========================================================================== ПЕРЕМЕННЫЕ ==============================================================
bool justPoweredOn = true;
uint16_t ff_x, ff_y, ff_z;
uint16_t speed = 20;
uint16_t scale = 30;
uint8_t custom_eff = 0;
byte binImage[8192];
uint8_t noisesmooth = 200;
uint16_t prevMaxNoiseDim = 0;             // Предыдущий размер шумового буфера (для перевыделения памяти)
uint8_t colorLoop = 1;                    // для шумовых эффнетов
uint32_t noise32_x[2] = {0};              // X-координаты для 3D шума
uint32_t noise32_y[2] = {0};              // Y-координаты для 3D шума
uint32_t noise32_z[2] = {0};              // Z-координаты для 3D шума
uint32_t scale32_x[2] = {0};              // Масштаб шума по X
uint32_t scale32_y[2] = {0};              // Масштаб шума по Y
float emitterX = 0.0f;                    // Позиция X эмиттера частиц
float emitterY = 0.0f;                    // Позиция Y эмиттера частиц
uint8_t centerX = 0;                      // Центр по X
uint8_t centerY = 0;                      // Центр по Y
int Painting = 0;                         // Режим рисования
CRGB DriwingColor = CRGB(255, 255, 255);  // Цвет рисования
CRGBPalette16 currentPalette;             // Текущая палитра для эффектов
uint8_t ihue = 0;                         // Текущий оттенок для эффектов
uint8_t hue, hue2;                        // Оттенки для градиентов
uint8_t deltaHue, deltaHue2;              // Шаг изменения оттенка
uint8_t step;                             // Шаг анимации
uint8_t pcnt;                             // Счётчик для палитр
uint8_t deltaValue;                       // Изменение яркости
float speedfactor;                        // Множитель скорости

// --------------------------------------------------------------------------
// Массивы состояния объектов, которые могут использоваться в любом эффекте
float trackingObjectPosX[trackingOBJECT_MAX_COUNT];
float trackingObjectPosY[trackingOBJECT_MAX_COUNT];
float trackingObjectSpeedX[trackingOBJECT_MAX_COUNT];
float trackingObjectSpeedY[trackingOBJECT_MAX_COUNT];
float trackingObjectShift[trackingOBJECT_MAX_COUNT];
uint8_t trackingObjectHue[trackingOBJECT_MAX_COUNT];
uint8_t trackingObjectState[trackingOBJECT_MAX_COUNT];
bool trackingObjectIsShift[trackingOBJECT_MAX_COUNT];
uint8_t enlargedObjectNUM;
long enlargedObjectTime[enlargedOBJECT_MAX_COUNT];
float liquidLampHot[enlargedOBJECT_MAX_COUNT];
float liquidLampSpf[enlargedOBJECT_MAX_COUNT];
unsigned liquidLampMX[enlargedOBJECT_MAX_COUNT];
unsigned liquidLampSC[enlargedOBJECT_MAX_COUNT];
unsigned liquidLampTR[enlargedOBJECT_MAX_COUNT];

// ======================================================= ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ ЭФФЕКТОВ ========================================================
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
void setModeSettings(uint8_t Scale, uint8_t Speed) {
  modes[currentMode].Scale = Scale ? Scale : pgm_read_byte(&defaultSettings[currentMode][2]);
  modes[currentMode].Speed = Speed ? Speed : pgm_read_byte(&defaultSettings[currentMode][1]);
  jsonWrite(configSetup, "sp", modes[currentMode].Speed);
  jsonWrite(configSetup, "sc", modes[currentMode].Scale);
  jsonWrite(configSetup, "br", modes[currentMode].Brightness);
  selectedSettings = 0U;
#if USE_BLYNK
  updateRemoteBlynkParams();
#endif
}
#endif

// ----------------------------------------------------------------------------
void blurScreen(fract8 blur_amount, CRGB *LEDarray) {
  blur2d(LEDarray, matrixWidth, matrixHeight, blur_amount);
}

// ----------------------------------------------------------------------------
void dimAll(uint8_t value, CRGB *LEDarray) {
  nscale8(LEDarray, usedLeds, value);
}

// ----------------------------------------------------------------------------
void setNoise(uint16_t x, uint16_t y, uint8_t val) {
  if (!noise || x >= prevMaxNoiseDim || y >= prevMaxNoiseDim || !noise[x] || !noise[y]) return;
  noise3d[0][x][y] = val;
}

// ----------------------------------------------------------------------------
uint8_t safeGetNoise(uint8_t layer, uint8_t x, uint8_t y) {
  if (!noise3d || !noise3d[layer] || !noise3d[layer][x]) return 0;
  return noise3d[layer][x][y];
}

// ----------------------------------------------------------------------------
void safeSetNoise(uint8_t layer, uint8_t x, uint8_t y, uint8_t val) {
  if (!noise3d || !noise3d[layer] || !noise3d[layer][x]) return;
  noise3d[layer][x][y] = val;
}

// ----------------------------------------------------------------------------
uint8_t getNoise(uint16_t x, uint16_t y) {
  if (!noise || x >= prevMaxNoiseDim || y >= prevMaxNoiseDim || !noise[x] || !noise[y]) return 0;
  return noise3d[0][x][y];
}

// ----------------------------------------------------------------------------
uint8_t mapsin8(uint8_t theta, uint8_t lowest, uint8_t highest) {
  uint8_t beatsin = sin8(theta);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatsin, rangewidth);
  return lowest + scaledbeat;
}

// ----------------------------------------------------------------------------
uint8_t mapcos8(uint8_t theta, uint8_t lowest, uint8_t highest) {
  uint8_t beatcos = cos8(theta);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatcos, rangewidth);
  return lowest + scaledbeat;
}

// ----------------------------------------------------------------------------
uint8_t myScale8(uint8_t x) {
  uint8_t x8 = x % 8U;
  uint8_t x4 = x8 % 4U;
  if (x4 == 0U)
    return (x8 == 0U) ? 0U : 255U;
  else if (x8 < 4U)
    return (1U + x4 * 72U);
  else
    return (253U - x4 * 72U);
}

// ----------------------------------------------------------------------------
// Рисование
void DrawLine(int x1, int y1, int x2, int y2, CRGB color) {
  int deltaX = abs(x2 - x1);
  int deltaY = abs(y2 - y1);
  int signX = x1 < x2 ? 1 : -1;
  int signY = y1 < y2 ? 1 : -1;
  int error = deltaX - deltaY;

  drawPixelXY(x2, y2, color);
  while (x1 != x2 || y1 != y2) {
    drawPixelXY(x1, y1, color);
    int error2 = error * 2;
    if (error2 > -deltaY) {
      error -= deltaY;
      x1 += signX;
    }
    if (error2 < deltaX) {
      error += deltaX;
      y1 += signY;
    }
  }
}

// ----------------------------------------------------------------------------
void DrawLineF(float x1, float y1, float x2, float y2, CRGB color) {
  float deltaX = std::fabs(x2 - x1);
  float deltaY = std::fabs(y2 - y1);
  float error = deltaX - deltaY;
  float signX = x1 < x2 ? 0.5f : -0.5f;
  float signY = y1 < y2 ? 0.5f : -0.5f;

  while (x1 != x2 || y1 != y2) {
    if ((signX > 0 && x1 > x2 + signX) || (signX < 0 && x1 < x2 + signX)) break;
    if ((signY > 0 && y1 > y2 + signY) || (signY < 0 && y1 < y2 + signY)) break;
    drawPixelXYF(x1, y1, color);
    float error2 = error;
    if (error2 > -deltaY) {
      error -= deltaY;
      x1 += signX;
    }
    if (error2 < deltaX) {
      error += deltaX;
      y1 += signY;
    }
  }
}

CRGB makeDarker(const CRGB& color, fract8 howMuchDarker) {
  CRGB newcolor = color;
  newcolor.fadeToBlackBy(howMuchDarker);
  return newcolor;
}

// ----------------------------------------------------------------------------
void drawPixelXYF(float x, float y, CRGB color) {
  if (x < -0.5 || x >= matrixWidth + 0.5 || y < -0.5 || y >= matrixHeight + 0.5) return;
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x >= matrixWidth - 1) x = matrixWidth - 1 - 0.001f;
  if (y >= matrixHeight - 1) y = matrixHeight - 1 - 0.001f;
  uint8_t xx = (x - (int)x) * 255, yy = (y - (int)y) * 255, ix = 255 - xx, iy = 255 - yy;
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy), WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)};
  for (uint8_t i = 0; i < 4; i++) {
    int16_t xn = x + (i & 1), yn = y + ((i >> 1) & 1);
    CRGB clr = getPixColorXY(xn, yn);
    clr.r = qadd8(clr.r, (color.r * wu[i]) >> 8);
    clr.g = qadd8(clr.g, (color.g * wu[i]) >> 8);
    clr.b = qadd8(clr.b, (color.b * wu[i]) >> 8);
    drawPixelXY(xn, yn, clr);
  }
}

// ----------------------------------------------------------------------------
void drawCircleF(float x0, float y0, float radius, CRGB color) {
  float x = 0, y = radius, error = 0;
  float delta = 1.0f - 2.0f * radius;

  while (y >= 0) {
    drawPixelXYF(fmod(x0 + x + matrixWidth, matrixWidth), y0 + y, color);
    drawPixelXYF(fmod(x0 + x + matrixWidth, matrixWidth), y0 - y, color);
    drawPixelXYF(fmod(x0 - x + matrixWidth, matrixWidth), y0 + y, color);
    drawPixelXYF(fmod(x0 - x + matrixWidth, matrixWidth), y0 - y, color);
    error = 2.0f * (delta + y) - 1.0f;
    if (delta < 0 && error <= 0) {
      ++x;
      delta += 2.0f * x + 1.0f;
      continue;
    }
    error = 2.0f * (delta - x) - 1.0f;
    if (delta > 0 && error > 0) {
      --y;
      delta += 1.0f - 2.0f * y;
      continue;
    }
    ++x;
    delta += 2.0f * (x - y);
    --y;
  }
}

// ----------------------------------------------------------------------------
void setCurrentPalette() {
  if (modes[currentMode].Scale > 100U) modes[currentMode].Scale = 100U;
  curPalette = palette_arr[(uint8_t)(modes[currentMode].Scale / 100.0F * ((sizeof(palette_arr) / sizeof(TProgmemRGBPalette16 *)) - 0.01F))];
}

// ----------------------------------------------------------------------------
void espModeState(uint8_t color) {
  if (loadingFlag) {
    loadingFlag = false;
    step = 0;
    hue2 = 0;
    deltaHue2 = 1;
    pcnt = 1;

    DrawLine(CENTER_X_MINOR, CENTER_Y_MINOR, CENTER_X_MAJOR + 1, CENTER_Y_MINOR, CHSV(color, 255, 210));
    DrawLine(CENTER_X_MINOR, CENTER_Y_MINOR - 1, CENTER_X_MAJOR + 1, CENTER_Y_MINOR - 1, CHSV(color, 255, 210));

    FastLED.show();
  }

  if (pcnt > 0 && pcnt < 200) {
    pcnt++;
    dimAll(200);

    uint8_t w = hue2 / 2;
    w = constrain(w, 0, (WIDTH / 2) - 1);

    uint8_t brightness = 210 - (deltaHue2 * 2);
    brightness = constrain(brightness, 50, 210);

    DrawLine(CENTER_X_MINOR - w, CENTER_Y_MINOR + deltaHue2, CENTER_X_MAJOR + w, CENTER_Y_MINOR + deltaHue2, CHSV(color, 255, brightness));
    DrawLine(CENTER_X_MINOR - w, CENTER_Y_MINOR - 1 - deltaHue2, CENTER_X_MAJOR + w, CENTER_Y_MINOR - 1 - deltaHue2, CHSV(color, 255, brightness));

    hue2 += 2;
    deltaHue2 += 1;

    if (CENTER_Y_MINOR + deltaHue2 >= HEIGHT - 1) {
      pcnt = 200;
    }
  }
  else {

    uint8_t waitColor = myTime.isTimeSet() ? 176 : 0;
    uint8_t bri = (step % 50 < 25) ? 200 : 100;
    leds[XY(CENTER_X_MINOR, 0)] = CHSV(waitColor, 255, bri);
  }

  step++;
  FastLED.show();
}

// ----------------------------------------------------------------------------
void drawRec(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, uint32_t color) {
  for (uint8_t y = startY; y < endY; y++) {
    for (uint8_t x = startX; x < endX; x++) {
      drawPixelXY(x, y, color);
    }
  }
}

// ----------------------------------------------------------------------------
void drawRecCHSV(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, CHSV color) {
  for (uint8_t y = startY; y < endY; y++) {
    for (uint8_t x = startX; x < endX; x++) {
      drawPixelXY(x, y, color);
    }
  }
}

// ----------------------------------------------------------------------------
uint8_t validMinMax(float val, uint8_t minV, uint32_t maxV) {
  uint8_t result;
  if (val <= minV) {
    result = minV;
  } else if (val >= maxV) {
    result = maxV;
  } else {
    result = ceil(val);
  }
  return result;
}

// ----------------------------------------------------------------------------
// альтернативный градиент для ламп собраных из лент с вертикальной компоновкой
// gradientHorizontal | gradientVertical менее производительный но работает на всех видах ламп
void gradientHorizontal(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, uint8_t start_color, uint8_t end_color, uint8_t start_br, uint8_t end_br, uint8_t saturate) {
  float step_color = 0;
  float step_br = 0;
  if (startX == endX) {
    endX++;
  }
  if (startY == endY) {
    endY++;
  }
  step_color = (end_color - start_color) / abs(startX - endX);
  if (start_color >  end_color) {
    step_color -= 1.2;
  } else {
    step_color += 1.2;
  }

  step_br = (end_br - start_br) / abs(startX - endX);
  if (start_br >  end_color) {
    step_br -= 1.2;
  } else {
    step_br += 1.2;
  }

  for (uint8_t x = startX; x < endX; x++) {
    for (uint8_t y = startY; y < endY; y++) {
      CHSV thisColor = CHSV((uint8_t) validMinMax((start_color + (x - startX) * step_color), 1, 254), saturate,
                            (uint8_t) validMinMax((start_br + (x - startX) * step_br), 0, 255) );
      drawPixelXY(x, y, thisColor);
    }
  }
}

// ----------------------------------------------------------------------------
void gradientVertical(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, uint8_t start_color, uint8_t end_color, uint8_t start_br, uint8_t end_br, uint8_t saturate) {
  float step_color = 0;
  float step_br = 0;
  if (startX == endX) {
    endX++;
  }
  if (startY == endY) {
    endY++;
  }
  step_color = (end_color - start_color) / abs(startY - endY);

  if (start_color >  end_color) {
    step_color -= 1.2;
  } else {
    step_color += 1.2;
  }

  step_br = (end_br - start_br) / abs(startY - endY);
  if (start_br >  end_color) {
    step_br -= 1.2;
  } else {
    step_br += 1.2;
  }
  for (uint8_t y = startY; y < endY; y++) {
    CHSV thisColor = CHSV( (uint8_t) validMinMax((start_color + (y - startY) * step_color), 0, 255), saturate, (uint8_t) validMinMax((start_br + (y - startY) * step_br), 0, 255) );
    for (uint8_t x = startX; x < endX; x++) {
      drawPixelXY(x, y, thisColor);
    }
  }
}

// ----------------------------------------------------------------------------
// gradientDownTop - более плавный градиент в отличие от gradientVertical
void gradientDownTop(uint8_t bottom, CHSV bottom_color, uint8_t top, CHSV top_color) {
  if (MatrixOrientation < 3 || MatrixOrientation == 7) {
    fill_gradient(leds, top * matrixWidth, top_color, bottom * matrixWidth, bottom_color, SHORTEST_HUES);
  } else {
    fill_gradient(leds, usedLeds - bottom * matrixWidth - 1, bottom_color, usedLeds - top * matrixWidth, top_color, SHORTEST_HUES);
  }
}

// ----------------------------------------------------------------------------
// функция чтения бинарного файла изображения из файловой системы лампы
void readBinFile(String fileName, size_t len) {

  File binFile = LittleFS.open("/" + fileName, "r");
  if (!binFile) {
    return;
  }
  size_t size = binFile.size();
  if (size > len) {
    binFile.close();
    return;
  }

  byte buffer[size];
  uint16_t amount;

  if (binFile == NULL) exit (1);
  binFile.seek(0);

  while (binFile.available()) {
    amount = binFile.read(buffer, size);
  }
  
  memcpy(binImage, buffer, amount);
  binFile.close();
}

// ----------------------------------------------------------------------------
// функция получения размера изображения из заголовка файла
uint16_t getSizeValue(byte* buffer, byte b) {
  return  (buffer[b + 1] << 8) + buffer[b];
}

// ----------------------------------------------------------------------------
// Рисует изображение с масштабированием по высоте и циклической прокруткой по горизонтали
void drawScaledImage(uint16_t imgW, uint16_t imgH, uint16_t offsetX) {
  const uint16_t HEADER = 16;
  const uint16_t BYTES_PER_PIXEL = 2;

  for (uint16_t y = 0; y < matrixHeight; y++) {
    uint16_t srcY = (y * imgH) / matrixHeight;
    uint16_t rowOffset = HEADER + srcY * imgW * BYTES_PER_PIXEL;

    for (uint16_t x = 0; x < matrixWidth; x++) {
      uint16_t srcX = (offsetX + x) % imgW;
      uint16_t pixIndex = rowOffset + srcX * BYTES_PER_PIXEL;
      uint8_t b1 = binImage[pixIndex];
      uint8_t b2 = binImage[pixIndex + 1];
      uint8_t r = (b2 & 0xF8);
      uint8_t g = ((b2 & 0x07) << 5) | ((b1 & 0xE0) >> 3);
      uint8_t b = (b1 & 0x1F) << 3;

      leds[XY(x, y)] = CRGB(r, g, b);
    }
  }
}

// ----------------------------------------------------------------------------
uint32_t colorDimm(uint32_t colorValue, long lenght, long pixel) {
  uint8_t red = (colorValue & 0x00FF0000) >> 16;
  uint8_t green = (colorValue & 0x0000FF00) >> 8;
  uint8_t blue = (colorValue & 0x000000FF);
  double prozent = 100 / lenght;
  red = red - red * ((prozent * pixel) / 100);
  green = green - green * ((prozent * pixel) / 100);
  blue = blue - blue * ((prozent * pixel) / 100);
  colorValue = red;
  colorValue = (colorValue << 8) + green;
  colorValue = (colorValue << 8) + blue;
  return colorValue;
}

// ----------------------------------------------------------------------------
void fillnoise8() {
  if (!noise || prevMaxNoiseDim == 0) return;
  uint16_t maxDim = prevMaxNoiseDim;
  for (uint16_t i = 0; i < maxDim; i++) {
    if (noise[i] == nullptr) continue;
    int32_t ioffset = (int32_t)scale * i;
    for (uint16_t j = 0; j < maxDim; j++) {
      int32_t joffset = (int32_t)scale * j;
      noise[i][j] = inoise8(noiseX + ioffset, noiseY + joffset, noiseZ);
    }
  }
  noiseZ += speed;
  noiseX += speed / 8;
  noiseY -= speed / 16;
}

// ----------------------------------------------------------------------------
void fillNoiseLED() {
  if (!noise || prevMaxNoiseDim == 0) return;

  uint8_t dataSmoothing = 0;
  if (speed < 50) {
    dataSmoothing = 200 - (speed * 4);
  }

  uint16_t maxDim = prevMaxNoiseDim;

  for (uint16_t i = 0; i < maxDim; i++) {
    int32_t ioffset = scale * (int32_t)i;
    for (uint16_t j = 0; j < maxDim; j++) {
      if (noise[i] == nullptr) continue;
      int32_t joffset = scale * (int32_t)j;
      uint8_t data = inoise8(noiseX + ioffset, noiseY + joffset, noiseZ);
      data = qsub8(data, 16);
      data = qadd8(data, scale8(data, 39));

      if (dataSmoothing) {
        uint8_t olddata = noise[i][j];
        uint8_t newdata = scale8(olddata, dataSmoothing) + scale8(data, 256 - dataSmoothing);
        data = newdata;
      }
      noise[i][j] = data;
    }
  }

  noiseZ += speed;
  noiseX += speed / 8;
  noiseY -= speed / 16;

  for (uint16_t x = 0; x < matrixWidth; x++) {
    for (uint16_t y = 0; y < matrixHeight; y++) {
      uint8_t index = 0;
      if (y < maxDim && noise[y] != nullptr && x < maxDim) {
        index = noise[y][x];
      }
      if (colorLoop) {
        index += ihue;
      }
      uint8_t bri = index;
      CRGB color = ColorFromPalette(currentPalette, index, bri);
      drawPixelXY(x, y, color);
    }
  }

  ihue += 1;
}

// ******************************************************************************************************************************************************

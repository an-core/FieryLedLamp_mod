// ************************************************************************ runningText.ino *************************************************************
#include <algorithm>
#include "Extern.h"
#include "Time.h"
// ------------------------

// =============================================================== ШРИФТ и ОТРИСОВКА СИМВОЛА НА МАТРИЦЕ ================================================
const FontDesc fontTable[3] PROGMEM = {
  {5,  8,  8, false, font5x8},
  {8, 13, 13, true, font8x13},
  {10, 16, 16, true, font10x16}
};
// -------------------------------------

// возвращает реальную ширину строки в пикселях
int getStringRealWidth(const char* str, uint8_t fontId) {
  int totalWidth = 0;
  size_t pos = 0;
  while (str[pos] != '\0') {
    uint8_t high = 0;
    uint8_t low = (uint8_t)str[pos];
    if (low < 128) {
      pos++;
    } else if ((low & 0xE0) == 0xC0) {
      high = low;
      pos++;
      if (str[pos] == '\0') break;
      low = (uint8_t)str[pos];
      pos++;
    } else {
      pos++;
      continue;
    }
    uint8_t rw = getCharRealWidth(high, low, fontId);
    totalWidth += rw;
    if (str[pos] != '\0') totalWidth += 1;
  }
  return totalWidth;
}

// возвращает реальную ширину символа
uint8_t getCharRealWidth(uint8_t high, uint8_t low, uint8_t fontId) {
  if (fontId == 0) return 3;

  if (high == 0xC2 && low == 0xB0) {
    if (fontId == 1) return 3;
    else if (fontId == 2) return 6;
    else if (fontId == 3) return 7;
    else return 3;
  }

  uint8_t width  = getFontWidth(fontId);
  uint8_t height = getFontHeight(fontId);
  uint16_t symIdx = 0;

  if (low < 128) {
    symIdx = low - 32;
  } else {
    return width;
  }
  if (symIdx >= 200) return width;

  const void* fontData = nullptr;
  if (fontId == 1) fontData = font5x8;
  else if (fontId == 2) fontData = font8x13;
  else if (fontId == 3) fontData = font10x16;
  if (!fontData) return width;

  int firstCol = width;
  int lastCol  = -1;
  for (uint8_t col = 0; col < width; col++) {
    uint16_t vertWord = 0;
    if (fontId == 1) {
      vertWord = pgm_read_byte(&((const uint8_t*)fontData)[symIdx * width + col]);
    } else {
      vertWord = pgm_read_word(&((const uint16_t*)fontData)[symIdx * width + col]);
    }
    for (uint8_t row = 0; row < height; row++) {
      if (vertWord & (1 << (height - 1 - row))) {
        if (col < firstCol) firstCol = col;
        if (col > lastCol)  lastCol  = col;
        break;
      }
    }
  }
  if (lastCol < 0) return 1;
  return lastCol - firstCol + 1;
}

// возвращает ширину символа
uint8_t getFontWidth(uint8_t fontId) {
  switch (fontId) {
    case 0: return 3;   // 3x5
    case 1: return 5;   // 5x8
    case 2: return 8;   // 8x13
    case 3: return 10;  // 10x16
    default: return 3;
  }
}

// возвращает высоту символа
uint8_t getFontHeight(uint8_t fontId) {
  switch (fontId) {
    case 0: return 5;
    case 1: return 8;
    case 2: return 13;
    case 3: return 16;
    default: return 5;
  }
}

void setFontSize(uint8_t fontId) {
  if (fontId > 2) fontId = 0;
  currentFont = fontId;

  switch (currentFont) {
    case 0: LET_WIDTH = 5;  LET_HEIGHT = 8;  break;
    case 1: LET_WIDTH = 8;  LET_HEIGHT = 13; break;
    case 2: LET_WIDTH = 10; LET_HEIGHT = 16; break;
  }

  textBaseY = (matrixHeight - LET_HEIGHT) / 2;

  int16_t maxOffset = matrixHeight - textBaseY - LET_HEIGHT;
  int16_t minOffset = -textBaseY;
  if (textYOffset < minOffset) textYOffset = minOffset;
  if (textYOffset > maxOffset) textYOffset = maxOffset;

  offset = matrixWidth + 10;
  loadingFlag = true;
}

int getTextPixelWidth(const char* str) {
  int width = 0;
  for (int i = 0; str[i] != '\0'; i++) {
    width += LET_WIDTH + 1;
  }
  width -= 1;
  return width;
}

uint8_t getOldVerticalByte(uint16_t glyphIdx, uint8_t row) {
  uint8_t result = 0;
  for (uint8_t bit = 0; bit < 5; bit++) {
    uint8_t vertByte = pgm_read_byte(&font5x8[glyphIdx][bit]);
    if (vertByte & (1 << (7 - row))) {
      result |= (1 << (7 - bit));
    }
  }
  return result;
}

CRGB getRunTextColor() {
  if (rainbowText) {
    static uint8_t rainbowOffset = 0;
    static uint32_t lastRainbowUpdate = 0;
    if (millis() - lastRainbowUpdate >= 120) {
      lastRainbowUpdate = millis();
      rainbowOffset = (rainbowOffset + 1) % 256;
    }
    uint8_t hue = (ColorRunningText + rainbowOffset) % 256;
    return CHSV(hue, 255, 255);
  } else if (runTextColorCycle) {
    uint8_t hue = (millis() / 35) % 256;
    return CHSV(hue, 255, 255);
  } else {
    return CHSV(ColorRunningText, 255, 255);
  }
}

void drawChar(int16_t x, int16_t y, uint8_t high, uint8_t low, CRGB color, uint8_t fontId) {
  if (fontId == 0) {
    // шрифт 3x5
    if (low >= '0' && low <= '9') {
      uint8_t num = low - '0';
      for (uint8_t col = 0; col < 3; col++) {
        uint8_t byte = pgm_read_byte(&clockFont3x5[num][col]);
        for (uint8_t row = 0; row < 5; row++) {
          if (byte & (1 << row)) {
            drawPixelXY(x + col, y + row, color);
          }
        }
      }
      return;
    }
    // символы погоды (минус, плюс, градус, C)
    uint8_t symIdx = 255;
    if (low == '-') symIdx = 0;
    else if (low == '+') symIdx = 1;
    else if (high == 0xC2 && low == 0xB0) symIdx = 2; // °
    else if (low == 'C' || low == 'c') symIdx = 3;
    if (symIdx < 4) {
      for (uint8_t col = 0; col < 3; col++) {
        uint8_t byte = pgm_read_byte(&weatherSym3x5[symIdx][col]);
        for (uint8_t row = 0; row < 5; row++) {
          if (byte & (1 << row)) {
            drawPixelXY(x + col, y + row, color);
          }
        }
      }
    }
    return;
  }

  uint8_t width  = getFontWidth(fontId);
  uint8_t height = getFontHeight(fontId);
  uint16_t symIdx = 0;

  if (low < 128) {
    symIdx = low - 32;
  } else if (high == 0xC2 && low == 0xB0) {
    drawDegreeSymbol(x, y, color, fontId);
    return;
  } else {
    return;
  }

  if (symIdx >= 200) return;

  const void* fontData = nullptr;
  if (fontId == 1) fontData = font5x8;
  else if (fontId == 2) fontData = font8x13;
  else if (fontId == 3) fontData = font10x16;
  if (!fontData) return;

  int firstCol = width;
  int lastCol  = -1;
  for (uint8_t col = 0; col < width; col++) {
    uint16_t vertWord = 0;
    if (fontId == 1) {
      vertWord = pgm_read_byte(&((const uint8_t*)fontData)[symIdx * width + col]);
    } else {
      vertWord = pgm_read_word(&((const uint16_t*)fontData)[symIdx * width + col]);
    }
    bool hasBit = false;
    for (uint8_t row = 0; row < height; row++) {
      if (vertWord & (1 << (height - 1 - row))) {
        hasBit = true;
        break;
      }
    }
    if (hasBit) {
      if (col < firstCol) firstCol = col;
      if (col > lastCol)  lastCol  = col;
    }
  }

  if (lastCol < 0) return;

  for (uint8_t col = firstCol; col <= lastCol; col++) {
    uint16_t vertWord = 0;
    if (fontId == 1) {
      vertWord = pgm_read_byte(&((const uint8_t*)fontData)[symIdx * width + col]);
    } else {
      vertWord = pgm_read_word(&((const uint16_t*)fontData)[symIdx * width + col]);
    }
    for (uint8_t row = 0; row < height; row++) {
      bool bit = (vertWord & (1 << (height - 1 - row))) != 0;
      if (bit) {
        int16_t px = x + (col - firstCol);
        int16_t py = y + row;
        if (px >= 0 && px < matrixWidth && py >= 0 && py < matrixHeight) {
          leds[XY(px, py)] = color;
        }
      }
    }
  }
}

void drawDegreeSymbol(int16_t x, int16_t y, CRGB color, uint8_t fontId) {
  if (fontId == 0) {
    for (uint8_t col = 0; col < 3; col++) {
      uint8_t byte = pgm_read_byte(&weatherSym3x5[2][col]);
      for (uint8_t row = 0; row < 5; row++) {
        if (byte & (1 << row)) {
          drawPixelXY(x + col, y + row, color);
        }
      }
    }
  }
  else if (fontId == 1) { // 5x8
    const int8_t points[6][2] = {{1, 0}, {0, 1}, {2, 1}, {0, 2}, {2, 2}, {1, 3}};
    for (uint8_t i = 0; i < 6; i++) {
      int16_t px = x + points[i][0];
      int16_t py = y + points[i][1] + 0;
      if (px >= 0 && px < matrixWidth && py >= 0 && py < matrixHeight)
        leds[XY(px, py)] = color;
    }
  }
  else if (fontId == 2) { // 8x13
    const int8_t points[20][2] = {{2, 0}, {3, 0}, {2, 1}, {3, 1}, {0, 2}, {1, 2}, {4, 2}, {5, 2}, {0, 3}, {1, 3}, {4, 3}, {5, 3}, {0, 4}, {1, 4}, {4, 4}, {5, 4}, {2, 5}, {3, 5}, {2, 6}, {3, 6}};
    for (uint8_t i = 0; i < 20; i++) {
      int16_t px = x + points[i][0];
      int16_t py = y + points[i][1];
      if (px >= 0 && px < matrixWidth && py >= 0 && py < matrixHeight)
        leds[XY(px, py)] = color;
    }
  }
  else if (fontId == 3) { // 10x16
    const int8_t points[32][2] = {{2, 0}, {3, 0}, {4, 0}, {2, 1}, {3, 1}, {4, 1}, {0, 2}, {1, 2}, {5, 2}, {6, 2}, {0, 3}, {1, 3}, {5, 3}, {6, 3}, {0, 4}, {1, 4}, {5, 4}, {6, 4}, {0, 5}, {1, 5}, {5, 5}, {6, 5}, {0, 6}, {1, 6}, {5, 6}, {6, 6}, {2, 7}, {3, 7}, {4, 7}, {2, 8}, {3, 8}, {4, 8}};
    for (uint8_t i = 0; i < 32; i++) {
      int16_t px = x + points[i][0];
      int16_t py = y + points[i][1];
      if (px >= 0 && px < matrixWidth && py >= 0 && py < matrixHeight)
        leds[XY(px, py)] = color;
    }
  }
}

void drawStaticString(int16_t x, int16_t y, const char* str, CRGB color, uint8_t fontId) {
  uint8_t width = getFontWidth(fontId);
  uint8_t spacing = 1;
  int16_t cursorX = x;

  size_t pos = 0;
  while (str[pos] != '\0') {
    uint8_t high = 0;
    uint8_t low = (uint8_t)str[pos];

    if (low < 128) {
      pos++;
    } else if ((low & 0xE0) == 0xC0) {
      high = low;
      pos++;
      if (str[pos] == '\0') break;
      low = (uint8_t)str[pos];
      pos++;
    } else {
      pos++;
      continue;
    }

    if (cursorX + width > 0 && cursorX < matrixWidth) {
      drawChar(cursorX, y, high, low, color, fontId);
    }

    cursorX += width + spacing;
  }
}

void drawLetter(uint8_t high, uint8_t low, int16_t xPos, CRGB letterColor, CRGB bgColor) {
  if (xPos <= -LET_WIDTH || xPos >= matrixWidth) return;
  int startCol = max(0, -xPos);
  int endCol = min((int)LET_WIDTH, (int)matrixWidth - xPos);
  if (endCol <= startCol) return;

  uint16_t symIdx = 0;

  if (low < 128) {
    symIdx = low - 32;
  } else if (high == 0xC2 && low == 0xB0) {
    drawDegreeSymbol(xPos, letterColor);
    return;
  } else if (high == 0xD0) {
    if (low >= 0x90 && low <= 0xBF) symIdx = (low - 0x90) + 95;
    else if (low == 0x81) symIdx = 159;
    else if (low == 0x84) symIdx = 160;
    else if (low == 0x86) symIdx = 161;
    else if (low == 0x87) symIdx = 162;
    else return;
  } else if (high == 0xD1) {
    if (low >= 0x80 && low <= 0x8F) symIdx = (low - 0x80) + 143;
    else if (low == 0x91) symIdx = 163;
    else if (low == 0x94) symIdx = 164;
    else if (low == 0x96) symIdx = 165;
    else if (low == 0x97) symIdx = 166;
    else return;
  } else {
    return;
  }

  if (currentFont == FONT_SMALL) {
    if (symIdx >= (sizeof(font5x8) / sizeof(font5x8[0]))) return;
  } else {
    if (symIdx >= 200) return;
  }

  int16_t left = xPos + startCol;
  int16_t top = textBaseY + textYOffset;
  int16_t right = xPos + endCol - 1;
  int16_t bottom = top + LET_HEIGHT - 1;

  if (!runTextOver) {
    for (int16_t px = left; px <= right; px++) {
      if (px < 0 || px >= matrixWidth) continue;
      for (int16_t py = top; py <= bottom && py < matrixHeight; py++) {
        if (py >= 0) {
          leds[XY(px, py)] = CRGB(0, 0, 0);
        }
      }
    }
  }

  for (int col = startCol; col < endCol; col++) {
    uint16_t vertWord = 0;
    if (currentFont == FONT_SMALL) vertWord = pgm_read_byte (&font5x8 [symIdx][col]);
    else if (currentFont == FONT_MEDIUM) vertWord = pgm_read_word (&font8x13 [symIdx][col]);
    else vertWord = pgm_read_word (&font10x16 [symIdx][col]);

    for (uint8_t row = 0; row < LET_HEIGHT; row++) {
      bool bit = (vertWord & (1 << (LET_HEIGHT - 1 - row))) != 0;
      if (MIRR_H) bit = !bit;

      int16_t px = xPos + col;
      int16_t py = textBaseY + textYOffset + row;

      if (px >= 0 && px < matrixWidth && py >= 0 && py < matrixHeight) {
        if (bit) {
          leds[XY(px, py)] = letterColor;
        }
      }
    }
  }
}

// для погоды символ градуса ° (когда вывод на матрицу по интервалу)
void drawDegreeSymbol(int16_t xPos, CRGB letterColor) {
  int16_t baseY = textBaseY + textYOffset;
  int16_t startX = xPos;
  int16_t startY;

  if (currentFont == FONT_SMALL) {
    // для шрифта 5x8
    startY = baseY + 4;
    const int8_t points[6][2] = {{1, 0}, {0, 1}, {2, 1}, {0, 2}, {2, 2}, {1, 3}};
    /*
       0 [ ][X][ ]
       1 [X][ ][X]
       2 [X][ ][X]
       3 [ ][X][ ]
    */

    for (uint8_t i = 0; i < 6; i++) {
      int16_t x = startX + points[i][0];
      int16_t y = startY + points[i][1];
      if (x >= 0 && x < matrixWidth && y >= 0 && y < matrixHeight) leds[XY(x, y)] = letterColor;
    }
  }

  else if (currentFont == FONT_MEDIUM) {
    // для шрифта 8x13
    startY = baseY + 6;
    const int8_t points[20][2] = {{2, 0}, {3, 0}, {2, 1}, {3, 1}, {0, 2}, {1, 2}, {4, 2}, {5, 2}, {0, 3}, {1, 3}, {4, 3}, {5, 3}, {0, 4}, {1, 4}, {4, 4}, {5, 4}, {2, 5}, {3, 5}, {2, 6}, {3, 6}};
    /*
       0  [ ][ ][X][X][ ][ ]
       1  [ ][ ][X][X][ ][ ]
       2  [X][X]      [X][X]
       3  [X][X]      [X][X]
       4  [X][X]      [X][X]
       5  [ ][ ][X][X][ ][ ]
       6  [ ][ ][X][X][ ][ ]
    */
    for (uint8_t i = 0; i < 20; i++) {
      int16_t x = startX + points[i][0];
      int16_t y = startY + points[i][1];
      if (x >= 0 && x < matrixWidth && y >= 0 && y < matrixHeight) {
        leds[XY(x, y)] = letterColor;
      }
    }
  }
  else {
    // для шрифта 10x16
    startY = baseY + 7;
    const int8_t points[32][2] = {{2, 0}, {3, 0}, {4, 0}, {2, 1}, {3, 1}, {4, 1}, {0, 2}, {1, 2}, {5, 2}, {6, 2}, {0, 3}, {1, 3}, {5, 3}, {6, 3}, {0, 4}, {1, 4}, {5, 4}, {6, 4},  {0, 5}, {1, 5}, {5, 5}, {6, 5}, {0, 6}, {1, 6}, {5, 6}, {6, 6}, {2, 7}, {3, 7}, {4, 7}, {2, 8}, {3, 8}, {4, 8}};
    /*
       0  [ ][ ][X][X][X][ ][ ]
       1  [ ][ ][X][X][X][ ][ ]
       2  [X][X]         [X][X]
       3  [X][X]         [X][X]
       4  [X][X]         [X][X]
       5  [X][X]         [X][X]
       6  [X][X]         [X][X]
       7  [ ][ ][X][X][X][ ][ ]
       8  [ ][ ][X][X][X][ ][ ]
    */
    for (uint8_t i = 0; i < 32; i++) {
      int16_t x = startX + points[i][0];
      int16_t y = startY + points[i][1];
      if (x >= 0 && x < matrixWidth && y >= 0 && y < matrixHeight) leds[XY(x, y)] = letterColor;
    }
  }
}

// ============================================================================ ЯРКОСТЬ ================================================================
uint8_t getBrightnessForPrintTime() {
  if (!myTime.isTimeSet()) {
    return modes[currentMode].Brightness; // если время не синхронизировано - возвращаем текущую яркость эффекта
  }
  time_t t = getCurrentLocalTime();
  struct tm *ti = localtime(&t);
  uint16_t minutes = ti->tm_hour * 60 + ti->tm_min;
  bool isNight = (NIGHT_HOURS_START >= NIGHT_HOURS_STOP) ? (minutes >= NIGHT_HOURS_START || minutes <= NIGHT_HOURS_STOP) : (minutes >= NIGHT_HOURS_START && minutes <= NIGHT_HOURS_STOP);
  day_night = !isNight;
  return isNight ? NIGHT_HOURS_BRIGHTNESS : DAY_HOURS_BRIGHTNESS;
}

// ======================================================================= БЕГУЩАЯ СТРОКА ==============================================================
boolean fillString(const char* text, CRGB letterColor, boolean itsText) {
  if (!text || text[0] == '\0') return true;

  static uint32_t scrollTimer = 0;
  static int16_t textPixelWidth = 0;
  static uint16_t symbolIdx = 0;
  static uint8_t lastSpeed = 0;

  if (lastSpeed != SpeedRunningText) {
    lastSpeed = SpeedRunningText;
    scrollTimer = millis() - 1;
    offset = matrixWidth + 10;
    textPixelWidth = 0;
    symbolIdx = 0;
  }

  if (loadingFlag) {
    textBaseY = (matrixHeight - LET_HEIGHT) / 2 + textYOffset;
    textBaseY = constrain(textBaseY, 0, matrixHeight - LET_HEIGHT);
    offset = matrixWidth + 8;
    loadingFlag = false;
    textPixelWidth = 0;
    symbolIdx = 0;

    size_t pos = 0;
    while (text[pos] != '\0') {
      uint8_t high = 0;
      uint8_t low = (uint8_t)text[pos];
      if (low < 128) pos++;
      else if ((low & 0xE0) == 0xC0) {
        high = low; pos++;
        if (text[pos] == '\0') break;
        low = (uint8_t)text[pos]; pos++;
      } else pos++;
      textPixelWidth += LET_WIDTH + SPACE;
    }
    if (textPixelWidth > 0) textPixelWidth -= SPACE;

    if (!itsText) {
      if (!runTextOver) {
        FastLED.clear();
      } else {
        int y_start = textBaseY + textYOffset - 1;
        int y_end   = y_start + LET_HEIGHT + 2;
        for (int y = max(0, y_start); y <= min((int)matrixHeight - 1, y_end); y++) {
          for (int x = 0; x < matrixWidth; x++) {
            leds[XY(x, y)] = CRGB::Black;
          }
        }
      }
    }
  }

  uint32_t delayMs = getRunningTextDelayMs();
  if (millis() - scrollTimer >= delayMs) {
    scrollTimer = millis();
    offset--;
    // if (offset <= -textPixelWidth - 4) {
    if (offset <= -textPixelWidth) {
      offset = matrixWidth + 8;
      return true;
    }
  }

  if (!runTextOver) {
    FastLED.clear();
  } else {
    int y_start = textBaseY + textYOffset - 1;
    int y_end   = y_start + LET_HEIGHT + 2;
    for (int y = max(0, y_start); y <= min((int)matrixHeight - 1, y_end); y++) {
      for (int x = 0; x < matrixWidth; x++) {
        leds[XY(x, y)] = CRGB::Black;
      }
    }
  }

  CRGB bgColor = CHSV(ColorRunningText + 96, 255, 80);
  int16_t cursorX = offset;
  size_t pos = 0;
  uint16_t currentSymbol = symbolIdx;

  while (text[pos] != '\0') {
    uint8_t high = 0;
    uint8_t low = (uint8_t)text[pos];

    if (low < 128) {
      pos++;
    }
    else if ((low & 0xE0) == 0xC0) {
      high = low;
      pos++;
      if (text[pos] == '\0') break;
      low = (uint8_t)text[pos];
      pos++;
    } else {
      pos++;
      continue;
    }

    if (cursorX + LET_WIDTH > 0 && cursorX < (int16_t)matrixWidth) {
      CRGB currentColor;
      if (itsText) {
        currentColor = letterColor;
      } else {
        if (rainbowText) {
          static uint8_t baseHue = 0;
          static uint32_t lastUpdate = 0;
          if (millis() - lastUpdate >= 80) {
            lastUpdate = millis();
            baseHue = (baseHue + 1) % 256;
          }
          uint8_t hue = (baseHue + currentSymbol * 8) % 256;
          currentColor = CHSV(hue, 255, 255);
        } else if (runTextColorCycle) {
          uint8_t hue = (millis() / 150) % 256;
          currentColor = CHSV(hue, 255, 255);
        } else {
          currentColor = CHSV(ColorRunningText, 255, 255);
        }
      }
      drawLetter(high, low, cursorX, currentColor, bgColor);
    }

    currentSymbol++;
    symbolIdx++;

    int16_t addSpace = SPACE;
    if (high == 0xC2 && low == 0xB0) {
      addSpace = SPACE - 1;
      if (addSpace < 0) addSpace = 0;
    }
    cursorX += LET_WIDTH + addSpace;
  }

  return false;
}

// -----------------------------------------------------------------
void showScrollingMessage(const char* message, CRGB color) {
  loadingFlag = true;
  textBaseY = (matrixHeight - LET_HEIGHT) / 2 + textYOffset;
  textBaseY = constrain(textBaseY, 0, matrixHeight - LET_HEIGHT);

  while (!fillString(message, color, true)) {
    FastLED.show();
    delay(getRunningTextDelayMs());
    yield();
  }
  FastLED.show();
}
// -----------------------------------------------------------------
uint32_t getRunningTextDelayMs() {
  return map(constrain(SpeedRunningText, 20, 220), 20, 220, 250, 15);
}

// ======================================================================== ВЫВОД ВРЕМЕНИ ==============================================================
// возвращает цвет для текста часов из настроек clock_hue
CRGB getClockTextColor() {
  if (rainbowClock) {
    static uint8_t clockRainbowOffset = 0;
    static uint32_t lastClockRainbowUpdate = 0;
    if (millis() - lastClockRainbowUpdate >= 60) {
      lastClockRainbowUpdate = millis();
      clockRainbowOffset = (clockRainbowOffset + 1) % 256;
    }
    uint8_t hue = (clockHue + clockRainbowOffset) % 256;
    return CHSV(hue, 220, 255);
  }
  else if (clockColorCycle) {
    uint8_t hue = (millis() / 35) % 256;
    return CHSV(hue, 220, 255);
  }
  else {
    return CHSV(clockHue, 220, 255);
  }
}

void printTime(bool onDemand) {
  if (isPrintingMessage) return;
  isPrintingMessage = true;

  if (!myTime.isTimeSet() && !onDemand) {
    isPrintingMessage = false;
    return;
  }

  static uint8_t lastMinuteChecked = 255;
  static uint8_t lastHourChecked = 255;
  bool needPrint = false;
  bool needAnnounce = false;

  if (onDemand) {
    needPrint = true;
    needAnnounce = true;
  } else if (PRINT_TIME > 0) {
    time_t t = getCurrentLocalTime();
    struct tm *ti = localtime(&t);
    uint8_t currentMinute = ti->tm_min;
    uint8_t currentHour = ti->tm_hour;

    if (PRINT_TIME >= 60) {
      if (currentMinute == 0 && lastHourChecked != currentHour) {
        needPrint = true;
        needAnnounce = true;
        lastHourChecked = currentHour;
      }
    } else {
      if (currentMinute % PRINT_TIME == 0 && lastMinuteChecked != currentMinute) {
        needPrint = true;
        needAnnounce = true;
        lastMinuteChecked = currentMinute;
      }
    }
  } else {
    isPrintingMessage = false;
    return;
  }

#if USE_MP3_PLAYER
  if (needAnnounce && mp3_player_connect == 4 && eff_sound_on && !isAnnouncing && !alarm_sound_flag && !sunset_sound_flag) {
    if (timeAnnounceEnabled && ((day_advert_sound_on && day_night) || (night_advert_sound_on && !day_night))) {
      previous_folder = mp3_folder;
      play_time_ADVERT();
    }
  }
#endif

  if (!(needPrint && runTimeTextEnabled)) {
    isPrintingMessage = false;
    return;
  }

  // вывод на матрицу
  time_t t = getCurrentLocalTime();
  struct tm *ti = localtime(&t);
  char displayStr[30];
  sprintf_P(displayStr, PSTR("%02d:%02d"), ti->tm_hour, ti->tm_min);

  bool wasOff = !ONflag;
  uint8_t oldBrightness = modes[currentMode].Brightness;
  bool oldRunTextEnabled = runTextEnabled;
  bool oldTextIsRunning = textIsRunning;
  bool oldLoadingFlag = loadingFlag;

  if (runTextEnabled) {
    runTextEnabled = false;
    runningTextTimer.setInterval(TIMER_DISABLED);
    textIsRunning = false;
  }

  loadingFlag = true;

  if (!ONflag) {
    FastLED.setBrightness(getBrightnessForPrintTime());
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
#endif
  }

  FastLED.clear();
  if (runTextOver) {
    int y_start = textBaseY + textYOffset - 1;
    int y_end   = y_start + LET_HEIGHT + 2;
    for (int y = max(0, y_start); y <= min((int)matrixHeight - 1, y_end); y++) {
      for (int x = 0; x < matrixWidth; x++) {
        leds[XY(x, y)] = CRGB::Black;
      }
    }
  }

  while (!fillString(displayStr, getClockTextColor(), false)) {
    FastLED.show();
    delay(getRunningTextDelayMs());
    yield();
  }

  FastLED.show();

  if (wasOff) {
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif
    FastLED.clear();
    FastLED.show();
    FastLED.setBrightness(oldBrightness);
  }

  if (oldRunTextEnabled) {
    runTextEnabled = true;
    if (IntervalrunText == 0) {
      runningTextTimer.setInterval(TIMER_DISABLED);
      textIsRunning = true;
      loadingFlag = true;
    } else {
      uint32_t intervalMs = static_cast<uint32_t>(IntervalrunText * 60000UL);
      runningTextTimer.setInterval(intervalMs);
      textIsRunning = false;
      runningTextTimer.reset();
    }
  }

  loadingFlag = oldLoadingFlag;
  isPrintingMessage = false;
}

// =========================================================================== ВЫВОД ПОГОДЫ ============================================================
#if LED_PANEL && USE_WEATHER
// возвращает цвет для текста погоды из настроек weather_hue
CRGB getWeatherTextColor() {
  if (rainbowWeather) {
    static uint8_t globalOffset = 0;
    static uint32_t lastRainbowUpdate = 0;
    if (millis() - lastRainbowUpdate >= 60) {
      lastRainbowUpdate = millis();
      globalOffset = (globalOffset + 1) % 256;
    }
    uint8_t hue = (weatherHue + globalOffset) % 256;
    return CHSV(hue, 220, 255);
  } else if (weatherColorCycle) {
    uint8_t hue = (millis() / 35) % 256;
    return CHSV(hue, 220, 255);
  } else {
    return CHSV(weatherHue, 220, 255);
  }
}

void printWeather() {
  if (isPrintingMessage) return;
  isPrintingMessage = true;

  if (!Weather::instance().isAvailable()) {
    isPrintingMessage = false;
    return;
  }

  static uint32_t lastCall = 0;
  if (millis() - lastCall < 1000) {
    isPrintingMessage = false;
    return;
  }
  lastCall = millis();

  static uint8_t lastPrintMinute = 255;
  static uint8_t lastAnnounceMinute = 255;
  time_t t = getCurrentLocalTime();
  struct tm *ti = localtime(&t);
  uint8_t currentMinute = ti->tm_min;
  uint8_t currentHour = ti->tm_hour;
  bool needText = false;
  bool needAnnounce = false;

  if (weatherTextJustEnabled) {
    weatherTextJustEnabled = false;
    lastPrintMinute = currentMinute;
    lastAnnounceMinute = currentMinute;
    isPrintingMessage = false;
    return;
  }

  // бегущая строка
  if (PRINT_WEATHER > 0 && runWeatherTextEnabled) {
    if (PRINT_WEATHER >= 60) {
      if (currentMinute == 0 && lastPrintMinute != 0) {
        needText = true;
        lastPrintMinute = currentMinute;
      }
    } else if (currentMinute % PRINT_WEATHER == 0 && lastPrintMinute != currentMinute) {
      needText = true;
      lastPrintMinute = currentMinute;
    }
  }

  // автоматическая озвучка
  if (weatherSpeakEnabled && PRINT_WEATHER > 0) {
    if (PRINT_WEATHER >= 60) {
      if (currentMinute == 0 && lastAnnounceMinute != 0) needAnnounce = true;
    } else if (currentMinute % PRINT_WEATHER == 0 && lastAnnounceMinute != currentMinute) {
      needAnnounce = true;
    }
  }

#if USE_MP3_PLAYER
  if (needAnnounce) {
    if (mp3_player_connect >= 4 && !isAnnouncing && !alarm_sound_flag && !sunset_sound_flag && !advert_flag && !weather_advert_flag) {
      previous_folder = mp3_folder;
      play_weather(false);
      lastAnnounceMinute = currentMinute;
    }
  }
#endif

  if (!needText) {
    isPrintingMessage = false;
    return;
  }

  // вывод на матрицу
  int tempInt = round(Weather::instance().getTemperature());
  String condition = Weather::instance().getCondition();
  char weatherStr[64];

  if (condition.length() > 0) {
    if (tempInt == 0) snprintf(weatherStr, sizeof(weatherStr), "0°C %s", condition.c_str());
    else if (tempInt > 0) snprintf(weatherStr, sizeof(weatherStr), "+%d°C %s", tempInt, condition.c_str());
    else snprintf(weatherStr, sizeof(weatherStr), "%d°C %s", tempInt, condition.c_str());
  } else {
    if (tempInt == 0) snprintf(weatherStr, sizeof(weatherStr), "0°C");
    else if (tempInt > 0) snprintf(weatherStr, sizeof(weatherStr), "+%d°C", tempInt);
    else snprintf(weatherStr, sizeof(weatherStr), "%d°C", tempInt);
  }

  bool wasOff = !ONflag;
  uint8_t oldBrightness = modes[currentMode].Brightness;
  bool oldRunTextEnabled = runTextEnabled;
  bool oldTextIsRunning = textIsRunning;
  bool oldLoadingFlag = loadingFlag;

  if (runTextEnabled) {
    runTextEnabled = false;
    runningTextTimer.setInterval(TIMER_DISABLED);
    textIsRunning = false;
  }

  loadingFlag = true;

  if (!ONflag) {
    FastLED.setBrightness(getBrightnessForPrintTime());
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
#endif
  }

  FastLED.clear();

  if (runTextOver) {
    int y_start = textBaseY + textYOffset - 1;
    int y_end = y_start + LET_HEIGHT + 2;
    for (int y = max(0, y_start); y <= min((int)matrixHeight - 1, y_end); y++) {
      for (int x = 0; x < matrixWidth; x++) {
        leds[XY(x, y)] = CRGB::Black;
      }
    }
  }

  while (!fillString(weatherStr, getWeatherTextColor(), false)) {
    FastLED.show();
    delay(getRunningTextDelayMs());
    yield();
  }
  FastLED.show();

  if (wasOff) {
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif
    FastLED.clear();
    FastLED.show();
    FastLED.setBrightness(oldBrightness);
  }

  if (oldRunTextEnabled) {
    runTextEnabled = true;
    if (IntervalrunText == 0) {
      runningTextTimer.setInterval(TIMER_DISABLED);
      textIsRunning = true;
      loadingFlag = true;
    } else {
      uint32_t intervalMs = static_cast<uint32_t>(IntervalrunText * 60000UL);
      runningTextTimer.setInterval(intervalMs);
      textIsRunning = false;
      runningTextTimer.reset();
    }
  }
  loadingFlag = oldLoadingFlag;
  isPrintingMessage = false;
}

#endif // USE_WEATHER

// ******************************************************************************************************************************************************

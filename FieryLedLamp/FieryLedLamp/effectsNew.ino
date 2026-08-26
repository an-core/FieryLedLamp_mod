// *************************************************************************** effectsNew.ino **********************************************************
#include "Time.h"
#include "Prototypes.h"
#include "Extern.h"
#include "data7x15flip.h" // FeatherCandle animation data (эффект Свеча)
// ----------------------------------------------------------------------

static const uint8_t exp_gamma[256] PROGMEM = {
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,
  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
  1,   2,   2,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,
  4,   4,   4,   4,   4,   5,   5,   5,   5,   5,   6,   6,   6,   7,   7,
  7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  10,  11,  11,  12,  12,
  12,  13,  13,  14,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,
  19,  20,  20,  21,  21,  22,  23,  23,  24,  24,  25,  26,  26,  27,  28,
  28,  29,  30,  30,  31,  32,  32,  33,  34,  35,  35,  36,  37,  38,  39,
  39,  40,  41,  42,  43,  44,  44,  45,  46,  47,  48,  49,  50,  51,  52,
  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,
  68,  70,  71,  72,  73,  74,  75,  77,  78,  79,  80,  82,  83,  84,  85,
  87,  89,  91,  92,  93,  95,  96,  98,  99,  100, 101, 102, 105, 106, 108,
  109, 111, 112, 114, 115, 117, 118, 120, 121, 123, 125, 126, 128, 130, 131,
  133, 135, 136, 138, 140, 142, 143, 145, 147, 149, 151, 152, 154, 156, 158,
  160, 162, 164, 165, 167, 169, 171, 173, 175, 177, 179, 181, 183, 185, 187,
  190, 192, 194, 196, 198, 200, 202, 204, 207, 209, 211, 213, 216, 218, 220,
  222, 225, 227, 229, 232, 234, 236, 239, 241, 244, 246, 249, 251, 253, 254,
  255
};

// ----------------------------------------------------------------------------------------------------------------------------------------------------

// ========================================================================== ВИНО ====================================================================
//    © SlingMaster | by Alex Dovby
// =======================================

void colorsWine() {
  uint8_t divider;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(20U + random8(200U), 200U);
    }
#endif
    loadingFlag = false;
    fillAll(CHSV(55U, 255U, 65U));
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    // minspeed 230 maxspeed 250 ============
    // minscale  40 maxscale  75 ============
    // красное вино hue > 0 & <=10
    // розовое вино hue > 10 & <=20
    // белое вино   hue > 20U & <= 40
    // шампанское   hue > 40U & <= 60

    deltaHue2 = 0U;      // count для замедления смены цвета
    step = deltaValue;   // чтообы при старте эффекта сразу покрасить лампу
    deltaHue = 1U;       // direction | 0 hue-- | 1 hue++ |
    hue = 55U;           // Start Color
    hue2 = 65U;          // Brightness
    pcnt = 0;
  }

  deltaHue2++;
  divider = 5 - floor((modes[currentMode].Scale - 1) / 20);
  if (hue >= 10 && hue2 < 100U) {
    hue2++;
  }

  if (hue < 10 && hue2 > 40U) {
    hue2--;
  }

  // изменение цвета вина -----
  if (deltaHue == 1U) {
    if (deltaHue2 % divider == 0) {
      hue++;
    }
  } else {
    if (deltaHue2 % divider == 0) {
      hue--;
    }
  }

  // сдвигаем всё вверх -----------
  for (uint8_t x = 0U; x < matrixWidth; x++) {
    for (uint8_t y = matrixHeight; y > 0U; y--) {
      drawPixelXY(x, y, getPixColorXY(x, y - 1U));
    }
  }

  if (hue > 40U) {
    pcnt = random(0, matrixWidth);
  } else {
    pcnt = 0;
  }

  for (uint8_t x = 0U; x < matrixWidth; x++) {
    if ((x == pcnt) && (pcnt > 0)) {
      drawPixelXY(x, 0U, CHSV(hue, 150U, hue2 + 20U + random(0, 50U)));
    } else {
      drawPixelXY(x, 0U, CHSV(hue, 255U, hue2));
    }
  }

  if  (hue == 0U) {
    deltaHue = 1U;
  }
  if (hue == 60U) {
    deltaHue = 0U;
  }
  step++;
}

// =============================================================================== ВЕРЕТЕНО ===========================================================
//             © SlingMaster
//          adapted © alvikskor
// =====================================

void Spindle() {
  static bool dark;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random8(1U, 100U), random8(100U, 255U));
    }
#endif
    loadingFlag = false;
    hue = random8(8) * 32;
    hue2 = 255U;
    dark = modes[currentMode].Scale < 76U;
  }

  if  (modes[currentMode].Scale < 81) {
    blurScreen(128U);
  } else if  (modes[currentMode].Scale < 86) {
    blurScreen(96U);
  } else if  (modes[currentMode].Scale < 91) {
    blurScreen(64U);
  } else if  (modes[currentMode].Scale < 96) {
    blurScreen(32U);
  }

  // <==== scroll =====
  for (uint8_t y = 0U ; y < matrixHeight; y++) {
    for (uint8_t x = 0U ; x < matrixWidth - 1; x++) {
      hue2--;
      if (dark) {   // black delimiter -----
        drawPixelXY(matrixWidth - 1, y, CHSV(hue, 255, hue2));
      } else {      // white delimiter -----
        drawPixelXY(matrixWidth - 1, y, CHSV(hue, 64 + hue2 / 2, 255 - hue2 / 4));
      }
      drawPixelXY(x, y,  getPixColorXY(x + 1,  y));
    }
  }
  if (modes[currentMode].Scale < 56) {

    return;
  }
  if (modes[currentMode].Scale < 61) {
    hue += 1;
  } else if (modes[currentMode].Scale < 66) {
    hue += 2;
  } else if (modes[currentMode].Scale < 71) {
    hue += 3;
  } else if (modes[currentMode].Scale < 76) {
    hue += 4;
  } else {
    hue += 3;
  }
}

// ======================================================================== ЗАВИТОК ===================================================================
//    © SlingMaster | by Alex Dovby
// ========================================

void Swirl() {
  static uint8_t lastHue = 0;
  if (modes[currentMode].Scale > 50) {
    Spindle();
    return;
  }

  uint8_t scaleVal = constrain(modes[currentMode].Scale, 1, 255);
  uint8_t divider = ((scaleVal - 1) / 20) % 5;

  static const uint32_t colors[5][6] PROGMEM = {
    {CRGB::Blue, CRGB::DarkRed, CRGB::Aqua, CRGB::Magenta, CRGB::Gold, CRGB::Green},
    {CRGB::Yellow, CRGB::LemonChiffon, CRGB::LightYellow, CRGB::Gold, CRGB::Chocolate, CRGB::Goldenrod},
    {CRGB::Green, CRGB::DarkGreen, CRGB::LawnGreen, CRGB::SpringGreen, CRGB::Cyan, CRGB::Black},
    {CRGB::Blue, CRGB::DarkBlue, CRGB::MidnightBlue, CRGB::MediumSeaGreen, CRGB::MediumBlue, CRGB::DeepSkyBlue},
    {CRGB::Magenta, CRGB::Red, CRGB::DarkMagenta, CRGB::IndianRed, CRGB::Gold, CRGB::MediumVioletRed}
  };

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(50U + random8(190U), 250U);
    }
#endif
    loadingFlag = false;
    FastLED.clear();
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    step = 0;
    deltaHue2 = 0;
    deltaHue = 0;
    hue2 = 0;
    lastHue = 0;
  }

  if (step == 0) {
    uint32_t color = colors[divider][hue];
    uint16_t idx = XY(hue2, deltaHue2);
    if (idx < usedLeds) {
      leds[idx] = color;
    }

#if defined(custom_eff) && (custom_eff == 1)
    deltaHue2++;
#else
    if (hue2 % 2 == 0) {
      deltaHue2++;
    }
#endif
    hue2++;
    if (hue2 >= matrixWidth) {
      hue2 = 0;
    }

    if (deltaHue2 >= matrixHeight) {
      deltaHue2 = 0;
      hue2 = random8(matrixWidth - 2);
      uint8_t newHue = random8(6);
      if (newHue == lastHue) {
        newHue = (newHue + 1) % 6;
      }
      lastHue = newHue;
      hue = newHue;
    }

    blurScreen(4U + random8(8));
  }

  step++;
  if (step >= deltaValue) {
    step = 0;
  }
}

// =================================================================== СТРЕЛКИ ========================================================================
int8_t arrow_x[4], arrow_y[4], stop_x[4], stop_y[4];
uint8_t arrow_direction; // 0x01 - слева направо; 0x02 - снизу вверх; 0х04 - справа налево; 0х08 - сверху вниз
uint8_t arrow_mode, arrow_mode_orig;// 0 - по очереди все варианты
// 1 - по очереди от края до края экрана;
// 2 - одновременно по горизонтали навстречу к ентру, затем одновременно по вертикали навстречу к центру
// 3 - одновременно все к центру
// 4 - по два (горизонталь / вертикаль) все от своего края к противоположному, стрелки смещены от центра на 1/3
// 5 - одновременно все от своего края к противоположному, стрелки смещены от центра на 1/3
bool arrow_complete, arrow_change_mode;
uint8_t arrow_hue[4];
uint8_t arrow_play_mode_count[6]; // Сколько раз проигрывать полностью каждый режим если вариант 0 - текущий счетчик
uint8_t arrow_play_mode_count_orig[6]; // Сколько раз проигрывать полностью каждый режим если вариант 0 - исходные настройки

void arrowsRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    FastLED.clear();
    arrow_complete = false;

    arrow_mode = (arrow_mode_orig == 0 || arrow_mode_orig > 5) ? random8(1, 5) : arrow_mode_orig;
    arrow_play_mode_count_orig[0] = 0;
    arrow_play_mode_count_orig[1] = 4; // 4 фазы - все стрелки показаны по кругу один раз - переходить к следующему ->
    arrow_play_mode_count_orig[2] = 4; // 2 фазы - гориз к центру (1), затем верт к центру (2) - обе фазы повторить по 2 раза -> 4
    arrow_play_mode_count_orig[3] = 4; // 1 фаза - все к центру (1) повторить по 4 раза -> 4
    arrow_play_mode_count_orig[4] = 4; // 2 фазы - гориз к центру (1), затем верт к центру (2) - обе фазы повторить по 2 раза -> 4
    arrow_play_mode_count_orig[5] = 4; // 1 фаза - все сразу (1) повторить по 4 раза -> 4
    for (uint8_t i = 0; i < 6; i++) {
      arrow_play_mode_count[i] = arrow_play_mode_count_orig[i];
    }
    arrowSetupForMode(arrow_mode, true);
  }

  dimAll(160);
  CHSV color;

  // движение стрелки - cлева направо
  if ((arrow_direction & 0x01) > 0) {
    color = CHSV(arrow_hue[0], 255, modes[currentMode].Brightness);
    for (int8_t x = 0; x <= 4; x++) {
      for (int8_t y = 0; y <= x; y++) {
        if (arrow_x[0] - x >= 0 && arrow_x[0] - x <= stop_x[0]) {
          CHSV clr = (x < 4 || (x == 4 && y < 2)) ? color : CHSV(0, 0, 0);
          drawPixelXY(arrow_x[0] - x, arrow_y[0] - y, clr);
          drawPixelXY(arrow_x[0] - x, arrow_y[0] + y, clr);
        }
      }
    }
    arrow_x[0]++;
  }

  // движение стрелки - cнизу вверх
  if ((arrow_direction & 0x02) > 0) {
    color = CHSV(arrow_hue[1], 255, modes[currentMode].Brightness);
    for (int8_t y = 0; y <= 4; y++) {
      for (int8_t x = 0; x <= y; x++) {
        if (arrow_y[1] - y >= 0 && arrow_y[1] - y <= stop_y[1]) {
          CHSV clr = (y < 4 || (y == 4 && x < 2)) ? color : CHSV(0, 0, 0);
          drawPixelXY(arrow_x[1] - x, arrow_y[1] - y, clr);
          drawPixelXY(arrow_x[1] + x, arrow_y[1] - y, clr);
        }
      }
    }
    arrow_y[1]++;
  }

  // движение стрелки - cправа налево
  if ((arrow_direction & 0x04) > 0) {
    color = CHSV(arrow_hue[2], 255, modes[currentMode].Brightness);
    for (int8_t x = 0; x <= 4; x++) {
      for (int8_t y = 0; y <= x; y++) {
        if (arrow_x[2] + x >= stop_x[2] && arrow_x[2] + x < matrixWidth) {
          CHSV clr = (x < 4 || (x == 4 && y < 2)) ? color : CHSV(0, 0, 0);
          drawPixelXY(arrow_x[2] + x, arrow_y[2] - y, clr);
          drawPixelXY(arrow_x[2] + x, arrow_y[2] + y, clr);
        }
      }
    }
    arrow_x[2]--;
  }

  // движение стрелки - cверху вниз
  if ((arrow_direction & 0x08) > 0) {
    color = CHSV(arrow_hue[3], 255, modes[currentMode].Brightness);
    for (int8_t y = 0; y <= 4; y++) {
      for (int8_t x = 0; x <= y; x++) {
        if (arrow_y[3] + y >= stop_y[3] && arrow_y[3] + y < matrixHeight) {
          CHSV clr = (y < 4 || (y == 4 && x < 2)) ? color : CHSV(0, 0, 0);
          drawPixelXY(arrow_x[3] - x, arrow_y[3] + y, clr);
          drawPixelXY(arrow_x[3] + x, arrow_y[3] + y, clr);
        }
      }
    }
    arrow_y[3]--;
  }

  switch (arrow_mode) {

    case 1:
      arrow_complete = false;
      switch (arrow_direction) {
        case 1: arrow_complete = arrow_x[0] > stop_x[0]; break;
        case 2: arrow_complete = arrow_y[1] > stop_y[1]; break;
        case 4: arrow_complete = arrow_x[2] < stop_x[2]; break;
        case 8: arrow_complete = arrow_y[3] < stop_y[3]; break;
      }

      arrow_change_mode = false;
      if (arrow_complete) {
        arrow_direction = (arrow_direction << 1) & 0x0F;
        if (arrow_direction == 0) arrow_direction = 1;
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[1]--;
          if (arrow_play_mode_count[1] == 0) {
            arrow_play_mode_count[1] = arrow_play_mode_count_orig[1];
            arrow_mode = random8(1, 5);
            arrow_change_mode = true;
          }
        }

        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;

    case 2:
      arrow_complete = false;
      switch (arrow_direction) {
        case 5: arrow_complete = arrow_x[0] > stop_x[0]; break;
        case 10: arrow_complete = arrow_y[1] > stop_y[1]; break;
      }

      arrow_change_mode = false;
      if (arrow_complete) {
        arrow_direction = arrow_direction == 5 ? 10 : 5;
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[2]--;
          if (arrow_play_mode_count[2] == 0) {
            arrow_play_mode_count[2] = arrow_play_mode_count_orig[2];
            arrow_mode = random8(1, 5);
            arrow_change_mode = true;
          }
        }

        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;

    case 3:
      if (matrixWidth >= matrixHeight)
        arrow_complete = arrow_x[0] > stop_x[0];
      else
        arrow_complete = arrow_y[1] > stop_y[1];

      arrow_change_mode = false;
      if (arrow_complete) {
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[3]--;
          if (arrow_play_mode_count[3] == 0) {
            arrow_play_mode_count[3] = arrow_play_mode_count_orig[3];
            arrow_mode = random8(1, 5);
            arrow_change_mode = true;
          }
        }

        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;

    case 4:
      switch (arrow_direction) {
        case 5: arrow_complete = arrow_x[0] > stop_x[0]; break;
        case 10: arrow_complete = arrow_y[1] > stop_y[1]; break;
      }

      arrow_change_mode = false;
      if (arrow_complete) {
        arrow_direction = arrow_direction == 5 ? 10 : 5;
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[4]--;
          if (arrow_play_mode_count[4] == 0) {
            arrow_play_mode_count[4] = arrow_play_mode_count_orig[4];
            arrow_mode = random8(1, 5);
            arrow_change_mode = true;
          }
        }

        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;

    case 5:
      if (matrixWidth >= matrixHeight)
        arrow_complete = arrow_x[0] > stop_x[0];
      else
        arrow_complete = arrow_y[1] > stop_y[1];

      arrow_change_mode = false;
      if (arrow_complete) {
        if (arrow_mode_orig == 0) {
          arrow_play_mode_count[5]--;
          if (arrow_play_mode_count[5] == 0) {
            arrow_play_mode_count[5] = arrow_play_mode_count_orig[5];
            arrow_mode = random8(1, 5);
            arrow_change_mode = true;
          }
        }
        arrowSetupForMode(arrow_mode, arrow_change_mode);
      }
      break;
  }
}

void arrowSetupForMode(uint8_t mode, bool change) {
  switch (mode) {
    case 1:
      if (change) arrow_direction = 1;
      arrowSetup_mode1();
      break;
    case 2:
      if (change) arrow_direction = 5;
      arrowSetup_mode2();
      break;
    case 3:
      if (change) arrow_direction = 15;
      arrowSetup_mode2();
      break;
    case 4:
      if (change) arrow_direction = 5;
      arrowSetup_mode4();
      break;
    case 5:
      if (change) arrow_direction = 15;
      arrowSetup_mode4();
      break;
  }
}
void arrowSetup_mode1() {
  // Слева направо
  if ((arrow_direction & 0x01) > 0) {
    arrow_hue[0] = random8();
    arrow_x[0] = 0;
    arrow_y[0] = matrixHeight / 2;
    stop_x [0] = matrixWidth + 7;
    stop_y [0] = 0;
  }
  // снизу вверх
  if ((arrow_direction & 0x02) > 0) {
    arrow_hue[1] = random8();
    arrow_y[1] = 0;
    arrow_x[1] = matrixWidth / 2;
    stop_y [1] = matrixHeight + 7;
    stop_x [1] = 0;
  }
  // справа налево
  if ((arrow_direction & 0x04) > 0) {
    arrow_hue[2] = random8();
    arrow_x[2] = matrixWidth - 1;
    arrow_y[2] = matrixHeight / 2;
    stop_x [2] = -7;
    stop_y [2] = 0;
  }
  // сверху вниз
  if ((arrow_direction & 0x08) > 0) {
    arrow_hue[3] = random8();
    arrow_y[3] = matrixHeight - 1;
    arrow_x[3] = matrixWidth / 2;
    stop_y [3] = -7;
    stop_x [3] = 0;
  }
}

void arrowSetup_mode2() {
  if ((arrow_direction & 0x01) > 0) {
    arrow_hue[0] = random8();
    arrow_x[0] = 0;
    arrow_y[0] = matrixHeight / 2;
    stop_x [0] = matrixWidth / 2 - 1;
    stop_y [0] = 0;
  }
  if ((arrow_direction & 0x02) > 0) {
    arrow_hue[1] = random8();
    arrow_y[1] = 0;
    arrow_x[1] = matrixWidth / 2;
    stop_y [1] = matrixHeight / 2 - 1;
    stop_x [1] = 0;
  }
  if ((arrow_direction & 0x04) > 0) {
    arrow_hue[2] = random8();
    arrow_x[2] = matrixWidth - 1;
    arrow_y[2] = matrixHeight / 2;
    stop_x [2] = matrixWidth / 2;
    stop_y [2] = 0;
  }
  if ((arrow_direction & 0x08) > 0) {
    arrow_hue[3] = random8();
    arrow_y[3] = matrixHeight - 1;
    arrow_x[3] = matrixWidth / 2;
    stop_y [3] = matrixHeight / 2;
    stop_x [3] = 0;
  }
}

void arrowSetup_mode4() {
  // Слева направо
  if ((arrow_direction & 0x01) > 0) {
    arrow_hue[0] = random8();
    arrow_x[0] = 0;
    arrow_y[0] = (matrixHeight / 3) * 2;
    stop_x [0] = matrixWidth + 7;
    stop_y [0] = 0;
  }
  // снизу вверх
  if ((arrow_direction & 0x02) > 0) {
    arrow_hue[1] = random8();
    arrow_y[1] = 0;
    arrow_x[1] = (matrixWidth / 3) * 2;
    stop_y [1] = matrixHeight + 7;
    stop_x [1] = 0;
  }
  // справа налево
  if ((arrow_direction & 0x04) > 0) {
    arrow_hue[2] = random8();
    arrow_x[2] = matrixWidth - 1;
    arrow_y[2] = matrixHeight / 3;
    stop_x [2] = -7;
    stop_y [2] = 0;
  }
  // сверху вниз
  if ((arrow_direction & 0x08) > 0) {
    arrow_hue[3] = random8();
    arrow_y[3] = matrixHeight - 1;
    arrow_x[3] = matrixWidth / 3;
    stop_y [3] = -7;
    stop_x [3] = 0;
  }
}

// ======================================================================= МАСЛЯНЫЕ КРАСКИ ============================================================
//      © SlingMaster | by Alex Dovby
// ===============================================

void OilPaints() {
  static uint16_t lastWidth = 0, lastHeight = 0;

  if (lastWidth != matrixWidth || lastHeight != matrixHeight) {
    lastWidth = matrixWidth;
    lastHeight = matrixHeight;
    loadingFlag = true;
  }

  uint8_t divider;
  uint8_t entry_point;
  uint16_t value;
  uint16_t max_val;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(1U + random8(252U), 1 + random8(219U));
    }
#endif
    loadingFlag = false;
    FastLED.clear();

    deltaValue = 255U - modes[currentMode].Speed + 1U;
    step = deltaValue;                      // чтобы при старте эффекта сразу покрасить лампу
    hue = floor(21.25 * (random8(11) + 1)); // начальный цвет
    deltaHue = hue - 22;                    // предыдущий цвет
    deltaHue2 = 80;                         // минимальная яркость

    if (matrixWidth < 16) {
      max_val = 1U << matrixWidth;
    } else {
      max_val = 65535U;
    }

    for (uint8_t x = 0; x < matrixWidth; x++) {
      trackingObjectHue[x] = 0;
    }
  }

  uint8_t centerY = matrixHeight / 2;
  if (matrixHeight % 2 == 0) centerY--;

  if (step >= deltaValue) {
    step = 0U;
  }

  if (step % centerY == 0) {
    divider = floor((modes[currentMode].Scale - 1) / 10);           // масштаб задаёт диапазон изменения цвета
    deltaHue = hue;                                                 // запомнить предыдущий цвет
    hue += 6 * divider;                                             // новый цвет
    hue2 = 255;                                                     // восстановить яркость
    deltaHue2 = 80 - floor(log(modes[currentMode].Brightness) * 6); // минимальная яркость

    entry_point = random8(matrixWidth);                             // стартовая позиция по X
    trackingObjectHue[entry_point] = hue;                           // установить цвет в стартовой точке
    drawPixelXY(entry_point, matrixHeight - 2, CHSV(hue, 255U, 255U));

    if (custom_eff == 1) {
      drawPixelXY(entry_point + 1, matrixHeight - 3, CHSV(hue + 30, 255U, 255U));
    }
  }

  if (random8(3) == 1) {
    // Движение влево
    for (uint8_t x = 1U; x < matrixWidth; x++) {
      if (trackingObjectHue[x] == hue) {
        trackingObjectHue[x - 1] = hue;
        break;
      }
    }
  } else {
    // Движение вправо
    for (uint8_t x = matrixWidth - 1; x > 0U; x--) {
      if (trackingObjectHue[x] == hue) {
        trackingObjectHue[x + 1] = hue;
        break;
      }
    }
  }

  for (uint8_t x = 0U; x < matrixWidth; x++) {
    uint8_t brightness = (trackingObjectHue[x] == hue) ? hue2 : deltaHue2;
    drawPixelXY(x, matrixHeight - 1, CHSV(trackingObjectHue[x], 255U, brightness));
  }

  if (hue2 > (deltaHue2 + 16)) {
    hue2 -= 16U;
  }

  if (matrixWidth < 16) {
    max_val = 1U << matrixWidth;
  } else {
    max_val = 65535U;
  }
  value = random16(max_val);
  for (uint8_t x = 0U; x < matrixWidth; x++) {
    if (bitRead(value, x) == 0) {
      for (uint8_t y = 0U; y < matrixHeight - 1; y++) {
        drawPixelXY(x, y, getPixColorXY(x, y + 1U));
      }
    }
  }

  step++;
}

// ============================================================================== РЕКИ БОТСВАНЫ =======================================================
//      © SlingMaster | by Alex Dovby
// ========================================

void flora() {
  uint32_t FLORA_COLOR = 0x2F1F00;
  uint8_t centerX_minor = (matrixWidth / 2) - ((matrixWidth - 1) & 0x01);
  uint8_t posX = floor(centerX_minor - matrixWidth * 0.3);
  uint8_t h = random8(matrixHeight - 6U) + 4U;

  DrawLine(posX + 1, 1U, posX + 1, h - 1, 0x000000);
  DrawLine(posX + 2, 1U, posX + 2, h, FLORA_COLOR);
  drawPixelXY(posX + 2, h - random8(floor(h * 0.5)), random8(2U) == 1 ? 0xFF00E0 : (random8(2U) == 1 ? 0xFFFF00 : 0x00FF00));
  drawPixelXY(posX + 1, h - random8(floor(h * 0.25)), random8(2U) == 1 ? 0xFF00E0 : 0xFFFF00);
  if (random8(2U) == 1) {
    drawPixelXY(posX + 1, floor(h * 0.5), random8(2U) == 1 ? 0xEF001F : 0x9FFF00);
  }
  h = floor(h * 0.65);
  if (matrixWidth > 8) {
    DrawLine(posX - 1, 1U, posX - 1, h - 1, 0x000000);
  }
  DrawLine(posX, 1U, posX, h, FLORA_COLOR);
  drawPixelXY(posX, h - random8(floor(h * 0.5)), random8(2U) == 1 ? 0xFF00E0 : 0xFFFF00);
}

// поднимающиеся пузырьки
void animeBobbles() {
  uint8_t centerX_major = matrixWidth / 2 + (matrixWidth % 2);
  // сдвигаем всё вверх
  for (uint8_t x = centerX_major; x < matrixWidth; x++) {
    for (uint8_t y = matrixHeight; y > 0U; y--) {
      if (getPixColorXY(x, y - 1) == 0xFFFFF7) {
        drawPixelXY(x, y, 0xFFFFF7);
        drawPixelXY(x, y - 1, getPixColorXY(0, y - 1));
      }
    }
  }
  if (step % 4 == 0) {
    drawPixelXY(centerX_major + random8(5), 0U, 0xFFFFF7);
    if (step % 12 == 0) {
      drawPixelXY(centerX_major + 2 + random8(3), 0U, 0xFFFFF7);
    }
  }
}

// вертикальная растяжка (для горизонтальной компоновки ленты)
void createScene(uint8_t idx) {
  uint8_t centerY_minor = (matrixHeight / 2) - ((matrixHeight - 1) & 0x01);
  switch (idx) {
    case 0:
      gradientDownTop(floor((matrixHeight - 1) * 0.5), CHSV(96, 255, 100), matrixHeight, CHSV(160, 255, 255));
      gradientDownTop(0, CHSV(96, 255, 255), centerY_minor, CHSV(96, 255, 100));
      break;
    case 1:
      gradientDownTop(floor((matrixHeight - 1) * 0.3), CHSV(96, 255, 100), matrixHeight, CHSV(130, 255, 220));
      gradientDownTop(0, CHSV(96, 255, 255), floor(matrixHeight * 0.3), CHSV(96, 255, 100));
      break;
    case 2:
      gradientDownTop(floor((matrixHeight - 1) * 0.5), CHSV(170, 255, 100), matrixHeight, CHSV(160, 255, 200));
      gradientDownTop(0, CHSV(100, 255, 255), centerY_minor, CHSV(170, 255, 100));
      break;
    case 3:
      gradientDownTop(floor((matrixHeight - 1) * 0.5), CHSV(95, 255, 55), matrixHeight, CHSV(70, 255, 200));
      gradientDownTop(0, CHSV(95, 255, 255), centerY_minor, CHSV(100, 255, 55));
      break;
    case 4:
      gradientDownTop(floor((matrixHeight - 1) * 0.3), CHSV(120, 255, 55), matrixHeight, CHSV(175, 255, 200));
      gradientDownTop(0, CHSV(120, 255, 255), floor(matrixHeight * 0.3), CHSV(120, 255, 55));
      break;
    default:
      gradientDownTop(floor((matrixHeight - 1) * 0.25), CHSV(180, 255, 85), matrixHeight, CHSV(160, 255, 200));
      gradientDownTop(0, CHSV(80, 255, 255), floor(matrixHeight * 0.25), CHSV(180, 255, 85));
      break;
  }
  flora();
}

// горизонтальная растяжка (для вертикальной компоновки ленты)
void createSceneM(uint8_t idx) {
  uint8_t centerY_minor = (matrixHeight / 2) - ((matrixHeight - 1) & 0x01);
  switch (idx) {
    case 0:
      gradientVertical(0, centerY_minor, matrixWidth, matrixHeight, 96, 150, 100, 255, 255U);
      gradientVertical(0, 0, matrixWidth, centerY_minor, 96, 96, 255, 100, 255U);
      break;
    case 1:
      gradientVertical(0, floor(matrixHeight * 0.3), matrixWidth, matrixHeight, 96, 120, 100, 220, 255U);
      gradientVertical(0, 0, matrixWidth, floor(matrixHeight * 0.3), 96, 96, 255, 100, 255U);
      break;
    case 2:
      gradientVertical(0, centerY_minor, matrixWidth, matrixHeight, 170, 160, 100, 200, 255U);
      gradientVertical(0, 0, matrixWidth, centerY_minor, 100, 170, 255, 100, 255U);
      break;
    case 3:
      gradientVertical(0, centerY_minor, matrixWidth, matrixHeight, 95, 65, 55, 200, 255U);
      gradientVertical(0, 0, matrixWidth, centerY_minor, 95, 100, 255, 55, 255U);
      break;
    case 4:
      gradientVertical(0, floor(matrixHeight * 0.3), matrixWidth, matrixHeight, 120, 160, 55, 200, 255U);
      gradientVertical(0, 0, matrixWidth, floor(matrixHeight * 0.3), 120, 120, 255, 55, 255U);
      break;
    default:
      drawRec(0, 0, matrixWidth, matrixHeight, 0x000050);
      break;
  }
  flora();
}

void BotswanaRivers() {
  static const bool ALT_GRADIENT = true;
  static uint16_t lastWidth = 0, lastHeight = 0;
  static uint8_t divider = 0;

  if (lastWidth != matrixWidth || lastHeight != matrixHeight) {
    lastWidth = matrixWidth;
    lastHeight = matrixHeight;
    loadingFlag = true;
  }

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(1U + random8(252U), 20 + random8(180U));
    }
#endif
    loadingFlag = false;
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    step = deltaValue;
    divider = floor((modes[currentMode].Scale - 1) / 20);
    if (ALT_GRADIENT) {
      createSceneM(divider);
    } else {
      createScene(divider);
    }
  }

  if (step >= deltaValue) {
    step = 0U;
  }

  if (getPixColorXY(0U, matrixHeight - 2) == CRGB::Black) {
    if (ALT_GRADIENT) {
      createSceneM(divider);
    } else {
      createScene(divider);
    }
  }

  if (!ALT_GRADIENT) {
    if (step % 2 == 0) {
      if (random8(6) == 1) {
        if (MatrixOrientation < 3 || MatrixOrientation == 7) {
          fill_gradient(leds, 0, CHSV(96U, 255U, 190U), random8(matrixWidth + random8(6)), CHSV(90U, 200U, 255U), SHORTEST_HUES);
        } else {
          fill_gradient(leds, usedLeds - random8(matrixWidth + random8(6)), CHSV(96U, 255U, 190U), usedLeds, CHSV(90U, 200U, 255U), SHORTEST_HUES);
        }
      } else {
        if (MatrixOrientation < 3 || MatrixOrientation == 7) {
          fill_gradient(leds, 0, CHSV(85U, 128U, 255U), random8(matrixWidth), CHSV(90U, 255U, 180U), SHORTEST_HUES);
        } else {
          fill_gradient(leds, usedLeds - random8(matrixWidth), CHSV(85U, 128U, 255U), usedLeds, CHSV(90U, 255U, 180U), SHORTEST_HUES);
        }
      }
    }
  }

  animeBobbles();
  if (custom_eff == 1) {
    blurRows(leds, matrixWidth, 3U, 10U);
  }
  step++;
}

// ========================================================================= АКВАРЕЛЬ =================================================================
//      © SlingMaster | by Alex Dovby
// =======================================

void SmearPaint(uint8_t obj[trackingOBJECT_MAX_COUNT]) {
  uint8_t divider;
  int temp;
  static const uint32_t colors[6][8] PROGMEM = {
    {0x2F0000,  0xFF4040, 0x6F0000, 0xAF0000, 0xff5f00, CRGB::Red, 0x480000, 0xFF0030},
    {0x002F00, CRGB::LawnGreen, 0x006F00, 0x00AF00, CRGB::DarkMagenta, 0x00FF00, 0x004800, 0x00FF30},
    {0x002F1F, CRGB::DarkCyan, 0x00FF7F, 0x007FFF, 0x20FF5F, CRGB::Cyan, 0x004848, 0x7FCFCF },
    {0x00002F, 0x5030FF, 0x00006F, 0x0000AF, CRGB::DarkCyan, 0x0000FF, 0x000048, 0x5F5FFF},
    {0x2F002F, 0xFF4040, 0x6F004A, 0xFF0030, CRGB::DarkMagenta, CRGB::Magenta, 0x480048, 0x3F00FF},
    {CRGB::Blue, CRGB::Red, CRGB::Gold, CRGB::Green, CRGB::DarkCyan, CRGB::DarkMagenta, 0x000000, 0xFF7F00 }
  };

  if (trackingObjectHue[5] == 1) {  // direction >>>
    obj[1]++;
    if (obj[1] >= obj[2]) {
      trackingObjectHue[5] = 0;     // swap direction
      obj[3]--;                     // new line
      if (step % 2 == 0) {
        obj[1]++;
      } else {
        obj[1]--;
      }
      obj[0]--;
    }
  } else {                          // direction <<<
    obj[1]--;
    if (obj[1] <= (obj[2] - obj[0])) {
      trackingObjectHue[5] = 1;     // swap direction
      obj[3]--;                     // new line
      if (obj[0] >= 1) {
        temp = obj[0] - 1;
        if (temp < 0) temp = 0;
        obj[0] = temp;
        obj[1]++;
      }
    }
  }

  if (obj[3] == 255) {
    deltaHue = 255;
  }

  divider = constrain((modes[currentMode].Scale - 1) / 16.7f, 0, 5);
  uint8_t colorIdx = constrain(hue, 0, 7);

  if (obj[1] >= matrixWidth || obj[3] == obj[4]) {
    deltaHue = 255;
  }

  uint8_t x = constrain(obj[1], 0, matrixWidth - 1);
  uint8_t y = constrain(obj[3], 0, matrixHeight - 1);

  CRGB color = pgm_read_dword(&colors[divider][colorIdx]);
  drawPixelXY(x, y, color);
}

//---------------------------------------
void Watercolor() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(1U + random8(252U), 1 + random8(250U));
    }
#endif
    loadingFlag = false;
    FastLED.clear();
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    step = deltaValue;
    hue = 0;
    deltaHue = 255; // last color
    trackingObjectHue[1] = (uint8_t)(matrixWidth * 0.25f);
    trackingObjectHue[3] = (uint8_t)(matrixHeight * 0.25f);
  }

  if (matrixWidth == 0 || matrixHeight == 0) return;

  if (step >= deltaValue) {
    step = 0U;
  }

  if (deltaHue == 255) {
    trackingObjectHue[0] = 4 + random8((uint8_t)(matrixWidth * 0.25f));
    trackingObjectHue[1] = random8(matrixWidth - trackingObjectHue[0]);
    int temp = trackingObjectHue[1] + trackingObjectHue[0];
    if (temp >= matrixWidth - 1) {
      temp = matrixWidth - 1;
      if (trackingObjectHue[1] > 1) {
        trackingObjectHue[1]--;
      } else {
        trackingObjectHue[1]++;
      }
    }
    trackingObjectHue[2] = (uint8_t)temp;
    if (matrixHeight > 4) {
      trackingObjectHue[3] = 3 + random8(matrixHeight - 4);
    } else {
      trackingObjectHue[3] = 1;
    }
    temp = trackingObjectHue[3] - random8(3) - 3;
    if (temp <= 0) {
      temp = 0;
    }
    trackingObjectHue[4] = (uint8_t)temp;
    trackingObjectHue[5] = 1;
    hue = random8(8);
    hue2 = 255;
    deltaHue = 0;
  }

  SmearPaint(trackingObjectHue);

  if (step % 2 == 0) {
    uint8_t blurAmount = beatsin8(1U, 1U, 6U);
    blurScreen(blurAmount);
  }
  step++;
}

// ======================================================================= СВЕЧА ======================================================================
//         адаптация © SottNick
//    github.com/mnemocron/FeatherCandle
//      modify & design © SlingMaster
// =========================================

const uint8_t  level = 160;
const uint8_t  low_level = 110;
const uint8_t *ptr  = anim;  // Current pointer into animation data
const uint8_t  w = 7;        // image width
const uint8_t  h = 15;       // image height
uint8_t img[w * h];          // Buffer for rendering image
uint8_t deltaX;              // position img (вычисляется при загрузке)
uint8_t last_brightness;

void FeatherCandleRoutine() {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    setModeSettings(1U + random8(99U), 190U + random8(65U));
  }
#endif

  if (loadingFlag) {
    FastLED.clear();
    hue = 0;
    trackingObjectState[0] = low_level;
    trackingObjectState[1] = low_level;
    trackingObjectState[2] = low_level;

    if (matrixWidth >= 7)
      deltaX = (matrixWidth / 2) - 3;
    else
      deltaX = 0;
    trackingObjectState[4] = matrixWidth / 2;
    loadingFlag = false;
  }

  uint8_t a = pgm_read_byte(ptr++);     // New frame X1/Y1
  if (a >= 0x90) {                      // EOD marker? (valid X1 never exceeds 8)
    ptr = anim;                         // Reset animation data pointer to start
    a   = pgm_read_byte(ptr++);         // and take first value
  }
  uint8_t x1 = a >> 4;                  // X1 = high 4 bits
  uint8_t y1 = a & 0x0F;                // Y1 = low 4 bits
  a  = pgm_read_byte(ptr++);            // New frame X2/Y2
  uint8_t x2 = a >> 4;                  // X2 = high 4 bits
  uint8_t y2 = a & 0x0F;                // Y2 = low 4 bits

  // Read rectangle of data from anim[] into portion of img[] buffer
  for (uint8_t y = y1; y <= y2; y++)
    for (uint8_t x = x1; x <= x2; x++) {
      img[y * w + x] = pgm_read_byte(ptr++);
    }

  uint8_t color = (modes[currentMode].Scale - 1U) * 2.57f;

  // draw flame -------------------
  for (uint8_t y = 1; y < h; y++) {
    if ((matrixHeight < 15) || (matrixWidth < 9)) {
      int16_t cx = matrixWidth / 2;
      if (y % 2 == 0) {
        if (cx - 1 >= 0 && cx - 1 < matrixWidth && 7 < matrixHeight)
          leds[XY(cx - 1, 7)] = CHSV(color, 255U, 55 + random8(200));
        if (cx < matrixWidth && 6 < matrixHeight)
          leds[XY(cx, 6)] = CHSV(color, 255U, 160 + random8(90));
        if (cx + 1 < matrixWidth && 6 < matrixHeight)
          leds[XY(cx + 1, 6)] = CHSV(color, 255U, 205 + random8(50));
        if (cx - 1 >= 0 && 5 < matrixHeight)
          leds[XY(cx - 1, 5)] = CHSV(color, 255U, 155 + random8(100));
        if (cx < matrixWidth && 5 < matrixHeight)
          leds[XY(cx, 5)] = CHSV(color - 10U, 255U, 120 + random8(130));
        if (cx < matrixWidth && 4 < matrixHeight)
          leds[XY(cx, 4)] = CHSV(color - 10U, 255U, 100 + random8(120));
        DrawLine(0, 2, matrixWidth - 1, 2, CRGB::Black);
      }
    } else {
      int i = (y - 1) * w;
      for (uint8_t x = 0; x < w; x++) {
        uint8_t brightness = img[i + x];
        uint16_t px = deltaX + x;
        if (px < matrixWidth && y < matrixHeight) {
          leds[XY(px, y)] = CHSV(brightness > 240 ? color : color - 10U, 255U, brightness);
        }
      }
    }

    // draw body FeatherCandle
    if (y <= 3) {
      if (y % 2 == 0) {
        gradientVertical(0, 0, matrixWidth, 2, color, color, 48, 128, 20U);
      }
    }

    // drops of wax move
    switch (hue) {
      case 0:
        if (trackingObjectState[0] < level) trackingObjectState[0]++;
        break;
      case 1:
        if (trackingObjectState[0] > low_level) trackingObjectState[0]--;
        if (trackingObjectState[1] < level) trackingObjectState[1]++;
        break;
      case 2:
        if (trackingObjectState[1] > low_level) trackingObjectState[1]--;
        if (trackingObjectState[2] < level) trackingObjectState[2]++;
        break;
      case 3:
        if (trackingObjectState[2] > low_level) {
          trackingObjectState[2]--;
        } else {
          hue++;
          // set random position drop of wax
          trackingObjectState[4] = (matrixWidth / 2) - 3 + random8(6);
          if (trackingObjectState[4] >= matrixWidth) trackingObjectState[4] = matrixWidth - 1;
          if (trackingObjectState[4] < 0) trackingObjectState[4] = 0;
        }
        break;
    }

    if (hue > 3) {
      hue++;
    } else {
      if (hue < 2) {
        if (trackingObjectState[4] < matrixWidth && 2 < matrixHeight)
          leds[XY(trackingObjectState[4], 2)] = CHSV(50U, 20U, trackingObjectState[0]);
      }
      if ((hue == 1) || (hue == 2)) {
        if (trackingObjectState[4] < matrixWidth && 1 < matrixHeight)
          leds[XY(trackingObjectState[4], 1)] = CHSV(50U, 15U, trackingObjectState[1]);
      }
      if (hue > 1) {
        if (trackingObjectState[4] < matrixWidth && 0 < matrixHeight)
          leds[XY(trackingObjectState[4], 0)] = CHSV(50U, 5U, trackingObjectState[2]);
      }
    }
  }
  // next -----------------
  if ((trackingObjectState[0] == level) || (trackingObjectState[1] == level) || (trackingObjectState[2] == level)) {
    hue++;
  }
}

// ===================================================================== ПЕСОЧНЫЕ ЧАСЫ ================================================================
//             © SlingMaster
// ===========================================

void Hourglass() {
  const float SIZE = 0.4f;
  const uint16_t h = floor(SIZE * matrixHeight);
  const uint16_t cx_left = (matrixWidth - 1) / 2;
  const uint16_t cx_right = matrixWidth / 2;
  const uint16_t route = matrixHeight - h - 1;
  const uint8_t STEP = 18U;
  static uint16_t pcnt = 0;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(15U + random8(225U), random8(255U));
    }
#endif
    loadingFlag = false;
    pcnt = 0;
    deltaHue2 = 0;
    hue2 = 0;

    FastLED.clear();
    hue = modes[currentMode].Scale * 2.55f;
    for (uint16_t x = 0; x < (matrixWidth / 2); x++) {
      for (uint16_t y = 0; y < h; y++) {
        uint8_t brightness = 255 - x * STEP;
        drawPixelXY(cx_left - x, matrixHeight - y - 1, CHSV(hue, 255, brightness));
        drawPixelXY(cx_right + x, matrixHeight - y - 1, CHSV(hue, 255, brightness));
      }
    }
  }

  if (hue2 == 0) {
    uint16_t posX = pcnt / 2;
    uint16_t posY = matrixHeight - h - pcnt;

    if ((posY < (matrixHeight - h - 2)) && (posY > deltaHue2)) {
      drawPixelXY(cx_right, posY, CHSV(hue, 255, 255));
      drawPixelXY(cx_right, posY - 2, CHSV(hue, 255, 255));
      drawPixelXY(cx_right, posY - 4, CHSV(hue, 255, 255));

      if (posY < (matrixHeight - h - 3)) {
        drawPixelXY(cx_right, posY + 1, CHSV(hue, 255, 0));
      }
    }

    if (pcnt % 2 == 0) {
      drawPixelXY(cx_left - posX, matrixHeight - deltaHue2 - 1, CHSV(hue, 255, 0));
      drawPixelXY(cx_left - posX, deltaHue2, CHSV(hue, 255, 255 - posX * STEP));
    } else {
      drawPixelXY(cx_right + posX, matrixHeight - deltaHue2 - 1, CHSV(hue, 255, 0));
      drawPixelXY(cx_right + posX, deltaHue2, CHSV(hue, 255, 255 - posX * STEP));
    }

    if (pcnt > matrixWidth - 1) {
      deltaHue2++;
      pcnt = 0;
      if (modes[currentMode].Scale > 95) {
        hue += 4U;
      }
    }

    pcnt++;
    if (deltaHue2 > h) {
      deltaHue2 = 0;
      hue2 = 1;
    }
  }

  if (hue2 > 0) {
    for (uint16_t x = 0; x < matrixWidth; x++) {
      for (uint16_t y = matrixHeight; y > 0; y--) {
        drawPixelXY(x, y, getPixColorXY(x, y - 1));
        drawPixelXY(x, y - 1, CRGB::Black);
      }
    }
    hue2++;
    if (hue2 > route) {
      hue2 = 0;
    }
  }
}

// ======================================================================== СПЕКТРУМ ==================================================================
//             © SlingMaster
//         source code © kostyamat
// ==========================================

void Spectrum() {
  static uint8_t customHue;
  static uint8_t hue2_local = 0;
  static uint8_t deltaHue_local = 0;
  static uint8_t deltaHue2_local = 0;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(1, 100U), random8(215, 255U));
    }
#endif
    loadingFlag = false;
    ff_y = map(matrixWidth, 8, 64, 310, 63);
    ff_z = ff_y;
    speedfactor = map(modes[currentMode].Speed, 1, 255, 32, 4);
    customHue = floor((modes[currentMode].Scale - 1U)) * 2.55f;
    FastLED.clear();
  }

  uint8_t color = customHue + hue;
  if (modes[currentMode].Scale >= 99) {
    if (hue2++ & 0x01 && deltaHue++ & 0x01 && deltaHue2++ & 0x01) hue += 8;
    fillMyPal16_2(customHue + hue, modes[currentMode].Scale & 0x01);
  } else {
    color = customHue;
    fillMyPal16_2(customHue + AURORA_COLOR_RANGE - beatsin8(AURORA_COLOR_PERIOD, 0U, AURORA_COLOR_RANGE * 2), modes[currentMode].Scale & 0x01);
  }

  for (uint16_t x = 0; x < matrixWidth; x++) {
    if (x % 2 == 0) {
      leds[XY(x, 0)] = CHSV(color, 255U, 128U);
    }

    float emitterX = ((random8(2) == 0U) ? 545.0f : 390.0f) / matrixHeight;
    for (uint16_t y = 2; y < matrixHeight - 1; y++) {
      polarTimer++;
      uint8_t noiseVal = inoise8(polarTimer % 2 + x * ff_z, y * 16 + polarTimer % 16, polarTimer / speedfactor);
      uint8_t sub = fabs((float)matrixHeight / 2.0f - (float)y) * emitterX;
      leds[XY(x, y)] = ColorFromPalette(myPal, qsub8(noiseVal, sub));
    }
  }
}

// ================================================================== СНЕГОПАД ========================================================================
void Snowfall() {
  static byte divider;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(100U), random8(100U, 190U));
    }
#endif
    loadingFlag = false;
    clearNoiseArr();

    divider = max(1U, min(4U, modes[currentMode].Scale / 25U));
  }

  if (divider == 1) {
    dimAll(40);
  }
  else if (divider == 2) {
    gradientVertical(0, 0, matrixWidth, matrixHeight, 160, 160, 255, 128, 255U);
  }
  else {
    FastLED.clear();
  }

  VirtualSnow(divider);
}

// метель - 2
#define e_sns_DENSE (32U)

void stormRoutine2()
{
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    uint8_t tmp = 175U + random8(39U);
    if (tmp & 0x01)
      setModeSettings(50U + random8(51U), tmp);
    else
      setModeSettings(50U + random8(24U), tmp);
  }
#endif

  bool isColored = modes[currentMode].Speed & 0x01;
  uint8_t Saturation = 0U;
  uint8_t e_TAIL_STEP = 127U;
  if (isColored)
    Saturation = modes[currentMode].Scale * 2.55;
  else
  {
    e_TAIL_STEP = 255U - modes[currentMode].Scale * 2.5;
  }
  for (uint8_t x = 0U; x < matrixWidth - 1U; x++)
  {
    if (!random8(e_sns_DENSE) && !getPixColorXY(wrapX(x), matrixHeight - 1U) && !getPixColorXY(wrapX(x + 1U), matrixHeight - 1U) && !getPixColorXY(wrapX(x - 1U), matrixHeight - 1U))
    {
      drawPixelXY(x, matrixHeight - 1U, CHSV(random8(), Saturation, random8(64U, 255U)));
    }
  }

  // сдвигаем по диагонали
  for (uint8_t y = 0U; y < matrixHeight - 1U; y++)
  {
    for (uint8_t x = 0; x < matrixWidth; x++)
    {
      drawPixelXY(wrapX(x + 1U), y, getPixColorXY(x, y + 1U));
    }
  }

  for (uint8_t i = 0U; i < matrixWidth; i++)
  {
    fadePixel(i, matrixHeight - 1U, e_TAIL_STEP);
  }
}

// ======================================================================= НОВОГОДНЯЯ ЁЛКА ============================================================
//             © SlingMaster
// ==========================================

void clearNoiseArr() {
  for (uint16_t x = 0; x < matrixWidth; x++) {
    for (uint16_t y = 0; y < matrixHeight; y++) {
      if (noise3d && noise3d[0] && noise3d[0][x]) noise3d[0][x][y] = 0;
      if (noise3d && noise3d[1] && noise3d[1][x]) noise3d[1][x][y] = 0;
    }
  }
}

void VirtualSnow(byte snow_type) {
  uint8_t posX = random8(matrixWidth - 1);
  static int deltaPos;
  byte delta = (snow_type == 3) ? 0 : 1;
  uint8_t centerY_major = matrixHeight / 2 + (matrixHeight % 2);

  for (uint8_t x = delta; x < matrixWidth - delta; x++) {
    if ((noise3d[0][x][matrixHeight - 2] == 0U) && (posX == x) && (random8(0, 2) == 0U)) {
      noise3d[0][x][matrixHeight - 1] = 1;
    } else {
      noise3d[0][x][matrixHeight - 1] = 0;
    }

    for (uint8_t y = 0U; y < matrixHeight - 1; y++) {
      switch (snow_type) {
        case 0:
          noise3d[0][x][y] = noise3d[0][x][y + 1];
          deltaPos = 0;
          break;
        case 1:
        case 2:
          noise3d[0][x][y] = noise3d[0][x][y + 1];
          deltaPos = 1 - random8(2);
          break;
        default:
          deltaPos = -1;
          if ((x == 0) && (y == 0) && (random8(2) == 0U)) {
            uint8_t randY = random8(centerY_major / 2, matrixHeight - centerY_major / 4);
            noise3d[0][matrixWidth - 1][randY] = 1;
          }
          if (x > matrixWidth - 2) {
            noise3d[0][matrixWidth - 1][y] = 0;
          }
          if (x < 1) {
            noise3d[0][x][y] = noise3d[0][x][y + 1];
          } else {
            noise3d[0][x - 1][y] = noise3d[0][x][y + 1];
          }
          break;
      }

      if (noise3d[0][x][y] > 0) {
        if (snow_type < 3) {
          if (y % 2 == 0U) {
            int16_t xn = x - ((x > 0) ? deltaPos : 0);
            if (xn >= 0 && xn < matrixWidth)
              leds[XY(xn, y)] = CHSV(160, 5U, random8(200U, 240U));
          } else {
            int16_t xn = x + deltaPos;
            if (xn >= 0 && xn < matrixWidth)
              leds[XY(xn, y)] = CHSV(160, 5U, random8(200U, 240U));
          }
        } else {
          leds[XY(x, y)] = CHSV(160, 5U, random8(200U, 240U));
        }
      }
    }
  }
}

void GreenTree(uint8_t tree_h) {
  hue = floor(step / 32) * 32U;
  for (uint8_t x = 0U; x < matrixWidth + 1; x++) {
    if (x % 8 == 0) {
      if (modes[currentMode].Scale < 60) {
        // зелёное дерево
        DrawLine(x - 1U - deltaValue, floor(tree_h * 0.70), x + 1U - deltaValue, floor(tree_h * 0.70), 0x002F00);
        DrawLine(x - 1U - deltaValue, floor(tree_h * 0.55), x + 1U - deltaValue, floor(tree_h * 0.55), 0x004F00);
        DrawLine(x - 2U - deltaValue, floor(tree_h * 0.35), x + 2U - deltaValue, floor(tree_h * 0.35), 0x005F00);
        DrawLine(x - 2U - deltaValue, floor(tree_h * 0.15), x + 2U - deltaValue, floor(tree_h * 0.15), 0x007F00);

        drawPixelXY(x - 3U - deltaValue, floor(tree_h * 0.15), 0x001F00);
        drawPixelXY(x + 3U - deltaValue, floor(tree_h * 0.15), 0x001F00);
        if ((x - deltaValue) >= 0) {
          gradientVertical(x - deltaValue, 0U, x - deltaValue, tree_h, 90U, 90U, 190U, 64U, 255U);
        }
      } else {
        // праздничное дерево
        drawPixelXY(x - 1 - deltaValue, floor(tree_h * 0.6), CHSV(step, 255U, 128 + random8(128)));
        drawPixelXY(x + 1 - deltaValue, floor(tree_h * 0.6), CHSV(step, 255U, 128 + random8(128)));

        drawPixelXY(x - deltaValue, floor(tree_h * 0.4), CHSV(step, 255U, 200U));

        drawPixelXY(x - deltaValue, floor(tree_h * 0.2), CHSV(step, 255U, 190 + random8(65)));
        drawPixelXY(x - 2 - deltaValue, floor(tree_h * 0.25), CHSV(step, 255U, 96 + random8(128)));
        drawPixelXY(x + 2 - deltaValue, floor(tree_h * 0.25), CHSV(step, 255U, 96 + random8(128)));

        drawPixelXY(x - 2 - deltaValue, 1U, CHSV(step, 255U, 200U));
        drawPixelXY(x - deltaValue, 0U, CHSV(step, 255U, 250U));
        drawPixelXY(x + 2 - deltaValue, 1U, CHSV(step, 255U, 200U));
        if ((x - deltaValue) >= 0) {
          gradientVertical(x - deltaValue, floor(tree_h * 0.75), x - deltaValue, tree_h, hue, hue, 250U, 0U, 128U);
        }
      }
    }
  }
}

void ChristmasTree() {
  static uint16_t lastWidth = 0, lastHeight = 0;
  static uint8_t tree_h = 0;

  if (lastWidth != matrixWidth || lastHeight != matrixHeight) {
    lastWidth = matrixWidth;
    lastHeight = matrixHeight;
    loadingFlag = true;
  }

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(100U), 10U + random8(128));
    }
#endif
    loadingFlag = false;
    clearNoiseArr();
    deltaValue = 0;
    step = deltaValue;
    FastLED.clear();

    if (matrixHeight > 16) {
      tree_h = 16;
    } else {
      tree_h = matrixHeight;
    }
  }

  FastLED.clear();

  GreenTree(tree_h);

  if (modes[currentMode].Scale < 60) {
    VirtualSnow(1);
  }
  if (modes[currentMode].Scale > 30) {
    deltaValue++;
  }
  if (deltaValue >= 8) {
    deltaValue = 0;
  }
  step++;
}

// ===================================================================== ПОБОЧНЫЙ ЭФФЕКТ ==============================================================
//             © SlingMaster
// ============================================

void ByEffect() {
  uint8_t saturation;
  uint8_t delta;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed 210
      setModeSettings(random8(100U), random8(200U));
    }
#endif
    loadingFlag = false;
    deltaValue = 0;
    step = deltaValue;
    FastLED.clear();
  }

  hue = floor(step / 32) * 32U;
  dimAll(180);

  saturation = 255U;
  delta = 0;
  for (uint8_t x = 0U; x < matrixWidth + 1 ; x++) {
    if (x % 8 == 0) {
      gradientVertical( x - deltaValue, floor(matrixHeight * 0.75), x + 1U - deltaValue, matrixHeight,  hue, hue + 2, 250U, 0U, 255U);
      if (modes[currentMode].Scale > 50) {
        delta = random8(200U);
      }
      drawPixelXY(x - 2 - deltaValue, floor(matrixHeight * 0.7), CHSV(step, saturation - delta, 128 + random8(128)));
      drawPixelXY(x + 2 - deltaValue, floor(matrixHeight * 0.7), CHSV(step, saturation, 128 + random8(128)));

      drawPixelXY(x - deltaValue, floor(matrixHeight * 0.6), CHSV(hue, 255U, 190 + random8(65)));
      if (modes[currentMode].Scale > 50) {
        delta = random8(200U);
      }
      drawPixelXY(x - 1 - deltaValue, CENTER_Y_MINOR, CHSV(step, saturation, 128 + random8(128)));
      drawPixelXY(x + 1 - deltaValue, CENTER_Y_MINOR, CHSV(step, saturation - delta, 128 + random8(128)));

      drawPixelXY(x - deltaValue, floor(matrixHeight * 0.4), CHSV(hue, 255U, 200U));
      if (modes[currentMode].Scale > 50) {
        delta = random8(200U);
      }
      drawPixelXY(x - 2 - deltaValue, floor(matrixHeight * 0.3), CHSV(step, saturation - delta, 96 + random8(128)));
      drawPixelXY(x + 2 - deltaValue, floor(matrixHeight * 0.3), CHSV(step, saturation, 96 + random8(128)));

      gradientVertical( x - deltaValue, 0U, x + 1U - deltaValue, floor(matrixHeight * 0.25),  hue + 2, hue, 0U, 250U, 255U);

      if (modes[currentMode].Scale > 50) {
        drawPixelXY(x + 3 - deltaValue, matrixHeight - 3U, CHSV(step, 255U, 255U));
        drawPixelXY(x - 3 - deltaValue, CENTER_Y_MINOR, CHSV(step, 255U, 255U));
        drawPixelXY(x + 3 - deltaValue, 2U, CHSV(step, 255U, 255U));
      }
    }
  }

  deltaValue++;
  if (deltaValue >= 8) {
    deltaValue = 0;
  }
  step++;
}

// =================================================================== СТРОБ, ХАОС, ДИФУЗИЯ ===========================================================
//             © SlingMaster
// =====================================

void StrobeAndDiffusion() {
  const uint8_t SIZE = 3U;
  const uint8_t DELTA = 1U;
  uint8_t STEP = 2U;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(1U + random8(100U), 1U + random8(150U));
    }
#endif
    loadingFlag = false;
    FPSdelay = 25U;
    hue2 = 1;
    FastLED.clear();
  }

  STEP = floor((255 - modes[currentMode].Speed) / 64) + 1U;

  if (modes[currentMode].Scale > 50) {
    // diffusion ---
    blurScreen(beatsin8(3, 64, 80));
    FPSdelay = LOW_DELAY;
    STEP = 1U;
    if (modes[currentMode].Scale < 75) {
      FPSdelay = 30;
      VirtualSnow(1);
    }
  } else {
    // strob -------
    if (modes[currentMode].Scale > 25) {
      dimAll(200);
      FPSdelay = 30;
    } else {
      dimAll(240);
      FPSdelay = 40;
    }
  }

  const uint8_t rows = (matrixHeight + 1) / 3U;
  deltaHue = floor(modes[currentMode].Speed / 64) * 64;
  bool dir = false;

  for (uint8_t y = 0; y < rows; y++) {
    uint8_t yy = y * 3 + DELTA;
    if (yy < matrixHeight) {
      if (dir) {
        if ((step % STEP) == 0) {
          drawPixelXY(matrixWidth - 1, yy, CHSV(step, 255U, 255U));
        } else {
          drawPixelXY(matrixWidth - 1, yy, CHSV(170U, 255U, 1U));
        }
      } else {
        if ((step % STEP) == 0) {
          drawPixelXY(0, yy, CHSV((step + deltaHue), 255U, 255U));
        } else {
          drawPixelXY(0, yy, CHSV(0U, 255U, 0U));
        }
      }
    }

    // сдвигаем слои
    if (dir) {  // <==
      for (uint16_t x = 1; x < matrixWidth; x++) {
        drawPixelXY(x - 1, yy, getPixColorXY(x, yy));
      }
    } else {    // ==>
      for (uint16_t x = 1; x < matrixWidth; x++) {
        drawPixelXY(matrixWidth - x, yy, getPixColorXY(matrixWidth - x - 1, yy));
      }
    }
    dir = !dir;
  }

  if (hue2 == 1) {
    step++;
    if (step >= 254) hue2 = 0;
  } else {
    step--;
    if (step < 1) hue2 = 1;
  }
}

// ========================================================================== ФЕЙЕРВЕРК ===============================================================
//             © SlingMaster
// =====================================

void VirtualExplosion(uint8_t f_type, int16_t timeline) {
  const uint8_t DELAY_SECOND_EXPLOSION = (uint8_t)(matrixHeight * 0.25f);
  uint8_t horizont = 1U;
  const int8_t STEP = 255 / matrixHeight;
  uint8_t firstColor = random8(255);
  uint8_t secondColor = 0;
  uint8_t saturation = 255U;
  switch (f_type) {
    case 0:
      secondColor = random(50U, 255U);
      saturation = random(245U, 255U);
      break;
    case 1: /* сакура */
      firstColor = random(210U, 230U);
      secondColor = random(65U, 85U);
      saturation = 255U;
      break;
    case 2: /* день Независимости */
      firstColor = random(160U, 170U);
      secondColor = random(25U, 50U);
      saturation = 255U;
      break;
    default: /* фризантемы */
      firstColor = random(30U, 40U);
      secondColor = random(25U, 50U);
      saturation = random(128U, 255U);
      break;
  }

  if ((timeline > matrixHeight - 1) && (timeline < (int16_t)(matrixHeight * 1.75f))) {
    // сдвиг шума вверх
    for (uint16_t x = 0; x < matrixWidth; x++) {
      for (uint16_t y = horizont; y < matrixHeight - 1; y++) {
        noise3d[0][x][y] = noise3d[0][x][y + 1];
        uint8_t bri = y * STEP;
        if (noise3d[0][x][y] > 0) {
          if (timeline > (matrixHeight + DELAY_SECOND_EXPLOSION)) {
            /* второй взрыв */
            drawPixelXY((x - 2 + random8(4)), y - 1, CHSV(secondColor + random8(16), saturation, bri));
          }
          if (timeline < (int16_t)((matrixHeight - DELAY_SECOND_EXPLOSION) * 1.75f)) {
            /* первый взрыв */
            drawPixelXY(x, y, CHSV(firstColor, 255U, bri));
          }
        }
      }
    }
    // случайная искра в верхней строке
    uint16_t posX = random8(matrixWidth);
    for (uint16_t x = 0; x < matrixWidth; x++) {
      if (posX == x) {
        noise3d[0][x][matrixHeight - 1] = (step % 2 == 0) ? 1 : 0;
      } else {
        noise3d[0][x][matrixHeight - 1] = 0;
      }
    }
  }
}

// --------------------------------------
void Firework() {
  const uint8_t MAX_BRIGHTNESS = 40U;
  const uint8_t DOT_EXPLOSION = (uint8_t)(matrixHeight * 0.95f);
  const uint8_t HORIZONT = (uint8_t)(matrixHeight * 0.25f);
  const uint8_t DELTA = 1U;
  const float stepH = matrixHeight / 128.0f;
  const uint8_t FPS_DELAY = 20U;
  const uint8_t STEP = 3U; // не используется
  const uint8_t skyColor = 156U;
  static uint8_t sizeH;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(1U + random8(100U), 1U + random8(250U));
    }
#endif
    loadingFlag = false;
    deltaHue2 = 0;
    FPSdelay = 255U;
    clearNoiseArr();
    FastLED.clear();
    step = 0U;
    deltaHue2 = floor(modes[currentMode].Scale / 26);
    hue = 48U;
    sizeH = matrixHeight;
    if (modes[currentMode].Speed > 85U) {
      sizeH = HORIZONT;
      FPSdelay = FPS_DELAY;
    }
    if (modes[currentMode].Speed <= 85U) {
      gradientVertical(0, 0, matrixWidth, matrixHeight, skyColor, skyColor, 96U, 0U, 255U);
    }
  }

  if (FPSdelay == 240U) {
    FPSdelay = FPS_DELAY;
  }

  if (FPSdelay > 230U) {
    /* вечереет */
    FPSdelay--;
    sizeH = (FPSdelay - 128U) * stepH;
    if (modes[currentMode].Speed <= 85U) {
      dimAll(225U);
      return;
    }
    if (sizeH > HORIZONT) {
      dimAll(200);
      return;
    }
    if (sizeH == HORIZONT) FPSdelay = FPS_DELAY;
  }

  if (step > DOT_EXPLOSION) {
    blurScreen(beatsin8(3, 64, 80));
  }
  if (step == DOT_EXPLOSION - 1) {
    FPSdelay = 70;
  }
  if (step > matrixHeight / 2) {
    dimAll(140);
  } else {
    dimAll(100);
  }

  if ((modes[currentMode].Speed > 85U) && (modes[currentMode].Speed < 180U)) {
    gradientVertical(0, 0, matrixWidth, HORIZONT, skyColor, skyColor, 48U, 0U, 255U);
  }

  VirtualExplosion(deltaHue2, step);

  if ((step > DOT_EXPLOSION) && (step < (int16_t)(matrixHeight * 1.5f))) {
    FPSdelay += 5U;
  }

  const uint8_t rows = (matrixHeight + 1) / 3U;
  deltaHue = floor(modes[currentMode].Speed / 64) * 64;

  if (step > matrixHeight / 2) {
    bool dir = false;
    for (uint8_t y = 0; y < rows; y++) {
      uint8_t yy = y * 3 + DELTA;
      if (yy < matrixHeight) {
        for (uint16_t x = 0; x < matrixWidth; x++) {
          if (dir) {  // <==
            if (x > 0)
              drawPixelXY(x - 1, yy, getPixColorXY(x, yy));
          } else {    // ==>
            if (x < matrixWidth - 1)
              drawPixelXY(matrixWidth - x, yy, getPixColorXY(matrixWidth - x - 1, yy));
          }
        }
        dir = !dir;
      }
    }
  }

  if (step < DOT_EXPLOSION) {
    FPSdelay++;
    if (matrixHeight < 20) {
      FPSdelay++;
    }

    if (custom_eff == 1) {
      DrawLine(0U, 0U, 0U, matrixHeight - step, CHSV(skyColor, 255U, 32U));
      DrawLine(matrixWidth - 1, 0U, matrixWidth - 1, matrixHeight - step, CHSV(skyColor, 255U, 32U));
    }

    uint8_t saturation = (step > (DOT_EXPLOSION - 2U)) ? 192U : 20U;
    uint8_t rndPos = 3U * deltaHue2 * 0.5f;
    uint16_t cx = matrixWidth / 2;
    drawPixelXY(cx - 1 + rndPos, step, CHSV(50U, saturation, 80U));                 // первая
    drawPixelXY(cx + 1 - rndPos, step - HORIZONT, CHSV(50U, saturation, 80U));      // вторая
    if (rndPos > 1) {
      drawPixelXY(cx + 4 - rndPos, step - HORIZONT + 2, CHSV(50U, saturation, 80U)); // третья
    }
    /* яркость неба */
    if (hue > 2U) hue -= 1U;
  }

  if (step > matrixHeight * 1.25f) {
    if (hue < MAX_BRIGHTNESS) hue += 2U;
  }

  if (step >= (int16_t)(matrixHeight * 2.0f)) {
    step = 0U;
    FPSdelay = FPS_DELAY;
    if (modes[currentMode].Scale < 5) {
      deltaHue2++;
    }
    if (deltaHue2 >= 4U) deltaHue2 = 0U;
  }
  step++;
}

// =================================================================== МЕЧТА ДИЗАЙНЕРА ================================================================
//             © SlingMaster
// =====================================

int getRandomPos(uint8_t STEP) {
  uint8_t val = floor(random(0, (STEP * 16 - matrixWidth - 1)) / STEP) * STEP;
  return -val;
}

int getHue(uint8_t x, uint8_t y) {
  return ( x * 32 +  y * 24U );
}

uint8_t getSaturationStep() {
  return (modes[currentMode].Speed > 170U) ? ((matrixHeight > 24) ? 12 : 24) : 0;
}

uint8_t getBrightnessStep() {
  return (modes[currentMode].Speed < 85U) ? ((matrixHeight > 24) ? 16 : 24) : 0;
}

void drawPalette(int posX, int posY, uint8_t STEP) {
  int PX, PY;
  const uint8_t SZ = STEP - 1;
  const uint8_t maxY = floor(matrixHeight / SZ);
  uint8_t sat = getSaturationStep();
  uint8_t br  = getBrightnessStep();

  FastLED.clear();
  for (uint8_t y = 0; y < maxY; y++) {
    for (uint8_t x = 0; x < 16; x++) {
      PY = y * STEP;
      PX = posX + x * STEP;
      if ((PX >= - STEP ) && (PY >= - STEP) && (PX < matrixWidth) && (PY < matrixHeight)) {
        drawRecCHSV(PX, PY, PX + SZ, PY + SZ, CHSV( getHue(x, y), (255U - sat * y), (240U - br * y)));
      }
    }
  }
}

void selectColor(uint8_t sc) {
  uint8_t offset = (matrixWidth >= 16) ? matrixWidth * 0.25 : 0;
  hue = getHue(random(offset, WIDTH - offset), random(matrixHeight));
  uint8_t sat = getSaturationStep();
  uint8_t br  = getBrightnessStep();

  for (uint8_t y = 0; y < matrixHeight; y++) {
    for (uint8_t x = offset; x < (matrixWidth - offset); x++) {
      CHSV curColor = CHSV(hue, (255U - sat * y), (240U - br * y));
      if (curColor == getPixColorXY(x, y)) {
        /* show srlect color */
        drawRecCHSV(x, y, x + sc, y + sc, CHSV( hue, 64U, 255U));
        FastLED.show();
        delay(400);
        drawRecCHSV(x, y, x + sc, y + sc, CHSV( hue, 255U, 255U));
        y = matrixHeight;
        x = matrixWidth;
      }
    }
  }
}

void WebTools() {
  const uint8_t FPS_D = 24U;
  static uint8_t STEP = 3U;
  static int posX = -STEP;
  static int posY = 0;
  static int nextX = -STEP * 2;
  static bool stop_moving = true;
  uint8_t speed = modes[currentMode].Speed > 65U ? modes[currentMode].Speed : 65U;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random(10U, 90U), random(10U, 255U));
    }
#endif
    loadingFlag = false;
    FPSdelay = 1U;
    step = 0;
    STEP = 2U + floor(modes[currentMode].Scale / 35);
    posX = 0;
    posY = 0;
    drawPalette(posX, posY, STEP);
  }

  if (step == 0) {    /* restart ----------- */
    nextX = 0;
    FPSdelay = FPS_D;
  }
  else if (step == speed / 16 + 1) { /* start move -------- 16*/
    nextX = getRandomPos(STEP);
    FPSdelay = FPS_D;
  }
  else if (step == speed / 10 + 1) { /* find --------------100 */
    nextX = getRandomPos(STEP);
    FPSdelay = FPS_D;
  }
  else if (step == speed / 7 + 1) { /* find 2 ----------- 150*/
    nextX = getRandomPos(STEP);
    FPSdelay = FPS_D;
  }
  else if (step == speed / 6 + 1) { /* find 3 -----------200 */
    nextX = - STEP * random(4, 8);
    // nextX = getRandomPos(STEP);
    FPSdelay = FPS_D;
  }
  else if (step == speed / 5 + 1) { /* select color ------220 */
    FPSdelay = 200U;
    selectColor(STEP - 1);
  }
  else if (step == speed / 4 + 1) { /* show color -------- 222*/
    FPSdelay = FPS_D;
    nextX = matrixWidth;
  }
  else if (step == speed / 4 + 3) {
    step = 252;
  }

  //}
  if (posX < nextX) posX++;
  if (posX > nextX) posX--;

  if (stop_moving)   {
    FPSdelay = 80U;
    step++;
  } else {
    drawPalette(posX, posY, STEP);
    if ((nextX == matrixWidth) || (nextX == 0)) {
      if (posX > 1) {
        gradientHorizontal(0, 0, (posX - 1), matrixHeight, hue, hue, 255U, 96U, 255U);
      }
      if (posX > 3) DrawLine(posX - 3, CENTER_Y_MINOR, posX - 3, CENTER_Y_MAJOR, CHSV( hue, 192U, 255U));
    }
  }

  stop_moving = (posX == nextX);
}

// ============================================================================= КОНТАКТЫ =============================================================
//             © Yaroslaw Turbin
//        Adaptation © SlingMaster
//          modifed © alvikskor
// =====================================

void Contacts() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random(25U, 90U), random(5U, 250U));
    }
#endif
    loadingFlag = false;
    FPSdelay = 80U;
    FastLED.clear();
  }

  uint16_t speed = modes[currentMode].Speed;
  uint16_t divisor = map(speed, 0, 255, 32, 1);
  divisor = constrain(divisor, 1, 32);
  int a = millis() / divisor;

  hue = floor(modes[currentMode].Scale / 14);
  for (int x = 0; x < matrixWidth; x++) {
    for (int y = 0; y < matrixHeight; y++) {
      int index = XY(x, y);
      uint8_t color1 = pgm_read_byte(&exp_gamma[sin8(cos8((x * 7 + a / 5)) - cos8((y * 10) + a / 3) / 4 + a )]);
      uint8_t color2 = pgm_read_byte(&exp_gamma[(sin8(x * 16 + a / 3) + cos8(y * 8 + a / 2)) / 2]);
      uint8_t color3 = pgm_read_byte(&exp_gamma[sin8(cos8(x * 8 + a / 3) + sin8(y * 8 + a / 4) + a)]);
      if (hue == 0) {
        leds[index].b = color3 >> 2;
        leds[index].g = color2;
        leds[index].r = 0;
      } else if (hue == 1) {
        leds[index].b = color1;
        leds[index].g = 0;
        leds[index].r = color3 >> 2;
      } else if (hue == 2) {
        leds[index].b = 0;
        leds[index].g = color1 >> 2;
        leds[index].r = color3;
      } else if (hue == 3) {
        leds[index].b = color1;
        leds[index].g = color2;
        leds[index].r = color3;
      } else if (hue == 4) {
        leds[index].b = color3;
        leds[index].g = color1;
        leds[index].r = color2;
      } else if (hue == 5) {
        leds[index].b = color2;
        leds[index].g = color3;
        leds[index].r = color1;
      } else if (hue >= 6) {
        leds[index].b = color3;
        leds[index].g = color1;
        leds[index].r = color2;
      }
    }
  }
}

// =========================================================================== ВОЛШЕБНЫЙ ФОНАРИК ======================================================
//             © SlingMaster
// =============================================

void MagicLantern() {
  static uint8_t saturation;
  static uint8_t brightness;
  static uint8_t low_br;
  static uint16_t lastWidth = 0, lastHeight = 0;

  if (lastWidth != matrixWidth || lastHeight != matrixHeight) {
    lastWidth = matrixWidth;
    lastHeight = matrixHeight;
    loadingFlag = true;
  }

  const uint8_t PADDING = matrixHeight * 0.25;
  const uint8_t WARM_LIGHT = 55U;
  const uint8_t STEP = 4U;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(100U), random8(40, 200U));
    }
#endif
    loadingFlag = false;
    deltaValue = 0;
    step = deltaValue;
    if (modes[currentMode].Speed > 52) {
      brightness = map(modes[currentMode].Speed, 1, 255, 50U, 250U);
      low_br = 50U;
    } else {
      brightness = 0U;
      low_br = 0U;
    }
    saturation = (modes[currentMode].Scale > 50U) ? 64U : 0U;
    if (abs(70 - modes[currentMode].Scale) <= 5) saturation = 170U;
    FastLED.clear();
  }

  dimAll(170);
  hue = (modes[currentMode].Scale > 95) ? floor(step / 32) * 32U : modes[currentMode].Scale * 2.55;

  uint8_t centerY_major = matrixHeight / 2 + (matrixHeight % 2);

  for (uint8_t x = 0U; x < matrixWidth + 1; x++) {
    // light ---
    if (low_br > 0) {
      gradientVertical(x - deltaValue, centerY_major, x + 1U - deltaValue, matrixHeight - PADDING - 1, WARM_LIGHT, WARM_LIGHT, brightness, low_br, saturation);
      gradientVertical(matrixWidth - x + deltaValue, centerY_major, matrixWidth - x + 1U + deltaValue, matrixHeight - PADDING - 1, WARM_LIGHT, WARM_LIGHT, brightness, low_br, saturation);
      gradientVertical(x - deltaValue, PADDING + 1, x + 1U - deltaValue, centerY_major, WARM_LIGHT, WARM_LIGHT, low_br + 10, brightness, saturation);
      gradientVertical(matrixWidth - x + deltaValue, PADDING + 1, matrixWidth - x + 1U + deltaValue, centerY_major, WARM_LIGHT, WARM_LIGHT, low_br + 10, brightness, saturation);
    } else {
      if (x % (STEP + 1) == 0) {
        leds[XY(random8(matrixWidth), random8(PADDING + 2, matrixHeight - PADDING - 2))] = CHSV(step - 32U, random8(128U, 255U), 255U);
      }
      if ((modes[currentMode].Speed < 25) & (low_br == 0)) {
        deltaValue = 0;
        if (x % 2 != 0) {
          gradientVertical(x - deltaValue, matrixHeight - PADDING, x + 1U - deltaValue, matrixHeight, hue, hue + 2, 64U, 20U, 255U);
          gradientVertical((matrixWidth - x + deltaValue), 0U, (matrixWidth - x + 1U + deltaValue), PADDING, hue, hue, 42U, 64U, 255U);
        }
      }
    }
    if (x % STEP == 0) {
      // body --
      gradientVertical(x - deltaValue, matrixHeight - PADDING, x + 1U - deltaValue, matrixHeight, hue, hue + 2, 255U, 20U, 255U);
      gradientVertical((matrixWidth - x + deltaValue), 0U, (matrixWidth - x + 1U + deltaValue), PADDING, hue, hue, 42U, 255U, 255U);
    }
  }

  deltaValue++;
  if (deltaValue >= STEP) {
    deltaValue = 0;
  }
  step++;
}

// ================================================================================= ОСЬМИНОГ =========================================================
//        © Stepko and Sutaburosu
//    Adapted and modifed © alvikskor
// Idea from https://www.youtube.com/watch?v=HsA-6KIbgto&ab_channel=GreatScott%21
// =================================================================================

void Octopus() {
  static uint16_t scaleVar = 0;
  static uint8_t stepVar = 0;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random(10U, 101U), random(150U, 255U));
    }
#endif
    loadingFlag = false;
    scaleVar = 0;
    stepVar = 0;

    int16_t cx = matrixWidth / 2;
    int16_t cy = matrixHeight / 2;
    for (int16_t x = -cx; x < cx + (matrixWidth % 2); x++) {
      for (int16_t y = -cy; y < cy + (matrixHeight % 2); y++) {
        uint16_t ix = x + cx;
        uint16_t iy = y + cy;
        if (ix < matrixWidth && iy < matrixHeight) {
          noise3d[0][ix][iy] = (atan2(x, y) / PI) * 128 + 127;
          noise3d[1][ix][iy] = hypot(x, y);
        }
      }
    }
  }

  uint8_t legs = modes[currentMode].Scale / 10;
  uint16_t color_speed;
  stepVar = modes[currentMode].Scale % 10;
  if (stepVar < 5)
    color_speed = scaleVar / (3 - stepVar / 2);
  else
    color_speed = scaleVar * (stepVar / 2 - 1);
  scaleVar++;

  for (uint16_t x = 0; x < matrixWidth; x++) {
    for (uint16_t y = 0; y < matrixHeight; y++) {
      uint8_t angle = noise3d[0][x][y];
      uint8_t radius = noise3d[1][x][y];
      uint8_t val = sin8(sin8((angle * 4 - (radius * (255 / matrixWidth))) / 4 + scaleVar) + radius * (255 / matrixWidth) - scaleVar * 2 + angle * legs);
      leds[XY(x, y)] = CHSV(color_speed - radius * (255 / matrixWidth), 255, val);
    }
  }

  FPSdelay = 255 - modes[currentMode].Speed;
}

// ======================================================================== АЛЕНЬКИЙ ЦВЕТОЧЕК =========================================================
//    © Stepko and © Sutaburosu
//     Adaptation © SlingMaster
//       Modifed © alvikskor
// =====================================

void FlowerRuta() {
  static uint16_t scale = 0;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(11U, 69U), random8(150U, 255U));
    }
#endif
    loadingFlag = false;
    FastLED.clear();

    uint8_t centerX = matrixWidth  / 2;
    uint8_t centerY = matrixHeight / 2;

    for (uint16_t x = 0; x < matrixWidth; x++) {
      for (uint16_t y = 0; y < matrixHeight; y++) {
        int16_t dx = x - centerX;
        int16_t dy = y - centerY;
        float angle = atan2(dy, dx);
        if (angle < 0) angle += 2 * PI;
        noise3d[0][x][y] = (angle / (2 * PI)) * 255.0f;
        noise3d[1][x][y] = hypot(dx, dy);
      }
    }

    scale = 0;
  }

  uint8_t petals = modes[currentMode].Scale / 10;
  if (petals < 1) petals = 1;
  if (petals > 6) petals = 6;

  uint8_t stepMode = modes[currentMode].Scale % 10;
  uint16_t colorSpeed;

  if (stepMode < 5) {
    colorSpeed = scale / (3 - stepMode / 2);
  } else {
    colorSpeed = scale * (stepMode / 2 - 1);
  }

  scale++;

  for (uint16_t x = 0; x < matrixWidth; x++) {
    for (uint16_t y = 0; y < matrixHeight; y++) {
      uint8_t angle  = noise3d[0][x][y];
      uint8_t radius = noise3d[1][x][y];
      uint8_t brightness = sin8( sin8( scale + angle * petals + (radius * 255 / max(matrixWidth, matrixHeight))) + scale * 4 + sin8( scale * 4 - radius * 255 / max(matrixWidth, matrixHeight)) + angle * petals);
      uint8_t hueVal = colorSpeed + radius * (255 / max(matrixWidth, matrixHeight));

      leds[XY(x, y)] = CHSV(hueVal, 255, brightness);
    }
  }
}

// ======================================================================== ЦВЕТОК ЛОТОСА =============================================================
//             © SlingMaster
// =========================================

void drawLotusFlowerFragment(uint8_t posX, byte line) {
  const uint8_t h = (matrixHeight > 24) ? matrixHeight * 0.9 : matrixHeight;
  uint8_t flover_color = 128 + abs(128 - hue);          // 128 -- 255
  uint8_t gleam = 255 - abs(128 - hue2);                // 255 -- 128
  float f_size = (128 - abs(128 - deltaValue)) / 150.0; // 1.0 -- 0.0
  const byte lowBri = 112U;
  // clear -----
  DrawLine(posX, 0, posX, h * 1.1, CRGB(0, 0, 0));

  switch (line) {
    case 0:
      gradientVertical(posX, 0, posX + 1, h * 0.22, 96, 96, 32, 255, 255U);                             // green leaf c
      gradientVertical(posX, h * 0.9, posX + 1, h * 1.1, 64, 48, 64, 205, gleam);                       // pestle
      gradientVertical(posX, 8, posX + 1, h * 0.6, flover_color, flover_color, 128, lowBri, 255U);          // ---
      break;
    case 2:
    case 6:
      gradientVertical(posX, h * 0.2, posX + 1, h - 4, flover_color, flover_color, lowBri, 255, gleam);     //  -->
      gradientVertical(posX, h * 0.05, posX + 1, h * 0.15, 96, 96, 32, 255, 255U);                      // green leaf
      break;
    case 3:
    case 5:
      gradientVertical(posX, h * 0.5, posX + 1, h - 2, flover_color, flover_color, lowBri, 255, 255U);      // ---->
      break;
    case 4:
      gradientVertical(posX, 1 + h * f_size, posX + 1, h, flover_color, flover_color, lowBri, 255, gleam);  // ------>
      break;
    default:
      gradientVertical(posX, h * 0.05, posX + 1, h * 0.2, 80, 96, 160, 64, 255U);                       // green leaf m
      break;
  }
}

//---------------------------------------
void LotusFlower() {
  const byte STEP_OBJ = 8;
  static uint8_t deltaSpeed = 0;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), random8(1, 255U));
    }
#endif
    loadingFlag = false;
    step = 0U;
    hue2 = 128U;
    deltaValue = 0;
    hue = 224;
    FPSdelay = SpeedFactor(160);
    FastLED.clear();
  }

  if (modes[currentMode].Speed > 128U) {
    if (modes[currentMode].Scale > 50) {
      deltaSpeed = 80U + (128U - deltaValue) / 1.25;
      FPSdelay = SpeedFactor(deltaSpeed);
      if (step % 256 == 0U ) hue += 32;           /* color morph */
    } else {
      FPSdelay = SpeedFactor(160);
      hue = 28U;
    }
    deltaValue++;     /* size morph  */
    /* <==== scroll ===== */
    drawLotusFlowerFragment(matrixWidth - 1, (step % STEP_OBJ));
    for (uint8_t y = 0U ; y < matrixHeight; y++) {
      for (uint8_t x = 0U ; x < matrixWidth; x++) {
        drawPixelXY(x - 1, y,  getPixColorXY(x,  y));
      }
    }
  } else {
    /* <==== morph ===== */
    for (uint8_t x = 0U ; x < matrixWidth; x++) {
      drawLotusFlowerFragment(x, (x % STEP_OBJ));
      if (x % 2U) {
        hue2++;         /* gleam morph */
      }
    }
    deltaValue++;       /* size morph  */
    if (modes[currentMode].Scale > 50) {
      hue += 8; /* color morph */
    } else {
      hue = 28U;
    }
  }
  step++;
}

// ============================================================================= ТОРНАДО ==============================================================
//  base code © Stepko, © Sutaburosu
//        and © SlingMaster
//   adapted and modifed © alvikskor
// =====================================

void Tornado() {
  static uint16_t lastWidth = 0, lastHeight = 0;
  static uint8_t centerX_major = 0;

  if (lastWidth != matrixWidth || lastHeight != matrixHeight) {
    lastWidth = matrixWidth;
    lastHeight = matrixHeight;
    loadingFlag = true;
    centerX_major = matrixWidth / 2 + (matrixWidth % 2);
  }

  const byte OFFSET = 1U;
  const uint8_t H = matrixHeight - OFFSET;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(100U, 255U), random8(20U, 100U));
    }
#endif
    loadingFlag = false;

    for (int16_t x = -centerX_major; x < (int16_t)centerX_major; x++) {
      for (int16_t y = -OFFSET; y < (int16_t)H; y++) {
        uint16_t xi = x + centerX_major;
        uint16_t yi = y + OFFSET;
        if (xi < matrixWidth && yi < matrixHeight) {
          noise3d[0][xi][yi] = 128 * (atan2(y, x) / PI);
          noise3d[1][xi][yi] = hypot(x, y);
        }
      }
    }
  }

  scale += modes[currentMode].Speed / 10;

  for (uint16_t x = 0; x < matrixWidth; x++) {
    for (uint16_t y = 0; y < matrixHeight; y++) {
      byte angle = noise3d[0][x][y];
      byte radius = noise3d[1][x][y];
      uint8_t brightness;
      if (y < (matrixHeight / 8)) {
        brightness = 255 - (((matrixHeight / 8) - y) * 16);
      } else {
        brightness = 255;
      }
      leds[XY(x, y)] = CHSV((angle * modes[currentMode].Scale / 10) - scale + (radius * modes[currentMode].Scale / 10), min(((uint16_t)y * 512U / (uint16_t)matrixHeight), 255U), brightness);
    }
  }
}

// ==================================================================== ПЛАЗМЕННЫЕ ВОЛНЫ ==============================================================
//              © Stepko
//        Adaptation © alvikskor
// ==========================================

void Plasma_Waves() {
  static int64_t frameCount = 0;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random8(100U), random8(40, 200U));
    }
#endif
    loadingFlag = false;
    hue = modes[currentMode].Scale / 10;
  }
  FPSdelay = 1;

  frameCount++;
  uint8_t t1 = cos8((42 * frameCount) / (132 - modes[currentMode].Speed / 2));
  uint8_t t2 = cos8((35 * frameCount) / (132 - modes[currentMode].Speed / 2));
  uint8_t t3 = cos8((38 * frameCount) / (132 - modes[currentMode].Speed / 2));

  for (uint16_t y = 0; y < matrixHeight; y++) {
    for (uint16_t x = 0; x < matrixWidth; x++) {
      uint8_t r = cos8((x << 3) + (t1 >> 1) + cos8(t2 + (y << 3) + modes[currentMode].Scale));
      uint8_t g = cos8((y << 3) + t1 + cos8((t3 >> 2) + (x << 3)) + modes[currentMode].Scale);
      uint8_t b = cos8((y << 3) + t2 + cos8(t1 + x + (g >> 2) + modes[currentMode].Scale));

      switch (hue) {
        case 0:
          r = pgm_read_byte(&exp_gamma[r]);
          g = pgm_read_byte(&exp_gamma[g]);
          b = pgm_read_byte(&exp_gamma[b]);
          break;
        case 1:
          r = pgm_read_byte(&exp_gamma[r]);
          b = pgm_read_byte(&exp_gamma[g]);
          g = pgm_read_byte(&exp_gamma[b]);
          break;
        case 2:
          g = pgm_read_byte(&exp_gamma[r]);
          r = pgm_read_byte(&exp_gamma[g]);
          b = pgm_read_byte(&exp_gamma[b]);
          break;
        case 3:
          r = pgm_read_byte(&exp_gamma[r]) / 2;
          g = pgm_read_byte(&exp_gamma[g]);
          b = pgm_read_byte(&exp_gamma[b]);
          break;
        case 4:
          r = pgm_read_byte(&exp_gamma[r]);
          g = pgm_read_byte(&exp_gamma[g]) / 2;
          b = pgm_read_byte(&exp_gamma[b]);
          break;
        case 5:
          r = pgm_read_byte(&exp_gamma[r]);
          g = pgm_read_byte(&exp_gamma[g]);
          b = pgm_read_byte(&exp_gamma[b]) / 2;
          break;
        case 6:
          r = pgm_read_byte(&exp_gamma[r]) * 3;
          g = pgm_read_byte(&exp_gamma[g]);
          b = pgm_read_byte(&exp_gamma[b]);
          break;
        case 7:
          r = pgm_read_byte(&exp_gamma[r]);
          g = pgm_read_byte(&exp_gamma[g]) * 3;
          b = pgm_read_byte(&exp_gamma[b]);
          break;
        case 8:
          r = pgm_read_byte(&exp_gamma[r]);
          g = pgm_read_byte(&exp_gamma[g]);
          b = pgm_read_byte(&exp_gamma[b]) * 3;
          break;

      }
      leds[XY(x, y)] = CRGB(r, g, b);
    }
  }
}

// ======================================================================= ЦВЕТНОЙ ПИТОН ==============================================================
//      base code WavingCell from © Stepko
//       Adaptation & modefed © alvikskor
// ========================================

uint32_t color_timer = millis();

void Colored_Python() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random8(100U), random8(1, 255U));
    }
#endif
    loadingFlag = false;
    step = 0;
  }
  uint16_t  t = millis() / (128 - (modes[currentMode].Speed / 2));
  uint8_t palette_number = modes[currentMode].Scale / 10;
  uint8_t thickness;
  if (palette_number < 9)
    step = palette_number;
  else if (millis() - color_timer > 30000) {
    color_timer = millis();
    step++;
    if (step > 8) step = 0;
  }
  switch (step) {
    case 0: currentPalette = CloudColors_p; break;
    case 1: currentPalette = AlcoholFireColors_p; break;
    case 2: currentPalette = OceanColors_p; break;
    case 3: currentPalette = ForestColors_p; break;
    case 4: currentPalette = RainbowColors_p; break;
    case 5: currentPalette = RainbowStripeColors_p; break;
    case 6: currentPalette = HeatColors_p; break;
    case 7: currentPalette = LavaColors_p; break;
    case 8: currentPalette = PartyColors_p;
  }
  switch (modes[currentMode].Scale % 5) {
    case 0: thickness = 5; break;
    case 1: thickness = 10; break;
    case 2: thickness = 20; break;
    case 3: thickness = 30; break;
    case 4: thickness = 40; break;
  }
  for (byte x = 0; x < matrixWidth; x++) {
    for (byte y = 0; y < matrixHeight; y++) {
      leds[XY(x, y)] = ColorFromPalette(currentPalette, ((sin8((x * thickness) + sin8(y * 5 + t * 5)) + cos8(y * 10)) + 1) + t * (modes[currentMode].Speed % 10)); //HeatColors_p -палитра, t*scale/10 -меняет скорость движения вверх, sin8(x*20) -меняет ширину рисунка
    }
  }
}

// ====================================================================== ЗВЁЗДЫ ======================================================================
//     © SottNick and  © Stepko
//      Adaptation © SlingMaster
// =====================================

void drawStar(float xlocl, float ylocl, float biggy, float little, int16_t points, float dangle, uint8_t koler) {
  float radius2 = 255.0 / points;
  for (int i = 0; i < points; i++) {
    DrawLine(xlocl + ((little * (sin8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128), ylocl + ((little * (cos8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128), xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128), ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128), ColorFromPalette(*curPalette, koler));
    DrawLine(xlocl + ((little * (sin8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128), ylocl + ((little * (cos8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128), xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128), ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128), ColorFromPalette(*curPalette, koler));
  }
}

// --------------------------------------
void EffectStars() {
#define STARS_NUM (8U)
#define STAR_BLENDER (128U)
#define CENTER_DRIFT_SPEED (6U)
  static uint8_t spd;
  static uint8_t points[STARS_NUM];
  static float color[STARS_NUM];
  static int delay_arr[STARS_NUM];
  static float counter;
  static float driftx;
  static float drifty;
  static float cangle;
  static float sangle;
  static uint8_t stars_count;
  static uint8_t blur;

  if (matrixWidth == 0 || matrixHeight == 0) return;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(100U), random8(80U, 255U));
    }
#endif
    loadingFlag = false;

    driftx = (float)matrixWidth / 2.0f;
    drifty = (float)matrixHeight / 2.0f;
    cangle = (float)(sin8(random8(25, 220)) - 128.0f) / 128.0f;
    sangle = (float)(sin8(random8(25, 220)) - 128.0f) / 128.0f;
    spd = modes[currentMode].Speed;
    blur = modes[currentMode].Scale / 2;
    stars_count = matrixWidth / 2U;
    if (stars_count > STARS_NUM) stars_count = STARS_NUM;
    counter = (float)(spd / 5 + 3U);
    for (uint8_t num = 0; num < stars_count; num++) {
      points[num] = map(modes[currentMode].Scale, 1, 255, 3U, 7U);
      delay_arr[num] = spd / 5 + (num << 2) + 2U;
      color[num] = random8();
    }
  }

  fadeToBlackBy(leds, usedLeds, 165);

  float speedFactor = ((float)spd / 380.0f + 0.05f);
  counter += speedFactor;

  float boundX = (float)matrixWidth / 4.0f;
  float boundY = (float)matrixHeight / 4.0f;

  if (driftx > (matrixWidth - boundX)) cangle = 0.0f - fabs(cangle);
  if (driftx < boundX) cangle = fabs(cangle);
  if ((uint16_t)counter % CENTER_DRIFT_SPEED == 0) driftx += cangle * speedFactor;

  if (drifty > (matrixHeight - boundY)) sangle = 0.0f - fabs(sangle);
  if (drifty < boundY) sangle = fabs(sangle);
  if ((uint16_t)counter % CENTER_DRIFT_SPEED == 0) drifty += sangle * speedFactor;

  for (uint8_t num = 0; num < stars_count; num++) {
    if (counter >= delay_arr[num]) {
      if (counter - delay_arr[num] <= matrixWidth + 5) {
        drawStar(driftx, drifty, 2 * (counter - delay_arr[num]), (counter - delay_arr[num]), points[num], STAR_BLENDER + color[num], color[num]);
        color[num] += speedFactor;
      } else {
        delay_arr[num] = counter;
      }
    }
  }

  blur2d(leds, matrixWidth, matrixHeight, blur);
}

// ====================================================================== ПЛАНЕТА ЗЕМЛЯ ===============================================================
//             © SlingMaster
// =====================================

// Эффект требует высоты матрицы не менее 16 пикселей
void PlanetEarth() {
  static uint16_t imgW = 0, imgH = 0;
  static bool imageLoaded = false;
  static uint16_t lastMatrixHeight = 0;

  if (lastMatrixHeight != matrixHeight) {
    lastMatrixHeight = matrixHeight;
    loadingFlag = true;
    imageLoaded = false;
  }

  if (matrixHeight < 16U) {
    if (loadingFlag) {
      loadingFlag = false;
      FastLED.clear();
    }
    return;
  }

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(128U, 10U + random8(230U));
    }
#endif
    loadingFlag = false;
    FPSdelay = 96U;
    FastLED.clear();

    String file_name;
    if (modes[currentMode].Scale < 50) {
      file_name = "globe0";
    } else if (matrixHeight >= 24U) {
      file_name = "globe_big";
    } else {
      file_name = "globe1";
    }

    String fullPath = "bin/" + file_name + ".img";
    readBinFile(fullPath, 4112);

    imgW = getSizeValue(binImage, 8);
    imgH = getSizeValue(binImage, 10);

    if (imgW > 0 && imgH > 0) {
      imageLoaded = true;
      ff_x = 0;
      drawScaledImage(imgW, imgH, ff_x);
    } else {
      imageLoaded = false;
    }
  }

  if (!imageLoaded) {
    dimAll(0);
    return;
  }

  ff_x++;
  if (ff_x >= imgW) {
    ff_x = 0;
  }
  drawScaledImage(imgW, imgH, ff_x);
}

// ======================================================================= БАМБУК =====================================================================
//             © SlingMaster
// ==================================

uint8_t nextColor(uint8_t posY, uint8_t base, uint8_t next ) {
  const byte posLine = (matrixHeight > 16) ? 4 : 3;
  if ((posY + 1 == posLine) | (posY == posLine)) {
    return next;
  } else {
    return base;
  }
}

// --------------------------------------
void Bamboo() {
  const uint8_t gamma[7] = {0, 32, 144, 160, 196, 208, 230};
  static float index;
  const byte DELTA = 4U;
  const uint8_t VG_STEP = 64U;
  const uint8_t V_STEP = 32U;
  const byte posLine = (matrixHeight > 16) ? 4 : 3;
  const uint8_t SX = 5;
  const uint8_t SY = 10;
  static float deltaX = 0;
  static bool direct = false;
  uint8_t posY;
  static uint8_t colLine;
  const float STP = 0.2;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random8(100U), random8(128, 255U));
    }
#endif
    loadingFlag = false;
    index = STP;
    uint8_t idx = map(modes[currentMode].Scale, 5, 95, 0U, 6U);;
    colLine = gamma[idx];
    step = 0U;
  }

  for (int y = 0; y < matrixHeight + SY; y++) {
    if (modes[currentMode].Scale < 50U) {
      if (step % 128 == 0U) {
        deltaX += STP * ((direct) ? -1 : 1);
        if ((deltaX > 1) | (deltaX < -1)) direct = !direct;
      }
    } else {
      deltaX = 0;
    }
    posY = y;
    for (int x = 0; x < matrixWidth + SX; x++) {
      if (y == posLine) {
        drawPixelXYF(x , y - 1, CHSV(colLine, 255U, 128U));
        drawPixelXYF(x, y, CHSV(colLine, 255U, 96U));
        if (matrixHeight > 16) {
          drawPixelXYF(x, y - 2, CHSV(colLine, 10U, 64U));
        }
      }
      if ((x % SX == 0U) & (y % SY == 0U)) {
        for (int i = 1; i < (SY - 3); i++) {
          if (i < 3) {
            posY = y - i + 1 - DELTA + index;
            drawPixelXYF(x - 3 + deltaX, posY, CHSV(nextColor(posY, 96, colLine), 255U, 255 - V_STEP * i));
            posY = y - i + index;
            drawPixelXYF(x + deltaX, posY, CHSV(nextColor(posY, 96, colLine), 255U, 255 - VG_STEP * i));
          }
          posY = y - i - DELTA + index;
          drawPixelXYF(x - 4 + deltaX, posY, CHSV(nextColor(posY, 96, colLine), 180U, 255 - V_STEP * i));
          posY = y - i + 1 + index;
          drawPixelXYF(x - 1 + deltaX, posY, CHSV(nextColor(posY, ((i == 1) ? 96 : 80), colLine), 255U, 255 - V_STEP * i));
        }
      }
    }
    step++;
  }
  if (index >= SY) {
    index = 0;
  }
  fadeToBlackBy(leds, usedLeds, 60);
  index += STP;
}

// ====================================================================== РАЗНОЦВЕТНЫЕ ОДУВАНЧИКИ =====================================================
//      Base Code © Less Lam
//          © SlingMaster
// https://editor.soulmatelights.com/gallery/2007-amber-rain
// ===========================================================

#define MAX_CIRCLES 128
class Circle {
  public:
    float thickness = 3.0;
    long startTime;
    uint16_t offset;
    int16_t centerX;
    int16_t centerY;
    int hue;
    int bpm = 10;

    void move() {
      centerX = random(0, matrixWidth);
      centerY = random(0, matrixHeight);
    }

    void scroll() {
      centerX--;
      if (centerX < 1) {
        centerX = matrixWidth - 1;
      }
      centerY++;
      if (centerY >= matrixHeight) {
        centerY = 0;
      }
    }

    void reset() {
      startTime = millis();
      centerX = random(0, matrixWidth);
      centerY = random(0, matrixHeight);
      hue = random(0, 255);
      offset = random(0, 60000 / bpm);
    }

    float radius() {
      float radius = beatsin16(modes[currentMode].Speed / 2.5f, 0, 500, offset) / 100.0f;
      return radius;
    }
};

namespace Circles {
Circle circles[MAX_CIRCLES];

void drawCircle(Circle& circle) {
  int16_t centerX = circle.centerX;
  int16_t centerY = circle.centerY;
  int hue = circle.hue;
  float radius = circle.radius();

  int16_t startX = centerX - ceil(radius);
  int16_t endX   = centerX + ceil(radius);
  int16_t startY = centerY - ceil(radius);
  int16_t endY   = centerY + ceil(radius);

  for (int16_t x = startX; x <= endX; x++) {
    for (int16_t y = startY; y <= endY; y++) {
      if (x < 0 || x >= matrixWidth || y < 0 || y >= matrixHeight) continue;
      uint16_t index = XY(x, y);
      if (index >= usedLeds) continue;

      double distance = sqrt(sq(x - centerX) + sq(y - centerY));
      if (distance > radius) continue;

      uint16_t brightness;
      if (radius < 1.0f) {
        brightness = 180;
      } else {
        double percentage = distance / radius;
        double fraction = 1.0 - percentage;
        brightness = 255.0 * fraction;
      }
      leds[index] += CHSV(hue, deltaValue, brightness);
    }
  }
}

void draw(bool setup) {
  fadeToBlackBy(leds, usedLeds, 100);
  int numCircles = (matrixWidth / 2);
  if (numCircles > MAX_CIRCLES) numCircles = MAX_CIRCLES;

  for (int i = 0; i < numCircles; i++) {
    if (setup) {
      circles[i].reset();
    } else {
      if (circles[i].radius() < 0.5f) {
        circles[i].scroll();
      }
    }
    drawCircle(circles[i]);
  }
 }
} // namespace Circles

// ==============
void Dandelions() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(1U, 100U), random8(10U, 255U));
    }
#endif
    loadingFlag = false;
    FastLED.clear();
    Circles::draw(true);
    deltaValue = 155 + modes[currentMode].Scale;
  }
  Circles::draw(false);
}

// ====================================================================== ЦИФРОВАЯ ТУРБУЛЕНТНОСТЬ =====================================================
//             © SlingMaster
// =====================================

void drawRandomCol(uint8_t x, uint8_t y, uint8_t offset, uint32_t count) {
  const byte STEP = 32;
  uint8_t D = (matrixHeight > 7) ? (matrixHeight / 8) : 1;
  uint8_t color = floor((float)y / D) * STEP + offset;

  if (count == 0U) {
    uint8_t b = (random8(8U) == 0U) ? ((step % 2U) ? 0 : 255) : 0;
    drawPixelXY(x, y, CHSV(color, 255, b));
  } else {
    uint8_t b = (bitRead(count, y) == 1U) ? ((step % 5U) ? 0 : 255) : 0;
    drawPixelXY(x, y, CHSV(color, 255, b));
  }
}

void Turbulence() {
  const byte STEP_COLOR = 255 / matrixHeight;
  const byte STEP_OBJ = 8;
  const byte DEPTH = 2;
  static uint32_t count = 0;
  uint32_t curColor;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(100U), random8(1, 255U));
    }
#endif
    loadingFlag = false;
    step = 0U;
    deltaValue = 0;
    hue = 0;
    count = 0;
    if (modes[currentMode].Speed < 20U) {
      FPSdelay = SpeedFactor(30);
    }
    FastLED.clear();
  }

  deltaValue++;     /* size morph */

  uint16_t cx = matrixWidth / 2;
  uint16_t ch = matrixHeight;

  /* <==== scroll =====> */
  for (uint16_t y = ch; y > 0; y--) {
    uint16_t yy = y - 1;
    drawRandomCol(0, yy, hue, count);
    drawRandomCol(matrixWidth - 1, yy, hue + 128U, count);

    // левая сторона
    for (uint16_t x = cx - 1; x > 0; x--) {
      if (x > cx) {
        if (random8(2) == 0U) {
          ;
        }
      }
      curColor = getPixColorXY(x - 1, yy);
      if (x < cx - DEPTH / 2) {
        drawPixelXY(x, yy, curColor);
      } else {
        if (curColor != 0U) drawPixelXY(x, yy, curColor);
      }
    }

    // правая сторона
    for (uint16_t x = cx + 1; x < matrixWidth; x++) {
      if (x < cx + DEPTH) {
        if (random8(2) == 0U) {
        }
      }
      curColor = getPixColorXY(x, yy);
      if (x > cx + DEPTH / 2) {
        drawPixelXY(x - 1, yy, curColor);
      } else {
        if (curColor != 0U) drawPixelXY(x - 1, yy, curColor);
      }
    }

    /* scroll center up ---- */
    for (uint16_t x = cx - DEPTH; x < cx + DEPTH; x++) {
      if (x < matrixWidth && y < ch) {
        drawPixelXY(x, y, makeDarker(getPixColorXY(x, yy), 128 / y));
        if (y == 1) {
          drawPixelXY(x, 0, CRGB::Black);
        }
      }
    }
  }

  if (modes[currentMode].Scale > 50) {
    count++;
    if (count % 256 == 0U) hue += 16U;
  } else {
    count = 0;
  }
  step++;
}

// =========================================================================== СЕРПАНТИН ==============================================================
//             © SlingMaster
// =====================================

void Serpentine() {
  const uint8_t PADDING = matrixHeight * 0.25f;
  const uint8_t BR_INTERWAL = (matrixHeight > 64) ? 1 : (64 / matrixHeight);
  const uint16_t DELTA = matrixWidth / 4;
  // ---------------------

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(4, 50), random8(4, 254U));
    }
#endif
    loadingFlag = false;
    deltaValue = 0;
    hue = 0;
    FastLED.clear();
  }
  // ---------------------

  byte step1 = map8(modes[currentMode].Speed, 10U, 60U);
  uint16_t ms = millis();
  double freq = 3000;
  byte fade = 180 - abs(128 - step);
  fadeToBlackBy(leds, usedLeds, fade);

  // -----------------
  for (uint16_t y = 0; y < matrixHeight; y++) {
    uint32_t yy = y * 256;
    uint32_t x1 = beatsin16(step1, matrixWidth, (matrixHeight - 1) * 256, matrixWidth, y * freq + 32768) / 2;

    uint8_t bright = 255 - (matrixHeight - y) * BR_INTERWAL;
    CRGB col1 = CHSV(ms / 29 + y * 256 / (matrixHeight - 1) + 128, 255, bright);
    CRGB col2 = CHSV(ms / 29 + y * 256 / (matrixHeight - 1), 255, bright);

    wu_pixel( x1 + hue * DELTA, yy - PADDING * (255 - hue), &col1);
    wu_pixel(((matrixWidth - 1) * 256 - (x1 + hue * DELTA)), yy - PADDING * hue, &col2);
  }

  step++;
  if (step % 64) {
    if (deltaValue == 0) {
      hue++;
      if (hue >= 255) deltaValue = 1;
    } else {
      hue--;
      if (hue < 1) deltaValue = 0;
    }
  }
}

// ====================================================================== СКАНЕР ======================================================================
//             © SlingMaster
// =====================================

void Scanner() {
  static byte i;
  static bool v_scanner = matrixHeight >= matrixWidth;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(0, 100), random8(128, 255U));
    }
    deltaValue = 0;
#endif
    loadingFlag = false;
    hue = modes[currentMode].Scale * 2.55;
    deltaHue = modes[currentMode].Scale;
    i = 5;
    FastLED.clear();
  }

  if (step % 2U == 0U) {
    if (deltaValue == 0U) {
      i++;
    } else {
      i--;
    }
    if (deltaHue == 0U) {
      hue++;
    }
  }
  if (i > 250) {
    i = 0;
    deltaValue = 0;
  }
  fadeToBlackBy(leds, usedLeds, v_scanner ? 50 : 30);

  if (v_scanner) {
    /* vertical scanner */
    if (i >= matrixHeight - 1) {
      deltaValue = 1;
    }

    for (uint16_t x = 0; x < matrixWidth; x++) {
      leds[XY(x, i)] = CHSV(hue, 255U, 180U);
      if ((x == i / 2.0) & (i % 2U == 0U)) {
        if (deltaValue == 0U) {
          drawPixelXYF(random(matrixWidth) - (random8(2U) ? 1.5 : 1), i * 0.9, CHSV(hue, 16U, 255U) );
        } else {
          drawPixelXYF(random(matrixWidth) - 1.5, i * 1.1, CHSV(hue, 16U, 255U) );
        }
      }
    }
  } else {
    /* horizontal scanner */
    if (i >= matrixWidth - 1) {
      deltaValue = 1;
    }

    for (uint16_t y = 0; y < matrixHeight; y++) {
      leds[XY(i, y)] = CHSV(hue, 255U, 180U);
      if ((y == i / 2.0) & (i % 2U == 0U)) {
        if (deltaValue == 0U) {
          drawPixelXYF(i * 0.9, random(matrixHeight) - (random8(2U) ? 1.5 : 1), CHSV(hue, 16U, 255U) );
        } else {
          drawPixelXYF( i * 1.1, random(matrixHeight) - 1.5, CHSV(hue, 16U, 255U) );
        }
      }
    }
  }
  step++;
}

// ====================================================================== НОЧНОЙ ГОРОД ================================================================
//             © SlingMaster
// =================================

void NightCity() {
  uint16_t temp = matrixHeight * 13U / 100U;
  const uint8_t PADDING = (temp > 2U) ? static_cast<uint8_t>(temp) : 2U;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(50, random8(2, 254U));
    }
#endif
    loadingFlag = false;
    hue = 64;

    for (uint16_t i = 0; i < matrixWidth; i++) {
      if (i % 6U == 0U) {
        noise3d[0][i][0] = PADDING + 2;
        noise3d[0][i][1] = PADDING + 3;
      }
    }
    FastLED.clear();
  }

  byte fade = 80;
  fadeToBlackBy(leds, usedLeds, fade);
  uint16_t xx = random16(matrixWidth);
  uint16_t yy = random16(matrixHeight);

  for (uint16_t y = 0; y < matrixHeight; y++) {
    for (uint16_t x = 0; x < matrixWidth; x++) {
      if (y > PADDING) {
        if (x % 6U == 0U) {
          uint8_t liftY = noise3d[0][x][1];
          if (liftY < matrixHeight) {
            leds[XY(x, liftY)] = CHSV(160, 255U, 255U);
          }
        } else {
          bool flag = (modes[currentMode].Scale > 50U) ? true : (x % 2U == 0U);
          if (flag && (y % 2U == 0U)) {
            if (x == xx && y == yy) {
              if (noise3d[0][x][y] == 0) {
                noise3d[0][x][y] = random8(1, 5);
                if (modes[currentMode].Speed > 80) {
                  uint16_t rx = random16(matrixWidth);
                  uint16_t ry = random16(PADDING + 1, matrixHeight - 1);
                  if (ry < matrixHeight) noise3d[0][rx][ry] = 6;
                }
                if (modes[currentMode].Speed > 160) {
                  uint16_t rx = random16(matrixWidth);
                  uint16_t ry = random16(PADDING + 1, matrixHeight - 1);
                  if (ry < matrixHeight) noise3d[0][rx][ry] = 6;
                }
              } else {
                noise3d[0][x][y] = 0;
              }
            }
            if (modes[currentMode].Speed > 250) {
              noise3d[0][x][y] = 2;
            }
            if (noise3d[0][x][y] > 0) {
              if (noise3d[0][x][y] == 1U) {
                leds[XY(x, y)] = CHSV(32U, 200U, 255U);
              } else {
                leds[XY(x, y)] = CHSV(128U, 32U, 255U);
              }
            }
          }
        }
      } else {
        if (y == PADDING) {
          leds[XY(x, y)] = CHSV(hue, 255U, 255U);
        } else {
          leds[XY(x, y)] = CHSV(96U, 128U, 80U + y * 32U);
        }
      }
    }
  }

  if (step % 4U == 0U) {
    for (uint16_t i = 0; i < matrixWidth; i++) {
      if (i % 6U == 0U) {
        uint8_t current = noise3d[0][i][1];
        uint8_t target  = noise3d[0][i][0];
        if (current < target) current++;
        if (current > target) current--;
        noise3d[0][i][1] = current;
      }
    }
  }

  if (step % 128U == 0U) {
    for (uint16_t i = 0; i < matrixWidth; i++) {
      if (i % 6U == 0U) {
        uint8_t target = random8(PADDING + 1, matrixHeight - 1);
        if (target % 2U == 1U) target++;
        if (target >= matrixHeight) target = matrixHeight - 1;
        noise3d[0][i][0] = target;
      }
    }
  }

  hue++;
  step++;
}

// =============================================================================== АВРОРА =============================================================
//             © SlingMaster
// =====================================

void Avrora() {
  static uint16_t lastWidth = 0, lastHeight = 0;

  if (lastWidth != matrixWidth || lastHeight != matrixHeight) {
    lastWidth = matrixWidth;
    lastHeight = matrixHeight;
  }

  const byte PADDING = matrixHeight * 0.25;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(50, random8(2, 254U));
    }
#endif
    loadingFlag = false;
    deltaValue = 0;
    hue = 0;
    FastLED.clear();
  }

  byte step1 = map8(modes[currentMode].Speed, 10U, 60U);
  uint16_t ms = millis();
  double freq = 3000;
  const byte fade = 30;
  fadeToBlackBy(leds, usedLeds, fade);

  for (uint16_t y = 0; y < matrixHeight; y++) {
    uint32_t yy = y * 256;
    uint32_t x1 = beatsin16(step1, matrixWidth, (matrixHeight - 1) * 256, matrixWidth, y * freq + 32768) / 1.5;

    /* change color -------- */
    byte cur_color = ms / 29 + y * 256 / matrixHeight;
    CRGB color = CHSV(cur_color, 255, 255 - y * matrixHeight / 8);
    byte br = constrain(255 - y * matrixHeight / 5, 0, 200);
    CRGB color2 = CHSV(cur_color - 32, 255 - y * matrixHeight / 4, br);

    wu_pixel(x1 + hue + PADDING * hue / 2, yy, &color);
    wu_pixel(((matrixWidth - 1) * 256 - (x1 + hue)), yy - PADDING * hue, &color2);
  }

  step++;
  if (step % 64) {
    if (deltaValue == 1) {
      hue++;
      if (hue >= 255) {
        deltaValue = 0;
      }
    } else {
      hue--;
      if (hue < 1) {
        deltaValue = 1;
      }
    }
  }
}

// ========================================================================= РАДУЖНОЕ ПЯТНО ===========================================================
//             © SlingMaster
// =====================================

void RainbowSpot() {
  static float distance;
  static uint8_t centerX, centerY;
  static uint8_t maxRadius;
  static uint8_t STEP;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(100), random8(2, 254U));
    }
#endif
    loadingFlag = false;
    deltaValue = modes[currentMode].Scale;
    hue = 96;
    emitterY = 0.0f;
    pcnt = random8(matrixHeight / 2);
    FastLED.clear();
  }

  centerX = matrixWidth / 2;
  centerY = matrixHeight / 2;
  maxRadius = max(centerX, centerY);
  STEP = (centerX > 0) ? (255 / centerX) : 1;

  float radius = abs(128 - step) / 127.0f * maxRadius;

  for (uint16_t x = 0; x < matrixWidth; x++) {
    for (uint16_t y = 0; y < matrixHeight; y++) {
      float dx = (float)x - (centerX - 1);
      float dy = (float)y - (centerY - emitterY);
      distance = sqrt(dx * dx + dy * dy);
      uint8_t distInt = (uint8_t)constrain(distance, 0, 255);

      uint8_t currentHue = step + (uint8_t)(distance * radius);
      uint8_t brightness = 200 - (uint8_t)(STEP * distance * 0.25f);

      if (distance < radius) {
        if (modes[currentMode].Scale > 50) {
          if ((x & 1) && (y & 1)) {
            float yf = y - centerY / 2.0f + emitterY;
            drawPixelXYF(x, yf, CHSV(currentHue, 255, 64));
          } else {
            leds[XY(x, y)] = CHSV(currentHue + 32, 255 - distInt, brightness);
          }
        } else {
          leds[XY(x, y)] = CHSV(currentHue, 255 - distInt, 255);
        }
      } else {
        if (modes[currentMode].Scale > 75) {
          leds[XY(x, y)] = CHSV(currentHue + 96, 255, brightness);
        } else {
          leds[XY(x, y)] = CHSV(currentHue, 255, brightness);
        }
      }
    }
  }

  if (modes[currentMode].Scale > 50) {
    if (emitterY > pcnt) {
      emitterY -= 0.25f;
    } else if (emitterY < pcnt) {
      emitterY += 0.25f;
    } else {
      pcnt = random8(centerY);
    }
  } else {
    emitterY = 0.0f;
  }

  blurScreen(48);
  step++;
}

// =========================================================================== ФОНТАН =================================================================
//             © SlingMaster
// =====================================

void Fountain() {
  uint8_t const gamma[6] = {0, 96, 128, 160, 240, 112};
  const byte PADDING = round(matrixHeight / 8);
  byte br;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(100), random8(2, 254U));
    }
#endif
    loadingFlag = false;
    deltaValue = modes[currentMode].Scale / 20;
    emitterY = 0;
    FastLED.clear();
  }

  float radius = abs(128 - step) / 127.0 * CENTER_Y_MINOR;
  for (uint8_t y = 0; y < matrixHeight; y++) {
    for (uint8_t x = 0; x < matrixWidth; x++) {

      if (x % 2 == 0) {
        br = constrain(255 / (emitterY + 1) * y, 48, 255);

        if ((x % 4) == 0) {
          hue = gamma[deltaValue];
          if (y == byte(emitterY - radius) + random8(1, 4)) {
            if (step % 2 == 0) {
              drawPixelXYF(x, y + 0.5, CHSV(hue, 200, 255));
            } else {
              drawPixelXY(x, y, CHSV(hue, 200, 255));
            }
          } else {
            drawPixelXY(x, y, CHSV(hue, 255, (y > emitterY - radius / 2) ? 0 : br));
          }
        } else {
          hue = gamma[deltaValue + 1];
          if (y == byte(emitterY * 0.70 + radius + random8(3))) {
            drawPixelXYF(x, y - 0.5, CHSV(hue - radius, 160, 255));
          } else {
            byte delta = emitterY * 0.70 + radius;
            drawPixelXY(x, y, CHSV(hue - radius, 255,  ( y > delta) ? 0 : br));
          }
        }
      } else {
        // clear blur ----
        if (pcnt > PADDING + 2) drawPixelXY(x, y, CRGB(0, 0, 0));
      }
    }
  }

  if ((emitterY <= PADDING * 2) | (emitterY > matrixHeight - PADDING - 1)) blurScreen(32);

  if (emitterY > pcnt) {
    emitterY -= 0.5;
    if (abs(pcnt - emitterY ) < PADDING) {
      if (emitterY > pcnt) emitterY -= 0.5;
    }
  } else {
    if (emitterY < pcnt) {
      emitterY += 3;
    } else {
      pcnt = random8(2, matrixHeight - PADDING - 1);
    }
  }
  step++;
}

// ==================================================================== РАДУЖНЫЕ КОЛЬЦА ===============================================================
//    base code © Martin Kleppe @aemkei
//             © SlingMaster
// =====================================

float codeEff(double t, double x, double y, float radius, uint8_t hueOffset, float fadeFactor = 1.0) {
  float distance = sqrt((x - CENTER_X_MAJOR) * (x - CENTER_X_MAJOR) + (y - CENTER_Y_MAJOR) * (y - CENTER_Y_MAJOR));
  float wave = sin16((t * 2.0 - distance + radius) * 8192.0) / 32767.0;
  wave = (wave + 1.0) / 2.0;
  wave *= 0.7;
  return wave * fadeFactor;
}

void drawFrame(double t, double x, double y, float radius, uint8_t hueOffset, float fadeFactor = 1.0) {
  float distance = sqrt((x - CENTER_X_MAJOR) * (x - CENTER_X_MAJOR) + (y - CENTER_Y_MAJOR) * (y - CENTER_Y_MAJOR));
  if (abs(distance - radius) < 2.0) {
    float frame = codeEff(t, x, y, radius, hueOffset, fadeFactor);
    if (frame > 0.01) {
      uint8_t brightness = (uint8_t)(frame * 255);
      CRGB color = ColorFromPalette(*curPalette, hueOffset, brightness);
      drawPixelXY(x, y, color);
    } else {
      drawPixelXY(x, y, CRGB(0, 0, 0));
    }
  }
}

void RainbowRings() {
  static uint32_t lastUpdateTime = 0;
  static float ringRadii[10];
  static uint8_t ringHues[10];
  static uint8_t activeRings = 0;
  static uint8_t prevScale = 0;
  static uint32_t colorChangeTime = 0;
  static uint8_t baseHue = 0;
  static const uint8_t maxActiveRings = 5;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(10U + random8(90U), 100U + random8(100U));
    }
#endif
    loadingFlag = false;
    prevScale = 0;
    setCurrentPalette();
    lastUpdateTime = millis();
    colorChangeTime = millis();
    baseHue = 0;
    activeRings = maxActiveRings;
    float spacing = max(matrixWidth, matrixHeight) * 2.0f / maxActiveRings;
    for (uint8_t i = 0; i < maxActiveRings; i++) {
      ringRadii[i] = i * spacing;
      ringHues[i] = baseHue + (i * (256 / maxActiveRings));
    }
  }

  if (prevScale != modes[currentMode].Scale) {
    prevScale = modes[currentMode].Scale;
    baseHue = map(modes[currentMode].Scale, 1U, 100U, 0U, 255U);
    for (uint8_t i = 0; i < activeRings; i++) {
      ringHues[i] = baseHue + (i * (256 / maxActiveRings));
    }
  }

  float speedFactor = (float)modes[currentMode].Speed / 255.0f;

  uint32_t colorInterval = 300 - (uint32_t)(speedFactor * 200);
  if (millis() - colorChangeTime > colorInterval) {
    baseHue += 2 + (uint8_t)(speedFactor * 5);
    for (uint8_t i = 0; i < activeRings; i++) {
      ringHues[i] = baseHue + (i * (256 / maxActiveRings));
    }
    colorChangeTime = millis();
  }

  float ringSpeed = 0.6f + speedFactor * 2.4f;
  uint32_t currentTime = millis();
  float deltaTime = (currentTime - lastUpdateTime) / 1000.0f;

  for (uint8_t i = 0; i < activeRings; i++) {
    ringRadii[i] += ringSpeed * deltaTime;
    float maxRadius = max(matrixWidth, matrixHeight) * 2.0f;
    if (ringRadii[i] >= maxRadius) {
      ringRadii[i] = 0.0f;
      ringHues[i] = baseHue + (i * (256 / maxActiveRings));
    }
  }
  lastUpdateTime = currentTime;

  float cx = matrixWidth / 2.0f;
  float cy = matrixHeight / 2.0f;

  for (uint16_t x = 0; x < matrixWidth; x++) {
    for (uint16_t y = 0; y < matrixHeight; y++) {
      float dx = (float)x - cx;
      float dy = (float)y - cy;
      float distance = sqrt(dx * dx + dy * dy);

      uint8_t totalR = 0, totalG = 0, totalB = 0;
      uint8_t ringsCount = 0;

      for (uint8_t i = 0; i < activeRings; i++) {
        float ringDist = fabs(distance - ringRadii[i]);
        if (ringDist < 2.0f) {
          float intensity = 1.0f - (ringDist / 2.0f);
          intensity = constrain(intensity, 0.0f, 1.0f);

          CRGB color = ColorFromPalette(*curPalette, ringHues[i], 255);
          totalR += (uint8_t)(color.r * intensity);
          totalG += (uint8_t)(color.g * intensity);
          totalB += (uint8_t)(color.b * intensity);
          ringsCount++;
        }
      }

      if (ringsCount > 0) {
        totalR /= ringsCount;
        totalG /= ringsCount;
        totalB /= ringsCount;
        uint16_t idx = XY(x, y);
        if (idx < usedLeds) {
          leds[idx] = CRGB(totalR, totalG, totalB);
        }
      } else {
        uint16_t idx = XY(x, y);
        if (idx < usedLeds) {
          leds[idx] = CRGB::Black;
        }
      }
    }
  }
}

// ============================================================================== БАБОЧКА =============================================================
void butterflyRoutine() {
  static uint32_t colorChangeTime = 0;
  static uint8_t baseHue = 0;
  static uint8_t prevScale = 0;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(10U + random8(90U), 100U + random8(100U));
    }
#endif
    loadingFlag = false;
    enlargedObjectNUM = map(modes[currentMode].Scale, 1U, 100U, 1U, min(static_cast<uint8_t>(enlargedOBJECT_MAX_COUNT), static_cast<uint8_t>(5)));
    setCurrentPalette();
    baseHue = map(modes[currentMode].Scale, 1U, 100U, 0U, 255U);
    prevScale = modes[currentMode].Scale;
    for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
      trackingObjectPosX[i] = random8(matrixWidth);
      trackingObjectPosY[i] = random8(matrixHeight);
      trackingObjectSpeedX[i] = (float)random8(10, 20) / 10.0 * (random8(2) ? 1 : -1);
      trackingObjectSpeedY[i] = (float)random8(10, 20) / 10.0 * (random8(2) ? 1 : -1);
      trackingObjectHue[i] = baseHue + (i * (256 / enlargedObjectNUM));
      trackingObjectState[i] = 0;
      trackingObjectIsShift[i] = true;
      enlargedObjectTime[i] = millis();
    }
    dimAll(0);
  }

  if (prevScale != modes[currentMode].Scale) {
    prevScale = modes[currentMode].Scale;
    baseHue = map(modes[currentMode].Scale, 1U, 100U, 0U, 255U);
    for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
      trackingObjectHue[i] = baseHue + (i * (256 / enlargedObjectNUM));
    }
  }

  float speedFactor = (float)modes[currentMode].Speed / 255.0;

  uint32_t colorInterval = 300 - (uint32_t)(speedFactor * 200);
  if (millis() - colorChangeTime > colorInterval) {
    baseHue += 2 + (uint8_t)(speedFactor * 5);
    for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
      trackingObjectHue[i] = baseHue + (i * (256 / enlargedObjectNUM));
    }
    colorChangeTime = millis();
  }

  dimAll(230);

  for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
    if (!trackingObjectIsShift[i]) continue;

    trackingObjectPosX[i] += trackingObjectSpeedX[i] * speedFactor;
    trackingObjectPosY[i] += trackingObjectSpeedY[i] * speedFactor;

    if (trackingObjectPosX[i] < 0 || trackingObjectPosX[i] >= matrixWidth) {
      trackingObjectSpeedX[i] = -trackingObjectSpeedX[i];
      trackingObjectPosX[i] = constrain(trackingObjectPosX[i], 0, matrixWidth - 1);
    }
    if (trackingObjectPosY[i] < 0 || trackingObjectPosY[i] >= matrixHeight) {
      trackingObjectSpeedY[i] = -trackingObjectSpeedY[i];
      trackingObjectPosY[i] = constrain(trackingObjectPosY[i], 0, matrixHeight - 1);
    }

    uint8_t wingPhase = (millis() - enlargedObjectTime[i]) / 100;
    float wingSize = 1.0 + 0.5 * sin((float)wingPhase * PI / 8.0);

    CRGB color = ColorFromPalette(*curPalette, trackingObjectHue[i]);

    drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], color);
    drawPixelXYF(trackingObjectPosX[i] + wingSize, trackingObjectPosY[i] + wingSize, makeDarker(color, 50));
    drawPixelXYF(trackingObjectPosX[i] - wingSize, trackingObjectPosY[i] + wingSize, makeDarker(color, 50));
    drawPixelXYF(trackingObjectPosX[i] + wingSize, trackingObjectPosY[i] - wingSize, makeDarker(color, 50));
    drawPixelXYF(trackingObjectPosX[i] - wingSize, trackingObjectPosY[i] - wingSize, makeDarker(color, 50));
  }
}

// ========================================================================== НОВЫЕ ЗВЁЗДЫ ============================================================
void newStars() {
#define MAX_STARS 30
#define TWO_PI 6.28318530718
  static uint32_t lastUpdateTime = 0;
  static struct Star {
    float x, y;
    uint8_t hue;
    float brightness;
    float speed;
    float size;
    bool active;
    float lifetime;
  } stars[MAX_STARS];
  static uint8_t activeStars = 0;
  static uint8_t prevScale = 0;
  static uint8_t baseHue = 0;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(10U + random8(90U), 100U + random8(100U));
    }
#endif
    loadingFlag = false;
    prevScale = 0;
    setCurrentPalette();
    dimAll(0);
    lastUpdateTime = millis();
    baseHue = map(modes[currentMode].Scale, 1U, 100U, 0U, 255U);
    activeStars = 0;
    for (uint8_t i = 0; i < MAX_STARS; i++) {
      stars[i].active = false;
    }
  }

  float speedFactor = (float)modes[currentMode].Speed / 255.0;

  if (prevScale != modes[currentMode].Scale) {
    prevScale = modes[currentMode].Scale;
    baseHue = map(modes[currentMode].Scale, 1U, 100U, 0U, 255U);
    for (uint8_t i = 0; i < MAX_STARS; i++) {
      if (stars[i].active) {
        stars[i].hue = baseHue + random8(32);
      }
    }
  }

  uint8_t dimValue = map(modes[currentMode].Scale, 1, 100, 225, 240);
  dimAll(dimValue);

  uint8_t desiredStars = map(modes[currentMode].Scale, 1, 100, 3, MAX_STARS);

  uint32_t currentTime = millis();
  float deltaTime = (currentTime - lastUpdateTime) / 1000.0;

  for (uint8_t i = 0; i < MAX_STARS; i++) {
    if (stars[i].active) {
      stars[i].brightness += stars[i].speed * deltaTime * (0.8 + speedFactor * 2.5);
      if (stars[i].brightness > TWO_PI) {
        stars[i].brightness -= TWO_PI;
      }
      float bright = (sin(stars[i].brightness) * 0.5 + 0.5) * (sin(stars[i].brightness * 1.5) * 0.5 + 0.5);
      bright = constrain(bright, 0.0, 1.0);
      uint8_t pixelBright = (uint8_t)(bright * 200);

      stars[i].lifetime -= deltaTime;

      if (pixelBright > 5 && stars[i].lifetime > 0) {
        CRGB color = CHSV(stars[i].hue, 200, pixelBright);
        if (stars[i].size <= 1.0) {
          drawPixelXY((uint8_t)stars[i].x, (uint8_t)stars[i].y, color);
        } else {
          uint8_t x = (uint8_t)stars[i].x;
          uint8_t y = (uint8_t)stars[i].y;
          drawPixelXY(x, y, color);
          if (x + 1 < matrixWidth) drawPixelXY(x + 1, y, color);
          if (y + 1 < matrixHeight) drawPixelXY(x, y + 1, color);
          if (x + 1 < matrixWidth && y + 1 < matrixHeight) drawPixelXY(x + 1, y + 1, color);
        }
      } else {
        if (stars[i].size <= 1.0) {
          drawPixelXY((uint8_t)stars[i].x, (uint8_t)stars[i].y, CRGB(0, 0, 0));
        } else {
          uint8_t x = (uint8_t)stars[i].x;
          uint8_t y = (uint8_t)stars[i].y;
          drawPixelXY(x, y, CRGB(0, 0, 0));
          if (x + 1 < matrixWidth) drawPixelXY(x + 1, y, CRGB(0, 0, 0));
          if (y + 1 < matrixHeight) drawPixelXY(x, y + 1, CRGB(0, 0, 0));
          if (x + 1 < matrixWidth && y + 1 < matrixHeight) drawPixelXY(x + 1, y + 1, CRGB(0, 0, 0));
        }
        stars[i].active = false;
        activeStars--;
      }
    }
  }

  if (activeStars < desiredStars) {
    uint8_t spawnChance = 10 + (uint8_t)(speedFactor * 15);
    if (random8(100) < spawnChance) {
      for (uint8_t i = 0; i < MAX_STARS; i++) {
        if (!stars[i].active) {
          stars[i].x = random8(matrixWidth);
          stars[i].y = random8(matrixHeight);
          stars[i].hue = baseHue + random8(32);
          stars[i].brightness = random8() / 255.0 * TWO_PI;
          stars[i].speed = random(600, 1800) / 1000.0;
          stars[i].size = random8(100) < 20 ? 2.0 : 1.0;
          stars[i].lifetime = random(2000, 5000) / 1000.0;
          stars[i].active = true;
          activeStars++;
          break;
        }
      }
    }
  }

  lastUpdateTime = currentTime;
}

// ============================================================================ ФЛАГ (ТРИКОЛОР) =======================================================
void FlagRoutine() {
  static uint32_t lastUpdateTime = 0;
  static float offset = 0.0;
  static float xOffset = 0.0;
  static uint8_t prevScale = 0;
  static uint16_t lastWidth = 0, lastHeight = 0;

  if (lastWidth != matrixWidth || lastHeight != matrixHeight) {
    lastWidth = matrixWidth;
    lastHeight = matrixHeight;
    loadingFlag = true;
  }

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(10U + random8(90U), 100U + random8(100U));
    }
#endif
    loadingFlag = false;
    prevScale = modes[currentMode].Scale;
    offset = 0.0;
    xOffset = 0.0;
    lastUpdateTime = millis();
    dimAll(0);
  }

  dimAll(240);

  float speedFactor = (float)modes[currentMode].Speed / 255.0;
  float scrollSpeed = 2.0 + speedFactor * 8.0;
  float xScrollSpeed = scrollSpeed * ((float)modes[currentMode].Scale / 100.0);
  float waveSpeed = 0.5 + speedFactor * 1.0;
  float waveAmplitude = map(modes[currentMode].Scale, 1U, 100U, 1.0, 4.0);

  uint8_t flagRepeats = map(modes[currentMode].Scale, 1U, 100U, 1U, 4U);
  if (prevScale != modes[currentMode].Scale) {
    prevScale = modes[currentMode].Scale;
    dimAll(0);
    lastUpdateTime = millis();
  }

  float flagHeight = (float)matrixHeight / flagRepeats;
  float stripeHeight = flagHeight / 3.0;

  uint32_t currentTime = millis();
  float deltaTime = (currentTime - lastUpdateTime) / 1000.0;
  if (deltaTime > 0.1) deltaTime = 0.1;

  offset += scrollSpeed * deltaTime;
  xOffset += xScrollSpeed * deltaTime;
  if (offset >= flagHeight) offset -= flagHeight;
  if (xOffset >= (float)matrixWidth) xOffset -= (float)matrixWidth;

  for (uint16_t x = 0; x < matrixWidth; x++) {
    for (uint16_t y = 0; y < matrixHeight; y++) {
      float waveOffset = waveAmplitude * sin(((float)x + xOffset) / matrixWidth * TWO_PI + currentTime / 1000.0 * waveSpeed);
      float adjustedY = (float)y + offset + waveOffset;
      if (adjustedY >= flagHeight) adjustedY -= flagHeight;
      if (adjustedY < 0) adjustedY += flagHeight;

      float stripePos = (flagHeight - adjustedY) / stripeHeight;
      uint8_t stripeIndex = (uint8_t)stripePos;
      CRGB color;
      switch (stripeIndex % 3) {
        case 0: color = CRGB(255, 255, 255); break;
        case 1: color = CRGB(0, 0, 255); break;
        case 2: color = CRGB(255, 0, 0); break;
      }
      drawPixelXY(x, y, color);
    }
  }

  lastUpdateTime = currentTime;
}

// ============================================================================== МЕТЕОР ==============================================================
void meteorRoutine() {
  static float posX = 0, posY = 0;
  static float speedX = 0, speedY = 0;
  static uint8_t tailLength = 0;
  static uint8_t prevScale = 0;
  static uint32_t lastSpark = 0;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(40U + random8(80U), 70U + random8(120U));
    }
#endif
    loadingFlag = false;
    prevScale = 0;
    tailLength = 7;
    lastSpark = 0;
    prevScale = 0;

    posX = random8(matrixWidth);
    posY = random8(matrixHeight);
    speedX = random(12, 28) * (random(0, 2) ? 1.0f : -1.0f);
    speedY = random(12, 28) * (random(0, 2) ? 1.0f : -1.0f);
    ballColor = CHSV(random(0, 9) * 28 + random(0, 40), 255, 255);
  }

  // регулировка длины хвоста ползунком Масштаб/Цвет
  if (prevScale != modes[currentMode].Scale) {
    prevScale = modes[currentMode].Scale;
    tailLength = map(modes[currentMode].Scale, 1, 100, 7, max(matrixWidth, matrixHeight) / 2 + 6);
  }

  // регулировка скорости
  float speedFactor = map(modes[currentMode].Speed, 1, 255, 8, 45) / 10.0f;

  posX += speedX * speedFactor * 0.08f;
  posY += speedY * speedFactor * 0.08f;

  // отскоки
  if (posX <= 0 || posX >= matrixWidth - 1) {
    speedX = -speedX;
    posX = constrain(posX, 0, matrixWidth - 1);
    if (random8(4) == 0) ballColor = CHSV(random(0, 9) * 28 + random(0, 40), 255, 255);
  }
  if (posY <= 0 || posY >= matrixHeight - 1) {
    speedY = -speedY;
    posY = constrain(posY, 0, matrixHeight - 1);
  }

  dimAll(205);

  uint8_t x = (uint8_t)posX;
  uint8_t y = (uint8_t)posY;

  leds[XY(x, y)] = CRGB::White;
  leds[XY(x, y)] += CRGB(255, 140, 40);

  if (x > 0) leds[XY(x - 1, y)] = ballColor;
  if (x < matrixWidth - 1) leds[XY(x + 1, y)] = ballColor;
  if (y > 0) leds[XY(x, y - 1)] = ballColor;
  if (y < matrixHeight - 1) leds[XY(x, y + 1)] = ballColor;

  float trailStepX = -speedX * 0.35f;
  float trailStepY = -speedY * 0.35f;

  for (uint8_t i = 1; i < tailLength; i++) {
    uint8_t tx = (uint8_t)(posX + trailStepX * i);
    uint8_t ty = (uint8_t)(posY + trailStepY * i);

    if (tx >= matrixWidth || ty >= matrixHeight) break;

    uint8_t bright = map(i, 1, tailLength, 220, 30);
    CRGB color = ballColor;
    color.nscale8(bright);
    leds[XY(tx, ty)] = color;
  }

  if (millis() - lastSpark > 40 && random8(100) < 60) {
    lastSpark = millis();
    uint8_t sx = x + random(-3, 4);
    uint8_t sy = y + random(-3, 4);
    if (sx < matrixWidth && sy < matrixHeight) {
      leds[XY(sx, sy)] = CRGB(255, 220, 100);
    }
  }
}

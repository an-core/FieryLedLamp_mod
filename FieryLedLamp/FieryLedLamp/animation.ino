// ************************************************************************* animation.ino *************************************************************
#include "Prototypes.h"
#include "Extern.h"
#include "Types.h"
#include "Constants.h"
// ----------------------

#if USE_ANIMATIONS

#include "animations.h"

const animation_t animations[] = {
  animation_heart,
  animation_mario
};

// -------------------------------------------------
#ifndef CountTokens
int CountTokens(const char* str, char delimiter) {
  int count = 0;
  if (!str || *str == '\0') return 0;
  while (*str) {
    if (*str != delimiter) {
      count++;
      while (*str && *str != delimiter) str++;
    } else {
      str++;
    }
  }
  return count;
}
#endif

#ifndef GetToken
String GetToken(const char* str, int index, char delimiter) {
  int current = 0;
  const char* start = str;
  while (*str) {
    if (*str == delimiter) {
      if (current == index) {
        int len = str - start;
        return String(start, len);
      }
      current++;
      start = str + 1;
    }
    str++;
  }
  if (current == index) {
    return String(start);
  }
  return "";
}
#endif

// --------------------------------------------------
void initAnimations() {
  String anim_list = "";
  String fullList = IMAGE_LIST;
  uint8_t use_num = 0;
  uint8_t idx = 0;
  uint8_t image_num = 0;
  String temp = fullList;
  while (temp.length() > 0) {
    int pos = temp.indexOf(',');
    if (pos == -1) {
      image_num++;
      break;
    } else {
      image_num++;
      temp = temp.substring(pos + 1);
    }
  }

  use_animations = new uint8_t[image_num];

  while (fullList.length() > 0) {
    int comma = fullList.indexOf(',');
    String name;
    if (comma == -1) {
      name = fullList;
      fullList = "";
    } else {
      name = fullList.substring(0, comma);
      fullList = fullList.substring(comma + 1);
    }
    name.trim();
    if (name.length() == 0) continue;
    if (idx == 0) {
      use_animations[use_num++] = 0;
      anim_list += name + ",";
#if MATRIX_LOG
      SYSLOG.add("Найденные анимации:");
      SYSLOG.add("   %s\t%dx%d", name.c_str(), animations[0].frame_width, animations[0].frame_height);
#endif
    } else {
      if (animations[idx].frame_width <= matrixWidth && animations[idx].frame_height <= matrixHeight) {
        use_animations[use_num++] = idx;
        anim_list += name + ",";
#if MATRIX_LOG
        SYSLOG.add("   %s\t%dx%d", name.c_str(), animations[idx].frame_width, animations[idx].frame_height);
#endif
      }
    }
    idx++;
  }

  if (anim_list.length() > 0) {
    anim_list.remove(anim_list.length() - 1);
  }

  animations_list = anim_list;
  max_image_num = use_num;

  int savedAnim = jsonReadtoInt(configSetup, "anim_sel");
  if (savedAnim >= 1 && savedAnim <= max_image_num) {
    specialTextEffectParam = savedAnim;
  } else {
    specialTextEffectParam = 1;
  }
}

static int8_t   currentImageIdx = 1;
static animation_t image_desc;
static int8_t   pos_x = 0, pos_y = 0;
static int8_t   edge_left = 0, edge_right = 0;
static int8_t   edge_bottom = 0, edge_top = 0;
static int8_t   rcNum = 0;
static uint8_t  frameNum = 0;
static uint8_t  frames_in_image = 0;
static bool     first_draw = false;
static bool     frame_completed = false;
static bool     image_completed = false;
static bool     draw_by_row = false;
static bool     flip_x = false, flip_y = false;
static bool     inverse_dir_x = false, inverse_dir_y = false;
static uint32_t last_draw_row = 0;
static uint32_t last_draw_frame = 0;
static uint32_t last_move_x = 0;
static uint32_t last_move_y = 0;

// ----------------------------------------------------------------------------
static void loadDescriptor(const animation_t (*src_desc)) {
  memcpy_P((void *)&image_desc, src_desc, sizeof(animation_t));
}

// ----------------------------------------------------------------------------
static void drawImageRow(uint8_t row, const uint16_t (*frame)) {
  if (!frame) return;

  uint8_t effectBrightness = (uint16_t)modes[currentMode].Brightness * modes[currentMode].Scale / 100U;
  effectBrightness = constrain(effectBrightness, 0, 255);

  int8_t y = flip_y ? pos_y + image_desc.frame_height - row - 1 : pos_y + row;
  if (y < 0 || y > matrixHeight - 1) return;

  for (uint8_t i = 0; i < image_desc.frame_width; i++) {
    int8_t x = flip_x ? pos_x + image_desc.frame_width - i - 1 : pos_x + i;
    if (x < 0 || x > matrixWidth - 1) continue;

    uint16_t clr = pgm_read_word(&(frame[(image_desc.frame_height - row - 1) * image_desc.frame_width + i]));

    if ((image_desc.options & 4) && clr == image_desc.transparent_color) continue;

    CRGB color = gammaCorrection(expandColor(clr));
    color.nscale8_video(effectBrightness);
    drawPixelXY(x, y, color);
  }
}

// ----------------------------------------------------------------------------
static void drawImageCol(uint8_t col, const uint16_t (*frame)) {
  if (!frame) return;

  uint8_t effectBrightness = (uint16_t)modes[currentMode].Brightness * modes[currentMode].Scale / 100U;
  effectBrightness = constrain(effectBrightness, 0, 255);

  int8_t x = flip_x ? pos_x + image_desc.frame_width - col - 1 : pos_x + col;
  if (x < 0 || x > matrixWidth - 1) return;

  for (uint8_t i = 0; i < image_desc.frame_width; i++) {
    int8_t y = flip_y ? pos_y + image_desc.frame_height - i - 1 : pos_y + i;
    if (y < 0 || y > matrixHeight - 1) continue;

    uint16_t clr = pgm_read_word(&(frame[(image_desc.frame_height - i - 1) * image_desc.frame_width + col]));

    if ((image_desc.options & 4) && clr == image_desc.transparent_color) continue;

    CRGB color = gammaCorrection(expandColor(clr));
    color.nscale8_video(effectBrightness);
    drawPixelXY(x, y, color);
  }
}

// ----------------------------------------------------------------------------
static void loadImageFrame(const uint16_t (*frame)) {
  rcNum = 0;
  for (uint8_t j = 0; j < image_desc.frame_height; j++) {
    drawImageRow(j, frame);
  }
}

// ----------------------------------------------------------------------------
void animationRoutine() {
  const uint16_t *ppFrame;

  uint8_t effectBrightness = (uint16_t)modes[currentMode].Brightness * modes[currentMode].Scale / 100U;
  effectBrightness = constrain(effectBrightness, 0, 255);

  if (loadingFlag) {
    FastLED.clear();

    currentImageIdx = (specialTextEffectParam >= 0) ? specialTextEffectParam : 1;

    if (currentImageIdx == 0 || currentImageIdx > max_image_num) {
      uint8_t att = 0;
      uint8_t idx = random8(1, max_image_num);
      while (max_image_num > 1 && idx == currentImageIdx && att < 6) {
        att++; idx++;
        if (idx > max_image_num) idx = 1;
      }
      currentImageIdx = idx;
    }

    animation_t anim = animations[use_animations[currentImageIdx - 1]];
    frames_in_image = 0;
    for (int i = 0; i < MAX_FRAMES_COUNT; i++) {
      if (anim.frames[i] == NULL) break;
      frames_in_image++;
    }
    loadDescriptor(&anim);

    frameNum = 0;
    last_draw_row = 0;
    last_draw_frame = 0;

    pos_x = image_desc.start_x;
    pos_y = image_desc.start_y;

    if (image_desc.options & 1) pos_x = (matrixWidth  - image_desc.frame_width) / 2;
    if (image_desc.options & 2) pos_y = (matrixHeight - image_desc.frame_height) / 2;

    flip_x = (image_desc.options & 64) > 0;
    flip_y = (image_desc.options & 128) > 0;

    inverse_dir_x = false;
    inverse_dir_y = false;

    if (image_desc.draw_frame_interval < 5) image_desc.draw_frame_interval = 5;

    draw_by_row = image_desc.draw_row_interval > 0;
    switch (image_desc.row_draw_direction) {
      case 0:  rcNum = image_desc.frame_height - 1; break;
      case 3:  rcNum = image_desc.frame_width - 1; break;
      default: rcNum = 0; break;
    }

    if (image_desc.options & 8) {
      CRGB color = image_desc.background_first_color;
      color.nscale8_video(effectBrightness);
      fillAll(color);
    } else if (image_desc.options & 16) {
      CRGB color = image_desc.background_color;
      color.nscale8_video(effectBrightness);
      fillAll(color);
    }

    loadingFlag = false;
    first_draw = true;
    image_completed = false;
    frame_completed = false;

    last_move_x = millis();
    last_move_y = millis();
  }

  if (!draw_by_row && image_desc.move_type != 0) {
    if (image_desc.move_x_interval > 0 && (millis() - last_move_x > image_desc.move_x_interval)) {
      last_move_x = millis();
      edge_left  = (image_desc.move_type & 256) ? (0 - image_desc.frame_width) : 0;
      edge_right = (image_desc.move_type & 256) ? matrixWidth : matrixWidth - image_desc.frame_width;

      if ( ((image_desc.move_type & 0x01) && !inverse_dir_x) || ((image_desc.move_type & 0x02) && inverse_dir_x) ) {
        pos_x++;
        if (pos_x > edge_right) {
          if ((image_desc.move_type & 16) && (image_desc.frame_width < matrixWidth || (image_desc.move_type & 256))) inverse_dir_x = !inverse_dir_x;
          if ((image_desc.move_type & 64) && (image_desc.frame_width < matrixWidth || (image_desc.move_type & 256))) flip_x = !flip_x;
          if (image_desc.move_type & 0x01) pos_x = inverse_dir_x ? edge_right : edge_left;
          else if (image_desc.move_type & 0x02) pos_x = inverse_dir_x ? edge_left : edge_right;
        }
      } else if ( ((image_desc.move_type & 0x02) && !inverse_dir_x) || ((image_desc.move_type & 0x01) && inverse_dir_x) ) {
        pos_x--;
        if (pos_x < edge_left) {
          if ((image_desc.move_type & 16) && (image_desc.frame_width < matrixWidth || (image_desc.move_type & 256))) inverse_dir_x = !inverse_dir_x;
          if ((image_desc.move_type & 64) && (image_desc.frame_width < matrixWidth || (image_desc.move_type & 256))) flip_x = !flip_x;
          if (image_desc.move_type & 0x01) pos_x = inverse_dir_x ? edge_right : edge_left;
          else if (image_desc.move_type & 0x02) pos_x = inverse_dir_x ? edge_left : edge_right;
        }
      }
    }

    if (image_desc.move_y_interval > 0 && (millis() - last_move_y > image_desc.move_y_interval)) {
      last_move_y = millis();
      edge_bottom = (image_desc.move_type & 512) ? (0 - image_desc.frame_height) : 0;
      edge_top    = (image_desc.move_type & 512) ? matrixHeight : matrixHeight - image_desc.frame_height;

      if ( ((image_desc.move_type & 0x04) && !inverse_dir_y) || ((image_desc.move_type & 0x08) && inverse_dir_y) ) {
        pos_y++;
        if (pos_y > edge_top) {
          if ((image_desc.move_type & 32) && (image_desc.frame_height < matrixHeight || (image_desc.move_type & 512))) inverse_dir_y = !inverse_dir_y;
          if ((image_desc.move_type & 128) && (image_desc.frame_height < matrixHeight || (image_desc.move_type & 512))) flip_y = !flip_y;
          if (image_desc.move_type & 0x04) pos_y = inverse_dir_y ? edge_top : edge_bottom;
          else if (image_desc.move_type & 0x08) pos_y = inverse_dir_y ? edge_bottom : edge_top;
        }
      } else if ( ((image_desc.move_type & 0x08) && !inverse_dir_y) || ((image_desc.move_type & 0x04) && inverse_dir_y) ) {
        pos_y--;
        if (pos_y < edge_bottom) {
          if ((image_desc.move_type & 32) && (image_desc.frame_height < matrixHeight || (image_desc.move_type & 512))) inverse_dir_y = !inverse_dir_y;
          if ((image_desc.move_type & 128) && (image_desc.frame_height < matrixHeight || (image_desc.move_type & 512))) flip_y = !flip_y;
          if (image_desc.move_type & 0x04) pos_y = inverse_dir_y ? edge_top : edge_bottom;
          else if (image_desc.move_type & 0x08) pos_y = inverse_dir_y ? edge_bottom : edge_top;
        }
      }
    }
  }

  if (draw_by_row && !frame_completed) {
    if (millis() - last_draw_row < image_desc.draw_row_interval) return;
  }

  bool need_change_frame = (millis() - last_draw_frame >= image_desc.draw_frame_interval);
  if (!need_change_frame) return;

  if (!first_draw && frame_completed && (image_desc.options & 16)) {
    CRGB color = image_desc.background_color;
    color.nscale8_video(effectBrightness);
    fillAll(color);
  }

  first_draw = false;
  frame_completed = false;
  image_completed = false;

  animation_t anim = animations[use_animations[currentImageIdx - 1]];
  ppFrame = anim.frames[frameNum];

  if (draw_by_row) {
    if (image_desc.row_draw_direction < 2) {
      drawImageRow(rcNum, ppFrame);
    } else {
      drawImageCol(rcNum, ppFrame);
    }
  } else {
    loadImageFrame(ppFrame);
  }

  if (draw_by_row) {
    if (image_desc.row_draw_direction == 0) {
      if (--rcNum < 0) {
        frame_completed = true;
        rcNum = image_desc.frame_height - 1;
      }
    } else if (image_desc.row_draw_direction == 1) {
      if (++rcNum >= image_desc.frame_height) {
        frame_completed = true;
        rcNum = 0;
      }
    } else if (image_desc.row_draw_direction == 2) {
      if (++rcNum >= image_desc.frame_width) {
        frame_completed = true;
        rcNum = 0;
      }
    } else if (image_desc.row_draw_direction == 3) {
      if (--rcNum < 0) {
        frame_completed = true;
        rcNum = image_desc.frame_width - 1;
      }
    }
    last_draw_row = millis();
  } else {
    frame_completed = true;
  }

  if (frame_completed && need_change_frame) {
    last_draw_frame = millis();
    if (++frameNum >= frames_in_image) {
      image_completed = true;
      frameNum = 0;
    }
  }
}

// для расширения 16-битного цвета
static const uint8_t PROGMEM gamma5[] = {
  0x00, 0x01, 0x02, 0x03, 0x05, 0x07, 0x09, 0x0b,
  0x0e, 0x11, 0x14, 0x18, 0x1d, 0x22, 0x28, 0x2e,
  0x36, 0x3d, 0x46, 0x4f, 0x59, 0x64, 0x6f, 0x7c,
  0x89, 0x97, 0xa6, 0xb6, 0xc7, 0xd9, 0xeb, 0xff
};
static const uint8_t PROGMEM gamma6[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08,
  0x09, 0x0a, 0x0b, 0x0d, 0x0e, 0x10, 0x12, 0x13,
  0x15, 0x17, 0x19, 0x1b, 0x1d, 0x20, 0x22, 0x25,
  0x27, 0x2a, 0x2d, 0x30, 0x33, 0x37, 0x3a, 0x3e,
  0x41, 0x45, 0x49, 0x4d, 0x52, 0x56, 0x5b, 0x5f,
  0x64, 0x69, 0x6e, 0x74, 0x79, 0x7f, 0x85, 0x8b,
  0x91, 0x97, 0x9d, 0xa4, 0xab, 0xb2, 0xb9, 0xc0,
  0xc7, 0xcf, 0xd6, 0xde, 0xe6, 0xee, 0xf7, 0xff
};

static uint32_t expandColor(uint16_t color) {
  return ((uint32_t)pgm_read_byte(&gamma5[ color >> 11 ]) << 16) | ((uint32_t)pgm_read_byte(&gamma6[(color >> 5) & 0x3F]) <<  8) | pgm_read_byte(&gamma5[ color & 0x1F]);
}

#endif // USE_ANIMATIONS

// *****************************************************************************************************************************************************

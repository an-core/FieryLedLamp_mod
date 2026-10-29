// ************************************************************************ ir_rf_functions.ino *********************************************************
#include "Types.h"
// ----------------

// ============================================================================= ФУНКЦИИ IR ============================================================

#if USE_IR_RECEIVER

// --------------------------------------------------------------------------------------

uint32_t getIRCode(const String& key) {
  return irManager.getCode(key);
}

// --------------------------------------------------------------------------------------

void IR_Receive_Handle() {
  if (!irEnabled) return;
  if (irrecv.decode(&results)) {
    lastIRCode = results.value;
    if (results.repeat) {
      if (millis() - IR_Repeat_Timer > IR_REPEAT_TIMER) {
        IR_Data_Ready = 2;
      }
    } else {
      IR_Code = (uint32_t)results.value;
      IR_Repeat_Timer = millis();
      IR_Data_Ready = 1;
    }
    irrecv.resume();
  }

  // таймаут ввода двухзначного номера эффекта
  if (Enter_Digit_1 && millis() - IR_Dgit_Enter_Timer > IR_DIGIT_ENTER_TIMER) {
    Enter_Digit_1 = 0;

    if (Enter_Number < MODE_AMOUNT) {
      currentMode = eff_num_correct[Enter_Number];
      jsonWrite(configSetup, "eff_sel", Enter_Number);
      jsonWrite(configSetup, "br", modes[currentMode].Brightness);
      jsonWrite(configSetup, "sp", modes[currentMode].Speed);
      jsonWrite(configSetup, "sc", modes[currentMode].Scale);

#if USE_MP3_PLAYER
      if (ONflag && mp3Enabled && mp3_player_connect >= 4 && eff_sound_on) {
        uint8_t newFolder = effects_folders[currentMode];
        if (mp3_folder != newFolder) {
          mp3_folder = newFolder;
          mp3_folder_last = newFolder;
          mp3SendPlayFolder(newFolder);
        }
      }
#endif

      SetBrightness(modes[currentMode].Brightness);
      loadingFlag = true;

      if (random_on && Favorites::instance().FavoritesRunning) selectedSettings = 1U;

#if USE_MQTT
      if (Wifi::instance().isConnected()) {
        Mqtt::instance().needToPublish = true;
      }
#endif
#if USE_BLYNK
      updateRemoteBlynkParams();
#endif
#if USE_MULTILAMP
      repeat_multiple_lamp_control = true;
#endif
    }
  }
}

#endif

// --------------------------------------------------------------------------------------

#if (USE_IR_RECEIVER || USE_RF_RECEIVER)
void IR_Time() {
#if USE_MP3_PLAYER
  if (mp3_player_connect >= 4) {
    mp3_pending_time_advert = true;
    mp3_pending_time_force  = true;
  } else
#endif
    if (myTime.isTimeSet()) {
      printTime(true);
    } else {
      fillString("NO TIME", CRGB::Red, true);
    }
}

void IR_Weather() {
#if USE_WEATHER
#if USE_MP3_PLAYER
  if (mp3_player_connect >= 4) {
    mp3_pending_weather_advert = true;
    mp3_pending_weather_force  = true;
  } else {
    uint8_t oldPrintWeather = PRINT_WEATHER;
    PRINT_WEATHER = 1;
    printWeather();
    PRINT_WEATHER = oldPrintWeather;
  }
#else
  uint8_t oldPrintWeather = PRINT_WEATHER;
  PRINT_WEATHER = 1;
  printWeather();
  PRINT_WEATHER = oldPrintWeather;
#endif // USE_MP3_PLAYER
#endif // USE_WEATHER
}
#endif // USE_IR_RECEIVER || USE_RF_RECEIVER

// --------------------------------------------------------------------------------------
#if USE_IR_RECEIVER
static void act_power() {
  if (IR_Data_Ready != 2) IR_Power();
}
static void act_mute() {
  if (IR_Data_Ready != 2) Mute();
}
static void act_prev() {
  if (IR_Data_Ready != 2) Prev_Next_eff(false);
}
static void act_next() {
  if (IR_Data_Ready != 2) Prev_Next_eff(true);
}
static void act_cycle() {
  if (IR_Data_Ready != 2) Cycle_on_off();
}
static void act_time() {
  if (IR_Data_Ready != 2) IR_Time();
}
static void act_weather() {
  if (IR_Data_Ready != 2) IR_Weather();
}
static void act_ip() {
  if (IR_Data_Ready != 2) Print_IP();
}
static void act_rnd() {
  if (IR_Data_Ready != 2) Current_Eff_Rnd_Def(true);
}
static void act_def() {
  if (IR_Data_Ready != 2) Current_Eff_Rnd_Def(false);
}
static void act_eq() {
  if (IR_Data_Ready != 2) IR_Equalizer();
}
static void act_fav_add() {
  if (IR_Data_Ready != 2) Favorit_Add_Del(true);
}
static void act_fav_del() {
  if (IR_Data_Ready != 2) Favorit_Add_Del(false);
}

static void act_br_up() {
  Bright_Up_Down(true);
}
static void act_br_down() {
  Bright_Up_Down(false);
}
static void act_sp_up() {
  Speed_Up_Down(true);
}
static void act_sp_down() {
  Speed_Up_Down(false);
}
static void act_sc_up() {
  Scale_Up_Down(true);
}
static void act_sc_down() {
  Scale_Up_Down(false);
}
static void act_vol_up() {
  Volum_Up_Down(true);
}
static void act_vol_down() {
  Volum_Up_Down(false);
}
static void act_fold_prev() {
  if (IR_Data_Ready != 2) Folder_Next_Prev(false);
}
static void act_fold_next() {
  if (IR_Data_Ready != 2) Folder_Next_Prev(true);
}

// цифры
static void act_d0() {
  if (IR_Data_Ready != 2) Digit_Handle(0);
}
static void act_d1() {
  if (IR_Data_Ready != 2) Digit_Handle(1);
}
static void act_d2() {
  if (IR_Data_Ready != 2) Digit_Handle(2);
}
static void act_d3() {
  if (IR_Data_Ready != 2) Digit_Handle(3);
}
static void act_d4() {
  if (IR_Data_Ready != 2) Digit_Handle(4);
}
static void act_d5() {
  if (IR_Data_Ready != 2) Digit_Handle(5);
}
static void act_d6() {
  if (IR_Data_Ready != 2) Digit_Handle(6);
}
static void act_d7() {
  if (IR_Data_Ready != 2) Digit_Handle(7);
}
static void act_d8() {
  if (IR_Data_Ready != 2) Digit_Handle(8);
}
static void act_d9() {
  if (IR_Data_Ready != 2) Digit_Handle(9);
}

// --------------------------------------------------------------------------------------

static const IRAction irActions[] PROGMEM = {
  { "IR_ON_OFF", IR_ON_OFF,
#if USE_2_PULTS
    IR2_ON_OFF,
#endif
    act_power, false
  },
  { "IR_MUTE", IR_MUTE,
#if USE_2_PULTS
    IR2_MUTE,
#endif
    act_mute, false
  },
  { "IR_PREV", IR_PREV,
#if USE_2_PULTS
    IR2_PREV,
#endif
    act_prev, false
  },
  { "IR_NEXT", IR_NEXT,
#if USE_2_PULTS
    IR2_NEXT,
#endif
    act_next, false
  },
  { "IR_CYCLE", IR_CYCLE,
#if USE_2_PULTS
    IR2_CYCLE,
#endif
    act_cycle, false
  },
  { "IR_BR_UP", IR_BR_UP,
#if USE_2_PULTS
    IR2_BR_UP,
#endif
    act_br_up, true
  },
  { "IR_BR_DOWN", IR_BR_DOWN,
#if USE_2_PULTS
    IR2_BR_DOWN,
#endif
    act_br_down,   true
  },
  { "IR_SP_UP", IR_SP_UP,
#if USE_2_PULTS
    IR2_SP_UP,
#endif
    act_sp_up, true
  },
  { "IR_SP_DOWN", IR_SP_DOWN,
#if USE_2_PULTS
    IR2_SP_DOWN,
#endif
    act_sp_down, true
  },
  { "IR_SC_UP", IR_SC_UP,
#if USE_2_PULTS
    IR2_SC_UP,
#endif
    act_sc_up, true
  },
  { "IR_SC_DOWN", IR_SC_DOWN,
#if USE_2_PULTS
    IR2_SC_DOWN,
#endif
    act_sc_down, true
  },
  { "IR_VOL_UP", IR_VOL_UP,
#if USE_2_PULTS
    IR2_VOL_UP,
#endif
    act_vol_up, true
  },
  { "IR_VOL_DOWN", IR_VOL_DOWN,
#if USE_2_PULTS
    IR2_VOL_DOWN,
#endif
    act_vol_down, true
  },
  { "IR_TIME", IR_TIME,
#if USE_2_PULTS
    IR2_TIME,
#endif
    act_time, false
  },
  { "IR_WEATHER", IR_WEATHER,
#if USE_2_PULTS
    IR2_WEATHER,
#endif
    act_weather, false
  },
  { "IR_IP", IR_IP,
#if USE_2_PULTS
    IR2_IP,
#endif
    act_ip, false
  },
  { "IR_FOLD_PREV", IR_FOLD_PREV,
#if USE_2_PULTS
    IR2_FOLD_PREV,
#endif
    act_fold_prev, false
  },
  { "IR_FOLD_NEXT", IR_FOLD_NEXT,
#if USE_2_PULTS
    IR2_FOLD_NEXT,
#endif
    act_fold_next, false
  },
  { "IR_RND", IR_RND,
#if USE_2_PULTS
    IR2_RND,
#endif
    act_rnd, false
  },
  { "IR_DEF", IR_DEF,
#if USE_2_PULTS
    IR2_DEF,
#endif
    act_def, false
  },
  { "IR_EQ", IR_EQ,
#if USE_2_PULTS
    IR2_EQ,
#endif
    act_eq, false
  },
  { "IR_FAV_ADD", IR_FAV_ADD,
#if USE_2_PULTS
    IR2_FAV_ADD,
#endif
    act_fav_add, false
  },
  { "IR_FAV_DEL", IR_FAV_DEL,
#if USE_2_PULTS
    IR2_FAV_DEL,
#endif
    act_fav_del, false
  },
  { "IR_1", IR_1,
#if USE_2_PULTS
    IR2_1,
#endif
    act_d1, false
  },
  { "IR_2", IR_2,
#if USE_2_PULTS
    IR2_2,
#endif
    act_d2, false
  },
  { "IR_3", IR_3,
#if USE_2_PULTS
    IR2_3,
#endif
    act_d3, false
  },
  { "IR_4", IR_4,
#if USE_2_PULTS
    IR2_4,
#endif
    act_d4, false
  },
  { "IR_5", IR_5,
#if USE_2_PULTS
    IR2_5,
#endif
    act_d5, false
  },
  { "IR_6", IR_6,
#if USE_2_PULTS
    IR2_6,
#endif
    act_d6, false
  },
  { "IR_7", IR_7,
#if USE_2_PULTS
    IR2_7,
#endif
    act_d7, false
  },
  { "IR_8", IR_8,
#if USE_2_PULTS
    IR2_8,
#endif
    act_d8, false
  },
  { "IR_9", IR_9,
#if USE_2_PULTS
    IR2_9,
#endif
    act_d9, false
  },
  { "IR_0", IR_0,
#if USE_2_PULTS
    IR2_0,
#endif
    act_d0, false
  },
};

static const size_t irActionsCount = sizeof(irActions) / sizeof(irActions[0]);

// --------------------------------------------------------------------------------------

void IR_Receive_Button_Handle() {
  if (!irEnabled) return;

  // 0 - помеха/пустой приём - игнорировать
  if (IR_Code == 0) return;

  for (size_t i = 0; i < irActionsCount; i++) {
    // 1 - динамический код - в приоритете
    uint32_t dynCode = irManager.getCode(irActions[i].name);
    if (dynCode != 0 && IR_Code == dynCode) {
      if (IR_Data_Ready == 2 && !irActions[i].allowRepeat) return;
      irActions[i].action();
      return;
    }
    // Fallback: статический код пульта-1 (только если динамический код не задан)
    if (dynCode == 0 && IR_Code == irActions[i].fallback) {
      if (IR_Data_Ready == 2 && !irActions[i].allowRepeat) return;
      irActions[i].action();
      return;
    }
#if USE_2_PULTS
    // Fallback: статический код пульта-2
    if (dynCode == 0 && irActions[i].fallback2 != 0 && IR_Code == irActions[i].fallback2) {
      if (IR_Data_Ready == 2 && !irActions[i].allowRepeat) return;
      irActions[i].action();
      return;
    }
#endif
  }

#if IR_LOG
  SYSLOG.add("IR_CODE = 0x%08X (unknown)", IR_Code);
#endif
}

// -----------------------
#endif // USE_IR_RECEIVER

// ============================================================================= ФУНКЦИИ RF ============================================================

#if USE_RF_RECEIVER
void RF_Receive_Handle() {
  if (!rfEnabled) return;
  if (rfReceiver.available()) {
    unsigned long rfCode = rfReceiver.getReceivedValue();
    if (rfCode != 0) {
      RF_Code = rfCode;
      RF_Data_Ready = 1;
#if RF_LOG
      SYSLOG.add("RF_CODE = 0x%lX", rfCode);
#endif
    }
    rfReceiver.resetAvailable();
  }
}

void RF_Receive_Button_Handle() {
  if (!rfEnabled) return;
  switch (RF_Code) {
    case RF_ON_OFF:
      IR_Power();
      break;

    case RF_MUTE:
      Mute();
      break;

    case RF_PREV:
      Prev_Next_eff(false);
      break;

    case RF_NEXT:
      Prev_Next_eff(true);
      break;

    case RF_CYCLE:
      Cycle_on_off();
      break;

    case RF_BR_UP:
      Bright_Up_Down(true);
      break;

    case RF_BR_DOWN:
      Bright_Up_Down(false);
      break;

    case RF_SP_UP:
      Speed_Up_Down(true);
      break;

    case RF_SP_DOWN:
      Speed_Up_Down(false);
      break;

    case RF_SC_UP:
      Scale_Up_Down(true);
      break;

    case RF_SC_DOWN:
      Scale_Up_Down(false);
      break;

    case RF_VOL_UP:
      Volum_Up_Down(true);
      break;

    case RF_VOL_DOWN:
      Volum_Up_Down(false);
      break;

    // время
    case RF_TIME:
      IR_Time();
      break;

    // погода
    case RF_WEATHER:
      IR_Weather();
      break;

    case RF_IP:
      Print_IP();
      break;

    case RF_FOLD_PREV:
      Folder_Next_Prev(false);
      break;

    case RF_FOLD_NEXT:
      Folder_Next_Prev(true);
      break;

    case RF_RND:
      Current_Eff_Rnd_Def(true);
      break;

    case RF_DEF:
      Current_Eff_Rnd_Def(false);
      break;

    case RF_EQ:
      IR_Equalizer();
      break;

    case RF_FAV_ADD:
      Favorit_Add_Del(true);
      break;

    case RF_FAV_DEL:
      Favorit_Add_Del(false);
      break;

    case RF_1: Digit_Handle(1); break;
    case RF_2: Digit_Handle(2); break;
    case RF_3: Digit_Handle(3); break;
    case RF_4: Digit_Handle(4); break;
    case RF_5: Digit_Handle(5); break;
    case RF_6: Digit_Handle(6); break;
    case RF_7: Digit_Handle(7); break;
    case RF_8: Digit_Handle(8); break;
    case RF_9: Digit_Handle(9); break;
    case RF_0: Digit_Handle(0); break;

    default:
      break;
  }

#if RF_LOG
  SYSLOG.add("RF_Code = 0x%X", RF_Code);
#endif

  RF_Data_Ready = 0;
}
#endif // USE_RF_RECEIVER

// ******************************************************************************************************************************************************

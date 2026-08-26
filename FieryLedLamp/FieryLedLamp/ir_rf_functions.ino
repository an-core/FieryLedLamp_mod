// ************************************************************************ ir_rf_functions.ino *********************************************************

// ============================================================================= ФУНКЦИИ IR ============================================================

#if USE_IR_RECEIVER

uint32_t getIRCode(const String& key) {
    return irManager.getCode(key);
}

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
    currentMode = eff_num_correct[Enter_Number];
    jsonWrite(configSetup, "eff_sel", Enter_Number);
    jsonWrite(configSetup, "br", modes[currentMode].Brightness);
    jsonWrite(configSetup, "sp", modes[currentMode].Speed);
    jsonWrite(configSetup, "sc", modes[currentMode].Scale);

#if USE_MP3_PLAYER
    mp3_folder = pgm_read_byte(&default_effects_folders[currentMode]);
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

void IR_Receive_Button_Handle() {
  if (!irEnabled) return;
  bool processed = false;

  // динамические коды
  uint32_t code_ON_OFF = irManager.getCode("IR_ON_OFF");
  uint32_t code_MUTE = irManager.getCode("IR_MUTE");
  uint32_t code_PREV = irManager.getCode("IR_PREV");
  uint32_t code_NEXT = irManager.getCode("IR_NEXT");
  uint32_t code_CYCLE = irManager.getCode("IR_CYCLE");
  uint32_t code_BR_UP = irManager.getCode("IR_BR_UP");
  uint32_t code_BR_DOWN = irManager.getCode("IR_BR_DOWN");
  uint32_t code_SP_UP = irManager.getCode("IR_SP_UP");
  uint32_t code_SP_DOWN = irManager.getCode("IR_SP_DOWN");
  uint32_t code_SC_UP = irManager.getCode("IR_SC_UP");
  uint32_t code_SC_DOWN = irManager.getCode("IR_SC_DOWN");
  uint32_t code_VOL_UP = irManager.getCode("IR_VOL_UP");
  uint32_t code_VOL_DOWN = irManager.getCode("IR_VOL_DOWN");
  uint32_t code_TIME = irManager.getCode("IR_TIME");
  uint32_t code_WEATHER = irManager.getCode("IR_WEATHER");
  uint32_t code_IP = irManager.getCode("IR_IP");
  uint32_t code_FOLD_PREV = irManager.getCode("IR_FOLD_PREV");
  uint32_t code_FOLD_NEXT = irManager.getCode("IR_FOLD_NEXT");
  uint32_t code_RND = irManager.getCode("IR_RND");
  uint32_t code_DEF = irManager.getCode("IR_DEF");
  uint32_t code_EQ = irManager.getCode("IR_EQ");
  uint32_t code_FAV_ADD = irManager.getCode("IR_FAV_ADD");
  uint32_t code_FAV_DEL = irManager.getCode("IR_FAV_DEL");
  uint32_t code_1 = irManager.getCode("IR_1");
  uint32_t code_2 = irManager.getCode("IR_2");
  uint32_t code_3 = irManager.getCode("IR_3");
  uint32_t code_4 = irManager.getCode("IR_4");
  uint32_t code_5 = irManager.getCode("IR_5");
  uint32_t code_6 = irManager.getCode("IR_6");
  uint32_t code_7 = irManager.getCode("IR_7");
  uint32_t code_8 = irManager.getCode("IR_8");
  uint32_t code_9 = irManager.getCode("IR_9");
  uint32_t code_0 = irManager.getCode("IR_0");

  switch (IR_Code) {
    case IR_ON_OFF:
#if USE_2_PULTS
    case IR2_ON_OFF:
#endif
      if (IR_Code == code_ON_OFF || IR_Code == IR_ON_OFF
#if USE_2_PULTS
          || IR_Code == IR2_ON_OFF
#endif
         ) {
        if (IR_Data_Ready != 2) IR_Power();
      }
      break;

    case IR_MUTE:
#if USE_2_PULTS
    case IR2_MUTE:
#endif
      if (IR_Code == code_MUTE || IR_Code == IR_MUTE
#if USE_2_PULTS
          || IR_Code == IR2_MUTE
#endif
         ) {
        if (IR_Data_Ready != 2) Mute();
      }
      break;

    case IR_PREV:
#if USE_2_PULTS
    case IR2_PREV:
#endif
      if (IR_Code == code_PREV || IR_Code == IR_PREV
#if USE_2_PULTS
          || IR_Code == IR2_PREV
#endif
         ) {
        Prev_Next_eff(false);
      }
      break;

    case IR_NEXT:
#if USE_2_PULTS
    case IR2_NEXT:
#endif
      if (IR_Code == code_NEXT || IR_Code == IR_NEXT
#if USE_2_PULTS
          || IR_Code == IR2_NEXT
#endif
         ) {
        Prev_Next_eff(true);
      }
      break;

    case IR_CYCLE:
#if USE_2_PULTS
    case IR2_CYCLE:
#endif
      if (IR_Code == code_CYCLE || IR_Code == IR_CYCLE
#if USE_2_PULTS
          || IR_Code == IR2_CYCLE
#endif
         ) {
        if (IR_Data_Ready != 2) Cycle_on_off();
      }
      break;

    case IR_BR_UP:
#if USE_2_PULTS
    case IR2_BR_UP:
#endif
      if (IR_Code == code_BR_UP || IR_Code == IR_BR_UP
#if USE_2_PULTS
          || IR_Code == IR2_BR_UP
#endif
         ) {
        Bright_Up_Down(true);
      }
      break;

    case IR_BR_DOWN:
#if USE_2_PULTS
    case IR2_BR_DOWN:
#endif
      if (IR_Code == code_BR_DOWN || IR_Code == IR_BR_DOWN
#if USE_2_PULTS
          || IR_Code == IR2_BR_DOWN
#endif
         ) {
        Bright_Up_Down(false);
      }
      break;

    case IR_SP_UP:
#if USE_2_PULTS
    case IR2_SP_UP:
#endif
      if (IR_Code == code_SP_UP || IR_Code == IR_SP_UP
#if USE_2_PULTS
          || IR_Code == IR2_SP_UP
#endif
         ) {
        Speed_Up_Down(true);
      }
      break;

    case IR_SP_DOWN:
#if USE_2_PULTS
    case IR2_SP_DOWN:
#endif
      if (IR_Code == code_SP_DOWN || IR_Code == IR_SP_DOWN
#if USE_2_PULTS
          || IR_Code == IR2_SP_DOWN
#endif
         ) {
        Speed_Up_Down(false);
      }
      break;

    case IR_SC_UP:
#if USE_2_PULTS
    case IR2_SC_UP:
#endif
      if (IR_Code == code_SC_UP || IR_Code == IR_SC_UP
#if USE_2_PULTS
          || IR_Code == IR2_SC_UP
#endif
         ) {
        Scale_Up_Down(true);
      }
      break;

    case IR_SC_DOWN:
#if USE_2_PULTS
    case IR2_SC_DOWN:
#endif
      if (IR_Code == code_SC_DOWN || IR_Code == IR_SC_DOWN
#if USE_2_PULTS
          || IR_Code == IR2_SC_DOWN
#endif
         ) {
        Scale_Up_Down(false);
      }
      break;

    case IR_VOL_UP:
#if USE_2_PULTS
    case IR2_VOL_UP:
#endif
      if (IR_Code == code_VOL_UP || IR_Code == IR_VOL_UP
#if USE_2_PULTS
          || IR_Code == IR2_VOL_UP
#endif
         ) {
        Volum_Up_Down(true);
      }
      break;

    case IR_VOL_DOWN:
#if USE_2_PULTS
    case IR2_VOL_DOWN:
#endif
      if (IR_Code == code_VOL_DOWN || IR_Code == IR_VOL_DOWN
#if USE_2_PULTS
          || IR_Code == IR2_VOL_DOWN
#endif
         ) {
        Volum_Up_Down(false);
      }
      break;

    // озвучка времени
    case IR_TIME:
#if USE_2_PULTS
    case IR2_TIME:
#endif
      if (IR_Code == code_TIME || IR_Code == IR_TIME
#if USE_2_PULTS
          || IR_Code == IR2_TIME
#endif
         ) {
        if (IR_Data_Ready != 2) {
#if USE_MP3_PLAYER
          if (mp3_player_connect == 4) {
            play_time_ADVERT(true);
          } else 
#endif
          if (myTime.isTimeSet()) {
            printTime(true);
          } else {
            fillString("NO TIME", CRGB::Red, true);
          }
        }
      }
      break;

    // озвучка погоды
    case IR_WEATHER:
#if USE_2_PULTS
    case IR2_WEATHER:
#endif
      if (IR_Code == code_WEATHER || IR_Code == IR_WEATHER
#if USE_2_PULTS
          || IR_Code == IR2_WEATHER
#endif
         ) {
        if (IR_Data_Ready != 2) {
#if USE_WEATHER
#if USE_MP3_PLAYER
          if (mp3_player_connect == 4) {
            play_weather(true);
          } else {
            uint8_t old = PRINT_WEATHER;
            PRINT_WEATHER = 1;
            printWeather();
            PRINT_WEATHER = old;
          }
#else
          uint8_t old = PRINT_WEATHER;
          PRINT_WEATHER = 1;
          printWeather();
          PRINT_WEATHER = old;
#endif // USE_MP3_PLAYER
#endif // USE_WEATHER
        }
      }
      break;

    case IR_IP:
#if USE_2_PULTS
    case IR2_IP:
#endif
      if (IR_Code == code_IP || IR_Code == IR_IP
#if USE_2_PULTS
          || IR_Code == IR2_IP
#endif
         ) {
        if (IR_Data_Ready != 2) Print_IP();
      }
      break;

    case IR_FOLD_PREV:
#if USE_2_PULTS
    case IR2_FOLD_PREV:
#endif
      if (IR_Code == code_FOLD_PREV || IR_Code == IR_FOLD_PREV
#if USE_2_PULTS
          || IR_Code == IR2_FOLD_PREV
#endif
         ) {
        Folder_Next_Prev(false);
      }
      break;

    case IR_FOLD_NEXT:
#if USE_2_PULTS
    case IR2_FOLD_NEXT:
#endif
      if (IR_Code == code_FOLD_NEXT || IR_Code == IR_FOLD_NEXT
#if USE_2_PULTS
          || IR_Code == IR2_FOLD_NEXT
#endif
         ) {
        Folder_Next_Prev(true);
      }
      break;

    case IR_RND:
#if USE_2_PULTS
    case IR2_RND:
#endif
      if (IR_Code == code_RND || IR_Code == IR_RND
#if USE_2_PULTS
          || IR_Code == IR2_RND
#endif
         ) {
        if (IR_Data_Ready != 2) Current_Eff_Rnd_Def(true);
      }
      break;

    case IR_DEF:
#if USE_2_PULTS
    case IR2_DEF:
#endif
      if (IR_Code == code_DEF || IR_Code == IR_DEF
#if USE_2_PULTS
          || IR_Code == IR2_DEF
#endif
         ) {
        if (IR_Data_Ready != 2) Current_Eff_Rnd_Def(false);
      }
      break;

    case IR_EQ:
#if USE_2_PULTS
    case IR2_EQ:
#endif
      if (IR_Code == code_EQ || IR_Code == IR_EQ
#if USE_2_PULTS
          || IR_Code == IR2_EQ
#endif
         ) {
        if (IR_Data_Ready != 2) IR_Equalizer();
      }
      break;

    case IR_FAV_ADD:
#if USE_2_PULTS
    case IR2_FAV_ADD:
#endif
      if (IR_Code == code_FAV_ADD || IR_Code == IR_FAV_ADD
#if USE_2_PULTS
          || IR_Code == IR2_FAV_ADD
#endif
         ) {
        if (IR_Data_Ready != 2) Favorit_Add_Del(true);
      }
      break;

    case IR_FAV_DEL:
#if USE_2_PULTS
    case IR2_FAV_DEL:
#endif
      if (IR_Code == code_FAV_DEL || IR_Code == IR_FAV_DEL
#if USE_2_PULTS
          || IR_Code == IR2_FAV_DEL
#endif
         ) {
        if (IR_Data_Ready != 2) Favorit_Add_Del(false);
      }
      break;

    // -----------------------------------------------------------------
    // Цифры для выбора эффекта
    case IR_1:
#if USE_2_PULTS
    case IR2_1:
#endif
      if (IR_Code == code_1 || IR_Code == IR_1
#if USE_2_PULTS
          || IR_Code == IR2_1
#endif
         ) {
        if (IR_Data_Ready != 2) Digit_Handle(1);
      }
      break;

    case IR_2:
#if USE_2_PULTS
    case IR2_2:
#endif
      if (IR_Code == code_2 || IR_Code == IR_2
#if USE_2_PULTS
          || IR_Code == IR2_2
#endif
         ) {
        if (IR_Data_Ready != 2) Digit_Handle(2);
      }
      break;

    case IR_3:
#if USE_2_PULTS
    case IR2_3:
#endif
      if (IR_Code == code_3 || IR_Code == IR_3
#if USE_2_PULTS
          || IR_Code == IR2_3
#endif
         ) {
        if (IR_Data_Ready != 2) Digit_Handle(3);
      }
      break;

    case IR_4:
#if USE_2_PULTS
    case IR2_4:
#endif
      if (IR_Code == code_4 || IR_Code == IR_4
#if USE_2_PULTS
          || IR_Code == IR2_4
#endif
         ) {
        if (IR_Data_Ready != 2) Digit_Handle(4);
      }
      break;

    case IR_5:
#if USE_2_PULTS
    case IR2_5:
#endif
      if (IR_Code == code_5 || IR_Code == IR_5
#if USE_2_PULTS
          || IR_Code == IR2_5
#endif
         ) {
        if (IR_Data_Ready != 2) Digit_Handle(5);
      }
      break;

    case IR_6:
#if USE_2_PULTS
    case IR2_6:
#endif
      if (IR_Code == code_6 || IR_Code == IR_6
#if USE_2_PULTS
          || IR_Code == IR2_6
#endif
         ) {
        if (IR_Data_Ready != 2) Digit_Handle(6);
      }
      break;

    case IR_7:
#if USE_2_PULTS
    case IR2_7:
#endif
      if (IR_Code == code_7 || IR_Code == IR_7
#if USE_2_PULTS
          || IR_Code == IR2_7
#endif
         ) {
        if (IR_Data_Ready != 2) Digit_Handle(7);
      }
      break;

    case IR_8:
#if USE_2_PULTS
    case IR2_8:
#endif
      if (IR_Code == code_8 || IR_Code == IR_8
#if USE_2_PULTS
          || IR_Code == IR2_8
#endif
         ) {
        if (IR_Data_Ready != 2) Digit_Handle(8);
      }
      break;

    case IR_9:
#if USE_2_PULTS
    case IR2_9:
#endif
      if (IR_Code == code_9 || IR_Code == IR_9
#if USE_2_PULTS
          || IR_Code == IR2_9
#endif
         ) {
        if (IR_Data_Ready != 2) Digit_Handle(9);
      }
      break;

    case IR_0:
#if USE_2_PULTS
    case IR2_0:
#endif
      if (IR_Code == code_0 || IR_Code == IR_0
#if USE_2_PULTS
          || IR_Code == IR2_0
#endif
         ) {
        if (IR_Data_Ready != 2) Digit_Handle(0);
      }
      break;

    default:
      break;
  }

#if IR_LOG
  SYSLOG.add("IR_CODE = 0x%08X", IR_Code);
#endif
}

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
#if USE_MP3_PLAYER
      if (mp3_player_connect == 4) {
        play_time_ADVERT(true);
      } else 
#endif
      if (myTime.isTimeSet()) {
        printTime(true);
      } else {
        fillString("NO TIME", CRGB::Red, true);
      }
      break;

    // погода
    case RF_WEATHER:
#if USE_WEATHER
#if USE_MP3_PLAYER
      if (mp3_player_connect == 4) {
        play_weather(true);
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

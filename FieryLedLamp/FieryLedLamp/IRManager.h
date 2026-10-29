// ************************************************************************* IRManager.h **************************************************************
#pragma once
// -------------

#if USE_IR_RECEIVER

#include <ArduinoJson.h>
#include "Ardu_Json.h"
#include <LittleFS.h>

class IRManager {
  private:
    struct IRCodeMap {
      uint32_t IR_ON_OFF;
      uint32_t IR_MUTE;
      uint32_t IR_PREV;
      uint32_t IR_NEXT;
      uint32_t IR_WEATHER;
      uint32_t IR_TIME;
      uint32_t IR_VOL_DOWN;
      uint32_t IR_VOL_UP;
      uint32_t IR_BR_DOWN;
      uint32_t IR_BR_UP;
      uint32_t IR_IP;
      uint32_t IR_0;
      uint32_t IR_1;
      uint32_t IR_2;
      uint32_t IR_3;
      uint32_t IR_4;
      uint32_t IR_5;
      uint32_t IR_6;
      uint32_t IR_7;
      uint32_t IR_8;
      uint32_t IR_9;
      uint32_t IR_CYCLE;
      uint32_t IR_EQ;
      uint32_t IR_SP_UP;
      uint32_t IR_SP_DOWN;
      uint32_t IR_FAV_ADD;
      uint32_t IR_FAV_DEL;
      uint32_t IR_RND;
      uint32_t IR_DEF;
      uint32_t IR_FOLD_PREV;
      uint32_t IR_FOLD_NEXT;
      uint32_t IR_SC_DOWN;
      uint32_t IR_SC_UP;
    };

    IRCodeMap currentCodes;
    String currentRemoteId;
    String currentRemoteName;
    // Массив пар {имя, указатель на поле} вместо std::map
    struct IRCodePair {
      const char* name;
      uint32_t* ptr;
    };

    static const uint8_t IR_CODE_COUNT = 33;
    IRCodePair codeMap[IR_CODE_COUNT];

    void initCodeMap() {
      codeMap[0]  = {"IR_ON_OFF",    &currentCodes.IR_ON_OFF};
      codeMap[1]  = {"IR_MUTE",      &currentCodes.IR_MUTE};
      codeMap[2]  = {"IR_PREV",      &currentCodes.IR_PREV};
      codeMap[3]  = {"IR_NEXT",      &currentCodes.IR_NEXT};
      codeMap[4]  = {"IR_WEATHER",   &currentCodes.IR_WEATHER};
      codeMap[5]  = {"IR_TIME",      &currentCodes.IR_TIME};
      codeMap[6]  = {"IR_VOL_DOWN",  &currentCodes.IR_VOL_DOWN};
      codeMap[7]  = {"IR_VOL_UP",    &currentCodes.IR_VOL_UP};
      codeMap[8]  = {"IR_BR_DOWN",   &currentCodes.IR_BR_DOWN};
      codeMap[9]  = {"IR_BR_UP",     &currentCodes.IR_BR_UP};
      codeMap[10] = {"IR_IP",        &currentCodes.IR_IP};
      codeMap[11] = {"IR_0",         &currentCodes.IR_0};
      codeMap[12] = {"IR_1",         &currentCodes.IR_1};
      codeMap[13] = {"IR_2",         &currentCodes.IR_2};
      codeMap[14] = {"IR_3",         &currentCodes.IR_3};
      codeMap[15] = {"IR_4",         &currentCodes.IR_4};
      codeMap[16] = {"IR_5",         &currentCodes.IR_5};
      codeMap[17] = {"IR_6",         &currentCodes.IR_6};
      codeMap[18] = {"IR_7",         &currentCodes.IR_7};
      codeMap[19] = {"IR_8",         &currentCodes.IR_8};
      codeMap[20] = {"IR_9",         &currentCodes.IR_9};
      codeMap[21] = {"IR_CYCLE",     &currentCodes.IR_CYCLE};
      codeMap[22] = {"IR_EQ",        &currentCodes.IR_EQ};
      codeMap[23] = {"IR_SP_UP",     &currentCodes.IR_SP_UP};
      codeMap[24] = {"IR_SP_DOWN",   &currentCodes.IR_SP_DOWN};
      codeMap[25] = {"IR_FAV_ADD",   &currentCodes.IR_FAV_ADD};
      codeMap[26] = {"IR_FAV_DEL",   &currentCodes.IR_FAV_DEL};
      codeMap[27] = {"IR_RND",       &currentCodes.IR_RND};
      codeMap[28] = {"IR_DEF",       &currentCodes.IR_DEF};
      codeMap[29] = {"IR_FOLD_PREV", &currentCodes.IR_FOLD_PREV};
      codeMap[30] = {"IR_FOLD_NEXT", &currentCodes.IR_FOLD_NEXT};
      codeMap[31] = {"IR_SC_DOWN",   &currentCodes.IR_SC_DOWN};
      codeMap[32] = {"IR_SC_UP",     &currentCodes.IR_SC_UP};
    }
    
    uint32_t parseHexOrDec(const String& str) {
      if (str.startsWith("0x") || str.startsWith("0X")) {
        return strtoul(str.c_str(), NULL, 16);
      }
      return str.toInt();
    }

  public:
    IRManager() {
      initCodeMap();
      //loadSavedRemote();
    }

    bool loadRemote(const String& remoteId) {
      File file = LittleFS.open("/ir_codes.json", "r");
      if (!file) {
        Serial.println("IR: Cannot open ir_codes.json");
        return false;
      }

      DynamicJsonDocument doc(8192);
      DeserializationError error = deserializeJson(doc, file);
      file.close();

      if (error) {
        Serial.println("IR: JSON parse error");
        return false;
      }

      JsonArray remotes = doc["remotes"];
      for (JsonObject remote : remotes) {
        if (remote["id"] == remoteId) {
          currentRemoteId = remoteId;
          currentRemoteName = remote["name"].as<String>();
          JsonObject codes = remote["codes"];

          for (uint8_t i = 0; i < IR_CODE_COUNT; i++) {
            String codeStr = codes[codeMap[i].name].as<String>();
            *codeMap[i].ptr = parseHexOrDec(codeStr);
          }

          saveSelectedRemote(remoteId);
          Serial.printf("IR: Loaded remote '%s'\n", currentRemoteName.c_str());
          return true;
        }
      }

      Serial.println("IR: Remote not found");
      return false;
    }

    void loadSavedRemote() {
      String savedId = loadSelectedRemote();
      if (savedId.length() > 0) {
        if (!loadRemote(savedId)) {
          loadFirstRemote();
        }
      } else {
        loadFirstRemote();
      }
    }

    void loadFirstRemote() {
      File file = LittleFS.open("/ir_codes.json", "r");
      if (!file) return;

      DynamicJsonDocument doc(8192);
      DeserializationError error = deserializeJson(doc, file);
      file.close();

      if (error) return;

      JsonArray remotes = doc["remotes"];
      if (remotes.size() > 0) {
        String firstId = remotes[0]["id"].as<String>();
        loadRemote(firstId);
      }
    }

    String getRemotesList() {
      File file = LittleFS.open("/ir_codes.json", "r");
      if (!file) return "[]";

      DynamicJsonDocument doc(8192);
      DeserializationError error = deserializeJson(doc, file);
      file.close();

      if (error) return "[]";

      String result = "[";
      JsonArray remotes = doc["remotes"];
      for (JsonObject remote : remotes) {
        if (result.length() > 1) result += ",";
        result += "{\"id\":\"" + remote["id"].as<String>() + "\",";
        result += "\"name\":\"" + remote["name"].as<String>() + "\"}";
      }
      result += "]";
      return result;
    }

        uint32_t getCode(const String& key) {
      for (uint8_t i = 0; i < IR_CODE_COUNT; i++) {
        if (key == codeMap[i].name) {
          return *codeMap[i].ptr;
        }
      }
      return 0;
    }

    String getCurrentRemoteId() const {
      return currentRemoteId;
    }

    String getCurrentRemoteName() const {
      return currentRemoteName;
    }

  private:
    void saveSelectedRemote(const String& remoteId) {
      File file = LittleFS.open("/ir_selected.txt", "w");
      if (file) {
        file.print(remoteId);
        file.close();
      }
    }

    String loadSelectedRemote() {
      File file = LittleFS.open("/ir_selected.txt", "r");
      if (file) {
        String id = file.readString();
        file.close();
        id.trim();
        return id;
      }
      return "";
    }
};

#endif // USE_IR_RECEIVER

// ****************************************************************************************************************************************************

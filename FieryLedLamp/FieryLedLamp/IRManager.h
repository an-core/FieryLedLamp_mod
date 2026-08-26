// ************************************************************************* IRManager.h **************************************************************
#pragma once
// -------------

#if USE_IR_RECEIVER

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <map>

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
    std::map<String, uint32_t*> codeMap;

    void initCodeMap() {
      codeMap["IR_ON_OFF"] = &currentCodes.IR_ON_OFF;
      codeMap["IR_MUTE"] = &currentCodes.IR_MUTE;
      codeMap["IR_PREV"] = &currentCodes.IR_PREV;
      codeMap["IR_NEXT"] = &currentCodes.IR_NEXT;
      codeMap["IR_WEATHER"] = &currentCodes.IR_WEATHER;
      codeMap["IR_TIME"] = &currentCodes.IR_TIME;
      codeMap["IR_VOL_DOWN"] = &currentCodes.IR_VOL_DOWN;
      codeMap["IR_VOL_UP"] = &currentCodes.IR_VOL_UP;
      codeMap["IR_BR_DOWN"] = &currentCodes.IR_BR_DOWN;
      codeMap["IR_BR_UP"] = &currentCodes.IR_BR_UP;
      codeMap["IR_IP"] = &currentCodes.IR_IP;
      codeMap["IR_0"] = &currentCodes.IR_0;
      codeMap["IR_1"] = &currentCodes.IR_1;
      codeMap["IR_2"] = &currentCodes.IR_2;
      codeMap["IR_3"] = &currentCodes.IR_3;
      codeMap["IR_4"] = &currentCodes.IR_4;
      codeMap["IR_5"] = &currentCodes.IR_5;
      codeMap["IR_6"] = &currentCodes.IR_6;
      codeMap["IR_7"] = &currentCodes.IR_7;
      codeMap["IR_8"] = &currentCodes.IR_8;
      codeMap["IR_9"] = &currentCodes.IR_9;
      codeMap["IR_CYCLE"] = &currentCodes.IR_CYCLE;
      codeMap["IR_EQ"] = &currentCodes.IR_EQ;
      codeMap["IR_SP_UP"] = &currentCodes.IR_SP_UP;
      codeMap["IR_SP_DOWN"] = &currentCodes.IR_SP_DOWN;
      codeMap["IR_FAV_ADD"] = &currentCodes.IR_FAV_ADD;
      codeMap["IR_FAV_DEL"] = &currentCodes.IR_FAV_DEL;
      codeMap["IR_RND"] = &currentCodes.IR_RND;
      codeMap["IR_DEF"] = &currentCodes.IR_DEF;
      codeMap["IR_FOLD_PREV"] = &currentCodes.IR_FOLD_PREV;
      codeMap["IR_FOLD_NEXT"] = &currentCodes.IR_FOLD_NEXT;
      codeMap["IR_SC_DOWN"] = &currentCodes.IR_SC_DOWN;
      codeMap["IR_SC_UP"] = &currentCodes.IR_SC_UP;
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
      loadSavedRemote();
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

          for (auto& kv : codeMap) {
            String codeStr = codes[kv.first].as<String>();
            *kv.second = parseHexOrDec(codeStr);
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
      auto it = codeMap.find(key);
      if (it != codeMap.end()) {
        return *it->second;
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

// ***************************************************************************** Eeprom.h ****************************************************************
#pragma once
// --------------
#include "Types.h"
#include <EEPROM.h>
#include "Prototypes.h"
#include "Extern.h"
#include "Constants.h"
// ----------------------

class Eeprom {
  public:
    static Eeprom& instance() {
      static Eeprom instance;
      return instance;
    }

// --------------------------------------------------------------------
    void InitEepromSettings(ModeType* modes, void (*restoreDefaultSettings)()) {
      if (!LittleFS.begin()) {
        Serial.println(F("Ошибка LittleFS"));
        return;
      }

      EEPROM.begin(EEPROM_TOTAL_BYTES_USED);
      delay(50);
      byte firstRunMark = (byte)EEPROM_FIRST_RUN_MARK;

      if (EEPROM.read(EEPROM_FIRST_RUN_ADDRESS) != firstRunMark) {
        restoreDefaultSettings();
        EEPROM.write(EEPROM_FIRST_RUN_ADDRESS, firstRunMark);

        for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
          EEPROM.put(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * i, modes[i]);
          EEPROM.commit();
          delay(10);
        }
      } // if (EEPROM.read(EEPROM_FIRST_RUN_ADDRESS) != firstRunMark)
      
      else {
        for (uint8_t i = 0; i < MODE_AMOUNT; i++)
          EEPROM.get(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * i, modes[i]);
      }

      if (LittleFS.begin(false)) {
        if (LittleFS.exists("/index.setup.gz")) {
          FileCopy(F("/index.setup.gz"), F("/index.json.gz"));
        }
      }

      T_flag = 1;

    }

// --------------------------------------------------------------------
    bool IsWelcomePageActive() const {
      return false;  // старая страница приветствия отключена
    }

// --------------------------------------------------------------------
    void EepromGet(ModeType* modes) {
      for (uint8_t i = 0; i < MODE_AMOUNT; i++)
        EEPROM.get(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * i, modes[i]);
    }

// --------------------------------------------------------------------
    void EepromPut(ModeType* modes) {
      for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
        EEPROM.put(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * i, modes[i]);
      }

      EEPROM.commit();

    }

// --------------------------------------------------------------------
// сохранение даты последнего обновления
void SaveUpdateDate(const String& date) {
  if (date.length() == 0) return;
  
  String shortDate = date.substring(0, 10);
  
  for (uint16_t i = 0; i < EEPROM_UPDATE_DATE_SIZE && i < shortDate.length(); i++) {
    EEPROM.write(EEPROM_UPDATE_DATE_ADDR + i, shortDate[i]);
  }

  EEPROM.write(EEPROM_UPDATE_DATE_ADDR + shortDate.length(), '\0');
  
  EEPROM.commit();
}

// --------------------------------------------------------------------
// загрузка даты последнего обновления
String LoadUpdateDate() {
  char buffer[EEPROM_UPDATE_DATE_SIZE] = {0};
  
  for (uint16_t i = 0; i < EEPROM_UPDATE_DATE_SIZE; i++) {
    char c = char(EEPROM.read(EEPROM_UPDATE_DATE_ADDR + i));
    if (c == '\0') break;
    buffer[i] = c;
  }
  
  return String(buffer);
}

// --------------------------------------------------------------------
// проверка, есть ли новое обновление
bool HasUpdateAvailable(const String& newDate) {
  if (newDate.length() == 0) return false;
  
  String currentDate = LoadUpdateDate();
  
  if (currentDate.length() == 0) {
    return true;
  }
  
  return newDate.substring(0, 10) > currentDate.substring(0, 10);
}

// --------------------------------------------------------------------
// проверка, есть ли новое обновление
bool CheckAndUpdateDate(const String& newDate) {
  if (newDate.length() < 10) return false;
  
  String currentDate = LoadUpdateDate();
  
  bool isDateValid = false;
  if (currentDate.length() >= 10) {
    if (currentDate[4] == '-' && currentDate[7] == '-') {
      bool allDigits = true;
      for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (!isdigit(currentDate[i])) {
          allDigits = false;
          break;
        }
      }
      if (allDigits) isDateValid = true;
    }
  }
  
  if (!isDateValid || currentDate.length() == 0) {
    SaveUpdateDate(newDate);
    return false;
  }
  
  String newDateOnly = newDate.substring(0, 10);
  String savedDateOnly = currentDate.substring(0, 10);
  
  if (newDateOnly > savedDateOnly) {
    SaveUpdateDate(newDate);
    return true;
  }
  
  return false;
}

// --------------------------------------------------------------------
#if BACKUP_CFG_FILES
    uint32_t WifiBackupCrc(const uint8_t* data, size_t len) {
      uint32_t crc = 0xFFFFFFFFUL;
      while (len--) {
        crc ^= *data++;
        for (uint8_t i = 0; i < 8; i++) {
          crc = (crc & 1U) ? ((crc >> 1) ^ 0xEDB88320UL) : (crc >> 1);
        }
      }

      return ~crc;

    }

// --------------------------------------------------------------------
    void WifiBackupCopy(char* dst, size_t dstSize, const String& src) {
      if (!dst || dstSize == 0) return;
      memset(dst, 0, dstSize);
      if (src.length() == 0) return;
      strncpy(dst, src.c_str(), dstSize - 1);
      dst[dstSize - 1] = 0;
    }

// --------------------------------------------------------------------
    bool SaveWifiBackupForGitHubOta(String& configSetup) {
      if (configSetup.length() == 0) return false;

      WifiBackupData data;
      memset(&data, 0, sizeof(data));
      data.magic = EEPROM_WIFI_BACKUP_MAGIC;
      data.version = EEPROM_WIFI_BACKUP_VERSION;
      data.pending = EEPROM_WIFI_BACKUP_PENDING_MARK;
      data.timeoutSec = (uint16_t)jsonReadtoInt(configWiFi, "TimeOut");

      WifiBackupCopy(data.ssid1, sizeof(data.ssid1), jsonRead(configWiFi, "ssid"));
      WifiBackupCopy(data.password1, sizeof(data.password1), jsonRead(configWiFi, "password"));

      data.crc32 = WifiBackupCrc((const uint8_t*)&data, sizeof(data) - sizeof(data.crc32));

      if (sizeof(data) > EEPROM_WIFI_BACKUP_SIZE) return false;
      const uint8_t* p = (const uint8_t*)&data;

      for (uint16_t i = 0; i < sizeof(data); i++) {
        EEPROM.write(EEPROM_WIFI_BACKUP_START_ADDRESS + i, p[i]);
      }

      for (uint16_t i = sizeof(data); i < EEPROM_WIFI_BACKUP_SIZE; i++) {
        EEPROM.write(EEPROM_WIFI_BACKUP_START_ADDRESS + i, 0);
      }

      return EEPROM.commit();
    }

// --------------------------------------------------------------------
    bool ReadWifiBackupRaw(WifiBackupData &data) {
      memset(&data, 0, sizeof(data));
      if (sizeof(data) > EEPROM_WIFI_BACKUP_SIZE) return false;
      uint8_t* p = (uint8_t*)&data;

      for (uint16_t i = 0; i < sizeof(data); i++) {
        p[i] = EEPROM.read(EEPROM_WIFI_BACKUP_START_ADDRESS + i);
      }

      if (data.magic != EEPROM_WIFI_BACKUP_MAGIC) return false;
      if (data.version != EEPROM_WIFI_BACKUP_VERSION) return false;
      uint32_t crc = WifiBackupCrc((const uint8_t*)&data, sizeof(data) - sizeof(data.crc32));
      if (crc != data.crc32) return false;
      return true;
    }

// --------------------------------------------------------------------
    bool IsWifiBackupAvailable() {
      WifiBackupData data;
      return ReadWifiBackupRaw(data) && data.pending == EEPROM_WIFI_BACKUP_PENDING_MARK;
    }

// --------------------------------------------------------------------
    void ClearWifiBackupPending() {
      WifiBackupData data;
      if (!ReadWifiBackupRaw(data)) return;
      data.pending = 0;
      data.crc32 = WifiBackupCrc((const uint8_t*)&data, sizeof(data) - sizeof(data.crc32));
      const uint8_t* p = (const uint8_t*)&data;

      for (uint16_t i = 0; i < sizeof(data); i++) {
        EEPROM.write(EEPROM_WIFI_BACKUP_START_ADDRESS + i, p[i]);
      }

      EEPROM.commit();
    }

// --------------------------------------------------------------------
    bool RestoreWifiBackupAfterGitHubOta(String &configSetup) {
      WifiBackupData data;
      if (!ReadWifiBackupRaw(data)) return false;
      if (data.pending != EEPROM_WIFI_BACKUP_PENDING_MARK) return false;

      if (configSetup == F("Failed") || configSetup == F("Large") || configSetup.length() == 0) {
        configSetup = F("{}");
      }

      jsonWrite(configWiFi, "TimeOut", (int)data.timeoutSec);
      jsonWrite(configWiFi, "ssid", String(data.ssid1));
      jsonWrite(configWiFi, "password", String(data.password1));
      ClearWifiBackupPending();

      return true;
    }
// --------------------------------------------------------------------
#endif // BACKUP_CFG_FILES

// --------------------------------------------------------------------
  private:
    Eeprom() = default;
    ~Eeprom() = default;
    Eeprom(const Eeprom&) = delete;
    Eeprom& operator=(const Eeprom&) = delete;

// --------------------------------------------------------------------
}; // class Eeprom

// ******************************************************************************************************************************************************

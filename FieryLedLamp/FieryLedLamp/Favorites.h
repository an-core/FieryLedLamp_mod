// ****************************************************************************** Favorites.h ************************************************************
#pragma once
// --------------
#include <EEPROM.h>
#include "Eeprom.h"
#include "Constants.h"
#include "Mqtt.h"
#include "Extern.h"
// ---------------------------

class Favorites {
  public:
    static Favorites& instance() {
      static Favorites instance;
      return instance;
    }

    void SetStatus(char* statusText);
    void ConfigureFavorites(const char* statusText);
    
    bool HandleFavorites(
      bool* ONflag,
      uint8_t* currentMode,
      bool* loadingFlag,
#if USE_DAWN
      uint8_t* dawnFlag,
#endif
#if USE_SUNSET
      uint8_t* sunsetFlag,
#endif
      uint8_t* random_on,
      uint8_t* selectedSettings,
      char* udpBuffer
    );
    
    void TurnFavoritesOff();

    uint8_t FavoritesRunning = 0;
    uint16_t Interval = DEFAULT_FAVORITES_INTERVAL;
    uint16_t Dispersion = DEFAULT_FAVORITES_DISPERSION;
    uint8_t UseSavedFavoritesRunning = 0;
    uint8_t FavoriteModes[MODE_AMOUNT] = {0};
    uint32_t nextModeAt = 0;
    bool rndCycle = false;

  private:
    Favorites() = default;
    ~Favorites() = default;
    Favorites(const Favorites&) = delete;
    Favorites& operator=(const Favorites&) = delete;

    bool isStatusTextCorrect(const char* statusText);
    uint8_t getStatusTextNormalComponentsCount();
    uint8_t getFavoritesRunning(const char* statusText);
    uint16_t getInterval(const char* statusText);
    uint16_t getDispersion(const char* statusText);
    uint8_t getUseSavedFavoritesRunning(const char* statusText);
    bool getModeOnOff(const char* statusText, uint8_t modeId);
    char* getLexNo(const char* statusText, uint8_t pos);
    uint8_t getNextFavoriteMode(uint8_t* currentMode);
    uint32_t getNextTime();

#ifdef USE_SHUFFLE_FAVORITES
    uint8_t shuffleFavoriteModes[MODE_AMOUNT];
    uint8_t shuffleCurrentIndex = MODE_AMOUNT;
#endif
}; // class Favorites

// *******************************************************************************************************************************************************

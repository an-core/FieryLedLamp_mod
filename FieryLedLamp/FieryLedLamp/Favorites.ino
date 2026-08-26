// **************************************************************************** Favrites.ino ************************************************************
#include "Favorites.h"
#include "Prototypes.h"
#include "Extern.h"
// --------------------------

void Favorites::SetStatus(char* statusText) {
  char buff[6];

  strcpy_P(statusText, PSTR("FAV "));
  itoa(FavoritesRunning, buff, 10);
  strcat(statusText, buff);

  strcat_P(statusText, PSTR(" "));
  buff[0] = '\0';
  itoa(Interval, buff, 10);
  strcat(statusText, buff);

  strcat_P(statusText, PSTR(" "));
  buff[0] = '\0';
  itoa(Dispersion, buff, 10);
  strcat(statusText, buff);

  strcat_P(statusText, PSTR(" "));
  buff[0] = '\0';
  itoa(UseSavedFavoritesRunning, buff, 10);
  strcat(statusText, buff);

  strcat_P(statusText, PSTR(" "));
  buff[0] = '\0';

  for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
    itoa(FavoriteModes[eff_num_correct[i]], buff, 10);
    strcat(statusText, buff);
    if (i < MODE_AMOUNT - 1) strcat_P(statusText, PSTR(" "));
    buff[0] = '\0';
  }
  *statusText = '\0';
}

void Favorites::ConfigureFavorites(const char* statusText) {
  FavoritesRunning = getFavoritesRunning(statusText);

  if (FavoritesRunning == 0) {
    nextModeAt = 0;
  }

  Interval = getInterval(statusText);
  Dispersion = getDispersion(statusText);
  UseSavedFavoritesRunning = getUseSavedFavoritesRunning(statusText);

  for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
    FavoriteModes[eff_num_correct[i]] = getModeOnOff(statusText, i);
  }

#ifdef USE_SHUFFLE_FAVORITES
  shuffleCurrentIndex = MODE_AMOUNT;
#endif
}

bool Favorites::HandleFavorites(
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
) {
  if (FavoritesRunning == 0 || !*ONflag || (*currentMode == EFF_WHITE_COLOR && FavoriteModes[EFF_WHITE_COLOR] == 0U)) {
    return false;
  }

#if USE_DAWN
  if (*dawnFlag == 1) return false;
#endif

#if USE_SUNSET
  if (*sunsetFlag == 1) return false;
#endif

  if (nextModeAt == 0) {
    nextModeAt = getNextTime();
    return false;
  }

  if (millis() >= nextModeAt) {
    *currentMode = getNextFavoriteMode(currentMode);
    SetBrightness(modes[*currentMode].Brightness);
    jsonWrite(configSetup, "br", modes[*currentMode].Brightness);
    jsonWrite(configSetup, "sp", modes[*currentMode].Speed);
    jsonWrite(configSetup, "sc", modes[*currentMode].Scale);

#if USE_MULTILAMP
    repeat_multiple_lamp_control = true;
#endif

    *loadingFlag = true;
    nextModeAt = getNextTime();

    if (*random_on) *selectedSettings = 1U;

    uint8_t ui_index = 0;

    for (ui_index = 0; ui_index < MODE_AMOUNT; ui_index++) {
      if (eff_num_correct[ui_index] == *currentMode) break;
    }

    jsonWrite(configSetup, "eff_sel", ui_index);

#if USE_MQTT
    if (Wifi::instance().isConnected()) {
      Mqtt::instance().needToPublish = true;
    }
#endif

    return true;
  }
  return false;
}

void Favorites::TurnFavoritesOff() {
  FavoritesRunning = 0;
  nextModeAt = 0;
}

bool Favorites::isStatusTextCorrect(const char* statusText) {
  char buff[MAX_UDP_BUFFER_SIZE];
  strcpy(buff, statusText);

  uint8_t lexCount = 0;
  char* p = strtok(buff, " ");
  while (p != NULL) {
    lexCount++;
    p = strtok(NULL, " ");
  }

  return lexCount == getStatusTextNormalComponentsCount();
}

uint8_t Favorites::getStatusTextNormalComponentsCount() {
  return 1 + 1 + 1 + 1 + 1 + MODE_AMOUNT; // "FAV" + вкл/выкл + интервал + разброс + useSaved + MODE_AMOUNT
}

uint8_t Favorites::getFavoritesRunning(const char* statusText) {
  char lexem[2];
  memset(lexem, 0, 2);
  char* p = getLexNo(statusText, 1);
  if (p) strcpy(lexem, p);
  return lexem[0] ? (strcmp(lexem, "1") == 0) : 0;
}

uint16_t Favorites::getInterval(const char* statusText) {
  char lexem[6];
  memset(lexem, 0, 6);
  char* p = getLexNo(statusText, 2);
  if (p) strcpy(lexem, p);
  return lexem[0] ? atoi(lexem) : DEFAULT_FAVORITES_INTERVAL;
}

uint16_t Favorites::getDispersion(const char* statusText) {
  char lexem[6];
  memset(lexem, 0, 6);
  char* p = getLexNo(statusText, 3);
  if (p) strcpy(lexem, p);
  return lexem[0] ? atoi(lexem) : DEFAULT_FAVORITES_DISPERSION;
}

uint8_t Favorites::getUseSavedFavoritesRunning(const char* statusText) {
  char lexem[2];
  memset(lexem, 0, 2);
  char* p = getLexNo(statusText, 4);
  if (p) strcpy(lexem, p);
  return lexem[0] ? (strcmp(lexem, "1") == 0) : 0;
}

bool Favorites::getModeOnOff(const char* statusText, uint8_t modeId) {
  char lexem[2];
  memset(lexem, 0, 2);
  char* p = getLexNo(statusText, modeId + 5);
  if (p) strcpy(lexem, p);
  return lexem[0] ? (strcmp(lexem, "1") == 0) : false;
}

char* Favorites::getLexNo(const char* statusText, uint8_t pos) {
  if (!isStatusTextCorrect(statusText)) return NULL;

  const uint8_t buffSize = MAX_UDP_BUFFER_SIZE;
  static char buff[MAX_UDP_BUFFER_SIZE];
  memset(buff, 0, buffSize);
  strcpy(buff, statusText);

  uint8_t lexPos = 0;
  char* p = strtok(buff, " ");
  while (p != NULL) {
    if (lexPos == pos) return p;
    p = strtok(NULL, " ");
    lexPos++;
  }
  return NULL;
}

#ifdef USE_SHUFFLE_FAVORITES
uint8_t Favorites::getNextFavoriteMode(uint8_t* currentMode) {
  uint8_t result;
  uint8_t count = MODE_AMOUNT;

  do {
    shuffleCurrentIndex++;
    if (shuffleCurrentIndex >= MODE_AMOUNT) {
      count = MODE_AMOUNT;
      if (rndCycle) {
        for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
          uint8_t j = random8(MODE_AMOUNT);
          result = shuffleFavoriteModes[i];
          shuffleFavoriteModes[i] = shuffleFavoriteModes[j];
          shuffleFavoriteModes[j] = result;
          if (FavoriteModes[i] == 0) count--;
        }
      } else {
        for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
          shuffleFavoriteModes[i] = (*currentMode + i + 1U) % MODE_AMOUNT;
          if (FavoriteModes[i] == 0) count--;
        }
      }
      shuffleCurrentIndex = 0;
    }
  }
  
  while ((FavoriteModes[shuffleFavoriteModes[shuffleCurrentIndex]] == 0U || shuffleFavoriteModes[shuffleCurrentIndex] == *currentMode) && count > 1U);

  if (count > 1U)
    result = shuffleFavoriteModes[shuffleCurrentIndex];
  else
    result = *currentMode + 1U < MODE_AMOUNT ? *currentMode + 1U : 0U;
  return result;
} // uint8_t Favorites::getNextFavoriteMode(uint8_t* currentMode)

#else
uint8_t Favorites::getNextFavoriteMode(uint8_t* currentMode) {
  uint8_t result = *currentMode;

  for (uint8_t n = 0; n < MODE_AMOUNT; n++) {
    if (eff_num_correct[n] == result)
    {
      result = n;
      break;
    }
  }

  for (uint8_t tryNo = 0; tryNo <= random(0, MODE_AMOUNT); tryNo++) {
    for (uint8_t i = (result + 1); i <= (result + MODE_AMOUNT); i++) {
      if (FavoriteModes[eff_num_correct[i < MODE_AMOUNT ? i : i - MODE_AMOUNT]] > 0) {
        result = i < MODE_AMOUNT ? i : i - MODE_AMOUNT;
        break;
      }
    }
    if (!rndCycle) break;
  }
  return eff_num_correct[result];
} // uint8_t Favorites::getNextFavoriteMode(uint8_t* currentMode)
#endif // USE_SHUFFLE_FAVORITES

uint32_t Favorites::getNextTime() {
  return millis() + Interval * 1000 + random(0, Dispersion + 1) * 1000;
}

// ******************************************************************************************************************************************************

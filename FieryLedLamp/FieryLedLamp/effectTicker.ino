// *************************************************************************** effectsTicker.ino ********************************************************
#include "Time.h"
#include "Extern.h"
#include "Prototypes.h"
// -------------------------

// буфер для кадров .out (выделяется один раз при запуске эффекта)
#if USE_SD
static uint8_t* outFrameBuffer = nullptr;
static size_t outFrameBufferSize = 0;
static bool outStartFailed = false;  // это защита от бесконечного рестарта

static bool ensureOutFrameBuffer(size_t needed) {
  if (needed <= outFrameBufferSize) return true;
  if (outFrameBuffer) {
    free(outFrameBuffer);
    outFrameBuffer = nullptr;
  }
  outFrameBuffer = (uint8_t*)malloc(needed);
  if (!outFrameBuffer) {
#if EFF_LOG
    SYSLOG.add("EFF_SD: не удалось выделить outFrameBuffer (%u байт)", (unsigned)needed);
#endif
    outFrameBufferSize = 0;
    return false;
  }
  outFrameBufferSize = needed;
  return true;
}
#endif // USE_SD

void effectsTick() {
  if (systemShuttingDown || !ONflag) return;
#if USE_SD
  if (currentMode == EFF_SD) {
    loadingFlag = false;

    if (outStartFailed) {
      FastLED.clear(); FastLED.show(); Eff_Tick(); return;
    }

    if (!outAnimationActive || !outFile) {
      if (lastOutFileName.length() > 0) {
        if (!startOutAnimation(lastOutFileName)) {
#if EFF_LOG
          SYSLOG.add("EFF_SD: не удалось запустить %s, откат на EFF_RAINBOW_VER", lastOutFileName.c_str());
#endif
          outStartFailed = true;
          outAnimationActive = false;
          currentMode = EFF_RAINBOW_VER;
          for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
            if (eff_num_correct[i] == currentMode) {
              jsonWrite(configSetup, "eff_sel", i);
              break;
            }
          }
          saveConfig("setup");
          loadingFlag = true;
          FastLED.clear(); FastLED.show(); Eff_Tick();
          return;
        }
        outStartFailed = false;
      } else {
        FastLED.clear(); FastLED.show(); Eff_Tick(); return;
      }
    }

    // расчёт задержки
    uint32_t baseDelay = outFrameDelay;
    uint32_t speedNum = modes[currentMode].Speed;
    if (speedNum < 1) speedNum = 128;
    uint32_t currentDelay = baseDelay * 128 / speedNum;
    if (currentDelay < 10) currentDelay = 10;

    uint32_t now = millis();
    if (now - outLastFrameTime >= currentDelay) {
      outLastFrameTime = now;

      // блочное чтение кадра: [delayByte] + [usedLeds * 3 байт GRB] (первый байт кадра (delay) пропустить - outFrameDelay уже посчитан при старте)
      size_t frameSize = (size_t)usedLeds * 3;
      if (!ensureOutFrameBuffer(frameSize)) {
        outFile.close();
        outAnimationActive = false;
        FastLED.clear(); FastLED.show(); Eff_Tick(); return;
      }

      int delayByte = outFile.read();
      if (delayByte == -1) {
        outFile.seek(0);
        outFile.read();
      }

      size_t bytesRead = outFile.read(outFrameBuffer, frameSize);
      if (bytesRead < frameSize) {
        outFile.seek(0);
        outFile.read();
        bytesRead = outFile.read(outFrameBuffer, frameSize);
        if (bytesRead < frameSize) {
          outFile.close();
          outAnimationActive = false;
#if EFF_LOG
          SYSLOG.add("EFF_SD: файл %s не содержит полного кадра", lastOutFileName.c_str());
#endif
          FastLED.clear(); FastLED.show(); Eff_Tick(); return;
        }
      }

      for (uint16_t i = 0; i < usedLeds; i++) {
        uint8_t r = outFrameBuffer[i * 3];
        uint8_t g = outFrameBuffer[i * 3 + 1];
        uint8_t b = outFrameBuffer[i * 3 + 2];

        uint8_t fileX = i % matrixWidth;
        uint8_t fileY = i / matrixWidth;
        uint8_t lampY = matrixHeight - 1 - fileY;
        drawPixelXY(fileX, lampY, CRGB(g, r, b)); // GRB + инверсия Y
      }

      FastLED.show();
    }

    Eff_Tick();
    return;
  }
#endif // USE_SD

#if USE_DAWN && USE_SUNSET
  if (!dawnFlag && !sunsetFlag && ONflag) {
#elif USE_DAWN
  if (!dawnFlag && ONflag) {
#elif USE_SUNSET
  if (!sunsetFlag && ONflag) {
#else
  if (ONflag) {
#endif
    if (justPoweredOn) {
      justPoweredOn = false;
      loadingFlag = true;
      effTimer = millis();
#if LED_PANEL
      needFullRedraw = true;
#endif
      clockNeedRedraw = true;
      resetTimerState();
    }

    switch (currentMode) {
#if USE_ANIMATIONS
      case EFF_ANIMATION:           LOW_DELAY_TICK { effTimer = millis(); animationRoutine();                Eff_Tick (); }  break;  // (  0U) Анимация
#endif
      case EFF_AVRORA:              HIGH_DELAY_TICK { effTimer = millis(); Avrora();                         Eff_Tick (); }  break;  // (  1U) Аврора
      case EFF_WATERCOLOR:          DYNAMIC_DELAY_TICK { effTimer = millis(); Watercolor();                  Eff_Tick (); }  break;  // (  2U) Акварель
      case EFF_FLOWERRUTA:          DYNAMIC_DELAY_TICK { effTimer = millis(); FlowerRuta();                  Eff_Tick (); }  break;  // (  3U) Аленький цветочек
      case EFF_BUTTERFLY:           LOW_DELAY_TICK { effTimer = millis(); butterflyRoutine();                Eff_Tick (); }  break;  // (  4U) Бабочка
      case EFF_EFF_POOL:            DYNAMIC_DELAY_TICK { effTimer = millis(); poolRoutine();                 Eff_Tick (); }  break;  // (  5U) Бассейн
      case EFF_BAMBOO :             DYNAMIC_DELAY_TICK { effTimer = millis(); Bamboo();                      Eff_Tick (); }  break;  // (  6U) Бамбук
      case EFF_MADNESS:             HIGH_DELAY_TICK { effTimer = millis(); madnessNoiseRoutine();            Eff_Tick (); }  break;  // (  7U) Безумие
      case EFF_CUBEROUTINE:         DYNAMIC_DELAY_TICK { effTimer = millis(); cubeRoutine();                 Eff_Tick (); }  break;  // (  8U) Блуждающий кубик
      case EFF_WATERFALL:           DYNAMIC_DELAY_TICK { effTimer = millis(); fire2012WithPalette();         Eff_Tick (); }  break;  // (  9U) Водопад
      case EFF_WATERFALL_4IN1:      DYNAMIC_DELAY_TICK { effTimer = millis(); fire2012WithPalette4in1();     Eff_Tick (); }  break;  // ( 10U) Водопад 4в1
      case EFF_WAVES:               DYNAMIC_DELAY_TICK { effTimer = millis(); WaveRoutine();                 Eff_Tick (); }  break;  // ( 11U) Волны
      case EFF_MAGICLANTERN:        DYNAMIC_DELAY_TICK { effTimer = millis(); MagicLantern();                Eff_Tick (); }  break;  // ( 12U) Волшебный Фонарик
      case EFF_WINE:                DYNAMIC_DELAY_TICK { effTimer = millis(); colorsWine();                  Eff_Tick (); }  break;  // ( 13U) Вино
      case EFF_WHIRL:               DYNAMIC_DELAY_TICK { effTimer = millis(); whirlRoutine(true);            Eff_Tick (); }  break;  // ( 14U) Вихри пламени
      case EFF_WHIRL_MULTI:         DYNAMIC_DELAY_TICK { effTimer = millis(); whirlRoutine(false);           Eff_Tick (); }  break;  // ( 15U) Вихри разноцветные
      case EFF_STARFALL:            DYNAMIC_DELAY_TICK { effTimer = millis(); stormRoutine2();               Eff_Tick (); }  break;  // ( 16U) Вьюга
      case EFF_STORMY_RAIN:         DYNAMIC_DELAY_TICK { effTimer = millis(); stormyRain();                  Eff_Tick (); }  break;  // ( 17U) Гроза в банке
      case EFF_DNA:                 LOW_DELAY_TICK { effTimer = millis(); DNARoutine();                      Eff_Tick (); }  break;  // ( 18U) ДНК
      case EFF_SMOKE:               DYNAMIC_DELAY_TICK { effTimer = millis(); MultipleStreamSmoke(false);    Eff_Tick (); }  break;  // ( 19U) Дым
      case EFF_SMOKE_COLOR:         DYNAMIC_DELAY_TICK { effTimer = millis(); MultipleStreamSmoke(true);     Eff_Tick (); }  break;  // ( 20U) Дым разноцветный
      case EFF_SMOKEBALLS:          LOW_DELAY_TICK { effTimer = millis(); smokeballsRoutine();               Eff_Tick (); }  break;  // ( 21U) Дымовые шашки
      case EFF_LIQUIDLAMP:          LOW_DELAY_TICK { effTimer = millis(); LiquidLampRoutine(true);           Eff_Tick (); }  break;  // ( 22U) Жидкая лампа
      case EFF_LIQUIDLAMP_AUTO:     LOW_DELAY_TICK { effTimer = millis(); LiquidLampRoutine(false);          Eff_Tick (); }  break;  // ( 23U) Жидкая лампа авто
      case EFF_SWIRL:               DYNAMIC_DELAY_TICK { effTimer = millis(); Swirl();                       Eff_Tick (); }  break;  // ( 24U) Завиток
      case EFF_STARS:               DYNAMIC_DELAY_TICK { effTimer = millis(); EffectStars();                 Eff_Tick (); }  break;  // ( 25U) Звезды
      case EFF_ZEBRA:               HIGH_DELAY_TICK { effTimer = millis(); zebraNoiseRoutine();              Eff_Tick (); }  break;  // ( 26U) Зебра
      case EFF_TIXYLAND:            DYNAMIC_DELAY_TICK { effTimer = millis(); TixyLand();                    Eff_Tick (); }  break;  // ( 27U) Земля Тикси
      case EFF_SNAKE_GAME:          DYNAMIC_DELAY_TICK { effTimer = millis(); snakeGameRoutine();            Eff_Tick (); }  break;  // ( 28U) Змейка (игра)
      case EFF_SNAKES:              LOW_DELAY_TICK { effTimer = millis(); snakesRoutine();                   Eff_Tick (); }  break;  // ( 29U) Змейки
      case EFF_FOUNTAIN:            DYNAMIC_DELAY_TICK { effTimer = millis(); starfield2Routine();           Eff_Tick (); }  break;  // ( 30U) Источник
      case EFF_DROP_IN_WATER:       DYNAMIC_DELAY_TICK { effTimer = millis(); DropInWater();                 Eff_Tick (); }  break;  // ( 31U) Капли на воде
      case EFF_DROPS:               LOW_DELAY_TICK { effTimer = millis(); newMatrixRoutine();                Eff_Tick (); }  break;  // ( 32U) Капли на стекле
      case EFF_LLAND:               DYNAMIC_DELAY_TICK { effTimer = millis(); LLandRoutine();                Eff_Tick (); }  break;  // ( 33U) Кипение
      case EFF_RINGS:               DYNAMIC_DELAY_TICK { effTimer = millis(); ringsRoutine();                Eff_Tick (); }  break;  // ( 34U) Кодовый замок
      case EFF_COMET:               DYNAMIC_DELAY_TICK { effTimer = millis(); RainbowCometRoutine();         Eff_Tick (); }  break;  // ( 35U) Комета
      case EFF_COMET_COLOR:         DYNAMIC_DELAY_TICK { effTimer = millis(); ColorCometRoutine();           Eff_Tick (); }  break;  // ( 36U) Комета одноцветная
      case EFF_COMET_TWO:           DYNAMIC_DELAY_TICK { effTimer = millis(); MultipleStream();              Eff_Tick (); }  break;  // ( 37U) Комета двойная
      case EFF_COMET_THREE:         DYNAMIC_DELAY_TICK { effTimer = millis(); MultipleStream2();             Eff_Tick (); }  break;  // ( 38U) Комета тройная
      case EFF_CONTACTS:            DYNAMIC_DELAY_TICK { effTimer = millis(); Contacts();                    Eff_Tick (); }  break;  // ( 39U) Контакты
      case EFF_SPARKLES:            DYNAMIC_DELAY_TICK { effTimer = millis(); sparklesRoutine();             Eff_Tick (); }  break;  // ( 40U) Конфетти
      case EFF_CUBERUBIKA:          DYNAMIC_DELAY_TICK { effTimer = millis(); cubeRubik();                   Eff_Tick (); }  break;  // ( 41U) Кубик Рубика
      case EFF_LAVA:                HIGH_DELAY_TICK { effTimer = millis(); lavaNoiseRoutine();               Eff_Tick (); }  break;  // ( 42U) Лава
      case EFF_LAVALAMP:            LOW_DELAY_TICK { effTimer = millis(); LavaLampRoutine();                 Eff_Tick (); }  break;  // ( 43U) Лавовая лампа
      case EFF_BUTTERFLYS_LAMP:     LOW_DELAY_TICK { effTimer = millis(); butterflysRoutine(false);          Eff_Tick (); }  break;  // ( 44U) Лампа с мотыльками
      case EFF_FOREST:              HIGH_DELAY_TICK { effTimer = millis(); forestNoiseRoutine();             Eff_Tick (); }  break;  // ( 45U) Лес
      case EFF_LUMENJER:            DYNAMIC_DELAY_TICK { effTimer = millis(); lumenjerRoutine();             Eff_Tick (); }  break;  // ( 46U) Люмeньep
      case EFF_MAGMA:               DYNAMIC_DELAY_TICK { effTimer = millis(); magmaRoutine();                Eff_Tick (); }  break;  // ( 47U) Магма
      case EFF_MARIO:               LOW_DELAY_TICK { effTimer = millis(); marioRoutine();                    Eff_Tick (); }  break;  // ( 48U) Марио (игра)
      case EFF_PAINTS:              DYNAMIC_DELAY_TICK { effTimer = millis(); OilPaints();                   Eff_Tick (); }  break;  // ( 49U) Масляные краски
      case EFF_MATRIX:              DYNAMIC_DELAY_TICK { effTimer = millis(); matrixRoutine();               Eff_Tick (); }  break;  // ( 50U) Матрица
      case EFF_TWINKLES:            DYNAMIC_DELAY_TICK { effTimer = millis(); twinklesRoutine();             Eff_Tick (); }  break;  // ( 51U) Мерцание
      case EFF_METEOR:              DYNAMIC_DELAY_TICK { effTimer = millis(); meteorRoutine();               Eff_Tick (); }  break;  // ( 52U) Метеор
      case EFF_METABALLS:           LOW_DELAY_TICK { effTimer = millis(); MetaBallsRoutine();                Eff_Tick (); }  break;  // ( 53U) Метоболз
      case EFF_WEB_TOOLS:           SOFT_DELAY_TICK { effTimer = millis(); WebTools();                       Eff_Tick (); }  break;  // ( 54U) Мечта дизайнера
      case EFF_MOSAIC:              DYNAMIC_DELAY_TICK { effTimer = millis(); squaresNdotsRoutine();         Eff_Tick (); }  break;  // ( 55U) Мозайка
      case EFF_BUTTERFLYS:          LOW_DELAY_TICK { effTimer = millis(); butterflysRoutine(true);           Eff_Tick (); }  break;  // ( 56U) Moтыльки
      case EFF_BBALLS:              LOW_DELAY_TICK { effTimer = millis(); BBallsRoutine();                   Eff_Tick (); }  break;  // ( 57U) Мячики
      case EFF_BALLS_BOUNCE:        LOW_DELAY_TICK { effTimer = millis(); bounceRoutine();                   Eff_Tick (); }  break;  // ( 58U) Мячики без границ
      case EFF_CHRISTMAS_TREE:      DYNAMIC_DELAY_TICK { effTimer = millis(); ChristmasTree();               Eff_Tick (); }  break;  // ( 59U) Новогодняя Елка
      case EFF_NEW_STARS:           LOW_DELAY_TICK { effTimer = millis(); newStars();                        Eff_Tick (); }  break;  // ( 60U) Новые Звёзды
      case EFF_NIGHTCITY:           HIGH_DELAY_TICK { effTimer = millis(); NightCity();                      Eff_Tick (); }  break;  // ( 61U) Ночной Город
      case EFF_FIRE:                DYNAMIC_DELAY_TICK { effTimer = millis(); fireRoutine();                 Eff_Tick (); }  break;  // ( 62U) Огонь
      case EFF_FIRE_2012:           DYNAMIC_DELAY_TICK { effTimer = millis(); fire2012again();               Eff_Tick (); }  break;  // ( 63U) Огонь 2012
      case EFF_FIRE_2018:           DYNAMIC_DELAY_TICK { effTimer = millis(); Fire2018_2();                  Eff_Tick (); }  break;  // ( 64U) Огонь 2018
      case EFF_FIRE_2020:           DYNAMIC_DELAY_TICK { effTimer = millis(); fire2020Routine2();            Eff_Tick (); }  break;  // ( 65U) Огонь 2020
      case EFF_FIRE_2021:           LOW_DELAY_TICK { effTimer = millis(); Fire2021Routine();                 Eff_Tick (); }  break;  // ( 66U) Огонь 2021
      case EFF_FIREFLY_TOP:         DYNAMIC_DELAY_TICK { effTimer = millis(); MultipleStream5();             Eff_Tick (); }  break;  // ( 67U) Огoнь верховой
      case EFF_FIREFLY:             DYNAMIC_DELAY_TICK { effTimer = millis(); MultipleStream3();             Eff_Tick (); }  break;  // ( 68U) Огoнь парящий
      case EFF_FIRESPARKS:          HIGH_DELAY_TICK { effTimer = millis(); FireSparks();                     Eff_Tick (); }  break;  // ( 69U) Огонь с искрами
      case EFF_COLOR_RAIN:          DYNAMIC_DELAY_TICK { effTimer = millis(); coloredRain();                 Eff_Tick (); }  break;  // ( 70U) Осадки
      case EFF_OSCILLATING:         DYNAMIC_DELAY_TICK { effTimer = millis(); oscillatingRoutine();          Eff_Tick (); }  break;  // ( 71U) Осциллятор
      case EFF_CLOUDS:              HIGH_DELAY_TICK { effTimer = millis(); cloudsNoiseRoutine();             Eff_Tick (); }  break;  // ( 72U) Облака
      case EFF_OCEAN:               HIGH_DELAY_TICK { effTimer = millis(); oceanNoiseRoutine();              Eff_Tick (); }  break;  // ( 73U) Океан
      case EFF_OCTOPUS:             DYNAMIC_DELAY_TICK { effTimer = millis(); Octopus();                     Eff_Tick (); }  break;  // ( 74U) Осьминог
      case EFF_RAINBOW_STRIPE:      HIGH_DELAY_TICK { effTimer = millis(); rainbowStripeNoiseRoutine();      Eff_Tick (); }  break;  // ( 75U) Павлин
      case EFF_HOURGLASS:           DYNAMIC_DELAY_TICK { effTimer = millis(); Hourglass();                   Eff_Tick (); }  break;  // ( 76U) Песочные часы
      case EFF_PAINTBALL:           DYNAMIC_DELAY_TICK { effTimer = millis(); lightBallsRoutine();           Eff_Tick (); }  break;  // ( 77U) Пейнтбол
      case EFF_PICASSO:             DYNAMIC_DELAY_TICK { effTimer = millis(); picassoSelector();             Eff_Tick (); }  break;  // ( 78U) Пикассо
      case EFF_PLASMA:              HIGH_DELAY_TICK { effTimer = millis(); plasmaNoiseRoutine();             Eff_Tick (); }  break;  // ( 79U) Плазма
      case EFF_SPIDER:              LOW_DELAY_TICK { effTimer = millis(); spiderRoutine();                   Eff_Tick (); }  break;  // ( 80U) Плазменная лампа
      case EFF_PLASMA_WAVES:        SOFT_DELAY_TICK { effTimer = millis(); Plasma_Waves();                   Eff_Tick (); }  break;  // ( 81U) Плазменные волны
      case EFF_FLAME:               LOW_DELAY_TICK { effTimer = millis(); execStringsFlame();                Eff_Tick (); }  break;  // ( 82U) Пламя
      case EFF_PLANETEARTH:         SOFT_DELAY_TICK { effTimer = millis(); PlanetEarth();                    Eff_Tick (); }  break;  // ( 83U) Планета Земля
      case EFF_BY_EFFECT:           DYNAMIC_DELAY_TICK { effTimer = millis(); ByEffect();                    Eff_Tick (); }  break;  // ( 84U) Побочный эффект
      case EFF_POPCORN:             LOW_DELAY_TICK { effTimer = millis(); popcornRoutine();                  Eff_Tick (); }  break;  // ( 85U) Попкорн
      case EFF_PRISMATA:            LOW_DELAY_TICK { effTimer = millis(); PrismataRoutine();                 Eff_Tick (); }  break;  // ( 86U) Призмата
      case EFF_ATTRACT:             DYNAMIC_DELAY_TICK { effTimer = millis(); attractRoutine();              Eff_Tick (); }  break;  // ( 87U) Притяжение
      case EFF_LEAPERS:             DYNAMIC_DELAY_TICK { effTimer = millis(); LeapersRoutine();              Eff_Tick (); }  break;  // ( 88U) Пpыгyны
      case EFF_PULSE:               DYNAMIC_DELAY_TICK { effTimer = millis(); pulseRoutine(2U);              Eff_Tick (); }  break;  // ( 89U) Пульс
      case EFF_PULSE_WHITE:         LOW_DELAY_TICK     { effTimer = millis(); pulseRoutine(8U);              Eff_Tick (); }  break;  // ( 90U) Пульс белый
      case EFF_PULSE_RAINBOW:       DYNAMIC_DELAY_TICK { effTimer = millis(); pulseRoutine(4U);              Eff_Tick (); }  break;  // ( 91U) Пульс радужный
      case EFF_RADIAL_WAWE:         DYNAMIC_DELAY_TICK { effTimer = millis(); RadialWave();                  Eff_Tick (); }  break;  // ( 92U) Радиальная волна
      case EFF_RAINBOW_VER:         DYNAMIC_DELAY_TICK { effTimer = millis(); rainbowRoutine();              Eff_Tick (); }  break;  // ( 93U) Радуга
      case EFF_RAINBOW:             HIGH_DELAY_TICK { effTimer = millis(); rainbowNoiseRoutine();            Eff_Tick (); }  break;  // ( 94U) Радуга 3D
      case EFF_RAINBOW_SPOT:        DYNAMIC_DELAY_TICK { effTimer = millis(); RainbowSpot();                 Eff_Tick (); }  break;  // ( 95U) Радужное Пятно
      case EFF_RAINBOW_RINGS:       DYNAMIC_DELAY_TICK { effTimer = millis(); RainbowRings();                Eff_Tick (); }  break;  // ( 96U) Радужные кольца
      case EFF_SNAKE:               DYNAMIC_DELAY_TICK { effTimer = millis(); MultipleStream8();             Eff_Tick (); }  break;  // ( 97U) Радужный змей
      case EFF_RAIN:                DYNAMIC_DELAY_TICK { effTimer = millis(); RainRoutine();                 Eff_Tick (); }  break;  // ( 98U) Разноцветный дождь
      case EFF_DANDELIONS:          SOFT_DELAY_TICK  { effTimer = millis(); Dandelions();                    Eff_Tick (); }  break;  // ( 99U) Разноцветные одуванчики
      case EFF_RIVERS:              DYNAMIC_DELAY_TICK { effTimer = millis(); BotswanaRivers();              Eff_Tick (); }  break;  // (100U) Реки Ботсваны
      case EFF_LIGHTERS:            DYNAMIC_DELAY_TICK { effTimer = millis(); lightersRoutine();             Eff_Tick (); }  break;  // (101U) Светлячки
      case EFF_LIGHTER_TRACES:      DYNAMIC_DELAY_TICK { effTimer = millis(); ballsRoutine();                Eff_Tick (); }  break;  // (102U) Светлячки со шлейфом
      case EFF_FEATHER_CANDLE:      DYNAMIC_DELAY_TICK { effTimer = millis(); FeatherCandleRoutine();        Eff_Tick (); }  break;  // (103U) Свеча
      case EFF_AURORA:              HIGH_DELAY_TICK { effTimer = millis(); polarRoutine();                   Eff_Tick (); }  break;  // (104U) Северное сияние
      case EFF_SERPENTINE:          HIGH_DELAY_TICK { effTimer = millis(); Serpentine();                     Eff_Tick (); }  break;  // (105U) Серпантин
      case EFF_SCANNER:             DYNAMIC_DELAY_TICK { effTimer = millis(); Scanner();                     Eff_Tick (); }  break;  // (106U) Сканер
      case EFF_SINUSOID3:           HIGH_DELAY_TICK { effTimer = millis(); Sinusoid3Routine();               Eff_Tick (); }  break;  // (107U) Синусоид
      case EFF_COLORS:              HIGH_DELAY_TICK { effTimer = millis(); colorsRoutine2();                 Eff_Tick (); }  break;  // (108U) Смена цвета
      case EFF_SNOW:                DYNAMIC_DELAY_TICK { effTimer = millis(); Snowfall();                    Eff_Tick (); }  break;  // (109U) Снегопад
      case EFF_SPECTRUM:            DYNAMIC_DELAY_TICK { effTimer = millis(); Spectrum();                    Eff_Tick (); }  break;  // (110U) Спектрум
      case EFF_SPIRO:               LOW_DELAY_TICK { effTimer = millis(); spiroRoutine();                    Eff_Tick (); }  break;  // (111U) Спирали
      case EFF_FLOCK:               LOW_DELAY_TICK { effTimer = millis(); flockRoutine(false);               Eff_Tick (); }  break;  // (112U) Стая
      case EFF_FLOCK_N_PR:          LOW_DELAY_TICK { effTimer = millis(); flockRoutine(true);                Eff_Tick (); }  break;  // (113U) Стая и хищник
      case EFF_ARROWS:              DYNAMIC_DELAY_TICK { effTimer = millis(); arrowsRoutine();               Eff_Tick (); }  break;  // (114U) Стрелки
      case EFF_STROBE:              LOW_DELAY_TICK { effTimer = millis(); StrobeAndDiffusion();              Eff_Tick (); }  break;  // (115U) Строб.Хаос.Дифузия
      case EFF_SHADOWS:             LOW_DELAY_TICK { effTimer = millis(); shadowsRoutine();                  Eff_Tick (); }  break;  // (116U) Тени
      case EFF_PACIFIC:             LOW_DELAY_TICK { effTimer = millis(); pacificRoutine();                  Eff_Tick (); }  break;  // (117U) Тихий океан
      case EFF_TORNADO:             LOW_DELAY_TICK { effTimer = millis(); Tornado();                         Eff_Tick (); }  break;  // (118U) Торнадо
      case EFF_SIMPLE_RAIN:         DYNAMIC_DELAY_TICK { effTimer = millis(); simpleRain();                  Eff_Tick (); }  break;  // (119U) Tyчкa в банке
      case EFF_FIREWORK:            SOFT_DELAY_TICK { effTimer = millis(); Firework();                       Eff_Tick (); }  break;  // (120U) Фейерверк
      case EFF_FIREWORK_2:          DYNAMIC_DELAY_TICK { effTimer = millis(); fireworksRoutine();            Eff_Tick (); }  break;  // (121U) Фейерверк 2
      case EFF_FAIRY:               DYNAMIC_DELAY_TICK { effTimer = millis(); fairyRoutine();                Eff_Tick (); }  break;  // (122U) Фея
      case EFF_FONTAN:              DYNAMIC_DELAY_TICK { effTimer = millis(); Fountain();                    Eff_Tick (); }  break;  // (123U) Фонтан
      case EFF_TRICOLOR:            LOW_DELAY_TICK { effTimer = millis(); FlagRoutine();                     Eff_Tick (); }  break;  // (124U) Флаг (Триколор)
      case EFF_COLOR:               LOW_DELAY_TICK  { effTimer = millis(); colorRoutine();                   Eff_Tick (); }  break;  // (125U) Цвет
      case EFF_COLORED_PYTHON:      LOW_DELAY_TICK { effTimer = millis(); Colored_Python();                  Eff_Tick (); }  break;  // (126U) Цветной Питон
      case EFF_SAND:                DYNAMIC_DELAY_TICK { effTimer = millis(); sandRoutine();                 Eff_Tick (); }  break;  // (127U) Цветные драже
      case EFF_COLOR_FRIZZLES:      SOFT_DELAY_TICK { effTimer = millis(); ColorFrizzles();                  Eff_Tick (); }  break;  // (128U) Цветные кудри
      case EFF_LOTUS:               DYNAMIC_DELAY_TICK { effTimer = millis(); LotusFlower();                 Eff_Tick (); }  break;  // (129U) Цветок лотоса
      case EFF_TURBULENCE:          DYNAMIC_DELAY_TICK { effTimer = millis(); Turbulence();                  Eff_Tick (); }  break;  // (130U) Цифровая турбулентность
      case EFF_SPHERES:             LOW_DELAY_TICK { effTimer = millis(); spheresRoutine();                  Eff_Tick (); }  break;  // (131U) Шapы
      case EFF_NEXUS:               LOW_DELAY_TICK { effTimer = millis(); nexusRoutine();                    Eff_Tick (); }  break;  // (132U) Nexus
      case EFF_COLOR_FADED:         LOW_DELAY_TICK { effTimer = millis(); colorFaded();                      Eff_Tick (); }  break;  // (133U) Цвет с затуханием
      case EFF_CLOCK:               DYNAMIC_DELAY_TICK { effTimer = millis(); clockRoutine(); effectDrawn = true; Eff_Tick (); } break;  // (134U) Часы

    } // switch
  } // if (ONflag)

#if USE_DAWN && USE_SUNSET
  else if (!dawnFlag && !sunsetFlag && !ONflag) {
    FastLED.clear();
    FastLED.show();
  }
#elif USE_DAWN
  else if (!dawnFlag && !ONflag) {
    FastLED.clear();
    FastLED.show();
  }
#elif USE_SUNSET
  else if (!sunsetFlag && !ONflag) {
    FastLED.clear();
    FastLED.show();
  }
#else
  else {
    FastLED.clear();
    FastLED.show();
  }
#endif
}

// ----------------------------------------------------------------------------------------
void changePower() {
#if EFF_LOG
  SYSLOG.add("CP: start, millis=%u, ONflag=%d", millis(), ONflag);
#endif
  systemShuttingDown = true;
#if EFF_LOG
  SYSLOG.add("CP: systemShuttingDown=true, millis=%u", millis());
#endif
  uint8_t targetBrightness;
  if (currentMode == EFF_CLOCK && nightModeBrightness > 0) {
    targetBrightness = nightModeBrightness;
  } else {
    targetBrightness = modes[currentMode].Brightness;
  }
#if EFF_LOG
  SYSLOG.add("CP: targetBrightness=%d, millis=%u", targetBrightness, millis());
#endif

  if (AutoBrightness && !day_night) {
    targetBrightness = constrain(targetBrightness >> AutoBrightness, 1, 100);
  }

#if USE_DAWN
  if (dawnFlag == 2) {
    dawnFlag = 0;
    ONflag = true;
  }
#endif

#if USE_SUNSET
  if (sunsetFlag == 2) {
    sunsetFlag = 0;
    ONflag = false;
#if USE_MP3_PLAYER
    if (mp3_player_connect == 4) {
      send_command(0x0E, FEEDBACK, 0, 0);
      delay(50);
      mp3_stop = true;
      pause_on = true;
    }
#endif
    FastLED.clear();
    FastLED.setBrightness(0);
    FastLED.show();
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif
    justPoweredOn = false;
    systemShuttingDown = false;
    return;
  }
#endif

  if (ONflag) { // включение
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
#endif

    justPoweredOn = true;
    loadingFlag = true;
#if LED_PANEL
    needFullRedraw = true;
#endif
    clockNeedRedraw = true;
    resetTimerState();

    if (runTextEnabled) {
      textIsRunning = false;
      offset = matrixWidth + 10;
    }

    FastLED.setBrightness(targetBrightness);
    FastLED.clear();

#if USE_MP3_PLAYER
    if (mp3Enabled && eff_sound_on && mp3_player_connect == 4) {
      mp3_folder = effects_folders[currentMode];
      play_sound();
    }
#endif

    systemShuttingDown = false;
    effectsTick();
    FastLED.show();

  } // if (ONflag) { // включение
  else { // выключение
    if (leds != nullptr) {
      FastLED.setBrightness(0);
      FastLED.clear();
      FastLED.show();
    }
#if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)
    digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
#endif

    timeout_save_file_changes = 0;
    manualOverride = true;
    manualOverrideUntil = millis() + 300000UL;

#if USE_MP3_PLAYER
    if (mp3Enabled && mp3_player_connect == 4) {
      send_command(0x0E, FEEDBACK, 0, 0);
      delay(50);
      mp3_stop = true;
      pause_on = true;
    }
#endif

#if USE_DAWN
    dawnFlag = 0;
#endif
#if USE_SUNSET
    sunsetFlag = 0;
#endif
    Favorites::instance().FavoritesRunning = false;

    justPoweredOn = false;
    loadingFlag = false;
    textIsRunning = false;
    offset = matrixWidth + 10;
    effTimer = 0;
    runTextOver = false;
    resetTimerState();
    systemShuttingDown = false;

#if USE_SD
    outStartFailed = false;
#endif
  } // else { // выключение

#if USE_MQTT
  if (Wifi::instance().isConnected()) {
    Mqtt::instance().needToPublish = true;
  }
#endif
} // void changePower()

// ----------------------------------------------------------------------------------------
void Eff_Tick() {
  if (systemShuttingDown) return;
  if (leds == nullptr) return;

  if (!ONflag) {
    FastLED.clear();
    FastLED.show();
    return;
  }
  if (currentMode == EFF_CLOCK && nightModeBrightness > 0) {
    FastLED.setBrightness(nightModeBrightness);
  }
  uint32_t now = millis();

#if USE_MP3_PLAYER
  static uint8_t lastMode = 255;
  if (mp3Enabled && lastMode != currentMode && ONflag && eff_sound_on && mp3_player_connect == 4) {
    mp3_folder = effects_folders[currentMode];
    play_sound();
    lastMode = currentMode;
  }
#endif

  // бегущая строка
  if (runTextEnabled) {
    if (!textIsRunning && (IntervalrunText == 0 || runningTextTimer.isReady())) {
      textIsRunning = true;
    }
  }

  // -------------------------------------------
  // мигание точек в дате и двоеточий в часах
  static uint32_t lastBlinkTimer = 0;
  if (now - lastBlinkTimer >= 20) {
    lastBlinkTimer = now;

    if (nightModeBrightness > 0) {
      globalPointBrightness = 255;
      globalDegBrightness = 255;
    } else {
      uint32_t phaseLong = now % 2000UL;
      uint8_t brLong = (phaseLong < 1000) ? map(phaseLong, 0, 1000, 10, 255) : map(phaseLong - 1000, 0, 1000, 255, 10);

      globalPointBrightness = dateSeparatorBlinking ? brLong : 255;

#if USE_WEATHER
      globalDegBrightness = degreeSymbolBlinking ? brLong : 255;
#endif
    }

    uint32_t phaseColon = now % 1000UL;
    uint8_t brColon;
    if (phaseColon < 250) brColon = 255;
    else if (phaseColon < 500) brColon = map(phaseColon - 250, 0, 250, 255, 0);
    else if (phaseColon < 750) brColon = 0;
    else brColon = map(phaseColon - 750, 0, 250, 0, 255);
    globalColonBrightness = brColon;
  }
  // -------------------------------------------

#if LED_PANEL
  if (currentMode != EFF_SD) {
    led_panel(textIsRunning);
  }
#endif

  // пропуск эффектов, когда бегущая строка в полноэкранном режиме
  if (runTextEnabled && textIsRunning && !runTextOver) {
    return;
  }
} // void Eff_Tick()

// ****************************************************************************************************************************************************

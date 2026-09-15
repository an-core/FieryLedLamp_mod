// *************************************************************************** Constants.h **************************************************************
#pragma once
// -----------------------
#include "ModuleConfig.h"
// -----------------------

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// ДЛЯ ESP32-S3: ОТКЛЮЧЕНИЕ И ВКЛЮЧЕНИЕ ПЛЕЕРА, ДИСПЛЕЯ (TM1637 / ST7789), ПУЛЬТА и КНОПКИ ОСУЩЕСТВЛЯЕТСЯ В ВЭБ-ИНТЕРФЕЙСЕ
// ДЛЯ ESP32: ПЛЕЕР, ДИСПЛЕЙ, ПУЛЬТ, КНОПКА, SD ОТКЛЮЧЕНЫ ПО УМОЛЧАНИЮ (ТАКЖЕ НЕ ЗАБУДЬТЕ УКАЗАТЬ #define USE_OTA 0 и Partition Scheme: No OTA (2MB APP / 2MB SPIFFS))
// ** если вы захотите для ESP32 включить, например, SD карту для .out эффектов, то измените константы в файле ModuleConfig.h
// *** поддержа использования нескольких последовательно соединённых одинаковых матриц включена по умолчанию
// -----------------------------------------------------------------------------------------------------------------------------------------------------

#define DISPLAY_IP_AT_START 1       // Показать при старте IP-адрес на матрице бегущей строкой: 0 - нет, 1 - да
// -----------------------------------------------------------------------------------------------------------------------------------------------------
#define FONT_SIZE 0                 // Шрифт текста бегущей строки (настраивается в WEB): 0 - маленький 5х8, 1 - средний 8х13, 2 - крупный 10х16
#define STATIC_FONT 0               // Шрифт для статики на матрице (часы, дата, погода): 0 - 3x5, 1 - 5x8, 2 - 8x13, 3 - 10x16
#define LED_CHIP 0                  // Тип чипа (настраивается в WEB): 0 - WS2812B (WS2811, SK6812), 1 - APA102 (SK9822)
#define WIDTH  (16)                 // Ширина одной матрицы (настраивается в WEB)
#define HEIGHT (16)                 // Высота одной матрицы (настраивается в WEB)
#define SEG_MATRIX_W 1              // Количество матриц по горизонтали (настраивается в WEB)
#define SEG_MATRIX_H 1              // Количество матриц по вертикали (настраивается в WEB)
#define COLOR_ORDER 0               // Порядок цветов светодиодов (настраивается в WEB): 0 - GRB, 1 - RGB, 2 - BRG, 3 - RBG, 4 - GBR, 5 - BGR
#define MATRIX_TYPE 0               // Тип матрицы (настраивается в WEB): 0 - Змейка, 1 - Параллельно
#define MATRIX_ORIENTATION 0        // Ориентация матрицы (настраивается в WEB): 0-7 (см. комментарий ниже)
// MATRIX_ORIENTATION:
// 0 - Левый Нижний-вправо        4 - Правый Верхний-влево
// 1 - Левый Нижний-вверх         5 - Правый Верхний-вниз
// 2 - Левый Верхний-вправо       6 - Правый Нижний-влево
// 3 - Левый Верхний-вниз         7 - Правый Нижний-вверх
// -----------------------------------------------------------------------------------------------------------------------------------------------------
#define USE_WEATHER 1               // Использовать получение данных погоды: 0 - нет, 1 - да
#define USE_DAWN 1                  // Использовать Будильник "Рассвет": 0 - нет, 1 - да
#define USE_SUNSET 1                // Использовать режим "Закат": 0 - нет, 1 - да
#define USE_SCHEDULE 1              // Использовать Расписание лампы: 0 - нет, 1 - да
#define USE_MULTILAMP 1             // Использовать режим "Мультилампа" (Синхронное управление несколькими лампами): 0 - нет, 1 - да
// -----------------------------------------------------------------------------------------------------------------------------------------------------
#define USE_MQTT 1                  // Использовать MQTT: 0 - нет, 1 - да (доп. настройки см. ниже)
// -----------------------------------------------------------------------------------------------------------------------------------------------------
#define USE_OTA 0                   // Использовать обновление прошивки по воздуху: 0 - нет, 1 - да
#define BACKUP_CFG_FILES 1          // Использовать резервное копирование настроек: 0 - нет, 1 - да
// -----------------------------------------------------------------------------------------------------------------------------------------------------
#define USE_ANIMATIONS 0            // Использовать встроенные анимационные эффекты: 0 - нет, 1 - да
// -----------------------------------------------------------------------------------------------------------------------------------------------------
#define STATUS_DEVICE 1             // Включить модальное окно Статусы устройств: 0 - нет, 1 - да
#define SOFT_INFO 1                 // Включить модальное окно Информация о ПО (в т.ч. информация о памяти): 0 - нет, 1 - да
// -----------------------------------------------------------------------------------------------------------------------------------------------------
#define DEBUG_ENABLED 0             // Использовать систему отладки: 0 - нет, 1 - да (доп. настройки см. ниже - стр.308-324)
// -----------------------------------------------------------------------------------------------------------------------------------------------------

// ============================================================== ПОДКЛЮЧЕНИЕ К ESP32 / ESP32-S3 =======================================================
#ifdef ESP32_USED
// -------------------------------------------------------------
#ifdef ESP32_S3_USED
// Пины ESP32-S3
#define LED_PIN (14U)   // DIN Пин матрицы 
#define CLK_PIN (13U)   // CLK (для чипа APA102)
#define BTN_PIN (7U)    // Пин кнопки   
#define MOSFET_PIN (6U)
#define MOSFET_LEVEL (HIGH)

#if USE_TM1637
  #define DIO (18U)
  #define CLK (21U)
#endif

#if USE_ST7789
  #define TFT_SCLK (12U)  // SCL (SCLK)
  #define TFT_MOSI (11U)  // SDA (MOSI)
  #define TFT_CS (10U)    // CS (Chip Select)
  #define TFT_DC (9U)     // DC (Data/Command)
  #define TFT_RST (5U)    // RES (reset)
  #define TFT_BL (4U)     // управление яркостью
#endif

#if USE_SD
  #define SD_CS_PIN (15U)
  #define SD_SCK_PIN (12U)
  #define SD_MOSI_PIN (11U)
  #define SD_MISO_PIN (2U)
#endif

#if USE_MP3_PLAYER
  #define MP3_TX_PIN (17U) // TX -> RX на плеере
  #define MP3_RX_PIN (16U) // RX -> TX на плеере
#endif

#if USE_IR_RECEIVER
  #define IR_RECEIVER_PIN (8U) // Пин ИК сенсора
#endif

#if USE_RF_RECEIVER
  #define RF_RECEIVER_PIN (19U) // Пин радиоприемника 433 МГц
#endif
// --------------------------------------------------------------
#else
// Пины ESP32
#define LED_PIN (13U)
#define CLK_PIN (12U)
#define BTN_PIN (35U)
#define MOSFET_PIN (33U)
#define MOSFET_LEVEL (HIGH)

#if USE_TM1637
  #define DIO (21U)
  #define CLK (22U)
#endif

#if USE_ST7789
  #define TFT_SCLK (18U)
  #define TFT_MOSI (23U)
  #define TFT_CS (17U)
  #define TFT_DC (32U)
  #define TFT_RST (4U)
  #define TFT_BL (27U)
#endif

#if USE_SD
  #define SD_CS_PIN (5U)
  #define SD_SCK_PIN (18U)
  #define SD_MISO_PIN (19U)
  #define SD_MOSI_PIN (23U)
#endif

#if USE_MP3_PLAYER
  #define MP3_TX_PIN (25U)
  #define MP3_RX_PIN (26U)
#endif

#if USE_IR_RECEIVER
  #define IR_RECEIVER_PIN (34U)
#endif

#if USE_RF_RECEIVER
  #define RF_RECEIVER_PIN (3U)
#endif

#endif // ESP32_S3_USED
#endif // ESP32_USED

// ----------------------------------------------------------------- ДОПОЛНИТЕЛЬНЫЕ НАСТРОЙКИ ----------------------------------------------------------
// Настройка кнопки
#if USE_BUTTON
  #define BUTTON_LOCK_ON_START 1 // Блокировка кнопки при старте
  #define BUTTON_STEP_TIMEOUT (100U)  // Таймаут удержания кнопки (мс)
  #define BUTTON_CLICK_TIMEOUT (500U) // Таймаут между нажатиями (мс)
  #define BUTTON_SET_DEBOUNCE_SENSORY (10U) // Время антидребезга (мс) для сенсорной кнопки
  #define BUTTON_SET_DEBOUNCE_MECHANICAL (60U) // Время антидребезга (мс) для механической кнопки
#endif
// --------------------------------------------------------------------------------------------------
// Настройка ИК пульта
#if USE_IR_RECEIVER
  #define USE_2_PULTS 0 // Использовать второй пульт: 0 - нет, 1 - да
  #define IR_REPEAT_TIMER 500 // Время ожидания повтора
  #define IR_TICK_TIMER 100 // Время между автоповтором
  #define IR_DIGIT_ENTER_TIMER 2000 // Время для ввода второй цифры номера эффекта
#endif
// --------------------------------------------------------------------------------------------------
// Настройка мп3 плеера
#if USE_MP3_PLAYER
 #ifdef DF_PLAYER_IS_ORIGINAL
  #define MP3_DELAY 500
 #else
  #define MP3_DELAY 100
#endif
  #define MP3_READ_TIMEOUT (800UL)
  #define MP3_CHECK_INTERVAL (60000UL)
  #define MP3_CHECK_TIMEOUT (300UL)
  #define MP3_CHECK_MAX_FAILS (3U)
  #define WEATHER_ADV_MIN_TEMP (-45)
  #define WEATHER_ADV_MAX_TEMP 45
  #define WEATHER_ADV_FIRST_FILE 200
  #define WEATHER_DESC_FIRST_FILE 300
  #define WEATHER_TEMP_TIMER ADVERT_TIMER_1
  #define WEATHER_DESC_TIMER ADVERT_TIMER_2
#endif // USE_MP3_PLAYER
// --------------------------------------------------------------------------------------------------
// Настройка дисплея ST7789
#if USE_ST7789
  #define TFT_BL_CH 7 // Канал подсветки
  #define TFT_BL_FREQ 5000 // Частота подсветки
  #define TFT_BL_BITS 8 // Разрядность
#endif
// --------------------------------------------------------------------------------------------------
// Настройка дисплея TM1637
#if USE_TM1637
  #define _empty 0x00
  #define _dash 0b01000000
  #define _deg 0b01100011
  #define _C 0b00111001
  #define _0 0b00111111
  #define _E 0b01111001
  #define _F 0b01110001
  #define WEATHER_ERR_TIME 60000UL
#endif
// --------------------------------------------------------------------------------------------------
#if USE_SCHEDULE
  #define MAX_SCHEDULE_ENTRIES 10
#endif
// --------------------------------------------------------------------------------------------------
#define CUSTOM_NTP_INDEX 2
#define DATETIME_STRLEN 20
#define TM_BASE_YEAR 1900
#define _GNU_SOURCE
#define myTime Time::instance()
#ifndef TZONE
  #define TZONE "GMT0"
#endif
// --------------------------------------------------------------------------------------------------
// Mqtt
#if USE_MQTT
  #define MQTT_RECONNECT_TIME (10U) // Время в секундах перед подключением к брокеру MQTT
#endif
// --------------------------------------------------------------------------------------------------
// Blynk
#define USE_BLYNK 0 // Использовать BLYNK: 0 - нет, 1 - да
#if USE_BLYNK
  #define CYCLE_LAST_EFFECT (MODE_AMOUNT - 1)
  #define BLYNK_AUTH_TOKEN "token" // Вставьте токен
  #define CYCLE_DONT_OFF (1U) // Не отключать режим Цикл при выключении лампы = 1U, отключать = 0U
  #define CYCLE_TIMER (60U) // Интервал смены эффектов
  #define CYCLE_TIMER_PLUS (0U) // + случайное время от нуля до 0U секунд
  #define CYCLE_1ST_EFFECT (0U) // С какого эффекта будет начинаться демонстрирация
#endif
// --------------------------------------------------------------------------------------------------
// Переключение избранных эффектов
#define DEFAULT_FAVORITES_INTERVAL (300U) // значение по умолчанию для интервала переключения избранных эффектов в секундах
#define DEFAULT_FAVORITES_DISPERSION (0U) // значение по умолчанию для разброса интервала переключения избранных эффектов в секундах
// --------------------------------------------------------------------------------------------------
// Настройки текста
#define TEXT_DIRECTION (1U) // 1 - по горизонтали, 0 - по вертикали
#define MIRR_V (0U) // отразить текст по вертикали
#define MIRR_H (0U) // отразить текст по горизонтали
#define SPACE (1U) // пробел между символами
#define LETTER_COLOR (CRGB::Blue)
#define FONT_SMALL 0 // Маленький шрифт (5x8)
#define FONT_MEDIUM 1 // Средний шрифт (8x13)
#define FONT_LARGE 2 // Крупный шрифт (10x16)
// --------------------------------------------------------------------------------------------------
#define ESP_HTTP_PORT (80U) // Для WiFi клиента
#define ESP_UDP_PORT (8888U) // Для UDP сервера
#if USE_OTA
  #define ESP_OTA_PORT (8266U) // Для обновления по воздуху
  #define ESP_CONF_TIMEOUT 420 // Таймаут OTA режима (7 минут)
#endif
#define USE_DEFAULT_SETTINGS_RESET 1 // Сброс настроек эффектов: 0 - откл, 1 - вкл
#define HEAP_SIZE_PRINT 0 // Вывод размера свободного ОЗУ: 0 - откл, 1 - вкл
#define MAIN_CYCLES_PER_SECOND 0 // Вывод FPS: 0 - откл, 1 - вкл
#define USE_LittleFS // Закомментируйте, если используете SPIFFS
#define TIMER_DISABLED 999999999UL
// --------------------------------------------------------------------------------------------------
#define SAVE_CONFIG_BIT 0x01 // Отложенное сохранение config.json
#define SAVE_ALARMS_BIT 0x02 // Отложенное сохранение config_alarm.json и config_sunset.json
#define SAVE_CYCLE_BIT 0x04 // Отложенное сохранение cycle.json
#define SAVE_MULTILAMP_BIT 0x08 // Отложенное сохранение config_multilamp.json
#define SAVE_SCHEDULE_BIT 0x10 // Отложенное сохранение config_schedule.json
// --------------------------------------------------------------------------------------------------
#define countof(a) (sizeof(a) / sizeof(a[0]))
#define USE_RANDOM_SETS_IN_APP // Совместимость с приложением FireLamp
// --------------------------------------------------------------------------------------------------
#define FASTLED_INTERRUPT_RETRY_COUNT (0) // Количество попыток повторной передачи кадра
// #define FASTLED_ESP32_RMT 0 // отключает RMT на ESP32 и S3
#define FASTLED_ESP32_RAW_PIN_ORDER
// --------------------------------------------------------------------------------------------------
#define LED_PANEL 1             // Включить опцию: 0 - нет, 1 - да (все настройки матрицы находятся тут)
#define MAX_MATRIX_WIDTH (128)  // Максимальная ширина матрицы
#define MAX_MATRIX_HEIGHT (128) // Максимальная высота матрицы
#define MAX_LEDS (MAX_MATRIX_WIDTH * MAX_MATRIX_HEIGHT)
#define MULTI_MATRIX 1          // Поддержка использования нескольких последовательно соединённых матриц: 0 - одна матрица, 1 - несколько матриц
#define SEGMENTS (1U)           // Сколько диодов в одном "пикселе" (для матрицы из кусков ленты)
#define BRIGHTNESS (40U)        // Стандартная максимальная яркость
#define CURRENT_LIMIT (4000U)   // Лимит тока (мА)
// --------------------------------------------------------------------------------------------------
#define DYNAMIC (0U)      // Динамическая задержка (используется бегунок Скорость)
#define SOFT_DELAY (1U)   // Задержка задается программно в теле эффекта
#define LOW_DELAY (15U)   // Низкая фиксированная задержка
#define HIGH_DELAY (50U)  // Высокая фиксированная задержка
#define DYNAMIC_DELAY_TICK  if (millis() - effTimer >= (256U - modes[currentMode].Speed))
#define HIGH_DELAY_TICK     if (millis() - effTimer >= 50)
#define LOW_DELAY_TICK      if (millis() - effTimer >= 15)
#define MICRO_DELAY_TICK    if (millis() - effTimer >= 2)
#define SOFT_DELAY_TICK     if (millis() - effTimer >= FPSdelay)
// --------------------------------------------------------------------------------------------------
#define SQRT_VARIANT sqrt3                   // выбор основной функции для вычисления квадратного корня sqrtf или sqrt3 для ускорения
#define trackingOBJECT_MAX_COUNT (100U)      // максимальное количество отслеживаемых объектов (очень влияет на расход памяти)
#define enlargedOBJECT_MAX_COUNT (WIDTH * 2) // максимальное количество сложных отслеживаемых объектов
#define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
#ifndef NUM_LAYERSMAX
  #define NUM_LAYERSMAX 2
#endif
// --------------------------------------------------------------------------------------------------
#if USE_ANIMATIONS
  #define MAX_FRAMES_COUNT 20
  #define MAX_IMAGE_WIDTH  16
  #define MAX_IMAGE_HEIGHT 16
  #define IMAGE_LIST "Сердце,Марио"
#endif
// --------------------------------------------------------------------------------------------------
#define MAX_UDP_BUFFER_SIZE (MODE_AMOUNT * 2 + 20) // Максимальный размер буфера
// --------------------------------------------------------------------------------------------------
#define EEPROM_PASSWORD_START_ADDRESS (0U)         // адрес для пароля к роутеру (НЕ ИСПОЛЬЗУЕТСЯ в текущей версии)
#define EEPROM_FIRST_RUN_ADDRESS (0x40U)           // адрес для признака первого запуска (64 байт)
#define EEPROM_MODES_START_ADDRESS (0x50U)         // адрес для настроек эффектов (80 байт)
#define EEPROM_MODE_STRUCT_SIZE (3U)               // размер структуры ModeType: яркость, скорость, масштаб (3 байта)
#define EEPROM_WIFI_BACKUP_START_ADDRESS (EEPROM_MODES_START_ADDRESS + MODE_AMOUNT * EEPROM_MODE_STRUCT_SIZE + 1) // Адрес для бэкапа WiFi
#define EEPROM_WIFI_BACKUP_SIZE (512U)             // размер области для бэкапа WiFi (512 байт)
#define EEPROM_UPDATE_DATE_ADDR (EEPROM_WIFI_BACKUP_START_ADDRESS + EEPROM_WIFI_BACKUP_SIZE) // адрес для даты обновления (после бэкапа WiFi)
#define EEPROM_UPDATE_DATE_SIZE 11                 // размер для даты (11 байт)
#define EEPROM_TOTAL_BYTES_USED (EEPROM_UPDATE_DATE_ADDR + EEPROM_UPDATE_DATE_SIZE)
#define EEPROM_FIRST_RUN_MARK (MODE_AMOUNT - 255)  // метка первого запуска
#define EEPROM_WRITE_DELAY (30000UL)               // задержка перед записью в EEPROM (30 секунд)
#define SAVE_FILE_DELAY_TIMEOUT (15000UL)          // задержка перед сохранением в файл (15 секунд)
// ------------------
#if BACKUP_CFG_FILES
  #define EEPROM_WIFI_BACKUP_START_ADDRESS 512
  #define EEPROM_WIFI_BACKUP_SIZE 256
  #define EEPROM_WIFI_BACKUP_MAGIC 0x57494649UL
  #define EEPROM_WIFI_BACKUP_VERSION 1
  #define EEPROM_WIFI_BACKUP_PENDING_MARK 0xAB
#endif
// --------------------------------------------------------------------------------------------------
// Интервалы задач (в loop)
const uint32_t MEDIUM_TASK_INTERVAL = 250;
const uint32_t SLOW_TASK_INTERVAL = 1000;
const uint32_t WIFI_TASK_INTERVAL = 200;
// --------------------------------------------------------------------------------------------------
#if USE_WEATHER
const uint32_t WEATHER_UPDATE_INTERVAL = 600000UL; // Интервал обновления погоды (10 минут)
#endif
// --------------------------------------------------------------------------------------------------
// Пароль для Редактора
#define FILEMANAGER_USERNAME "ledlamp"     // можно оставить, либо изменить
#define FILEMANAGER_PASSWORD "12345"       // лучше изменить на другой
// --------------------------------------------------------------------------------------------------
// Система отладки (включите логи, которые нужно посмотреть на WEB странице или в мониторе порта)
#if DEBUG_ENABLED
  #define SYSLOG SystemLog::instance()
  #define WIFI_LOG 0        // WiFi подключение
  #define MATRIX_LOG 0      // Матрица (буферы)
  #define EFF_LOG 1
  #define MQTT_LOG 0        // MQTT
  #define MP3_LOG 0         // MP3 плеер
  #define BUTTON_LOG 0      // Кнопка
  #define IR_LOG 0          // Пульт IR
  #define RF_LOG 0          // Пульт RF (433Mhz)
  #define WEATHER_LOG 0     // Погода
  #define SCHEDULE_LOG 0    // Расписание
  #define ST7789_LOG 0      // Дисплей ST7789
  #define TM1637_LOG 0      // Дисплей TM1637
  #define SD_LOG 1          // SD карта
  #define MEMORY_LOG 0      // Память
  #define TELNET_LOG 0      // Telnet
  #define UPDATE_LOG 0      // Обновление прошивки
  #define GENERAL_LOG 0     // Общие логи
#endif

// =====================================================================================================================================================
// Вызов эффектов осуществляется в файле effetTicker
#define EFF_ANIMATION           (  0U)    // Анимация
#define EFF_AVRORA              (  1U)    // Аврора
#define EFF_WATERCOLOR          (  2U)    // Акварель
#define EFF_FLOWERRUTA          (  3U)    // Аленький цветочек
#define EFF_BUTTERFLY           (  4U)    // Бабочка
#define EFF_EFF_POOL            (  5U)    // Бассейн
#define EFF_BAMBOO              (  6U)    // Бамбук
#define EFF_MADNESS             (  7U)    // Безумие
#define EFF_CUBEROUTINE         (  8U)    // Блуждающий кубик
#define EFF_WATERFALL           (  9U)    // Водопад
#define EFF_WATERFALL_4IN1      ( 10U)    // Водопад 4в1
#define EFF_WAVES               ( 11U)    // Волны
#define EFF_MAGICLANTERN        ( 12U)    // Волшебный Фонарик
#define EFF_WINE                ( 13U)    // Вино
#define EFF_WHIRL               ( 14U)    // Вихри пламени
#define EFF_WHIRL_MULTI         ( 15U)    // Вихри разноцветные
#define EFF_STARFALL            ( 16U)    // Вьюга
#define EFF_STORMY_RAIN         ( 17U)    // Гроза в банке
#define EFF_DNA                 ( 18U)    // ДНК
#define EFF_SMOKE               ( 19U)    // Дым
#define EFF_SMOKE_COLOR         ( 20U)    // Дым разноцветный
#define EFF_SMOKEBALLS          ( 21U)    // Дымовые шашки
#define EFF_LIQUIDLAMP          ( 22U)    // Жидкая лампа
#define EFF_LIQUIDLAMP_AUTO     ( 23U)    // Жидкая лампа авто
#define EFF_SWIRL               ( 24U)    // Завиток
#define EFF_STARS               ( 25U)    // Звезды
#define EFF_ZEBRA               ( 26U)    // Зебра
#define EFF_TIXYLAND            ( 27U)    // Земля Тикси
#define EFF_SNAKE_GAME          ( 28U)    // Змейка (игра)
#define EFF_SNAKES              ( 29U)    // Змейки
#define EFF_FOUNTAIN            ( 30U)    // Источник
#define EFF_DROP_IN_WATER       ( 31U)    // Капли на воде
#define EFF_DROPS               ( 32U)    // Капли на стекле
#define EFF_LLAND               ( 33U)    // Кипение
#define EFF_RINGS               ( 34U)    // Кодовый замок
#define EFF_COMET               ( 35U)    // Комета
#define EFF_COMET_COLOR         ( 36U)    // Комета одноцветная
#define EFF_COMET_TWO           ( 37U)    // Комета двойная
#define EFF_COMET_THREE         ( 38U)    // Комета тройная
#define EFF_CONTACTS            ( 39U)    // Контакты
#define EFF_SPARKLES            ( 40U)    // Конфетти
#define EFF_CUBERUBIKA          ( 41U)    // Кубик Рубика
#define EFF_LAVA                ( 42U)    // Лава
#define EFF_LAVALAMP            ( 43U)    // Лавовая лампа
#define EFF_BUTTERFLYS_LAMP     ( 44U)    // Лампа с мотыльками
#define EFF_FOREST              ( 45U)    // Лес
#define EFF_LUMENJER            ( 46U)    // Люмeньep
#define EFF_MAGMA               ( 47U)    // Магма
#define EFF_MARIO               ( 48U)    // Марио (игра)
#define EFF_PAINTS              ( 49U)    // Масляные краски
#define EFF_MATRIX              ( 50U)    // Матрица
#define EFF_TWINKLES            ( 51U)    // Мерцание
#define EFF_METEOR              ( 52U)    // Метеор
#define EFF_METABALLS           ( 53U)    // Метоболз
#define EFF_WEB_TOOLS           ( 54U)    // Мечта дизайнера
#define EFF_MOSAIC              ( 55U)    // Мозайка
#define EFF_BUTTERFLYS          ( 56U)    // Moтыльки
#define EFF_BBALLS              ( 57U)    // Мячики
#define EFF_BALLS_BOUNCE        ( 58U)    // Мячики без границ
#define EFF_CHRISTMAS_TREE      ( 59U)    // Новогодняя Елка
#define EFF_NEW_STARS           ( 60U)    // Новые Звёзды
#define EFF_NIGHTCITY           ( 61U)    // Ночной Город
#define EFF_FIRE                ( 62U)    // Огонь
#define EFF_FIRE_2012           ( 63U)    // Огонь 2012
#define EFF_FIRE_2018           ( 64U)    // Огонь 2018
#define EFF_FIRE_2020           ( 65U)    // Огонь 2020
#define EFF_FIRE_2021           ( 66U)    // Огонь 2021
#define EFF_FIREFLY_TOP         ( 67U)    // Огoнь верховой
#define EFF_FIREFLY             ( 68U)    // Огoнь парящий
#define EFF_FIRESPARKS          ( 69U)    // Огонь с искрами
#define EFF_COLOR_RAIN          ( 70U)    // Осадки
#define EFF_OSCILLATING         ( 71U)    // Осциллятор
#define EFF_CLOUDS              ( 72U)    // Облака
#define EFF_OCEAN               ( 73U)    // Океан
#define EFF_OCTOPUS             ( 74U)    // Осьминог
#define EFF_RAINBOW_STRIPE      ( 75U)    // Павлин
#define EFF_HOURGLASS           ( 76U)    // Песочные часы
#define EFF_PAINTBALL           ( 77U)    // Пейнтбол
#define EFF_PICASSO             ( 78U)    // Пикассо
#define EFF_PLASMA              ( 79U)    // Плазма
#define EFF_SPIDER              ( 80U)    // Плазменная лампа
#define EFF_PLASMA_WAVES        ( 81U)    // Плазменные волны
#define EFF_FLAME               ( 82U)    // Пламя
#define EFF_PLANETEARTH         ( 83U)    // Планета Земля
#define EFF_BY_EFFECT           ( 84U)    // Побочный эффект
#define EFF_POPCORN             ( 85U)    // Попкорн
#define EFF_PRISMATA            ( 86U)    // Призмата
#define EFF_ATTRACT             ( 87U)    // Притяжение
#define EFF_LEAPERS             ( 88U)    // Пpыгyны
#define EFF_PULSE               ( 89U)    // Пульс
#define EFF_PULSE_WHITE         ( 90U)    // Пульс белый
#define EFF_PULSE_RAINBOW       ( 91U)    // Пульс радужный
#define EFF_RADIAL_WAWE         ( 92U)    // Радиальная волна
#define EFF_RAINBOW_VER         ( 93U)    // Радуга
#define EFF_RAINBOW             ( 94U)    // Радуга 3D
#define EFF_RAINBOW_SPOT        ( 95U)    // Радужное Пятно
#define EFF_RAINBOW_RINGS       ( 96U)    // Радужные кольца
#define EFF_SNAKE               ( 97U)    // Радужный змей
#define EFF_RAIN                ( 98U)    // Разноцветный дождь
#define EFF_DANDELIONS          ( 99U)    // Разноцветные одуванчики
#define EFF_RIVERS              (100U)    // Реки Ботсваны
#define EFF_LIGHTERS            (101U)    // Светлячки
#define EFF_LIGHTER_TRACES      (102U)    // Светлячки со шлейфом
#define EFF_FEATHER_CANDLE      (103U)    // Свеча
#define EFF_AURORA              (104U)    // Северное сияние
#define EFF_SERPENTINE          (105U)    // Серпантин
#define EFF_SCANNER             (106U)    // Сканер
#define EFF_SINUSOID3           (107U)    // Синусоид
#define EFF_COLORS              (108U)    // Смена цвета
#define EFF_SNOW                (109U)    // Снегопад
#define EFF_SPECTRUM            (110U)    // Спектрум
#define EFF_SPIRO               (111U)    // Спирали
#define EFF_FLOCK               (112U)    // Стая
#define EFF_FLOCK_N_PR          (113U)    // Стая и хищник
#define EFF_ARROWS              (114U)    // Стрелки
#define EFF_STROBE              (115U)    // Строб.Хаос.Дифузия
#define EFF_SHADOWS             (116U)    // Тени
#define EFF_PACIFIC             (117U)    // Тихий океан
#define EFF_TORNADO             (118U)    // Торнадо
#define EFF_SIMPLE_RAIN         (119U)    // Tyчкa в банке
#define EFF_FIREWORK            (120U)    // Фейерверк
#define EFF_FIREWORK_2          (121U)    // Фейерверк 2
#define EFF_FAIRY               (122U)    // Фея
#define EFF_TRICOLOR            (123U)    // Флаг (Триколор)
#define EFF_FONTAN              (124U)    // Фонтан
#define EFF_COLOR               (125U)    // Цвет
#define EFF_COLORED_PYTHON      (126U)    // Цветной Питон
#define EFF_SAND                (127U)    // Цветные драже
#define EFF_COLOR_FRIZZLES      (128U)    // Цветные кудри
#define EFF_LOTUS               (129U)    // Цветок лотоса
#define EFF_TURBULENCE          (130U)    // Цифровая турбулентность
#define EFF_SPHERES             (131U)    // Шapы
#define EFF_NEXUS               (132U)    // Nexus
#define EFF_COLOR_FADED         (133U)    // Цвет с затуханием  
#define EFF_CLOCK               (134U)    // Часы
#define EFF_SD                  (135U)    // эффекты .out
#define MODE_AMOUNT             (136U)    // Количество эффектов

// --------------------------------------------------------------------------------------------------
// Массив настроек эффектов по умолчанию
// {Яркость, Скорость, Масштаб}
static const uint8_t defaultSettings[][3] PROGMEM = {
  {  50, 128, 80 }, // Анимация
  {  35,  90,  50}, // Аврора
  {  25, 200,  65}, // Акварель
  {  20, 215,  60}, // Аленький цветочек
  {  20,  11,   3}, // Бабочка
  {  25, 185,  63}, // Бассейн
  {  20, 215,  49}, // Бамбук
  {  35,  20,  60}, // Безумие
  {  20, 150,  50}, // Блуждающий кубик
  {  30, 212,  54}, // Водопад
  {  20, 195,  22}, // Водопад 4в1
  {  40, 233,  80}, // Волны
  {  45, 175,  60}, // Волшебный Фонарик
  {  80, 205,  40}, // Вино
  {  25, 210,   1}, // Вихри пламени
  {  20, 210,  86}, // Вихри разноцветные
  {  55, 191,  54}, // Вьюга
  {  40, 210,   8}, // Гроза в банке
  {  30,  80,  95}, // ДНК
  {  25, 195, 100}, // Дым
  {  25, 190,  30}, // Дым разноцветный
  {  30, 170,  25}, // Дымовые шашки
  {  20, 110,   1}, // Жидкая лампа
  {  20, 124,  39}, // Жидкая лампа авто
  {  30, 195,  70}, // Завиток
  {  25, 215,  99}, // Звезды
  {  15,   8,  21}, // Зебра
  {  20, 212,  76}, // Земля Тикси
  {  14, 190,  30}, // Змейка (игра)
  {  20,  40,  15}, // Змейки
  {  25, 233, 100}, // Источник
  {  20, 200,  55}, // Капли на воде
  {  20,  40,  59}, // Капли на стекле
  {  15, 240,  18}, // Кипение
  {  30, 205,  91}, // Кодовый замок
  {  20, 205,  28}, // Комета
  {  20, 212,  69}, // Комета одноцветная
  {  25, 186,  19}, // Комета двойная
  {  25, 186,   9}, // Комета тройная
  {  25, 200,  60}, // Контакты
  {  30, 142,  63}, // Конфетти
  {  45, 222,  92}, // Кубик Рубика
  {  15,   9,  24}, // Лава
  {  15, 240,   1}, // Лавовая лампа
  {  30,  61,  20}, // Лампа с мотыльками
  {  15,  15,  95}, // Лес
  {  20, 200,  40}, // Люмeньep
  {  16, 205,  13}, // Магма
  {  14, 150,  50}, // Марио (игра)
  {  15, 195,  50}, // Масляные краски
  {  25, 186,  23}, // Матрица
  {  25, 235,   4}, // Мерцание
  {  20, 150,  50}, // Метеор
  {  15,  72,   3}, // Метоболз
  {  28,  70,  20}, // Мечта дизайнера
  {  15, 205,  50}, // Мозайка
  {  20,  61,  20}, // Moтыльки
  {  15, 255,  26}, // Мячики
  {  25, 255,  85}, // Мячики без границ
  {  30, 165,  30}, // Новогодняя Ёлка
  {  25,  18,  59}, // Новые Звёзды
  {  35,  50,  25}, // Ночной Город
  {  50, 225,   1}, // Огонь
  {  15, 220,  63}, // Огонь 2012
  {  100, 199, 11}, // Огонь 2018
  {  20, 225,  11}, // Огонь 2020
  {  25, 150,  22}, // Огонь 2021
  {  26, 190,  15}, // Огoнь верховой
  {  30, 200,  15}, // Огoнь парящий
  {  30,  80,  64}, // Огонь с искрами
  {  20, 205,  49}, // Осадки
  {  15, 208,  42}, // Осциллятор
  {  20,  15,  34}, // Облака
  {  20,   8,  12}, // Океан
  {  15, 200,  51}, // Осьминог
  {  20,   5,  12}, // Павлин
  {  20, 150,   1}, // Песочные часы
  {  25, 243, 100}, // Пейнтбол
  {  15, 220,  40}, // Пикассо
  {  10,  20,  35}, // Плазма
  {  10,  30,  82}, // Плазменная лампа
  {  15,  15,  50}, // Плазменные волны
  {  30,  45,   3}, // Пламя
  {  25, 128,  75}, // Планета Земля
  {  40, 165,  30}, // Побочный эффект
  {  10,  70,  16}, // Попкорн
  {  15, 100,  88}, // Призмата
  {  20, 205,  65}, // Притяжение
  {  25, 203,   5}, // Пpыгyны
  {  20, 241,   6}, // Пульс
  {  20, 241,  11}, // Пульс белый
  {  20, 244,  31}, // Пульс радужный
  {  15, 220,  50}, // Радиальная волна
  {  50, 215,  50}, // Радуга
  {  50,  13,  60}, // Радуга 3D
  {  40, 245,  40}, // Радужное Пятно
  {  39, 250,  79}, // Радужные кольца
  {  15, 205, 100}, // Радужный змей
  {  15, 205,   1}, // Разноцветный дождь
  {  20,  50,  90}, // Разноцветные одуванчики
  {  12, 175,  50}, // Реки Ботсваны
  {  15, 180,  23}, // Светлячки
  {  15, 185,  93}, // Светлячки со шлейфом
  {  20, 220,   8}, // Свеча
  {  15, 160,  64}, // Северное сияние
  {  15,  75,  50}, // Серпантин
  {  50, 230,  40}, // Сканер
  {  20, 127,  75}, // Синусоид
  {  25, 240,   1}, // Смена цвета
  {  10, 205,  90}, // Снегопад
  {  25, 175, 100}, // Спектрум
  {  15,  45,   3}, // Спирали
  {  15, 136,   4}, // Стая
  {  15, 128,  80}, // Стая и хищник
  {  80, 165,  40}, // Стрелки
  {  25, 104,  20}, // Строб.Хаос.Дифузия
  {  50, 160,  33}, // Тени
  {  20, 127, 100}, // Тихий океан
  {  15, 127,  50}, // Торнадо
  {  50, 210,   2}, // Tyчкa в банке
  {  25, 180,  70}, // Фейерверк
  {  15, 240,  75}, // Фейерверк 2
  {  20, 212,  85}, // Фея
  {  20, 100,   1}, // Флаг (Триколор)
  {  40, 250,  75}, // Фонтан
  {  20, 240,  65}, // Цвет
  {  20, 127,  92}, // Цветной Питон
  {  15, 195,  80}, // Цветные драже
  {  25, 128,  60}, // Цветные кудри
  {  15, 150,  45}, // Цветок лотоса
  {  15, 215,  35}, // Цифровая турбулентность
  {  20,  50,   5}, // Шapы
  {  25,  85,  20}, // Nexus
  {   2,   1,   3}, // Цвет с затуханием
  {  30, 130,  80}, // Часы
  { 105, 128,  50}  // Эффекты с SD
};

// --------------------------------------------------------------------------------------------------
// Массив папок MP3 для эффектов
static const uint8_t default_effects_folders[MODE_AMOUNT] PROGMEM = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
  51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75,
  76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100,
  101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120,
  121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135
};

// ******************************************************************************************************************************************************

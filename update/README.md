| =========================== ESP32-WROOM ========================= |
| LED_PIN (13U), CLK_PIN (12U)                                      |
|---------------------------------|---------------------------------|
| ВАРИАНТ_1:                      | ВАРИАНТ_2:                      |
|_________________________________|_________________________________|
| #define USE_DAWN 1              | #define USE_DAWN 1              |
| #define USE_SUNSET 1            | #define USE_SUNSET 1            |
| #define USE_SCHEDULE 1          | #define USE_SCHEDULE 1          |
| #define USE_WEATHER 1           | #define USE_WEATHER 1           |
| #define MULTI_MATRIX 1          | #define MULTI_MATRIX 1          |
| #define USE_MULTILAMP 1         | #define LED_PANEL 1             |
| #define LED_PANEL 1             | #define DISPLAY_IP_AT_START 1   |
| #define DISPLAY_IP_AT_START 1   |                                 |
| #define USE_MQTT 1              |                                 |
| #define BACKUP_CFG_FILES 1      |                                 |
| #define SOFT_INFO 1             |                                 |
| #define STATUS_DEVICE 1         |                                 |
|---------------------------------|---------------------------------|
| FieryLedLamp.ino.bootloader.bin - 0x1000                          |
| FieryLedLamp.ino.partitions.bin - 0x8000                          |
| FieryLedLamp.ino.bin - 0x10000                                    |
| FieryLedLamp.littlefs.bin - 0x210000                              |
*********************************************************************

| =============================== ESP32-S3 ============================= |
| LED_PIN (14U), CLK_PIN (13U), BTN_PIN (7U), MOSFET_PIN (6U)            |
| IR_RECEIVER (8U), RF_RECEIVER (19U)                                    |
| TM1637: DIO (18U), CLK (21U)                                           |
| ST7789: SCLK (12U), MOSI (11U), CS (10U), DC (13U), RST (5U), BL (4U)  |
| MP3_PLAYER: MP3_TX_PIN (17U), MP3_RX_PIN (16U)                         |
|------------------------------------------------------------------------|
| #define MULTI_MATRIX 1         | #define USE_BUTTON 1                  |
| #define USE_OTA 1              | #define USE_ST7789 1                  |
| #define BACKUP_CFG_FILES 1     | #define USE_TM1637 1                  |
| #define SOFT_INFO 1            | #define USE_IR_RECEIVER 1             |
| #define STATUS_DEVICE 1        | #define USE_RF_RECEIVER 1             |
| #define DISPLAY_IP_AT_START 1  | #define USE_MP3_PLAYER 1              |
| #define USE_MULTILAMP 1        | * эти модули отключаемые через web    |
| #define USE_WEATHER 1          |---------------------------------------|
| #define USE_SCHEDULE 1         |                                       |
| #define USE_DAWN 1             |                                       |
| #define USE_SCHEDULE 1         |                                       |
|--------------------------------|---------------------------------------|
| FieryLedLamp.ino.bootloader.bin - 0x0                                  |
| FieryLedLamp.ino.partitions.bin - 0x8000                               |
| FieryLedLamp.ino.bin - 0x10000                                         |
| FieryLedLamp.littlefs.bin - 0xc90000                                   |
**************************************************************************

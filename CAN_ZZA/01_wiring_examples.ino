/*
  Arduino Nano:
                  +---+                +-----+
              +---|PWR|----------------| USB |---+
              |   +---+                +-----+   |
              |                                  |
              |                          AREF[ ] |
              |                           GND[ ] |
              |               ___          13[ ] | MCP2515 SCK
              | [ ]IOREF     /   \         12[ ] | MCP2515 MISO
  MCP2515 RST | [ ]RESET    /  U  \       ~11[ ] | MCP2515 MOSI
              | [ ]3.3V     |  N  |       ~10[ ] | MCP2515 CS
              | [ ]5V       \  O  /        ~9[ ] |
              | [ ]GND       \___/          8[ ]~|
              | [ ]GND                           |
              | [ ]Vin                      7[ ] |
              | [ ]                        ~6[ ]~|
        RANDR | [ ]A0                      ~5[ ] |
   OLED_CLK   | [ ]A1      I C S P          4[ ] | OLED_2_CS
   OLED_MOSI  | [ ]A2    RST SCK MISO INT1/~3[ ] | OLED_1_CS
   OLED_RESET | [ ]A3    [ ] [ ] [ ]* INT0/ 2[ ] | DCC Optocopler / or MCP2515 INT
   OLED_DC    | [ ]A4    [ ] [ ] [ ]    TX->1[ ] |
   OLED_0_CS  | [ ]A5    GND MOSI 5V    RX<>0[ ] |
              |                                  |
              +----------------------------------+

  Arduino MEGA:
                  +---+                            +-----+
              +---|PWR|----------------------------| USB |---+
              |   +---+                            +-----+   |
              |                                      AREF[ ] |
              |                                       GND[ ] |
              |                    ___                 13[ ] |
              | [ ]IOREF          /   \                12[ ] |
  MCP2515 RST | [ ]RESET         /  M  \               11[ ] |
              | [ ]3.3V          |  E  |               10[ ] |
              | [ ]5V            |  G  |                9[ ] |
              | [ ]GND           \  G  /                8[ ] |
              | [ ]GND            \___/                      |
              | [ ]Vin                                  7[ ] |
              | [ ]                                     6[ ] |
        RANDR | [ ]A0                                   5[ ] |
   OLED_CLK   | [ ]A1           I C S P                 4[ ] |
   OLED_MOSI  | [ ]A2         RST SCK MISO        INT5/ 3[ ] |
   OLED_RESET | [ ]A3         [ ] [ ] [ ]*        INT4/ 2[ ] | DCC Optocopler / or MCP2515 INT
   OLED_DC    | [ ]A4         [ ] [ ] [ ]           TX->1[ ] |
   OLED_0_CS  | [ ]A5         GND MOSI 5V           RX<>0[ ] |
   OLED_1_CS  | [ ]A6                                        |
   OLED_2_CS  | [ ]A7                              TX3/14[ ] |
              |                                    RX3/15[ ] |
   OLED_3_CS  | [ ]A8                              TX2/16[ ] |
   OLED_4_CS  | [ ]A9                              RX2/17[ ] |
   OLED_5_CS  | [ ]A10                             TX1/18[ ] |
   OLED_6_CS  | [ ]A11                             RX1/19[ ] |
              | [ ]A12   +--- 52 | SCK             SDA/20[ ] |
              | [ ]A13 G | +- 50 | MISO            SCL/21[ ] |
              | [ ]A14 N | |                              5  |
              | [ ]A15 D | |                              V  |
              |        [][][][][][][][][][][][][][][][][][]  |
              |        [][][][][][][][][][][][][][][][][][]  |
              |        G | |                              5  |
              |        N | |                              V  |
              |        D | +- 51 | MOSI                      |
              |          +--- 53 | CS                        |
              +----------------------------------------------+

  Teensy 3.5:
  USE HCPL-260L-000E instead of 6N137
                           +-----+
              +------------| USB |------------+
              | [ ]GND     +-----+     VIn[ ] |
              | [ ]0                  AGND[ ] |
              | [ ]1                  3.3V[ ] |
              | [ ]2         ___     A9/23[ ] |
              | [ ]3        /   \    A8/22[ ] |
              | [ ]4       /  T  \   A7/21[ ] |
              | [ ]5       |  E  |   A6/20[ ] |
   DCC Opto   | [ ]6       |  E  |   A5/19[ ] |
              | [ ]7       |  N  |   A4/18[ ] |
   OLED_RESET | [ ]8       |  S  |   A3/17[ ] |
   OLED_DC    | [ ]9       \  Y  /   A2/16[ ] |
   OLED_0_CS  | [ ]10/CS    \___/    A1/15[ ] | OLED_2_CS + OLED_3_CS
   OLED_MOSI  | [ ]11/MOSI0          A0/14[ ] | OLED_1_CS + OLED_4_CS
              | [ ]12/MISO0        SCK0/13[ ] | OLED_CLK
   OLED_VCC   | [ ]3.3V                GND[ ] | OLED_GND
              | [ ]24             A22/DAC1[ ] |
              | [ ]25             A21/DAC0[ ] |
              | [ ]26               A20/39[ ] |
              | [ ]27               A19/38[ ] |
              | [ ]28               A18/37[ ] |
              | [ ]29/CAN0TX        A17/36[ ] |
              | [ ]30/CAN0RX        A16/35[ ] |
              | [ ]31/A12           A15/34[ ] |
              | [ ]32/A13           A14/33[ ] |
              +-------------------------------+

  LOLIN(WEMOS) D1 R2 & mini:
  IFDEF ARDUINO_ESP8266_WEMOS_D1MINI
  Boards: http://arduino.esp8266.com/stable/package_esp8266com_index.json
  USE HCPL-260L-000E instead of 6N137
                              +-----+
                    +---------| USB |---------+
                    |         +-----+         |
                    |                         |
                    | [ ]5V            3V3[ ] | OLED_VCC
           OLED_GND | [ ]GND            D8[ ] | OLED_RESET
  SD_CS & OLED_3_CS | [ ]D4/LED         D7[ ] | OLED_MOSI & SD_MOSI
          OLED_2_CS | [ ]D3             D6[ ] | OLED_MISO & SD_MISO
          OLED_1_CS | [ ]D2             D5[ ] | OLED_CLK  & SD_CLK
          OLED_0_CS | [ ]D1             D0[ ] | OLED_DC
          OLED_4_CS | [ ]RX             A0[ ] |
          OLED_5_CS | [ ]TX            RST[ ] |
                    |                         |
                    |                         |
                    |                         |
                    +-------------------------+


  LOLIN D32:
  IFDEF ARDUINO_LOLIN_D32
  BOARDS: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  USE HCPL-260L-000E instead of 6N137
                    +-----------------------------+
                    |                             |
                    | [ ]3V3               GND[ ] |
                    | [ ]RST           MOSI/23[ ] |
                    | [ ]VP/36          SCL/22[ ] |
                    | [ ]VN/39            1/TX[ ] |
                    | [ ]34               3/RX[ ] |
                    | [ ]32             SDA/21[ ] |
                    | [ ]33            MISO/19[ ] |
                    | [ ]25/DAC_1       SCK/18[ ] |
                    | [ ]26/DAC_2        SS/ 5[ ] |
                    | [ ]27                 17[ ] |
                    | [ ]14                 16[ ] |
                    | [ ]12                  4[ ] |
                    | [ ]13                  0[ ] |
                    | [ ]EN                  2[ ] |
                    | [ ]USB                15[ ] |
                    | [ ]BAT               GND[ ] |
                    |                             |
                    |           +-----+           |
                    | +-----+   | RST |   +-----+ |
                    +-| BAT |---+-----+---| USB |-+
                      +-----+             +-----+

  LOLIN D32 PRO:
  IFDEF ARDUINO_LOLIN_D32_PRO
  BOARDS: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  USE HCPL-260L-000E instead of 6N137

                    +-----------------------------+
                    |                             |
                    | [ ]3V3               GND[ ] |
                    | [ ]RST           MOSI/23[ ] | MOSI
                    | [ ]VP/36          SCL/22[ ] |
                    | [ ]VN/39            1/TX[ ] |
                    | [ ]34               3/RX[ ] |
                    | [ ]32/TFT_LED     SDA/21[ ] |
                    | [ ]33/TFT_RST    MISO/19[ ] | MISO
                    | [ ]25/DAC_1       SCK/18[ ] | SCK
                    | [ ]26/DAC_2        SS/ 5[ ] |
                    | [ ]27/TFT_DC          NC[ ] |
                    | [ ]14/TFT_CS          NC[ ] |
                    | [ ]12/TS_CS      TF-CS/4[ ] | SD_CS
                    | [ ]13                  0[ ] |
                    | [ ]EN                  2[ ] |
                    | [ ]USB                15[ ] |
                    | [ ]BAT               GND[ ] |
                    |-----+                 +-----|
                    | I2C |                 |     |
                    |-----+                 | TFT |
                    |-----+                 |     |
                    | BAT |                 +-----|
                    |-----+              +-----+  |
                    |       +-----+      | RST |  |
                    +-------| USB |------+-----+--+
                            +-----+





*/

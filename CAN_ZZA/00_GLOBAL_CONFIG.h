/******************************************************************************
 * DEFINE FONTS
 ******************************************************************************/
#define FONT_4x6      u8g2_font_4x6_t_german
#define FONT_5x8      u8g2_font_5x8_t_german
#define FONT_6x13B    u8g2_font_6x13B_t_german
#define FONT_PS_11X17 u8g2_font_tpss_t_german

/******************************************************************************
 * Usage options:
 ******************************************************************************/
#define DEBUG
#define DEBUG_WIFI

/******************************************************************************
 * PROTOCOL OPTIONS:
 * !!! Never use DCC, MACAN, WiFi together
 * !!! only 1 options is possible
 ******************************************************************************/
//#define USE_MACAN
//#define USE_DCC
#define USE_WIFI


#define DYNAMIC_CLOCK
#define USE_ANALOG_CLOCK

/******************************************************************************
 * WIFI USAGE OPTIONS:
 * please check 23_wifi.h
 **************************** 
 * IF USING TELNET:
 * please check 24_telnet.h
 ****************************
 * IF USING MQTT:
 * please check 25_mqtt.h
 ****************************
 * !!! EITHER USE TELNET or MQTT not both together
 ******************************************************************************/
#ifdef USE_WIFI
  //#define USE_MANUAL_IP
  #define USE_WIFI_CLOCK
  #define USE_TELNET
  //#define USE_MQTT
#endif

/******************************************************************************
 * Generell Setup Daten:
 ******************************************************************************/
#define GleisSeite_Links    0      // An enum uses 2 byte RAM and this uses more byte in the code also ;-(
#define GleisSeite_Rechts   1      // Without enum we save 1 byte RAM and 18 bytes FLASH !

// Constants for the UpdateDisplay variable:
#define DONT_UPD_DISP  0
#define UPD_DISP_ONCE  1  // Update the display once. It has a rolling text UpdateDisplay is set to UPD_DISP_ROLL oterwise its set to DONT_UPD_DISP
#define UPD_DISP_ROLL  2  // Display is updated preiodic because a rolling display is active
#define UPD_DISP_STOP  3  // Stop the rolling display

// If the following two lines are enabled the displayed text will change randomly
//#define RAND_CHANGE_MINTIME     5      // Minimal time between two display changes [s]
//#define RAND_CHANGE_MAXTIME    20      // Maximal time between two display changes [s]

#ifdef USE_DCC
  #if (defined(__MK20DX256__) || defined(__MK64FX512__)|| defined(__MK66FX1M0__)) // TEENSY 3.2, 3.5, 3.6
    #define DCC_SIGNAL_PIN    6   // Connected to the opto coppler which reads the DCC signals
                                  // USE HCPL-260L-000E instead of 6N137
  #else
    #define DCC_SIGNAL_PIN    2   // Connected to the opto coppler which reads the DCC signals
  #endif
#endif

#if (defined(__MK20DX256__) || defined(__MK64FX512__)|| defined(__MK66FX1M0__)) // TEENSY 3.2, 3.5, 3.6
  #define UNUSED_AIN_PIN A10
  #define OLED_MOSI   11
  #define OLED_CLK    13
  #define OLED_DC      9
  #define OLED_RESET   8
  #define OLED_0_CS   10
  #define OLED_1_CS   14
  #define OLED_2_CS   15
  #define OLED_3_CS   16
  #define OLED_4_CS   17
  #define OLED_5_CS   18
  #define OLED_6_CS   19
#else

#if defined(__AVR_ATmega328P__)  // Arduino UNO, NANO
 #define UNUSED_AIN_PIN A0
 #define OLED_CLK    A1
 #define OLED_MOSI   A2
 #define OLED_RESET  A3
 #define OLED_DC     A4
 #define OLED_0_CS   A5
 #define OLED_1_CS   3
 #define OLED_2_CS   4
#else

#if defined(__AVR_ATmega2560__)  // Arduino MEGA2560
 #define UNUSED_AIN_PIN A0
 #define OLED_CLK    A1 // OLD: A5
 #define OLED_MOSI   A2 // OLD: A4
 #define OLED_RESET  A3
 #define OLED_DC     A4 // OLD: A2
 #define OLED_0_CS   A5 // OLD: A1
 #define OLED_1_CS   A6
 #define OLED_2_CS   A7
 #define OLED_3_CS   A8
 #define OLED_4_CS   A9
 #define OLED_5_CS   A10
 #define OLED_6_CS   A11
#else

 #define UNUSED_AIN_PIN A0
 #define OLED_CLK    14
 #define OLED_MOSI   13
 #define OLED_RESET  12
 #define OLED_DC     16
 #define OLED_0_CS   15
 #define OLED_1_CS   5//1
 #define OLED_2_CS   4//3
 /*#define OLED_3_CS   5
 #define OLED_4_CS   4
 #define OLED_5_CS   0
 #define OLED_6_CS   2
*/
#endif
#endif
#endif


const bool    GREEN = 1;
const bool    RED   = 0;
const bool    ON  = 1;
const bool    OFF = 0;

//#ifdef DEBUG
  #define SERIAL_BAUDRATE         115200  // Attention the serial monitor of the Arduino IDE must use the same baud rate
//#endif

int base_address = 145;

typedef struct
{ 
  U8G2    *oled;
  char     RailNr[4];
  uint8_t  RailSide;
  uint8_t  UpdateDisplay;
  uint8_t  offset;
  uint8_t  subset;
  uint8_t  Msg_No_Displayed;
  uint8_t  Late;
} OLED_T;

typedef struct
{
  char     RailNr[4];
} RAILS_T;

/************************************************************************
 * TEENSY 3.x HARDWARE SPI (MOSI0, SCK0)
 * not possible on ARDUINO WITH and MäCAN enabled.
 ************************************************************************
 * Display Rotation:
 * U8G2_R0 => No rotation, landscape
 * U8G2_R2 => 180 degree clockwise rotation
 ************************************************************************
 * #define OLED_MOSI   11
 * #define OLED_CLK    13
 * #define OLED_DC      8
 * #define OLED_RESET   9
 * #define OLED_0_CS   10
 * #define OLED_1_CS   14
 * #define OLED_2_CS   15
 * #define OLED_3_CS   16
 * #define OLED_4_CS   17
 * #define OLED_5_CS   18
 * #define OLED_6_CS   19
 * 
 * U8G2_SSD1306_128X32_UNIVISION_F_4W_HW_SPI  oledX(U8G2_R0, OLED_X_CS, OLED_DC, OLED_RESET);
 * U8G2_SSD1306_128X32_UNIVISION_F_4W_HW_SPI  oledN(U8G2_R0, OLED_N_CS, OLED_DC);
 ************************************************************************/
#if (defined(__MK20DX256__) || defined(__MK64FX512__)|| defined(__MK66FX1M0__))
  U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled0(U8G2_R2, OLED_CLK, OLED_MOSI, OLED_0_CS, OLED_DC, OLED_RESET); // Gleis 1a
  U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled1(U8G2_R2, OLED_CLK, OLED_MOSI, OLED_1_CS, OLED_DC);             // Gleis 1 - vorne
  U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled2(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_2_CS, OLED_DC);             // Gleis 1 - hinten
//  U8G2_SSD1306_128X32_UNIVISION_F_4W_HW_SPI  oled0(U8G2_R2, OLED_0_CS, OLED_DC, OLED_RESET);  // Gleis 1a
//  U8G2_SSD1306_128X32_UNIVISION_F_4W_HW_SPI  oled1(U8G2_R2, OLED_1_CS, OLED_DC);              // Gleis 1 - vorne
//  U8G2_SSD1306_128X32_UNIVISION_F_4W_HW_SPI  oled2(U8G2_R0, OLED_2_CS, OLED_DC);              // Gleis 1 - hinten
  //U8G2_SSD1306_128X32_UNIVISION_F_4W_HW_SPI  oled3(U8G2_R0, OLED_3_CS, OLED_DC);              // Gleis 2 - vorne
  //U8G2_SSD1306_128X32_UNIVISION_F_4W_HW_SPI  oled4(U8G2_R0, OLED_4_CS, OLED_DC);              // Gleis 2 - hinten
  //U8G2_SSD1306_128X32_UNIVISION_F_4W_HW_SPI  oled5(U8G2_R0, OLED_5_CS, OLED_DC);              // Gleis 3 - vorne
  //U8G2_SSD1306_128X32_UNIVISION_F_4W_HW_SPI  oled6(U8G2_R0, OLED_6_CS, OLED_DC);              // Gleis 3 - hinten
#else

/************************************************************************
 * SOFTWARE SPI
 * on Arduino UNO & MEGA
 ************************************************************************
 * Display Rotation:
 * U8G2_R0 => No rotation, landscape
 * U8G2_R2 => 180 degree clockwise rotation
 ************************************************************************
 * #define OLED_CLK     A1
 * #define OLED_MOSI    A2
 * #define OLED_RESET   A3
 * #define OLED_DC      A4
 * #define OLED_0_CS    A5
 * #define OLED_1_CS    A6 // MEGA // UNO => 3
 * #define OLED_2_CS    A7 // MEGA // UNO => 4
 * #define OLED_3_CS    A8 // MEGA
 * #define OLED_4_CS    A9 // MEGA
 * #define OLED_5_CS   A10 // MEGA
 * #define OLED_6_CS   A11 // MEGA
 * 
 * U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oledX(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_X_CS, OLED_DC, OLED_RESET);
 * U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oledN(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_N_CS, OLED_DC);
************************************************************************/
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__) // Arduino UNO, NANO, MEGA
  U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled0(U8G2_R2, OLED_CLK, OLED_MOSI, OLED_0_CS, OLED_DC, OLED_RESET); // Gleis 1a
  U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled1(U8G2_R2, OLED_CLK, OLED_MOSI, OLED_1_CS, OLED_DC);             // Gleis 1 - vorne
  U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled2(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_2_CS, OLED_DC);             // Gleis 1 - hinten
  //U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled3(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_3_CS, OLED_DC);             // Gleis 2 - vorne
  //U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled4(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_4_CS, OLED_DC);             // Gleis 2 - hinten
  //U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled5(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_5_CS, OLED_DC);             // Gleis 3 - vorne
  //U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled6(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_6_CS, OLED_DC);             // Gleis 3 - hinten
#else
  U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled0(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_0_CS, OLED_DC, OLED_RESET); // Gleis 1a
  U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled1(U8G2_R2, OLED_CLK, OLED_MOSI, OLED_1_CS, OLED_DC);             // Gleis 1 - vorne
  U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled2(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_2_CS, OLED_DC);             // Gleis 1 - hinten

#endif
#endif

const RAILS_T rail_definition[] = { // DEFINE RAILS, only needed for address calculation
                               // MUST not match with assignment on oleds
                               "1a",
                                "1",
//                                "2", 
//                                "3", 
};

OLED_T oleds[] = { //OLED, RailNr, RailSide, UpdateDisplay
                  {  &oled0, "1a",  GleisSeite_Rechts, UPD_DISP_ONCE, 0, 0, 0, 0},
                  {  &oled1,  "1", GleisSeite_Rechts, UPD_DISP_ONCE, 0, 0, 0, 0},
                  {  &oled2,  "1",  GleisSeite_Links, UPD_DISP_ONCE, 0, 0, 0, 0},
//                  {  &oled3,  "2", GleisSeite_Rechts, UPD_DISP_ONCE, 0, 0, 0, 0},
//                  {  &oled4,  "2",  GleisSeite_Links, UPD_DISP_ONCE, 0, 0, 0, 0},
//                  {  &oled5,  "3", GleisSeite_Rechts, UPD_DISP_ONCE, 0, 0, 0, 0},
//                  {  &oled6,  "3",  GleisSeite_Links, UPD_DISP_ONCE, 0, 0, 0, 0}
};

#define RAIL_COUNT (sizeof(rail_definition)/sizeof(RAILS_T))
#define OLED_COUNT (sizeof(oleds)/sizeof(OLED_T))



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

*/

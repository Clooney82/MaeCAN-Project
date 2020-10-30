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
//#define DEBUG           // General debug messages
//#define DEBUG_WIFI      // debug WiFi
//#define DEBUG_TELNET     // debug telnet
//#define DEBUG_HTTP

/******************************************************************************
 * PROTOCOL OPTIONS:
 * !!! Never use DCC, MACAN, WiFi together
 * !!! only 1 options is possible
 ******************************************************************************/
//#define USE_MACAN     // enable this if you want to use MäCAN Interface
//#define USE_DCC       // enable this if you want to use DCC
#define USE_WIFI      // enable this if you want wifi
                      // only Available on ESP8266 hardware.
//#define ESP32

/******************************************************************************
 * Alternativ Display Hardware
 * DEFAULT: SPI Displays
 ******************************************************************************/
//#define USE_I2C       // enable this if you use I2C Displays, instead of SPI
                        // Additional Hardware and wiring needed
                        // !!! us at your on risk !!!
                        // !!! NOT TESTED by me!  !!!

/******************************************************************************
 * Enable different addition FEATURES
 * DYNAMIC_CLOCK -> Automatik running time.
 *                  starting at 8am during boot and increase in 1min steps every
 *                  6000ms.
 * DYNAMIC_DEPARTURE_TIME -> Automated Departure Time calculation
 *                            based on currenttime + add_dep_time
 * USE_ANALOG_CLOCK -> Feature to Display an analog Clock while Message 0 and 1
 *                      is shown.
 * RANDOM_LATE      -> Puts random delay to the trains
 ******************************************************************************/
#define DYNAMIC_CLOCK
//#define DYNAMIC_DEPARTURE_TIME
#define USE_ANALOG_CLOCK
//#define RANDOM_LATE

/******************************************************************************
 * WIFI USAGE OPTIONS:
 * please check 23_wifi.h
 **************************** 
 * IF USING TELNET:
 * please check 24_telnet.h
 ******************************************************************************/
#ifdef USE_WIFI
  //#define USE_MANUAL_IP     // Manual IP Setup
  #define USE_WIFI_CLOCK      // Enable option to control clock via WiFi (telnet)
  #define USE_TELNET          // Use telnet to control Displays
  #define USE_HTTP            // Use http Webserver to control Displays (work in progress)
#endif

/******************************************************************************
 * Generell Setup:
 ******************************************************************************
 * Define for: on which Side of the OLED the rail is
 ******************************************************************************/
#define GleisSeite_Links    0      // An enum uses 2 byte RAM and this uses more byte in the code also ;-(
#define GleisSeite_Rechts   1      // Without enum we save 1 byte RAM and 18 bytes FLASH !

/******************************************************************************
 * Starting Address of the Messages
 ******************************************************************************/
int base_address = 145;

uint8_t add_dep_time = 15;    // add xx Min departure time ( current time + xx )
/******************************************************************************
 * Constants for the UpdateDisplay variable:
 ******************************************************************************/
#define DONT_UPD_DISP  0
#define UPD_DISP_ONCE  1  // Update the display once. It has a rolling text UpdateDisplay is set to UPD_DISP_ROLL oterwise its set to DONT_UPD_DISP
#define UPD_DISP_ROLL  2  // Display is updated preiodic because a rolling display is active
#define UPD_DISP_STOP  3  // Stop the rolling display

/******************************************************************************
 * If the following two lines are enabled the displayed text will change randomly
 ******************************************************************************/
//#define RAND_CHANGE_MINTIME     5      // Minimal time between two display changes [s]
//#define RAND_CHANGE_MAXTIME    20      // Maximal time between two display changes [s]

/******************************************************************************
 * PIN definition for DCC
 ******************************************************************************/
#ifdef USE_DCC
  #if (defined(__MK20DX256__) || defined(__MK64FX512__)|| defined(__MK66FX1M0__)) // TEENSY 3.2, 3.5, 3.6
    #define DCC_SIGNAL_PIN    6   // Connected to the opto coppler which reads the DCC signals
                                  // USE HCPL-260L-000E instead of 6N137
  #else
    #define DCC_SIGNAL_PIN    2   // Connected to the opto coppler which reads the DCC signals
  #endif
#endif

/******************************************************************************
 * PIN usage Definitions:
 ******************************************************************************
 * DEFAULT PIN assignment for Teensy
 ******************************************************************************/
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
/******************************************************************************
 * DEFAULT PIN assignment for Arduino UNO and NANO
 ******************************************************************************/
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
/******************************************************************************
 * DEFAULT PIN assignment for Arduino Mega
 ******************************************************************************/
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
/******************************************************************************
* PIN assignment in case of other HW !!!
* 
* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
* !!! MANUAL ADJUSTMENT NEEDED !!!
* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
* 
******************************************************************************/
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

/******************************************************************************
 * DO NOT CHANGE
 * >>>>>>>>>>>>
 ******************************************************************************/
const bool    GREEN = 1;
const bool    RED   = 0;
const bool    ON  = 1;
const bool    OFF = 0;

#define SERIAL_BAUDRATE         115200  // Attention the serial monitor of the Arduino IDE must use the same baud rate

typedef struct
{ 
  U8G2    *oled;
#ifdef USE_I2C
  uint8_t  OLED_Enable_Pin;
#endif
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
/******************************************************************************
 * <<<<<<<<<<<<<
 * DO NOT CHANGE
 ******************************************************************************/

/******************************************************************************
 * Display Hardware Setup:
 ******************************************************************************
 * I2C HARDWARE
 ************************************************************************
 * Display Rotation:
 * U8G2_R0 => No rotation, landscape
 * U8G2_R2 => 180 degree clockwise rotation
 ************************************************************************/
#ifdef USE_I2C
  U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C oled(U8G2_R2, U8X8_PIN_NONE); // 55-60 ms update time, full frame buffer:       512 bytes RAM
#else
/************************************************************************
 * TEENSY 3.x HARDWARE SPI (MOSI0, SCK0)
 * not possible on ARDUINO WITH and MäCAN enabled.
 ************************************************************************
 * Display Rotation:
 * U8G2_R0 => No rotation, landscape
 * U8G2_R2 => 180 degree clockwise rotation
 ************************************************************************
 * HARDWARE SPI:
 * U8G2_SSD1306_128X32_UNIVISION_F_4W_HW_SPI  oledX(U8G2_R0, OLED_X_CS, OLED_DC, OLED_RESET);
 * U8G2_SSD1306_128X32_UNIVISION_F_4W_HW_SPI  oledN(U8G2_R0, OLED_N_CS, OLED_DC);
 * SOFTWARE SPI:
 * U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oledX(U8G2_R0, OLED_X_CS, OLED_DC, OLED_RESET);
 * U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oledN(U8G2_R0, OLED_N_CS, OLED_DC);
 ************************************************************************/
#if (defined(__MK20DX256__) || defined(__MK64FX512__)|| defined(__MK66FX1M0__))
  U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled0(U8G2_R2, OLED_CLK, OLED_MOSI, OLED_0_CS, OLED_DC, OLED_RESET); // Gleis 1a
  U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled1(U8G2_R2, OLED_CLK, OLED_MOSI, OLED_1_CS, OLED_DC);             // Gleis 1 - vorne
  U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled2(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_2_CS, OLED_DC);             // Gleis 1 - hinten
  //U8G2_SSD1306_128X32_UNIVISION_F_4W_HW_SPI  oled0(U8G2_R2, OLED_0_CS, OLED_DC, OLED_RESET);  // Gleis 1a
  //U8G2_SSD1306_128X32_UNIVISION_F_4W_HW_SPI  oled1(U8G2_R2, OLED_1_CS, OLED_DC);              // Gleis 1 - vorne
  //U8G2_SSD1306_128X32_UNIVISION_F_4W_HW_SPI  oled2(U8G2_R0, OLED_2_CS, OLED_DC);              // Gleis 1 - hinten
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
/************************************************************************
 * CONFIG for all other Hardeware:
 * e.g Wemos D1 mini
 ************************************************************************/
  U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled0(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_0_CS, OLED_DC, OLED_RESET); // Gleis 1a
  U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled1(U8G2_R2, OLED_CLK, OLED_MOSI, OLED_1_CS, OLED_DC);             // Gleis 1 - vorne
  U8G2_SSD1306_128X32_UNIVISION_F_4W_SW_SPI  oled2(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_2_CS, OLED_DC);             // Gleis 1 - hinten

#endif
#endif
#endif

/******************************************************************************
 * RAIL SETUP:
 * ------------------
 * DEFINE RAILS, only needed for address calculation
 * MUST not match with assignment on oleds.
 * e.g. if you have 4 RAIL, you need 4 entries here
 ******************************************************************************/
const RAILS_T rail_definition[] = {
//                               "1a",
                                "1",
//                                "2", 
//                                "3", 
//                                "4"
};

/******************************************************************************
 * OLED Configuration for I2C usage
 * One Entry per OLED
 ******************************************************************************/
#ifdef USE_I2C
OLED_T oleds[] = { //OLED,   OLED_Enable_Pin, RailNr, RailSide,          UpdateDisplay
                  {  &oled,       1,         "1a",   GleisSeite_Rechts, UPD_DISP_ONCE, 0, 0, 0, 0},
                  {  &oled,       2,         "1",    GleisSeite_Rechts, UPD_DISP_ONCE, 0, 0, 0, 0},
                  {  &oled,       3,         "1",    GleisSeite_Links,  UPD_DISP_ONCE, 0, 0, 0, 0},
};
#else
/******************************************************************************
 * OLED Configuration for SPI usage
 * One Entry per OLED
 ******************************************************************************/
OLED_T oleds[] = { //OLED, RailNr, RailSide, UpdateDisplay
                  {  &oled0, "1a", GleisSeite_Rechts, UPD_DISP_ONCE, 0, 0, 0, 0},
//                  {  &oled1,  "1", GleisSeite_Rechts, UPD_DISP_ONCE, 0, 0, 0, 0},
//                  {  &oled2,  "1", GleisSeite_Links,  UPD_DISP_ONCE, 0, 0, 0, 0},
//                  {  &oled3,  "2", GleisSeite_Rechts, UPD_DISP_ONCE, 0, 0, 0, 0},
//                  {  &oled4,  "2",  GleisSeite_Links, UPD_DISP_ONCE, 0, 0, 0, 0},
//                  {  &oled5,  "3", GleisSeite_Rechts, UPD_DISP_ONCE, 0, 0, 0, 0},
//                  {  &oled6,  "3",  GleisSeite_Links, UPD_DISP_ONCE, 0, 0, 0, 0}
};
#endif
/******************************************************************************
 * DO NOT CHANGE
 * >>>>>>>>>>>>
 ******************************************************************************/
#define RAIL_COUNT (sizeof(rail_definition)/sizeof(RAILS_T))
#define OLED_COUNT (sizeof(oleds)/sizeof(OLED_T))

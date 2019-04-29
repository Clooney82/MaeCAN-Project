/******************************************************************************
 * Allgemeine Setup Daten:
 ******************************************************************************/
#define VERS_HIGH 0       // Versionnumber before dot
#define VERS_LOW  3       // Versionnumber after dot
#define BOARD_NUM 1       // Identificationnumber of the board (Display on CS2/3)
#define UID 0x10540020    // CAN-UID - please change. Must be unique
// UID = 0x1 device.type VERS_HIGH VERS_LOW BOARD_NUM


const uint8_t ANZ_ADDONS = 2;         // Number of AddOn Platins (max. 8 per I2C Bus)
// => On more than 6 Moduls ATMega328 will run out of memory
// => Max. 32 AddOns, max. 8 per I2C Bus
// ANZ_S88_ADDONS + ANZ_ACC_ADDONS !!!

// Wire 0 will allways be used, enable for wire1-3
//#define USE_WIRE1
//#define USE_WIRE2
//#define USE_WIRE3

#if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))  // teensy 3.0/3.1-3.2/LC/3.5/3.6
uint8_t  i2c_bus0_len = 2;    // Number of Moduls on I2C Bus 0 - max 8 - only Teensy LC, 3.1, 3.2, 3.5, 3.6
uint8_t  i2c_bus1_len = 0;    // Number of Moduls on I2C Bus 1 - max 8 - only Teensy LC, 3.1, 3.2, 3.5, 3.6
uint8_t  i2c_bus2_len = 0;    // Number of Moduls on I2C Bus 2 - max 8 - only Teensy               3.5, 3.6
uint8_t  i2c_bus3_len = 0;    // Number of Moduls on I2C Bus 3 - max 8 - only Teensy                    3.6
#endif


/******************************************************************************
 * EEPROM-Registers
 ******************************************************************************/
#define REG_UID   105   // 4 bytes

/******************************************************************************
 * enable debugging
 * Debbuging is only possible with max 2 AddOns (if using an Arduino)
 ******************************************************************************/
//#define DEBUG_SERIAL
//#define DEBUG_SETUP
//#define DEBUG_SETUP_S88
//#define DEBUG_SETUP_ACC
//#define DEBUG_S88
//#define DEBUG_MCAN
//#define DEBUG_CAN
//#define run_fake_s88_events   // don´t use if connected to a CS

//#define DEBUG_CONFIG
//#define DEBUG_LED
//#define DEBUG_LED_TEST
//#define DEBUG_INPUT
//#define DEBUG_DS
//#define run_fake_acc_commands // don´t use if connected to a CS

/******************************************************************************
 * Common constants:
 ******************************************************************************/
#define INDIVIDUAL_SWITCHTIME
const bool    ACC_DCC = 0;
const bool    ACC_MM  = 1;
const bool    GREEN = 1;
const bool    RED   = 0;
const bool    ON  = 1;
const bool    OFF = 0;
const bool    POWER_ON  = 1;
const bool    POWER_OFF = 0;
const bool    BUTTON_PRESSED     = 1;
const bool    BUTTON_NOT_PRESSED = 0;
const uint8_t TYPE_TURNOUT       = 0;
const uint8_t TYPE_UNCOUPLER     = 1;
const uint8_t TYPE_SINGLE_BUTTON = 2;


#if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))  // teensy 3.0/3.1-3.2/LC/3.5/3.6
const int STATUS_LED_PIN = 13;
#else
const int STATUS_LED_PIN = 9;
#endif

unsigned long previousMillis = 0;
unsigned long currentMillis  = 0;
unsigned long interval = 1000;
bool state_LED = false;

/******************************************************************************
 * Variables:
 ******************************************************************************/
uint8_t  i2c_bus_con  = 0;    // current used I2C Bus
uint8_t  CONFIG_NUM;          // number of configurationspoints
bool     config_poll = false;
byte     uid_mat[4];
uint8_t  config_index = 0;
bool     config_sent = 0;
bool     locked = false;
uint8_t  board_num = 0;


#if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))  // teensy 3.0/3.1-3.2/LC/3.5/3.6
  // include only needed libraries !!!
  #include <mcp23017.h>
  #ifdef USE_WIRE1
    #include <mcp23017_w1.h>
  #endif
  #ifdef USE_WIRE2
    #include <mcp23017_w2.h>
  #endif
  #ifdef USE_WIRE3
    #include <mcp23017_w3.h>
  #endif
#else
  #include <Adafruit_MCP23017.h>
#endif

#if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))  // teensy 3.0/3.1-3.2/LC/3.5/3.6
  // only create needed ones. must be done manually
  MCP23017    AddOn[8];                // Create AddOn
  #ifdef USE_WIRE1
    MCP23017_W1 AddOn_W1[1];             // Create AddOn
  #endif
  #ifdef USE_WIRE2
    MCP23017_W2 AddOn_W2[1];             // Create AddOn
  #endif
  #ifdef USE_WIRE3
    MCP23017_W3 AddOn_W3[1];             // Create AddOn
  #endif
#else
  Adafruit_MCP23017 AddOn[ANZ_ADDONS];    // Create AddOn
#endif

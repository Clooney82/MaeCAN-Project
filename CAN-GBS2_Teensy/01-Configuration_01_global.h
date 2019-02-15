
/******************************************************************************
 * Allgemeine Setup Daten:
 ******************************************************************************/
#define VERS_HIGH 0       // Versionsnummer vor dem Punkt
#define VERS_LOW  1       // Versionsnummer nach dem Punkt
#define BOARD_NUM 1       // Identifikationsnummer des Boards (Anzeige in der CS2)
#define UID 0x10560101    // CAN-UID - please change. Must be unique
// UID = 0x1 device.type VERS_HIGH VERS_LOW BOARD_NUM

const uint8_t USE_ONBOARD = 0;        // On-Board Ausgänge benutzen: 0 = Nein; 1 = Ja
                                      // => USE_ONBOARD wird als S88 nicht unterstützt, 
                                      // => da nur 8 bzw. 12 PINS verfügbar und normal 16 Rückmelder pro S88 Modul vorhanden sind.
                                      // => ggf. folgt später noch eine Anpassung für die onboard Pins eines teensy

const uint8_t ANZ_ACC_ONBOARD = 0;    // Anzahl an Ausgängen Onboard: Default = 8, S88 = 0

const uint8_t ANZ_ADDONS = 8;         // Anzahl AddOn Platinen (max 8 pro I2C Bus)
                                      // => Ab >6 Modulen wird der Arbeitsspeicher auf den ATMega328 knapp.
                                      // => Maximal 32 AddOns, 8 Stk pro I2C Bus
                                      // ANZ_S88_ADDONS + ANZ_ACC_ADDONS !!!

uint8_t  i2c_bus_con  = 0;    // Aktuell verwendeter I2C Bus
#if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))  // teensy 3.0/3.1-3.2/LC/3.5/3.6
  uint8_t  i2c_bus0_len = 2;    // Anzahl Module an I2C Bus 0 - maximal 8 - muss nur geändert werden, wenn mehr I2C-Busse verwendet werden (nur Teensy 3.x)
  uint8_t  i2c_bus1_len = 2;    // Anzahl Module an I2C Bus 1 - maximal 8 - nur Teensy LC, 3.1, 3.2, 3.5, 3.6
  uint8_t  i2c_bus2_len = 2;    // Anzahl Module an I2C Bus 2 - maximal 8 - nur Teensy               3.5, 3.6
  uint8_t  i2c_bus3_len = 2;    // Anzahl Module an I2C Bus 3 - maximal 8 - nur Teensy                    3.6
#endif

/******************************************************************************
 * EEPROM-Register - S88
 ******************************************************************************/
#define REG_UID        105   // 4 bytes

/******************************************************************************
 * Debugging einschalten
 * Debbuging is only possible with max 2 AddOns (if using an Arduino)
 ******************************************************************************/
//#define DEBUG_SERIAL
//#define DEBUG_SETUP
//#define DEBUG_SETUP_S88
//#define DEBUG_SETUP_ACC
//#define DEBUG_S88
//#define DEBUG_MCAN
//#define DEBUG_CAN
//#define run_fake_s88_events

//#define DEBUG_CONFIG
//#define DEBUG_LED
//#define DEBUG_INPUT
//#define run_fake_acc_commands

/******************************************************************************
 * Allgemeine Konstanten:
 ******************************************************************************/
#define INDIVIDUAL_SWITCHTIME
#define GREEN 1
#define RED   0
#define ON    1
#define OFF   0
#define POWER_ON  1
#define POWER_OFF 0
#define BUTTON_PRESSED     1
#define BUTTON_NOT_PRESSED 0
#define TYPE_WEICHE       0
#define TYPE_ENTKUPPLER   1
#define MOMENT     0
#define DAUER      1
#define TORANTRIEB 2

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
 * Variablen der Configuration:
 ******************************************************************************/
uint8_t  CONFIG_NUM;   // Anzahl der Konfigurationspunkte
bool     config_poll = false;
byte     uid_mat[4];
uint8_t  config_index = 0;
bool     config_sent = 0;
bool     locked = false;


#if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))  // teensy 3.0/3.1-3.2/LC/3.5/3.6
  MCP23017 AddOn[ANZ_ADDONS];   // Create AddOn
#else
  Adafruit_MCP23017 AddOn[ANZ_ADDONS];   // Create AddOn
#endif

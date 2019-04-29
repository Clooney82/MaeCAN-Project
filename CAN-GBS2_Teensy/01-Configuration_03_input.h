/******************************************************************************
 * Common setup data:
 ******************************************************************************/
#define LED_FEEDBACK                    // active to enable LEDs to show ACC state

const uint8_t ANZ_ACC_ADDONS = 1;      // Number of INPUT AddOns (max 8 per I2C Bus)
#ifdef LED_FEEDBACK
  const uint8_t ANZ_ACC_PER_ADDON = 4;  // Number of ACCs per AddOn
#else
  const uint8_t ANZ_ACC_PER_ADDON = 8;  // Number of ACCs per AddOn
#endif
/*---------------------------------------
 Number of ACC per AddOn Moduls:
   => with LED Feedback = 4
   => without LED Feedback = 8
   => Relais = 8
   => Weichen = 8
 Anzahl an Ausgängen pro AddOn Platine:
   => Stellpult mit LED Anzeige = 4
   => Stellpult ohne LED Anzeige = 8
   => Relais = 8
   => Weichen = 8
   => Weichen mit Lagerückmeldung = 5 (aktuelle AddOn-HW unterstützt dies noch nicht.)
   ==>> evtl. ab AddOn-HW Rev. c
 ---------------------------------------*/

int base_address = 1;
/*---------------------------------------
 => Base address ( x = base_address + 1 ) for initial address configuration (see config_own_adresses_manual() )
 => base_address =   0 ===> 1, 2, 3, 4, ...
 => base_address = 100 ===> 101, 102, 103, 104, ...
 ---------------------------------------*/
/******************************************************************************
 * EEPROM-Register - INPUT
 ******************************************************************************/
#define REG_PROT 9     // Register Address for protocol
#define REG_INPUT_SETUP 135

/******************************************************************************
   Common constants:
 ******************************************************************************/

#ifdef DEBUG_CONFIG
  byte byteRead;
  String stringRead;
#endif
#ifdef run_fake_acc_commands
  uint8_t tmp = 0;
#endif

String string1;
String string2;
String string3;
String string4;

int start_adrs_channel = 2;

typedef struct {
  int reg_locid;        // EEPROM-Register of Local-IDs
  int reg_type;         // EEPROM-Register for ACC types
  int reg_state;        // EEPROM-Register for storing ACC status
  int reg_prot;         // EEPROM-Register for storing ACC protocol

  uint16_t locID;       // LocalID
  uint8_t prot;

  uint8_t Modul;        // 0 = On-Board; 1 = Modul1; 2 = Modul2, ...  MAYBE DELETE
  uint8_t pin_grn;      // PIN green
  uint8_t pin_red;      // PIN red
  #ifdef LED_FEEDBACK
    uint8_t pin_led_grn;  // PIN green
    uint8_t pin_led_red;  // PIN red
  #endif

  uint8_t acc_type  = TYPE_TURNOUT;
  // INPUT
  unsigned long pushed  = 0;
  // OUTPUT
  bool state_is  = 0;   // ...
  bool state_set = 0;   // ...
  bool power_is  = 0;   // current status
  bool power_set = 0;   // target status
  uint8_t i2c_bus = 0;  // # of I2C Bus
  uint8_t board_num = 0;// # of board on i2c bus (0-3)
} acc_input;


const uint8_t NUM_ACCs = (ANZ_ACC_ADDONS * ANZ_ACC_PER_ADDON);

acc_input acc_articles[NUM_ACCs];

unsigned long previousMillis_input = 0;
const uint16_t acc_interval = 500;
const uint8_t input_interval = 100;

/******************************************************************************
   variables for acc:
 ******************************************************************************/
uint8_t  CONFIG_NUM_INPUT;
uint16_t prot;
uint16_t prot_old;
bool pushed_red;
bool pushed_grn;

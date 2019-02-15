/******************************************************************************
 * Allgemeine Setup Daten:
 ******************************************************************************/
bool     use_L88 = true;  // use_L88 => false = USE CS2/3 S88 Bus ; true = USE Link L88
uint16_t modulID = 179;   // ID des Link L88                  - MODIFIED - DEFAULT: 0

const uint8_t ANZ_S88_ADDONS = 4;         // Anzahl AddOn S88 Platinen (max 8 pro I2C Bus)
const uint8_t ANZ_S88_PER_ADDON = 16;     // DEFAULT: 16 pro AddOn

/* default
bool     use_bus[4]   = { 1, 0, 0, 0 };
uint16_t start_bus[4] = { 1, 1, 1, 1 };
uint8_t  len_bus[4]   = { 1, 1, 1, 1 };
*/

bool     use_bus[]   = { 1, 1, 1, 0 };
uint16_t start_bus[] = { 1, 1, 1, 1 };
uint8_t  len_bus[]   = { 1, 6, 4, 1 };   // 1 + 6 + 4 = 11x S88 AddOn Module

/*  use_bus[4] = { Link L88 Interner BUS0 (1-16) oder CS2/3plus , Link L88 BUS1 , Link L88 BUS2 , Link L88 BUS3 }
 *  start_bus[4] mit welchem S88 Modul am jeweiligen BUS soll begonnen werden
 *  len_bus[4] wie viele S88 Module sind am jeweiligen BUS angeschlossen
 */

/******************************************************************************
 * EEPROM-Register - S88
 ******************************************************************************/
#define REG_use_L88    120
#define REG_modulID    121   // 2 bytes

#define REG_use_bus0   123
#define REG_len_bus0   124
#define REG_start_bus0 125

#define REG_use_bus1   126
#define REG_len_bus1   127
#define REG_start_bus1 128

#define REG_use_bus2   129
#define REG_len_bus2   130
#define REG_start_bus2 131

#define REG_use_bus3   132
#define REG_len_bus3   133
#define REG_start_bus3 134

const int NUM_S88 = ANZ_S88_ADDONS * ANZ_S88_PER_ADDON;

typedef struct {
  uint16_t rm;
  bool state_is;        // aktueller LED Schaltzustand - kann ggf. weggelassen werden, dann werden aber in jedem loop() die LEDs neu angesteuert
  bool state_set;       // LED Soll Zustand
} s88_contacts_struct;

uint8_t bus         = 0;
int     contact     = 0;
uint8_t s88_modul   = 0;
uint8_t addon_pin   = 0;
uint8_t addon_modul = 0;


uint16_t curr_bus[4];
uint16_t last_bus[4];

s88_contacts_struct s88_contacts[NUM_S88];

/******************************************************************************
 * Variablen für das initiale abfragen der Rückmeldekontakte
 ******************************************************************************/
uint8_t CONFIG_NUM_S88;   // Anzahl der Konfigurationspunkte
uint8_t checked_S88 = 0;
unsigned long currentMillis_s88read = 0;
unsigned long previousMillis_s88read = 0;
const long interval_s88read = 200;
bool new_s88_setup_needed = false;


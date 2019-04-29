/******************************************************************************
 * Common setup data:
 ******************************************************************************/
bool     use_L88 = true;  // use_L88 => false = USE CS2/3 S88 Bus ; true = USE Link L88
uint16_t modulID = 0;   // ID of Link L88                  - MODIFIED - DEFAULT: 0

const uint8_t ANZ_S88_ADDONS = 1;        // Number of S88 AddOns (max 8 per I2C Bus)
const uint8_t ANZ_S88_PER_ADDON = 16;     // DEFAULT: 16 per AddOn

bool     use_bus[4]   = { 1, 0, 0, 0 };
uint16_t start_bus[4] = { 1, 1, 1, 1 };
uint8_t  len_bus[4]   = { 1, 1, 1, 1 };

/*  use_bus[4]    = { Link L88 Interner BUS0 (1-16) oder CS2/3plus , Link L88 BUS1 , Link L88 BUS2 , Link L88 BUS3 }
 *  start_bus[4]  = start with S88 Modul # on defined BUS
 *  len_bus[4]    = number of S88 Moduls on defined BUS
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
  uint16_t rm;          // RM Adress
  bool state_is;        // current LED State
  bool state_set;       // target  LED State
  uint8_t i2c_bus = 0;  // I2C Bus
  uint8_t board_num = 0;// Board# on I2C-BUS (0-4)
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
 * variables for initial scanning of feedback contacts
 ******************************************************************************/
uint8_t CONFIG_NUM_S88;
uint8_t checked_S88 = 0;
unsigned long currentMillis_s88read = 0;
unsigned long previousMillis_s88read = 0;
const long interval_s88read = 20;
bool new_s88_setup_needed = false;

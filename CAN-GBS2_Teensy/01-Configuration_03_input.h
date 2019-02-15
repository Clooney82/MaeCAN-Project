/******************************************************************************
 * Allgemeine Setup Daten:
 ******************************************************************************/
#define LED_FEEDBACK                    // Werden LEDs zu Anzeige der Stellung benutzt

const uint8_t ANZ_ACC_ADDONS = 4;         // Anzahl INPUT AddOn Platinen (max 8 pro I2C Bus)
#ifdef LED_FEEDBACK
  const uint8_t ANZ_ACC_PER_ADDON = 4;  // Anzahl an Ausgängen pro AddOn Platine: Default = 4
#else
  const uint8_t ANZ_ACC_PER_ADDON = 8;  // Anzahl an Ausgängen pro AddOn Platine: Default = 8
#endif
/*---------------------------------------
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
 => Basisadresse ( x = base_address + 1 ) für die initiale Adresskonfiguration (see config_own_adresses_manual() )
 => base_address =   0 ===> 1, 2, 3, 4, ...
 => base_address = 100 ===> 101, 102, 103, 104, ...
 ---------------------------------------*/
/******************************************************************************
 * EEPROM-Register - INPUT
 ******************************************************************************/
#define REG_PROT 9     // Speicheradress für Protokoll - Global für alle ACC gleich.
#define REG_INPUT_SETUP    135

/******************************************************************************
   Allgemeine Konstanten:
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
  int reg_locid;        // EEPROM-Register der Local-IDs
  int reg_type;         // EEPROM-Register für die ACC Typen
  int reg_state;        // EEPROM-Register zum Speichern des Schaltzustand

  uint16_t locID;       // LocalID
  uint16_t prot;

  uint8_t Modul;        // 0 = On-Board; 1 = Modul1; 2 = Modul2, ...
  uint8_t pin_grn;      // PIN Grün
  uint8_t pin_red;      // PIN Rot
  #ifdef LED_FEEDBACK
    uint8_t pin_led_grn;  // PIN Grün
    uint8_t pin_led_red;  // PIN Rot
  #endif

  uint8_t acc_type  = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler
  // INPUT
  unsigned long pushed  = 0;
  // OUTPUT
  bool state_is  = 0;   // ...
  bool state_set = 0;   // ...
  bool power_is  = 0;   // Strom ein-/aus
  bool power_set = 0;   // Strom ein-/ausschalten
} acc_input;


const uint8_t NUM_ACCs = (ANZ_ACC_ADDONS * ANZ_ACC_PER_ADDON) ; // = 4 * USE_ONBOARD + ANZ_ACC_PER_ADDON * ANZ_ACC_ADDONS

acc_input acc_articles[NUM_ACCs];

unsigned long previousMillis_input = 0;
const uint16_t acc_interval = 500;
const uint8_t input_interval = 100;

/******************************************************************************
   Variablen der Magnetartikel:
 ******************************************************************************/
uint8_t  CONFIG_NUM_INPUT;   // Anzahl der Konfigurationspunkte
uint16_t prot;
uint16_t prot_old;
bool pushed_red;
bool pushed_grn;



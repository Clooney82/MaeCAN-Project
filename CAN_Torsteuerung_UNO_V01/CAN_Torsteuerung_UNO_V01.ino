/*
 * MäCAN-Relais AddOn, Software-Version 0.51
 *
 *  Created by Maximilian Goldschmidt <maxigoldschmidt@gmail.com>
 *  Modified by Jochen Kielkopf.
 *  Do with this whatever you want, but keep thes Header and tell
 *  the others what you changed!
 *
 *  Last edited: 2018-01-06
 *
 *  Noch Lokschuppen
 *
 *  Relais 1 = AUF / ZU
 *  Relais 2 = Strom
 */

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <MCAN.h>
#include <EEPROM.h>
#include <Adafruit_MCP23017.h>

/******************************************************************************
 * EEPROM-Register-Übersicht
 * # 0 - 20 -> CONFIG:
 * ------------------------------------------------------------------------
 *  5 -  8 -> UID
 *       9 -> PROTOKOLL
 *      10 -> Initial Config ( 0 = unkonfiguriert, 1 = konfiguriert )
 *      11 -> SWITCHMODE
 * 12 - 13 -> SWITCHTIME
 *      14 -> FEEDBACK
 * 15 - 19 -> reserved / free
 *     >20 -> ACCs
 * ========================================================================
 * #21 - XX -> Adressbereich, Pin, Modul (Max 167 ACCs):
 * ------------------------------------------------------------------------
 *  2 byte -> LocalID       // wird aktuell genutzt
 *  2 byte -> Adresse       // wird nicht aktuell genutzt
 *  1 byte -> Status        // wird nicht aktuell genutzt
 *  1 byte -> Modul ( 0 = Decoder, 1 = AddOn 1, 2 = AddOn 2 ) // wird nicht aktuell genutzt
 * ------------------------------------------------------------------------
 * ------------------------------------------------------------------------
 * Formeln zu Berechnung der Speicherregister
 * ------------------------------------------------------------------------
 * LocalID = (20+(6*i)-5) (20+(6*i)-4)
 * Adresse = (20+(6*i)-3) (20+(6*i)-2)
 * Status  = (20+(6*i)-1)
 * Modul   = (20+(6*i))
 * ========================================================================
 ******************************************************************************/

/******************************************************************************
 * Allgemeine Setup Daten:
 ******************************************************************************/
#define VERS_HIGH 0       //Versionsnummer vor dem Punkt
#define VERS_LOW  51       //Versionsnummer nach dem Punkt
#define BOARD_NUM 1       //Identifikationsnummer des Boards (Anzeige in der CS2)
//#define UID 0x1055278    //CAN-UID
//#define UID 0x19F53500    // CAN-UID - please change. Must be unique
//#define BOARD_NUM 2       // Identifikationsnummer des Boards (Anzeige in der CS2)
//#define UID 0x19549995    // CAN-UID - please change. Must be unique
//#define UID 0x10540041      // S88-Modul #1
//#define UID 0x10540042      // S88-Modul #2
//#define UID 0x10053016    // Stellpult #1
//#define UID 0x10053017    // Stellpult #2
//#define UID 0x10053018    // Stellpult #3
#define UID 0x10055001    // Tor_Relais #3
// UID = 0x1 device.type VERS_HIGH VERS_LOW BOARD_NUM

const int PINS_ONBOARD = 8;
//const int PINS_ONBOARD = 12;
const int PINS_ADDONS  = 16;

const int onboard_pins[8] = {0,1,3,4,5,6,7,8};
//const int onboard_pins[12] = {0,1,3,4,5,6,7,8,A3,A2,A1,A0}  // verfügbare On-Board Ausgänge

const uint8_t USE_ONBOARD = 1;       // On-Board Ausgänge benutzen: 0 = Nein; 1 = Ja
const uint8_t ANZ_ACC_ONBOARD = 2;    // Anzahl an Ausgängen Onboard: Default = 4

const uint8_t ANZ_ADDONS  = 4;       // Anzahl AddOn Platinen (max 8)
const uint8_t ANZ_ACC_PER_ADDON = 8; // Anzahl an Ausgängen pro AddOn Platine: Default = 8
                                      // => Motor   = 4
                                      // => Relais  = 8
                                      // => Weichen = 8
                                      // => Weichen mit Lagerückmeldung = 5 (aktuelle AddOn-HW unterstützt dies noch nicht.)
                                      // ==>> evtl. ab AddOn-HW Rev. c

int base_address = 93;
/*---------------------------------------
 => Basisadresse ( x = base_address) für die initiale Adresskonfiguration (see config_own_adresses_manual() )
 => base_address =   1 ===> 1, 2, 3, 4, ...
 => base_address = 101 ===> 101, 102, 103, 104, ...
 ---------------------------------------*/
/******************************************************************************
   Debugging einschalten
 ******************************************************************************/
//#define DEBUG_SERIAL
//#define DEBUG_SETUP
//#define DEBUG_SETUP_ACC
//#define DEBUG_CONFIG
//#define DEBUG_INPUT
//#define DEBUG_MCAN
//#define DEBUG_CAN
//#define DEBUG_ACC
//#define run_fake_acc_commands
int tmp;
#define nolock
/******************************************************************************
   Allgemeine Konstanten:
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
#define TYPE_WEICHESERVO  2
#define TYPE_RELAIS       3
#define TYPE_TORMOTOR    4
#define MOMENT     0
#define DAUER      1
#define TORANTRIEB 2

const int STATUS_LED_PIN = 9;
const int REG_UID        =  5;
const int REG_PROT       =  9;         // Protokoll - Global für alle ACC gleich.
const int REG_CONFIG     = 10;
const int REG_SWITCHMODE = 13;
const int REG_SWITCHTIME = 14;
const String string1 = "Adresse Ausgang ";
const String string2 = "_1_2048";
String string3;

int start_adrs_channel = 3;

typedef struct {
  uint16_t locID;       // LocalID
  uint8_t  Modul;        // 0 = On-Board; 1 = Modul1; 2 = Modul2, ...
  uint8_t  pin_grn;      // PIN Grün
  uint8_t  pin_red;      // PIN Rot
  uint8_t  pin_on;
  uint8_t  pin_off;
  int  reg_locid;       // EEPROM-Register der Local-IDs
  bool state_is  = 0;       // ...
  bool state_set = 0;       // ...
  bool power_is  = 0;   // Strom ein-/aus
  bool power_set = 0;   // Strom ein-/ausschalten
  int reg_type;         // EEPROM-Register für die ACC Typen
  uint8_t acc_type;
  #ifdef INDIVIDUAL_SWITCHTIME
    unsigned long active  = 0;
  #endif
} acc_Magnet;

const uint8_t NUM_ACCs = (ANZ_ACC_ONBOARD * USE_ONBOARD) + (ANZ_ADDONS * ANZ_ACC_PER_ADDON);

acc_Magnet acc_articles[ NUM_ACCs ]; // = 4 * USE_ONBOARD + ANZ_ACC_PER_ADDON * ANZ_ADDONS

Adafruit_MCP23017 AddOn[ANZ_ADDONS];   // Create AddOn


unsigned long previousMillis  = 0;
unsigned long currentMillis   = 0;
unsigned long interval        = 1000;
bool          state_LED       = OFF;

/******************************************************************************
 * Variablen der Magnetartikel:
 ******************************************************************************/
uint8_t  CONFIG_NUM;     //Anzahl der Konfigurationspunkte
uint16_t hash;
bool     config_poll  = false;
byte     uid_mat[4];
uint8_t  config_index =   0;
uint16_t switchtime   = 15000;
uint16_t switchtime_other = 250;
uint8_t  switchmode   = TORANTRIEB;
uint16_t prot;
uint16_t prot_old;
bool     locked       = true;

/******************************************************************************
 * Benötigtes:
 ******************************************************************************/
MCAN mcan;
MCANMSG can_frame_out;
MCANMSG can_frame_in;

CanDevice device;

//#############################################################################
// Setup
//#############################################################################
void setup() {
  #ifdef DEBUG_SERIAL
    Serial.begin(111520);
  #endif
  #ifdef DEBUG_SETUP
    Serial.println("-----------------------------------");
    Serial.println(" - Setup                         -");
    Serial.println("-----------------------------------");
  #endif
  uid_mat[0] = byte(UID >> 24);
  uid_mat[1] = byte(UID >> 16);
  uid_mat[2] = byte(UID >> 8);
  uid_mat[3] = byte(UID);

  if( (EEPROM.read(REG_UID  )==uid_mat[0]) &&
      (EEPROM.read(REG_UID+1)==uid_mat[1]) &&
      (EEPROM.read(REG_UID+2)==uid_mat[2]) &&
      (EEPROM.read(REG_UID+3)==uid_mat[3]) ){
    #ifdef DEBUG_SETUP
      Serial.println(" - UID was not changed: No Setup of EEPROM needed.");
    #endif
  } else {
    #ifdef DEBUG_SETUP
      Serial.println(" - UID was changed: Initital Setup of EEPROM");
    #endif
    EEPROM.put(REG_CONFIG, 0);
    EEPROM.put(REG_UID  , uid_mat[0]);
    EEPROM.put(REG_UID+1, uid_mat[1]);
    EEPROM.put(REG_UID+2, uid_mat[2]);
    EEPROM.put(REG_UID+3, uid_mat[3]);
    EEPROM.put(REG_SWITCHMODE, switchmode);
    //EEPROM.put(REG_SWITCHTIME, (switchtime >> 16));
    EEPROM.put(REG_SWITCHTIME+1, byte(switchtime >> 8));
    EEPROM.put(REG_SWITCHTIME+2, byte(switchtime));
    EEPROM.put(REG_PROT, 1);    // 0 = DCC, 1=MM
  }

  pinMode(STATUS_LED_PIN, OUTPUT);
  blink_LED();
  hash = mcan.generateHash(UID);

  //Geräteinformationen:
  device.versHigh = VERS_HIGH;
  device.versLow = VERS_LOW;
  device.hash = hash;
  device.uid = UID;
  device.artNum = "10155";
  device.name = "MCAN TorRelais";
  device.boardNum = BOARD_NUM;
  device.type = 0x0055;

  switchmode = EEPROM.read(REG_SWITCHMODE);
  //switchtime = (EEPROM.read(REG_SWITCHTIME) << 16) | (EEPROM.read(REG_SWITCHTIME+1) << 8) | EEPROM.read(REG_SWITCHTIME+2);
  switchtime =                                       (EEPROM.read(REG_SWITCHTIME+1) << 8) | EEPROM.read(REG_SWITCHTIME+2);

  if(!EEPROM.read(REG_PROT)){
    prot = DCC_ACC;
  } else {
    prot = MM_ACC;
  }
  prot_old = prot;

  CONFIG_NUM = start_adrs_channel + NUM_ACCs - 1;

  setup_acc();

  #ifdef DEBUG_MCAN
    mcan.initMCAN(true);
  #else
    mcan.initMCAN();
  #endif

  #ifdef DEBUG_SETUP
    Serial.println("-----------------------------------");
    Serial.println(" - Setup completed               -");
    Serial.println("-----------------------------------");
    Serial.print(" - Board-UID:  0x");
    Serial.println(UID, HEX);
    Serial.print(" - Board-Num:  ");
    Serial.println(BOARD_NUM);
    Serial.print(" - Config Num: ");
    Serial.println(CONFIG_NUM);
    Serial.print(" - Num ACCs:   ");
    Serial.println(NUM_ACCs);
    Serial.print(" - Switchtime:   ");
    Serial.println(switchtime);
    Serial.print(" - SwitchMode:   ");
    Serial.println(switchmode);
    Serial.println("-----------------------------------");
    Serial.println(" - Device is now ready...        -");
    Serial.println("-----------------------------------");
  #endif

  attachInterrupt(digitalPinToInterrupt(2), interruptFn, LOW);

  blink_LED();
}


//#############################################################################
// Blink Status LED
//#############################################################################
void blink_LED() {
  state_LED = !state_LED;
  digitalWrite(STATUS_LED_PIN, state_LED);
}


//#############################################################################
// Setup Funktion zum Einrichten der ACCs
//#############################################################################
//-----------------------------------------------------------------------------
// alt:      acc_articles[num].adrs_channel = num + start_adrs_channel;
// neu:      acc_articles[num].adrs_channel = (2*num) + start_adrs_channel;
// start = 2
// num = 0
//
// Acc_Num > Config_Index && Config_Indes => Acc_Num
// ( 0 * 2) + 2 =  2       =>  2 / 2 - 1 = 0
//                         =>  3 / 2 - 1 = 0
// ( 1 * 2) + 2 =  4       =>  4 / 2 - 1 = 1
//                         =>  5 / 2 - 1 = 1
// ( 2 * 2) + 2 =  6       =>  6 / 2 - 1 = 2
//                         =>  7 / 2 - 1 = 2
// ( 3 * 2) + 2 =  8       =>  8 / 2 - 1 = 3
//                         =>  9 / 2 - 1 = 3
// ( 4 * 2) + 2 = 10       => 10 / 2 - 1 = 4
//       ...
//                16       => 16 / 2 = 8
//                17       => 17 / 2 = 8
//-----------------------------------------------------------------------------
void setup_acc() {
  //--------------------------------------------------------------------------------------------------
  // START - Setup ACCs
  //--------------------------------------------------------------------------------------------------
  #ifdef DEBUG_SETUP
    Serial.print(millis());
    Serial.println(" - Setting up Accs");
    Serial.println("-----------------------------------");
  #endif
  //--------------------------------------------------------------------------------------------------
  // Load Default Values
  //--------------------------------------------------------------------------------------------------
  if (EEPROM.read(REG_CONFIG) == 0) {
    EEPROM.put(REG_CONFIG, 1);
    #ifdef DEBUG_SETUP
      Serial.print(millis());
      Serial.println(" - ACC Setup not done.");
      Serial.print(millis());
      Serial.println(" - Loading default values.");
    #endif
    config_own_adresses_manual();
  }
  //--------------------------------------------------------------------------------------------------
  // Setup PINs
  //--------------------------------------------------------------------------------------------------
  int num = 0;
  if(USE_ONBOARD == 1){
    int pin = 0;
    for(int i = 0; i < ANZ_ACC_ONBOARD; i++){
      acc_articles[num].Modul = 0;

      acc_articles[num].reg_locid = (20 + (6 * (num + 1)) - 5);
      acc_articles[num].locID     = (EEPROM.read( acc_articles[num].reg_locid ) << 8) | (EEPROM.read( acc_articles[num].reg_locid + 1 ));

      acc_articles[num].reg_type  = (20 + (6 * (num + 1))    );
      acc_articles[num].acc_type  = EEPROM.read( acc_articles[num].reg_type );

      acc_articles[num].pin_grn = onboard_pins[pin];
      pinMode(acc_articles[num].pin_grn, OUTPUT);
      pin++;
      acc_articles[num].pin_red = onboard_pins[pin];
      pinMode(acc_articles[num].pin_red, OUTPUT);
      pin++;

      //#define TYPE_WEICHE       0
      //#define TYPE_ENTKUPPLER   1
      //#define TYPE_SERVO 2
      //#define TYPE_RELAIS       3
      //#define TYPE_TORMOTOR    4
      if ( acc_articles[num].acc_type == TYPE_TORMOTOR ) {
        acc_articles[num].pin_on  = onboard_pins[pin];
        pinMode(acc_articles[num].pin_on,  OUTPUT);
        pin++;
        acc_articles[num].pin_off = onboard_pins[pin];
        pinMode(acc_articles[num].pin_off, OUTPUT);
        pin++;
      }

      #ifdef DEBUG_SETUP_ACC
        Serial.println();
        Serial.print("Setup ACC# ");
        Serial.println(num);
        Serial.print("-> Modul: ");
        Serial.println(acc_articles[num].Modul);
        Serial.print("-> Local-ID: ");
        Serial.println(acc_articles[num].locID);
        Serial.print("-> Adresse: ");
        Serial.println(mcan.getadrs(prot, acc_articles[num].locID));
        Serial.print("-> ACC Type: ");
        Serial.println(acc_articles[num].acc_type);
        Serial.println("-----------------------------------");
      #endif
      num++;

    }
  }

  if(ANZ_ADDONS > 0){
    for(int m = 0; m < ANZ_ADDONS; m++){
      AddOn[m].begin(m);
      int pin = 0;
      for(int i = 0; i < ANZ_ACC_PER_ADDON; i++){
        acc_articles[num].Modul = m + 1;

        acc_articles[num].reg_locid = (20 + (6 * (num + 1)) - 5);
        acc_articles[num].locID     = (EEPROM.read( acc_articles[num].reg_locid ) << 8) | (EEPROM.read( acc_articles[num].reg_locid + 1 ));

        acc_articles[num].reg_type  = (20 + (6 * (num + 1))    );
        acc_articles[num].acc_type  = EEPROM.read( acc_articles[num].reg_type );

        acc_articles[num].pin_grn = pin;
        AddOn[m].pinMode(acc_articles[num].pin_grn, OUTPUT);
        pin++;
        acc_articles[num].pin_red = pin;
        AddOn[m].pinMode(acc_articles[num].pin_red, OUTPUT);
        pin++;

        //#define TYPE_WEICHE       0
        //#define TYPE_ENTKUPPLER   1
        //#define TYPE_SERVO 2
        //#define TYPE_RELAIS       3
        //#define TYPE_TORMOTOR    4
        if ( acc_articles[num].acc_type == TYPE_TORMOTOR ) {
          acc_articles[num].pin_on  = pin;
          AddOn[m].pinMode(acc_articles[num].pin_on,  OUTPUT);
          pin++;
          acc_articles[num].pin_off = pin;
          AddOn[m].pinMode(acc_articles[num].pin_off, OUTPUT);
          pin++;
        }

        #ifdef DEBUG_SETUP_ACC
          Serial.println();
          Serial.print("Setup ACC# ");
          Serial.println(num);
          Serial.print("-> Modul: ");
          Serial.println(acc_articles[num].Modul);
          Serial.print("-> Local-ID: ");
          Serial.println(acc_articles[num].locID);
          Serial.print("-> Adresse: ");
          Serial.println(mcan.getadrs(prot, acc_articles[num].locID));
          Serial.print("-> ACC Type: ");
          Serial.println(acc_articles[num].acc_type);
          Serial.println("-----------------------------------");
        #endif
        num++;
      }
    }
  }
  #ifdef DEBUG_SETUP
    Serial.print(millis());
    Serial.println(" - ...completed");
  #endif
  signal_setup_successfull();

}

//#############################################################################
// Manuelles configurieren der Adressen, wenn sich UID ändert.
//#############################################################################
void config_own_adresses_manual() {
  uint16_t adrsss;
  for (int i = 0; i < NUM_ACCs; i++) {
    acc_articles[i].reg_locid = (20 + (6 * (i + 1)) - 5);
    acc_articles[i].reg_type  = (20 + (6 * (i + 1))    );

    switch (i) {
      case 0:
        acc_articles[i].acc_type = TYPE_TORMOTOR;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 1:
        acc_articles[i].acc_type = TYPE_TORMOTOR;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 2:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 3:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 4:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 5:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 6:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 7:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 8:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 9:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 10:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 11:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 12:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 13:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 14:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 15:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 16:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 17:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 18:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 19:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 20:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 21:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 22:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 23:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 24:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 25:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 26:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 27:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 28:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 29:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 30:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      case 31:
        acc_articles[i].acc_type = TYPE_RELAIS;   // 0 = Weiche/Signal ; 1 = Entkuppler
        adrsss = base_address + i;
        break;
      default:
        acc_articles[i].acc_type = TYPE_RELAIS;
        adrsss = base_address + i;
        break;
    }

    acc_articles[i].locID = mcan.generateLocId(prot, adrsss);

    byte locid_high = acc_articles[i].locID >> 8;
    byte locid_low = acc_articles[i].locID;

    EEPROM.put(acc_articles[i].reg_locid, locid_high);
    EEPROM.put(acc_articles[i].reg_locid + 1, locid_low);
    EEPROM.put(acc_articles[i].reg_type, acc_articles[i].acc_type);

    acc_articles[i].power_is = 0;
    acc_articles[i].power_set = acc_articles[i].power_is;
  }

}


//#############################################################################
// Erfolgreiches Setup signalisieren
//#############################################################################
void signal_setup_successfull() {
  for (int i = 0; i < 20; i++) {
    blink_LED();
    delay(100);
  }
}


//#############################################################################
// Change LocalID when protocol has changed
//#############################################################################
void change_prot() {
  #ifdef DEBUG_CONFIG
    Serial.print(millis());
    Serial.print("Changing protocol from ");
    if (prot_old == DCC_ACC ) {
      Serial.print("DCC to ");
    } else {
      Serial.print("MM to ");
    }
    if (prot == DCC_ACC ) {
      Serial.println("DCC");
    } else {
      Serial.println("MM");
    }
  #endif
  for(int i = 0; i < NUM_ACCs; i++) {
    acc_articles[i].locID = (EEPROM.read( acc_articles[i].reg_locid ) << 8) | (EEPROM.read( acc_articles[i].reg_locid + 1 ));
    uint16_t adrsss = mcan.getadrs(prot_old, acc_articles[ config_index - start_adrs_channel ].locID);
    acc_articles[i].locID = mcan.generateLocId(prot, adrsss );
    byte locid_high = acc_articles[i].locID >> 8;
    byte locid_low = acc_articles[i].locID;
    EEPROM.put(acc_articles[i].reg_locid, locid_high);
    EEPROM.put(acc_articles[i].reg_locid + 1, locid_low);
  }
}


//#############################################################################
// Ausführen, wenn eine Nachricht verfügbar ist.
// Nachricht wird geladen und anhängig vom CAN-Befehl verarbeitet.
//#############################################################################
void interruptFn() {
  can_frame_in = mcan.getCanFrame();
  incomingFrame();
}


//#############################################################################
// Eingehene CAN Frame überprüfen
//#############################################################################
void incomingFrame() {
  //==================================================================================================
  // Frames ohne UID
  //==================================================================================================
  if ((can_frame_in.cmd == PING) && (can_frame_in.resp_bit == 0)) {
    //------------------------------------------------------------------------------------------------
    // START - PING Frame           - Auf Ping Request antworten
    //------------------------------------------------------------------------------------------------
    #ifdef DEBUG_CAN
      Serial.print(millis());
      Serial.println(" - Sending ping response.");
    #endif
    mcan.sendPingFrame(device, true);

    //------------------------------------------------------------------------------------------------
    // ENDE - PING Frame
    //------------------------------------------------------------------------------------------------
  } else if ((can_frame_in.cmd == SYS_CMD) && (can_frame_in.resp_bit == 0)) {
      if ( ( (uid_mat[0] == can_frame_in.data[0]) &&
             (uid_mat[1] == can_frame_in.data[1]) &&
             (uid_mat[2] == can_frame_in.data[2]) &&
             (uid_mat[3] == can_frame_in.data[3])
           ) || (
            (0 == can_frame_in.data[0]) &&
            (0 == can_frame_in.data[1]) &&
            (0 == can_frame_in.data[2]) &&
            (0 == can_frame_in.data[3])
          ) ){
      //----------------------------------------------------------------------------------------------
      // START - STOP / GO / HALT     -
      //----------------------------------------------------------------------------------------------
      uint8_t sub_cmd = can_frame_in.data[4];
      if ( (sub_cmd == SYS_STOP) || (sub_cmd == SYS_HALT) ) {
        //--------------------------------------------------------------------------------------------
        // STOP oder HALT             - Eingaben verhindern.
        //--------------------------------------------------------------------------------------------
        #ifdef DEBUG_CAN
          Serial.print(millis());
          Serial.println(" - recieved HALT or STOP -> System locked.");
        #endif
        locked = true;

      } else if (sub_cmd == SYS_GO) {
        //--------------------------------------------------------------------------------------------
        // GO                         - Eingaben zulassen.
        //--------------------------------------------------------------------------------------------
        #ifdef DEBUG_CAN
          Serial.print(millis());
          Serial.println(" - System unlocked.");
        #endif
        locked = false;

      }
    }
    //------------------------------------------------------------------------------------------------
    // ENDE - STOP / GO / HALT
    //------------------------------------------------------------------------------------------------
  } else if ((can_frame_in.cmd == SWITCH_ACC) && (can_frame_in.resp_bit == 0)) {
    //------------------------------------------------------------------------------------------------
    // START - ACC Frame            - Püfen auf Schaltbefehle
    //------------------------------------------------------------------------------------------------
    uint16_t locid = (can_frame_in.data[2] << 8) | can_frame_in.data[3];
    #ifdef DEBUG_CAN
      Serial.print(millis());
      Serial.print(" - Recieved ACC-Frame for ACC: ");
      Serial.println(mcan.getadrs(prot, locid));
    #endif
    for (int i = 0; i < NUM_ACCs; i++) {
      if (locid == acc_articles[i].locID) {
        #ifdef DEBUG_CAN
          Serial.print(millis());
          Serial.println("    => match found -> Set target state.");
        #endif
        acc_articles[i].state_set = can_frame_in.data[4];
        acc_articles[i].power_set = can_frame_in.data[5];
        break;
      }

    }
    //------------------------------------------------------------------------------------------------
    // ENDE - ACC Frame
    //------------------------------------------------------------------------------------------------
/*
  } else if((can_frame_in.cmd == S88_EVENT) && (can_frame_in.resp_bit == 1)){
    // ------------------------------------------------------------------------------------------------
    // START - S88 Frame            - Püfen auf S88 Events
    // ------------------------------------------------------------------------------------------------
    uint16_t incoming_modulID = (can_frame_in.data[0] << 8 | can_frame_in.data[1]);   // bsp. 0x00b3 => 179 (L88)
    uint16_t kontakt = (can_frame_in.data[2] << 8 | can_frame_in.data[3]);            // bsp. 0x03fe => 1022 => Bus 1, Modul 2, Kontakt 6
    //uint8_t old_state = can_frame_in.data[4];                                         // bsp. 0x01   => 1
    uint8_t curr_state = can_frame_in.data[5];                                        // bsp. 0x00   => 0
    //uint16_t s_time = (can_frame_in.data[6] << 8 | can_frame_in.data[7]);             // bsp. 0x0190 => 400
    #ifdef DEBUG_CAN
      Serial.print(millis());
      Serial.print(" - Recieved S88-Frame for modul: ");
      Serial.print(incoming_modulID);
      Serial.print(" / contact: ");
      Serial.println(kontakt);
    #endif
    for(int i = 0; i < NUM_ACCs; i++){
      if ( (incoming_modulID == modulID) && (kontakt == acc_articles[i].kontakt) ) {
        #ifdef DEBUG_CAN
          Serial.println("    => match found -> Set target state.");
        #endif
        acc_articles[i].state_set = curr_state;
        break;
      }
    }
  }
    // ------------------------------------------------------------------------------------------------
    // ENDE - S88 Frame
    // ------------------------------------------------------------------------------------------------
*/
  } else if ((uid_mat[0] == can_frame_in.data[0]) && (uid_mat[1] == can_frame_in.data[1]) && (uid_mat[2] == can_frame_in.data[2]) && (uid_mat[3] == can_frame_in.data[3])) {
  //==================================================================================================
  // Befehle nur für eine UID
  //==================================================================================================
    #ifdef DEBUG_CAN
      Serial.print(millis());
      Serial.println(" - Recieved frame only for me:");
    #endif
    if ((can_frame_in.cmd == CONFIG) && (can_frame_in.resp_bit == 0)) {
      //----------------------------------------------------------------------------------------------
      // START - Config Frame       -
      //----------------------------------------------------------------------------------------------
      #ifdef DEBUG_CAN
        Serial.print(millis());
        Serial.print(" - Recieved config frame.  - INDEX:");
        Serial.println(can_frame_in.data[4]);
      #endif
      config_poll = true;
      config_index = can_frame_in.data[4];
      //----------------------------------------------------------------------------------------------
      // ENDE - Config Frame
      //----------------------------------------------------------------------------------------------
    } else if ((can_frame_in.cmd == SYS_CMD) && (can_frame_in.resp_bit == 0) && (can_frame_in.data[4] == SYS_STAT)) {
      //----------------------------------------------------------------------------------------------
      // START - Status Frame       -
      //----------------------------------------------------------------------------------------------
      if(can_frame_in.data[5] == 1){                          // Protokoll schreiben ( MM oder DCC)
        prot_old = prot;
        if(!can_frame_in.data[7]){
          prot = DCC_ACC;
        }else{
          prot = MM_ACC;
        }
        if(prot != prot_old){
          EEPROM.put(REG_PROT, can_frame_in.data[7]);
          change_prot();
        }
        mcan.statusResponse(device, can_frame_in.data[5]);

      } else if(can_frame_in.data[5] == 2){                   // SWITCHTIME
        EEPROM.put(REG_SWITCHTIME, can_frame_in.data[6]);
        EEPROM.put(REG_SWITCHTIME+1, can_frame_in.data[7]);
        switchtime = (can_frame_in.data[6] << 8) | can_frame_in.data[6];
        mcan.statusResponse(device, can_frame_in.data[5]);

      } else if(can_frame_in.data[5] >= start_adrs_channel){
        //if(can_frame_in.data[5] == acc_articles[can_frame_in.data[5]-start_adrs_channel].adrs_channel)
        {
          acc_articles[can_frame_in.data[5]-start_adrs_channel].locID = mcan.generateLocId(prot, (can_frame_in.data[6] << 8) | can_frame_in.data[7] );
          byte locid_high = acc_articles[can_frame_in.data[5]-start_adrs_channel].locID >> 8;
          byte locid_low = acc_articles[can_frame_in.data[5]-start_adrs_channel].locID;
          EEPROM.put(acc_articles[can_frame_in.data[5]-start_adrs_channel].reg_locid, locid_high);
          EEPROM.put(acc_articles[can_frame_in.data[5]-start_adrs_channel].reg_locid + 1, locid_low);
          mcan.statusResponse(device, can_frame_in.data[5]);

        }
      }
      //----------------------------------------------------------------------------------------------
      // ENDE - Status Frame
      //----------------------------------------------------------------------------------------------
    }
  //==================================================================================================
  // ENDE - Befehle nur für eine UID
  //==================================================================================================
  }

}

//#############################################################################
// Funktion zum schalten der Ausgänge
//#############################################################################
void switchAcc(int acc_num, bool color, bool power, bool save_state=false) {
  /*
   * acc_num    => Number ACC
   * color      => 0 - red, 1 - green
   * save_state => 0 - do not save status, 1 - save status
   * power      => 0 - off, 1 - on
   */
  #ifdef DEBUG_ACC
    Serial.print(millis());
    Serial.print(" -- ACC - Adresse: ");
    Serial.print(mcan.getadrs(prot, acc_articles[acc_num].locID));
    Serial.print(" ");
  #endif

  if ( ( acc_articles[acc_num].acc_type == TYPE_WEICHE ) || ( acc_articles[acc_num].acc_type == TYPE_ENTKUPPLER ) ) {
    switch (color) {
      case RED:
        #ifdef DEBUG_ACC
          Serial.print(" - Weiche/Signal (Moment) (rot|rund)");
          Serial.print(" - Power: ");
          Serial.println(power);
        #endif
        if(acc_articles[acc_num].Modul == 0){
          digitalWrite(acc_articles[acc_num].pin_red, power);
        } else if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
          AddOn[acc_articles[acc_num].Modul].digitalWrite(acc_articles[acc_num].pin_red, power);
        }
        break;
      case GREEN:
        #ifdef DEBUG_ACC
          Serial.print(" - Weiche/Signal (Moment) (grün|gerade)");
          Serial.print(" - Power: ");
          Serial.println(power);
        #endif
        if(acc_articles[acc_num].Modul == 0){
          digitalWrite(acc_articles[acc_num].pin_grn, power);
        } else if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
          AddOn[acc_articles[acc_num].Modul].digitalWrite(acc_articles[acc_num].pin_grn, power);
        }
        break;
    }
  } else if ( acc_articles[acc_num].acc_type == TYPE_RELAIS ) {
    switch (color) {
      case RED:
        #ifdef DEBUG_ACC
          Serial.println(" - Dauerstrom (rot|rund)");
        #endif
        if(acc_articles[acc_num].Modul == 0){
          digitalWrite(acc_articles[acc_num].pin_grn, POWER_OFF);
          digitalWrite(acc_articles[acc_num].pin_red, POWER_ON);
        } else if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
          AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_grn, POWER_OFF);
          AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_red, POWER_ON);
        }
        break;
      case GREEN:
        #ifdef DEBUG_ACC
          Serial.println(" - Dauerstrom (gün|gerade)");
        #endif
        if(acc_articles[acc_num].Modul == 0){
          digitalWrite(acc_articles[acc_num].pin_red, POWER_OFF);
          digitalWrite(acc_articles[acc_num].pin_grn, POWER_ON);
        } else if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
          AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_red, POWER_OFF);
          AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_grn, POWER_ON);
        }
        break;
    }
  } else if ( acc_articles[acc_num].acc_type == TYPE_TORMOTOR ) {
    switch (color) {
      case RED:
        #ifdef DEBUG_ACC
          Serial.print(" - Lokschuppentor schließen");
          Serial.print(" - Power: ");
          Serial.println(power);
        #endif
        if(acc_articles[acc_num].Modul == 0){
          digitalWrite(acc_articles[acc_num].pin_grn, OFF);
          delay(100);
          digitalWrite(acc_articles[acc_num].pin_red, ON);
          delay(250);
          digitalWrite(acc_articles[acc_num].pin_red, OFF);
          delay(100);
        } else if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
          AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_grn, OFF);
          delay(100);
          AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_red, ON);
          delay(250);
          AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_red, OFF);
          delay(100);
        }
        break;
      case GREEN:
        #ifdef DEBUG_ACC
          Serial.print(" - Lokschuppentor öffnen");
          Serial.print(" - Power: ");
          Serial.println(power);
        #endif
        if(acc_articles[acc_num].Modul == 0){
          digitalWrite(acc_articles[acc_num].pin_red, OFF);
          delay(100);
          digitalWrite(acc_articles[acc_num].pin_grn, ON);
          delay(250);
          digitalWrite(acc_articles[acc_num].pin_grn, OFF);
          delay(100);
        } else if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
          AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_red, OFF);
          delay(100);
          AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_grn, ON);
          delay(250);
          AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_grn, OFF);
          delay(100);
        }
        break;
    }
    if (power == ON) {
      if(acc_articles[acc_num].Modul == 0){
        digitalWrite(acc_articles[acc_num].pin_off, OFF);
        delay(100);
        digitalWrite(acc_articles[acc_num].pin_on, ON);
        delay(250);
        digitalWrite(acc_articles[acc_num].pin_on, OFF);
        delay(100);
      } else if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_off, OFF);
        delay(100);
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_on, ON);
        delay(250);
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_on, OFF);
        delay(100);
      }
    } else {
      if(acc_articles[acc_num].Modul == 0){
        digitalWrite(acc_articles[acc_num].pin_on, OFF);
        delay(100);
        digitalWrite(acc_articles[acc_num].pin_off, ON);
        delay(250);
        digitalWrite(acc_articles[acc_num].pin_off, OFF);
        delay(100);
      } else if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_on, OFF);
        delay(100);
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_off, ON);
        delay(250);
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_off, OFF);
        delay(100);
      }
    }
  }

  blink_LED();
  mcan.switchAccResponse(device, acc_articles[acc_num].locID, acc_articles[acc_num].state_is);
  acc_articles[acc_num].state_is = color;
  acc_articles[acc_num].power_set = power;
  acc_articles[acc_num].power_is = acc_articles[acc_num].power_set;
  #ifdef INDIVIDUAL_SWITCHTIME
    if(power == 0) {
      acc_articles[acc_num].active = 0;
    }
    if(power == 1) {
      acc_articles[acc_num].active = millis();
    }
  #endif
  blink_LED();

}


//#############################################################################
// XXXXXXXXXXXXXXXXXXXXXXXX
//#############################################################################



//#############################################################################
// Main loop
//#############################################################################
void loop() {
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    blink_LED();

    if (locked == true) {
      interval = 100;
    } else {
      interval = 1000;
    }

    #ifdef run_fake_acc_commands
      interval = random(1000, 5000);
      tmp = random(0, NUM_ACCs);
      acc_articles[tmp].state_set = !acc_articles[tmp].state_set;
      acc_articles[tmp].power_set = !acc_articles[tmp].power_set;
      #ifdef DEBUG_SERIAL
        Serial.print(currentMillis);
        Serial.print(" - Article: ");
        Serial.print(mcan.getadrs(prot, acc_articles[tmp].locID));
        Serial.print(" - State: ");
        Serial.print(acc_articles[tmp].state_set);
        Serial.print(" - Power: ");
        Serial.println(acc_articles[tmp].power_set);
      #endif
    #endif
  }
  //------------------------------------------------------------------------------------------------
  // Wenn Konfig angefragt keine Abarbeitung von ACC Events oder sonstigem.
  // Grund: Die Abarbeitung der Konfig abfrage, sollte nicht zu lange dauern
  //        Es gehen dabei keine ACC Events verloren.
  //------------------------------------------------------------------------------------------------
  if (config_poll){
    //================================================================================================
    // CONFIG_POLL
    //================================================================================================
    if(config_index == 0) mcan.sendDeviceInfo(device, CONFIG_NUM);
    if(config_index == 1) mcan.sendConfigInfoDropdown(device, 1, 2, EEPROM.read(REG_PROT), "Protokoll_DCC_MM");
    if(config_index == 2) mcan.sendConfigInfoSlider(device, 2, 2500, 25000, switchtime, "Schaltzeit_2500_25000");
    if(config_index >= 3) {
      string3 = string1 + ( config_index - start_adrs_channel + 1 ) + string2;
      uint16_t adrs = mcan.getadrs(prot, acc_articles[ config_index - start_adrs_channel ].locID);
      mcan.sendConfigInfoSlider(device, config_index, 1, 2048, adrs, string3);
    }
    if (config_index == CONFIG_NUM) {
      locked = false;
      #ifdef DEBUG_SERIAL
        Serial.print(currentMillis);
        Serial.print(" - CONFIG SENT");
        Serial.println(" - System unlocked");
      #endif
    }
    config_poll = false;

    //================================================================================================
    // ENDE - CONFIG_POLL
    //================================================================================================
#ifdef nolock
  } else {
#else
  } else if (!locked) {   // only as Input
#endif
    //================================================================================================
    // STELL SCHLEIFE
    //================================================================================================
    uint16_t switchtime_tmp = 250;
    for(int i = 0; i < NUM_ACCs; i++){
      if (acc_articles[i].acc_type == TYPE_TORMOTOR ) {
        switchtime_tmp = switchtime;
      } else {
        switchtime_tmp = switchtime_other;
      }
      if ( (acc_articles[i].state_is != acc_articles[i].state_set) || (acc_articles[i].power_is != acc_articles[i].power_set) ) {
        #ifdef DEBUG_ACC
          Serial.print(millis());
          Serial.print(" - Switching: ");
          Serial.print("state_is: ");
          Serial.print(acc_articles[i].state_is);
          Serial.print(" => state_set: ");
          Serial.println(acc_articles[i].state_set);
          Serial.print(millis());
          Serial.print(" - Power: ");
          Serial.print("power_is: ");
          Serial.print(acc_articles[i].power_is);
          Serial.print(" => power_set: ");
          Serial.println(acc_articles[i].power_set);
        #endif
        switchAcc(i, acc_articles[i].state_set, acc_articles[i].power_set);
      #ifdef INDIVIDUAL_SWITCHTIME
      } else if ( (acc_articles[i].power_is == 1 ) && (millis() - acc_articles[i].active >= switchtime_tmp) ){
      #else
      } else if ( (acc_articles[i].power_is == 1 ) && (millis() - previousMillis >= switchtime_tmp) ){
      #endif
        #ifdef DEBUG_ACC
          Serial.print(millis());
          Serial.print(" - Power is still on.");
          Serial.println(" WE NEED TO DO SOMETHING !!!!" );
        #endif
        switchAcc(i, acc_articles[i].state_is, POWER_OFF);
        //acc_articles[i].power_set = POWER_OFF;
      }
    }
    //================================================================================================
    // ENDE - STELL SCHLEIFE
    //================================================================================================
  }
//  #ifndef INDIVIDUAL_SWITCHTIME
//    if (currentMillis - previousMillis >= switchtime) {
//      previousMillis = currentMillis;
//    }
//  #endif
//  delay(100);
}


//#############################################################################
// ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE
//#############################################################################

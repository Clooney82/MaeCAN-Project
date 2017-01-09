/*
 * MäCAN-Stellpult AddOn, Software-Version 0.3
 *
 *  Created by Maximilian Goldschmidt <maxigoldschmidt@gmail.com>
 *  Modified by Jochen Kielkopf.
 *  Do with this whatever you want, but keep thes Header and tell
 *  the others what you changed!
 *
 *  Last edited: 2017-01-09
 */

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <MCAN.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_MCP23017.h>

/******************************************************************************
 * EEPROM-Register
 * # 0 - 20 -> CONFIG:
 * ------------------------------------------------------------------------
 *       0 -> Initial Config ( 0 = unkonfiguriert, 1 = konfiguriert )
 *  1 -  4 -> UID
 *       5 -> SWITCHMODE
 *  6 -  7 -> SWITCHTIME
 *       8 -> FEEDBACK
 *       9 -> PROTOKOLL
 * 10 - 19 -> reserved / free
 *      20 -> Anzahl ACCs
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
 * LocalID = (20+(6*i)-5) (20+(6*i)-4)    - acc_articles[num].reg_locid = (20+(6*(num+1))-5);
 * Adresse = (20+(6*i)-3) (20+(6*i)-2)    - not used
 * Status  = (20+(6*i)-1)                 - acc_articles[num].reg_state = (20+(6*(num+1))-1);
 * Typ     = (20+(6*i))                   - acc_articles[num].reg_type  = (20+(6*(num+1)));
 * ========================================================================
 ******************************************************************************/
#define VERS_HIGH 0       // Versionsnummer vor dem Punkt
#define VERS_LOW  3       // Versionsnummer nach dem Punkt
/******************************************************************************
 * Allgemeine Setup Daten:
 ******************************************************************************/
#define BOARD_NUM 1       // Identifikationsnummer des Boards (Anzeige in der CS2)
#define UID 0x10053075    // CAN-UID (default) - please change!!!

const uint8_t USE_ONBOARD = 0;      // On-Board Ausgänge benutzen: 0 = Nein; 1 = Ja
                                    // On-Board currently not supported
const uint8_t ANZ_ADDONS = 6;       // Anzahl AddOn Platinen (max 8)
                                    // => Ohne LED_FEEDBACK: ab > 7 AddOns wird der Arbeitsspeicher knapp
#define LED_FEEDBACK                 //

#ifdef LED_FEEDBACK
  const uint8_t ANZ_ACC_PER_ADDON = 4; // Anzahl an Ausgängen pro AddOn Platine: Default = 4
#else
  const uint8_t ANZ_ACC_PER_ADDON = 8; // Anzahl an Ausgängen pro AddOn Platine: Default = 8
#endif
                               // => Stellpult mit LED Anzeige = 4
                               // => Stellpult ohne LED Anzeige = 8
                               // => Relais = 8
                               // => Weichen = 8
                               // => Weichen mit Lagerückmeldung = 5 (aktuelle AddOn-HW unterstützt dies noch nicht.)
                               // ==>> evtl. ab AddOn-HW Rev. c

int base_address = 0;          // => Basisadresse ( x = base_address + 1 ) für die initiale Adresskonfiguration (see config_own_adresses_manual() )
                               // => base_address =   0 ===> 1, 2, 3, 4, ...
                               // => base_address = 100 ===> 101, 102, 103, 104, ...

/******************************************************************************
 * Debugging einschalten
 ******************************************************************************/
//#define DEBUG

/******************************************************************************
* Allgemeine Konstanten:
******************************************************************************/
#define RED   0
#define GREEN 1
#define POWER_ON  1
#define POWER_OFF 0
#define BUTTON_PRESSED     1
#define BUTTON_NOT_PRESSED 0
const int REG_PROT = 9;     // Speicheradress für Protokoll - Global für alle ACC gleich.
String string1 = "Adresse Ausgang ";
String string2 = "_1_2048";
String string3;

int start_adrs_channel = 2;

typedef struct {
  uint16_t locID;       // LocalID
  uint8_t Modul;        // 0 = On-Board; 1 = Modul1; 2 = Modul2, ...
  uint8_t pin_grn;      // PIN Grün
  uint8_t pin_red;      // PIN Rot
  int reg_locid;        // EEPROM-Register der Local-IDs
  int adrs_channel;     // Konfigkanäle für die Adressen
  bool pushed_red=0;
  bool pushed_grn=0;
  #ifdef LED_FEEDBACK
    uint8_t pin_led_grn;  // PIN Grün
    uint8_t pin_led_red;  // PIN Rot
    int reg_state;        // EEPROM-Register zum Speichern des Schaltzustand
    bool state_is;        // ...
    bool state_set;       // ...
    bool power_is=0;
    bool power_set=0;     // Strom ein-/ausschalten
  #endif
  bool acc_type=0;      // 0 = Weiche/Signal ; 1 = Entkuppler
  int reg_type;         // EEPROM-Register für die ACC Typen
} acc_Magnet;

acc_Magnet acc_articles[ (4 * USE_ONBOARD) + (ANZ_ADDONS * ANZ_ACC_PER_ADDON) ]; // = 4 * USE_ONBOARD + ANZ_ACC_PER_ADDON * ANZ_ADDONS

Adafruit_MCP23017 AddOn[ANZ_ADDONS];   // Create AddOn

uint8_t NUM_ACCs;

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
const long interval = 1000;
bool state_LED = false;

/******************************************************************************
 * Variablen der Magnetartikel:
 ******************************************************************************/
uint8_t CONFIG_NUM;     //Anzahl der Konfigurationspunkte
uint16_t hash;
bool config_poll = false;
byte uid_mat[4];
uint8_t  config_index = 0;
uint16_t prot;
uint16_t prot_old;
bool pushed_red;
bool pushed_grn;

/******************************************************************************
 * Benötigtes:
 ******************************************************************************/
MCAN mcan;
MCANMSG can_frame_out;
MCANMSG can_frame_in;

CanDevice device;

/******************************************************************************
 * Setup
 ******************************************************************************/
void setup() {
  #ifdef DEBUG
    Serial.begin(9600);
  #endif
  uid_mat[0] = UID >> 24;
  uid_mat[1] = UID >> 16;
  uid_mat[2] = UID >> 8;
  uid_mat[3] = UID;

  if( (EEPROM.read(1)==uid_mat[0]) && (EEPROM.read(2)==uid_mat[1]) && (EEPROM.read(3)==uid_mat[2]) && (EEPROM.read(4)==uid_mat[3]) ){
  } else {
    #ifdef DEBUG
      Serial.print("Initital Setup of EEPROM");
    #endif
    EEPROM.put(0, 0);
    EEPROM.put(1, uid_mat[0]);
    EEPROM.put(2, uid_mat[1]);
    EEPROM.put(3, uid_mat[2]);
    EEPROM.put(4, uid_mat[3]);
    EEPROM.put(REG_PROT, 1);    // 0 = DCC, 1=MM
    #ifdef DEBUG
      Serial.println("...completed.");
    #endif
  }

  pinMode(9, OUTPUT);
  digitalWrite(9,state_LED);
  hash = mcan.generateHash(UID);

  //Geräteinformationen:
  device.versHigh = VERS_HIGH;
  device.versLow = VERS_LOW;
  device.hash = hash;
  device.uid = UID;
  device.artNum = "MäCAN-0053";
  device.name = "MäCAN Stellpult";
  device.boardNum = BOARD_NUM;
  device.type = MCAN_STELLPULT;

  if(!EEPROM.read(REG_PROT)){
    prot = DCC_ACC;
  }else{
    prot = MM_ACC;
  }
  prot_old = prot;

  NUM_ACCs = ANZ_ACC_PER_ADDON * ANZ_ADDONS;
  #ifdef DEBUG
    Serial.print("Num ACCs: ");
    Serial.println(NUM_ACCs);
  #endif
  if(USE_ONBOARD == 1){
    NUM_ACCs = NUM_ACCs + 2;
  }
  #ifdef DEBUG
    Serial.print("Num ACCs: ");
    Serial.println(NUM_ACCs);
  #endif
  CONFIG_NUM = start_adrs_channel + ( 2 * NUM_ACCs ) - 1;
  #ifdef DEBUG
    Serial.print("Config Num: ");
    Serial.println(CONFIG_NUM);
  #endif

  setup_acc();
  #ifdef LED_FEEDBACK
    test_leds();
  #endif
  
  #ifdef DEBUG
    Serial.print("Initial CAN-Bus");
  #endif
  mcan.initMCAN(true);
  attachInterrupt(digitalPinToInterrupt(2), interruptFn, LOW);
  #ifdef DEBUG
    Serial.println("...completed.");
  #endif

#ifdef LED_FEEDBACK
  #ifdef DEBUG
    Serial.print("Restoring previous Acc states.");
  #endif
  restore_last_state();
  #ifdef DEBUG
    Serial.println(" ...  completed.");
  #endif
#endif
  
  state_LED = 1;
  digitalWrite(9,state_LED);
  #ifdef DEBUG
    Serial.println("Setup completed.");
    Serial.println("Device is now ready...");
    Serial.println("-----------------------------------");
  #endif

}

/*
 * Setup Funktion zum Einrichten der ACCs
 */
void setup_acc() {
  #ifdef DEBUG
    Serial.print("Setting up Accs");
  #endif
  // setup mainboard pins
  int num = 0;
  if(USE_ONBOARD == 1){
    acc_articles[num].Modul = 0;
    acc_articles[num].pin_grn = 0;
    acc_articles[num].pin_red = 1;
    acc_articles[num].reg_locid = (20+(6*(num+1))-5);
    acc_articles[num].locID = (EEPROM.read( acc_articles[num].reg_locid ) << 8) | (EEPROM.read( acc_articles[num].reg_locid + 1 ));
    acc_articles[num].adrs_channel = num + start_adrs_channel;
    #ifdef LED_FEEDBACK
      acc_articles[num].reg_state = (20+(6*(num+1))-1);
      acc_articles[num].state_is = EEPROM.read( acc_articles[num].reg_state );
      acc_articles[num].state_set = acc_articles[num].state_is;
    #endif
    acc_articles[num].reg_type = (20+(6*(num+1)));
    acc_articles[num].acc_type = EEPROM.read( acc_articles[num].reg_type );
    #ifdef DEBUG2
      Serial.println();
      Serial.print("Setup ACC# ");
      Serial.println(num);
      Serial.print("-> Modul: ");
      Serial.println(acc_articles[num].Modul);
      Serial.print("-> Local-ID: ");
      Serial.println(acc_articles[num].locID);
      Serial.print("-> Adresse: ");
      Serial.println(mcan.getadrs(prot, acc_articles[num].locID));
      Serial.print("-> Pin GREEN: ");
      Serial.println(acc_articles[num].pin_grn);
      Serial.print("-> Pin RED  : ");
      Serial.println(acc_articles[num].pin_red);
    #endif
    num++;
    acc_articles[num].Modul = 0;
    acc_articles[num].pin_grn = 3;
    acc_articles[num].pin_red = 4;
    acc_articles[num].reg_locid = (20+(6*(num+1))-5);
    acc_articles[num].locID = (EEPROM.read( acc_articles[num].reg_locid ) << 8) | (EEPROM.read( acc_articles[num].reg_locid + 1 ));
    acc_articles[num].adrs_channel = num + start_adrs_channel;
    #ifdef LED_FEEDBACK
      acc_articles[num].reg_state = (20+(6*(num+1))-1);
      acc_articles[num].state_is = EEPROM.read( acc_articles[num].reg_state );
      acc_articles[num].state_set = acc_articles[num].state_is;
    #endif
    acc_articles[num].reg_type = (20+(6*(num+1)));
    acc_articles[num].acc_type = EEPROM.read( acc_articles[num].reg_type );
    #ifdef DEBUG2
      Serial.println();
      Serial.print("Setup ACC# ");
      Serial.println(num);
      Serial.print("-> Modul: ");
      Serial.println(acc_articles[num].Modul);
      Serial.print("-> Local-ID: ");
      Serial.println(acc_articles[num].locID);
      Serial.print("-> Adresse: ");
      Serial.println(mcan.getadrs(prot, acc_articles[num].locID));
      Serial.print("-> Pin GREEN: ");
      Serial.println(acc_articles[num].pin_grn);
      Serial.print("-> Pin RED  : ");
      Serial.println(acc_articles[num].pin_red);
    #endif
    num++;
    acc_articles[num].Modul = 0;
    acc_articles[num].pin_grn = 5;
    acc_articles[num].pin_red = 6;
    acc_articles[num].reg_locid = (20+(6*(num+1))-5);
    acc_articles[num].locID = (EEPROM.read( acc_articles[num].reg_locid ) << 8) | (EEPROM.read( acc_articles[num].reg_locid + 1 ));
    acc_articles[num].adrs_channel = num + start_adrs_channel;
    #ifdef LED_FEEDBACK
      acc_articles[num].reg_state = (20+(6*(num+1))-1);
      acc_articles[num].state_is = EEPROM.read( acc_articles[num].reg_state );
      acc_articles[num].state_set = acc_articles[num].state_is;
    #endif
    acc_articles[num].reg_type = (20+(6*(num+1)));
    acc_articles[num].acc_type = EEPROM.read( acc_articles[num].reg_type );
    #ifdef DEBUG2
      Serial.println();
      Serial.print("Setup ACC# ");
      Serial.println(num);
      Serial.print("-> Modul: ");
      Serial.println(acc_articles[num].Modul);
      Serial.print("-> Local-ID: ");
      Serial.println(acc_articles[num].locID);
      Serial.print("-> Adresse: ");
      Serial.println(mcan.getadrs(prot, acc_articles[num].locID));
      Serial.print("-> Pin GREEN: ");
      Serial.println(acc_articles[num].pin_grn);
      Serial.print("-> Pin RED  : ");
      Serial.println(acc_articles[num].pin_red);
    #endif
    num++;
    acc_articles[num].Modul = 0;
    acc_articles[num].pin_grn = 7;
    acc_articles[num].pin_red = 8;
    acc_articles[num].reg_locid = (20+(6*(num+1))-5);
    acc_articles[num].locID = (EEPROM.read( acc_articles[num].reg_locid ) << 8) | (EEPROM.read( acc_articles[num].reg_locid + 1 ));
    acc_articles[num].adrs_channel = num + start_adrs_channel;
    #ifdef LED_FEEDBACK
      acc_articles[num].reg_state = (20+(6*(num+1))-1);
      acc_articles[num].state_is = EEPROM.read( acc_articles[num].reg_state );
      acc_articles[num].state_set = acc_articles[num].state_is;
    #endif
    acc_articles[num].reg_type = (20+(6*(num+1)));
    acc_articles[num].acc_type = EEPROM.read( acc_articles[num].reg_type );
    #ifdef DEBUG2
      Serial.println();
      Serial.print("Setup ACC# ");
      Serial.println(num);
      Serial.print("-> Modul: ");
      Serial.println(acc_articles[num].Modul);
      Serial.print("-> Local-ID: ");
      Serial.println(acc_articles[num].locID);
      Serial.print("-> Adresse: ");
      Serial.println(mcan.getadrs(prot, acc_articles[num].locID));
      Serial.print("-> Pin GREEN: ");
      Serial.println(acc_articles[num].pin_grn);
      Serial.print("-> PIN RED: ");
      Serial.println(acc_articles[num].pin_red);
    #endif
    num++;
    for(int i = 0; i < 4; i++){
      pinMode(acc_articles[i].pin_red, INPUT);
      digitalWrite(acc_articles[i].pin_red, HIGH);   // Activate Internal Pull-Up Resistor
      pinMode(acc_articles[i].pin_grn, INPUT);
      digitalWrite(acc_articles[i].pin_grn, HIGH);   // Activate Internal Pull-Up Resistor
    }

  }

  if(ANZ_ADDONS > 0){
    for(int m = 0; m < ANZ_ADDONS; m++){
      AddOn[m].begin(m);
      int pin = 0;
      for(int i = 0; i < ANZ_ACC_PER_ADDON; i++){
        acc_articles[num].Modul = m + 1;
        acc_articles[num].pin_grn = pin;
        #ifdef LED_FEEDBACK
          acc_articles[num].pin_led_grn = pin + 8;
        #endif
        pin++;
        acc_articles[num].pin_red = pin;
        #ifdef LED_FEEDBACK
          acc_articles[num].pin_led_red = pin + 8;
        #endif
        pin++;

        AddOn[m].pinMode(acc_articles[num].pin_grn, INPUT);
        AddOn[m].pullUp(acc_articles[num].pin_grn, HIGH);   // Activate Internal Pull-Up Resistor
        AddOn[m].pinMode(acc_articles[num].pin_red, INPUT);
        AddOn[m].pullUp(acc_articles[num].pin_red, HIGH);   // Activate Internal Pull-Up Resistor
        #ifdef LED_FEEDBACK
          AddOn[m].pinMode(acc_articles[num].pin_led_grn, OUTPUT);
          AddOn[m].pinMode(acc_articles[num].pin_led_red, OUTPUT);
        #endif

        acc_articles[num].reg_locid = (20+(6*(num+1))-5);
        acc_articles[num].locID = (EEPROM.read( acc_articles[num].reg_locid ) << 8) | (EEPROM.read( acc_articles[num].reg_locid + 1 ));
        acc_articles[num].adrs_channel = num + start_adrs_channel;
        #ifdef LED_FEEDBACK
          acc_articles[num].reg_state = (20+(6*(num+1))-1);
          acc_articles[num].state_is = EEPROM.read( acc_articles[num].reg_state );
          acc_articles[num].state_set = acc_articles[num].state_is;
        #endif
        acc_articles[num].reg_type = (20+(6*(num+1)));
        acc_articles[num].acc_type = EEPROM.read( acc_articles[num].reg_type );

        #ifdef DEBUG2
          Serial.println();
          Serial.print("Setup ACC# ");
          Serial.println(num);
          Serial.print("-> Modul: ");
          Serial.println(acc_articles[num].Modul);
          Serial.print("-> Local-ID: ");
          Serial.println(acc_articles[num].locID);
          Serial.print("-> Adresse: ");
          Serial.println(mcan.getadrs(prot, acc_articles[num].locID));
          Serial.print("-> Pin GREEN: ");
          Serial.println(acc_articles[num].pin_grn);
          Serial.print("-> Pin RED  : ");
          Serial.println(acc_articles[num].pin_red);
          Serial.print("-> Pin GREEN LED: ");
          Serial.println(acc_articles[num].pin_grn);
          Serial.print("-> Pin RED   LED: ");
          Serial.println(acc_articles[num].pin_red);
        #endif


        num++;
      }
    }
  }

  if (EEPROM.read(0) == 0) {
    EEPROM.put(0, 1);
    for(int i = 0; i < NUM_ACCs; i++) {
      /*
      uint16_t adrsss = i + 1;
      acc_articles[i].locID = mcan.generateLocId(prot, adrsss );
      byte locid_high = acc_articles[i].locID >> 8;
      byte locid_low = acc_articles[i].locID;
      EEPROM.put(acc_articles[i].reg_locid, locid_high);
      EEPROM.put(acc_articles[i].reg_locid + 1, locid_low);
      */
      config_own_adresses_manual();
    }
  }
  #ifdef DEBUG
    Serial.println("...completed");
  #endif
  signal_setup_successfull();

}

void config_own_adresses_manual(){
  uint16_t adrsss;
  for(int i = 0; i < NUM_ACCs; i++) {
    switch (i) {
    case 0:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 1:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 2:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 3:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 4:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 5:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 6:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 7:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 8:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 9:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 10:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 11:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 12:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 13:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 14:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 15:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 16:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 17:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 18:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 19:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 20:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 21:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 22:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 23:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 24:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 25:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 26:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 27:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler 
      adrsss = base_address + i;
      break;
    case 28:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler
      adrsss = base_address + i;
      break;
    case 29:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler
      adrsss = base_address + i;
      break;
    case 30:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler
      adrsss = base_address + i;
      break;
    case 31:
      acc_articles[i].acc_type = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler
      adrsss = base_address + i;
      break;
    }
    acc_articles[i].locID = mcan.generateLocId(prot, adrsss );
    byte locid_high = acc_articles[i].locID >> 8;
    byte locid_low = acc_articles[i].locID;
    EEPROM.put(acc_articles[i].reg_locid, locid_high);
    EEPROM.put(acc_articles[i].reg_locid + 1, locid_low);
    EEPROM.put(acc_articles[i].reg_type, acc_articles[i].acc_type);
  }
  
}

/*
 *
 */
#ifdef LED_FEEDBACK
void test_leds(){
  for (int i = 0; i < NUM_ACCs; i++) {
    if (acc_articles[i].Modul == 0) {
      digitalWrite(acc_articles[i].pin_led_red, 1);
      digitalWrite(acc_articles[i].pin_led_grn, 1);
    } else if ( (acc_articles[i].Modul > 0) && (acc_articles[i].Modul <= 8) ) {
      AddOn[acc_articles[i].Modul - 1].digitalWrite(acc_articles[i].pin_led_red, 1);
      AddOn[acc_articles[i].Modul - 1].digitalWrite(acc_articles[i].pin_led_grn, 1);
    }
  }
  delay(10000);
  for (int i = 0; i < NUM_ACCs; i++) {
    if (acc_articles[i].Modul == 0) {
      digitalWrite(acc_articles[i].pin_led_red, 0);
      digitalWrite(acc_articles[i].pin_led_grn, 0);
    } else if ( (acc_articles[i].Modul > 0) && (acc_articles[i].Modul <= 8) ) {
      AddOn[acc_articles[i].Modul - 1].digitalWrite(acc_articles[i].pin_led_red, 0);
      AddOn[acc_articles[i].Modul - 1].digitalWrite(acc_articles[i].pin_led_grn, 0);
    }
  }

}
#endif

/*
 * 
 */
#ifdef LED_FEEDBACK
void restore_last_state() {
  for(int i = 0; i < NUM_ACCs; i++){
    if(acc_articles[i].Modul > 0){
      if ( acc_articles[i].acc_type == 1 ) {
        AddOn[acc_articles[i].Modul-1].digitalWrite(acc_articles[i].pin_led_red, LOW);
        AddOn[acc_articles[i].Modul-1].digitalWrite(acc_articles[i].pin_led_grn, LOW);
      } else {
        switch (acc_articles[i].state_is) {
          case 0:   // grün
              AddOn[acc_articles[i].Modul-1].digitalWrite(acc_articles[i].pin_led_red, HIGH);
              AddOn[acc_articles[i].Modul-1].digitalWrite(acc_articles[i].pin_led_grn, LOW);
            break;
          case 1:   // rot
              AddOn[acc_articles[i].Modul-1].digitalWrite(acc_articles[i].pin_led_red, LOW);
              AddOn[acc_articles[i].Modul-1].digitalWrite(acc_articles[i].pin_led_grn, HIGH);
            break;
        }
      }
    }
  }
}
#endif

/*
 * Change LocalID when protocol has changed
 */
void change_prot(){
  for(int i = 0; i < NUM_ACCs; i++) {
    acc_articles[i].locID = (EEPROM.read( acc_articles[i].reg_locid ) << 8) | (EEPROM.read( acc_articles[i].reg_locid + 1 ));
    uint16_t adrsss = mcan.getadrs(prot_old, acc_articles[i].locID);
    acc_articles[i].locID = mcan.generateLocId(prot, adrsss );
    byte locid_high = acc_articles[i].locID >> 8;
    byte locid_low = acc_articles[i].locID;
    EEPROM.put(acc_articles[i].reg_locid, locid_high);
    EEPROM.put(acc_articles[i].reg_locid + 1, locid_low);
  }
  signal_setup_successfull();

}

/*
 *  Erfolgreiches Setup signalisieren
 */
void signal_setup_successfull(){
  for(int i=0; i < 20; i++){
    state_LED = !state_LED;
    digitalWrite(9,state_LED);
    delay(100);
  }
}


/*
 * Funktion zum schalten der Ausgänge
 */
#ifdef LED_FEEDBACK
void switchAcc(int acc_num, bool set_state){
  if(!set_state){                   // rot
    if(acc_articles[acc_num].Modul == 0){
      if(acc_articles[acc_num].acc_type == 0) {
        digitalWrite(acc_articles[acc_num].pin_led_grn, LOW);
        digitalWrite(9,0);
        delay(20);
        #ifdef DEBUG
            Serial.print("Green: off / ");
        #endif
      }
      digitalWrite(acc_articles[acc_num].pin_led_red, HIGH);
      digitalWrite(9,1);
        #ifdef DEBUG
            Serial.println("Red: on");
        #endif
    } else if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
      int pin = acc_articles[acc_num].pin_led_grn;
      if(acc_articles[acc_num].acc_type == 0) {
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(pin, LOW);
        digitalWrite(9,0);
        delay(20);
        #ifdef DEBUG
            Serial.print("Green: off / ");
        #endif
      }
      pin = acc_articles[acc_num].pin_led_red;
      AddOn[acc_articles[acc_num].Modul-1].digitalWrite(pin, HIGH);
      digitalWrite(9,1);
        #ifdef DEBUG
            Serial.println("Red: on");
        #endif
    }
    #ifdef DEBUG
      Serial.print("Switching ACC-No. ");
      Serial.print(mcan.getadrs(prot, acc_articles[acc_num].locID));
      Serial.print(" on modul ");
      Serial.print(acc_articles[acc_num].Modul);
      Serial.println(" to RED.");
    #endif
  } else if(set_state){             // grün
    if(acc_articles[acc_num].Modul == 0){
      if(acc_articles[acc_num].acc_type == 0) {
        digitalWrite(acc_articles[acc_num].pin_led_red, LOW);
        digitalWrite(9,0);
        delay(20);
        #ifdef DEBUG
            Serial.print("Red: off / ");
        #endif
      }
      digitalWrite(acc_articles[acc_num].pin_led_grn, HIGH);
      digitalWrite(9,1);
        #ifdef DEBUG
            Serial.println("Green: on");
        #endif
    } else if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
      int pin = acc_articles[acc_num].pin_led_red;
      if(acc_articles[acc_num].acc_type == 0) {
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(pin, LOW);
        digitalWrite(9,0);
        delay(20);
        #ifdef DEBUG
            Serial.print("Red: off / ");
        #endif
      }
      pin = acc_articles[acc_num].pin_led_grn;
      AddOn[acc_articles[acc_num].Modul-1].digitalWrite(pin, HIGH);
      digitalWrite(9,1);
        #ifdef DEBUG
            Serial.println("Green: on");
        #endif
    }
    #ifdef DEBUG
      Serial.print("Switching ACC-No. ");
      Serial.print(mcan.getadrs(prot, acc_articles[acc_num].locID));
      Serial.print(" on modul ");
      Serial.print(acc_articles[acc_num].Modul);
      Serial.println(" to GREEN.");
    #endif
  }
  digitalWrite(9,0);
  //mcan.sendAccessoryFrame(device, (EEPROM.read( acc_articles[acc_num].reg_locid ) << 8) | (EEPROM.read( acc_articles[acc_num].reg_locid + 1 )), acc_articles[acc_num].state_is, true);
  // << keine Bestätigung als Stellpult senden!!!
  acc_articles[acc_num].state_is = set_state;
  EEPROM.put(acc_articles[acc_num].reg_state, acc_articles[acc_num].state_is);
  digitalWrite(9,1);
}
#endif

/*
 * Ausführen, wenn eine Nachricht verfügbar ist.
 * Nachricht wird geladen und anhängig vom CAN-Befehl verarbeitet.
 */
void interruptFn(){
  can_frame_in = mcan.getCanFrame();
  #ifdef LED_FEEDBACK
    accFrame();
  #endif
  pingFrame();
  configFrame();
  statusFrame();

}

/*
 * Prüfen auf Schaltbefehl.
 */
#ifdef LED_FEEDBACK
void accFrame(){
  if((can_frame_in.cmd == SWITCH_ACC) && (can_frame_in.resp_bit == 0)){     //Abhandlung bei gültigem Weichenbefehl
    uint16_t locid = (can_frame_in.data[2] << 8) | can_frame_in.data[3];
    #ifdef DEBUG
      Serial.print("Recieved ACC-Frame for ACC: ");
      Serial.println(mcan.getadrs(prot, locid));
    #endif
    for(int i = 0; i < NUM_ACCs; i++){
      if(locid == acc_articles[i].locID){                                              //Auf benutzte Adresse überprüfen
        acc_articles[i].state_set = can_frame_in.data[4];
        #ifdef LED_FEEDBACK
        if (acc_articles[i].acc_type == 1) {
          acc_articles[i].power_set = can_frame_in.data[5];
        }
        #endif
        #ifdef DEBUG
          Serial.println(" => match found -> Set target state.");
        #endif
        break;
      }
    }
  }

}
#endif

/*
 * Auf Ping Request antworten.
 */
void pingFrame(){
  if((can_frame_in.cmd == PING) && (can_frame_in.resp_bit == 0)){          //Auf Ping Request antworten
    mcan.sendPingFrame(device, true);
    #ifdef DEBUG
      Serial.println("Sending ping response.");
    #endif
  }

}

/*
 *
 */
void configFrame(){
  if((can_frame_in.cmd == CONFIG) && (can_frame_in.resp_bit == 0)){
    if((uid_mat[0] == can_frame_in.data[0])&&(uid_mat[1] == can_frame_in.data[1])&&(uid_mat[2] == can_frame_in.data[2])&&(uid_mat[3] == can_frame_in.data[3])){
      config_poll = true;
      config_index = can_frame_in.data[4];
      #ifdef DEBUG
        Serial.println("Recieved config frame.");
      #endif
    }
  }

}

/*
 *
 */
void statusFrame(){
  if((can_frame_in.cmd == SYS_CMD) && (can_frame_in.resp_bit == 0) && (can_frame_in.data[4] == SYS_STAT)){
    if((uid_mat[0] == can_frame_in.data[0])&&(uid_mat[1] == can_frame_in.data[1])&&(uid_mat[2] == can_frame_in.data[2])&&(uid_mat[3] == can_frame_in.data[3])){
      if(can_frame_in.data[5] == 1){                          // Protokoll schreiben ( MM oder DCC)
        prot_old = prot;
        if(!can_frame_in.data[7]){
          prot = DCC_ACC;
        }else{
          prot = MM_ACC;
        }
        if(prot != prot_old){
          EEPROM.put(REG_PROT, can_frame_in.data[7]);
          #ifdef DEBUG
            Serial.print("Changing protocol: ");
            Serial.print(prot_old);
            Serial.print(" -> ");
            Serial.println(prot);
          #endif
          change_prot();
        }
        mcan.statusResponse(device, can_frame_in.data[5]);

      } else if(can_frame_in.data[5] >= start_adrs_channel){
        /*
        if(can_frame_in.data[5] == acc_articles[can_frame_in.data[5]-start_adrs_channel].adrs_channel){
          #ifdef DEBUG
            Serial.print("Changing address: ");
            Serial.print(mcan.getadrs(prot, acc_articles[can_frame_in.data[5]-start_adrs_channel].locID));
            Serial.print(" -> ");
          #endif
          acc_articles[can_frame_in.data[5]-start_adrs_channel].locID = mcan.generateLocId(prot, (can_frame_in.data[6] << 8) | can_frame_in.data[7] );
          byte locid_high = acc_articles[can_frame_in.data[5]-start_adrs_channel].locID >> 8;
          byte locid_low = acc_articles[can_frame_in.data[5]-start_adrs_channel].locID;
          EEPROM.put(acc_articles[can_frame_in.data[5]-start_adrs_channel].reg_locid, locid_high);
          EEPROM.put(acc_articles[can_frame_in.data[5]-start_adrs_channel].reg_locid + 1, locid_low);
          mcan.statusResponse(device, can_frame_in.data[5]);
          #ifdef DEBUG
            Serial.println(mcan.getadrs(prot, acc_articles[can_frame_in.data[5]-start_adrs_channel].locID));
          #endif

        }
        */
        //int tmp = config_index / 2;
        if (can_frame_in.data[5] % 2) {
          // TYP:
          #ifdef DEBUG
            Serial.print("Changing Acc Type: ");
            Serial.print(acc_articles[ ( can_frame_in.data[5] / 2 ) - 1 ].acc_type);
            Serial.print(" -> ");
          #endif
          acc_articles[ ( can_frame_in.data[5] / 2 ) - 1 ].acc_type = can_frame_in.data[7];
          EEPROM.put(acc_articles[ ( can_frame_in.data[5] / 2 ) - 1 ].reg_type, can_frame_in.data[7]);
          mcan.statusResponse(device, can_frame_in.data[5]);
          #ifdef DEBUG
            Serial.println(acc_articles[ ( can_frame_in.data[5] / 2 ) - 1 ].acc_type);
            Serial.println(" ==> 0 = Weiche/Signal ; 1 = Entkuppler");
          #endif
        } else {
          // ADRESSE:
          #ifdef DEBUG
            Serial.print("Changing address: ");
            Serial.print(mcan.getadrs(prot, acc_articles[ ( can_frame_in.data[5] / 2 ) - 1 ].locID));
            Serial.print(" -> ");
          #endif
          acc_articles[ ( can_frame_in.data[5] / 2 ) - 1 ].locID = mcan.generateLocId(prot, (can_frame_in.data[6] << 8) | can_frame_in.data[7] );
          byte locid_high = acc_articles[ ( can_frame_in.data[5] / 2 ) - 1 ].locID >> 8;
          byte locid_low = acc_articles[ ( can_frame_in.data[5] / 2 ) - 1 ].locID;
          EEPROM.put(acc_articles[ ( can_frame_in.data[5] / 2 ) - 1 ].reg_locid, locid_high);
          EEPROM.put(acc_articles[ ( can_frame_in.data[5] / 2 ) - 1 ].reg_locid + 1, locid_low);
          mcan.statusResponse(device, can_frame_in.data[5]);
          #ifdef DEBUG
            Serial.println(mcan.getadrs(prot, acc_articles[ ( can_frame_in.data[5] / 2 ) - 1 ].locID));
          #endif
        }

      }
    }
  }

}


/*
 * switch LED on/off
 */
#ifdef LED_FEEDBACK
void switchLED(uint8_t acc_num, bool color, bool power) {
  /*
   * acc_num    => Number ACC
   * color      => 0 - red, 1 - green
   * save_state => 0 - do not save status, 1 - save status
   * power      => 0 - off, 1 - on
   */
  bool save_state = 0;
  if (acc_articles[acc_num].acc_type == 0) {
    save_state = 1;
    if ( color == 0 ) {
      #ifdef DEBUG
          Serial.println("Red: on / Green: off");
      #endif
      if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
        // AddOn
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_led_red, HIGH);
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_led_grn, LOW);
      } else {
        // Onboard
        digitalWrite(acc_articles[acc_num].pin_led_red, HIGH);
        digitalWrite(acc_articles[acc_num].pin_led_grn, LOW);
      }
    } else {
      #ifdef DEBUG
          Serial.println("Red: off / Green: on");
      #endif
    if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
      // AddOn
      AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_led_grn, HIGH);
      AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_led_red, LOW);
    } else {
      // Onboard
      digitalWrite(acc_articles[acc_num].pin_led_grn, HIGH);
      digitalWrite(acc_articles[acc_num].pin_led_red, LOW);
    }
      
    }
  } else if ( (acc_articles[acc_num].acc_type == 1) && (power == 1) ) {
    if ( color == 0 ) {
      #ifdef DEBUG
          Serial.println("Red: on");
      #endif
      if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_led_red, HIGH);
      } else {
        digitalWrite(acc_articles[acc_num].pin_led_red, HIGH);
      }      
    } else {
      #ifdef DEBUG
          Serial.println("Green: on");
      #endif
      if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_led_grn, HIGH);
      } else {
        digitalWrite(acc_articles[acc_num].pin_led_grn, HIGH);
      }      
    }
    acc_articles[acc_num].power_is = acc_articles[acc_num].power_set;
  } else if ( (acc_articles[acc_num].acc_type == 1) && (power == 0) ) {
    if ( color == 0 ) {
      #ifdef DEBUG
          Serial.println("Red: off");
      #endif
      if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_led_red, LOW);
      } else {
        digitalWrite(acc_articles[acc_num].pin_led_red, LOW);
      }      
    } else {
      #ifdef DEBUG
          Serial.println("Green: off");
      #endif
      if( (acc_articles[acc_num].Modul > 0) && (acc_articles[acc_num].Modul <= 8) ){
        AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin_led_grn, LOW);
      } else {
        digitalWrite(acc_articles[acc_num].pin_led_grn, LOW);
      }      
    }
    acc_articles[acc_num].power_is = acc_articles[acc_num].power_set;
  }

  acc_articles[acc_num].state_is = color;
  if (save_state) {
    EEPROM.put(acc_articles[acc_num].reg_state, acc_articles[acc_num].state_is);
  }
}
#endif

/*
 * push button action
 */
void button_pushed(uint8_t acc_num, bool color, bool state) {
  /*
   * color:     0 = red
   *            1 = green
   * state:     0 = not pushed
   *            1 = pushed
   */
  if (state) {
    /*
     * Button pushed
     */
    if (color == RED ) {
      /*
       * red
       */
      #ifdef DEBUG
        Serial.print("Send switchcommand RED to ACC #");
        Serial.println(mcan.getadrs(prot, acc_articles[acc_num].locID));
      #endif
      if(acc_articles[acc_num].pushed_red != state) {   // Sende 1 mal Schaltbefehlt für Weichen & Signale
        mcan.sendAccessoryFrame(device, acc_articles[acc_num].locID, color, false, state);
        acc_articles[acc_num].pushed_red = 1;
        #ifdef LED_FEEDBACK
          switchLED(acc_num, color, state);
        #endif
      } else if(acc_articles[acc_num].acc_type == 1) {        // Sende wiederholt Schaltbefehlt für Entkuppler
        mcan.sendAccessoryFrame(device, acc_articles[acc_num].locID, color, false, state);
      }
      
    } else {
      /*
       * green
       */
      #ifdef DEBUG
        Serial.print("Send switchcommand GREEN to ACC #");
        Serial.println(mcan.getadrs(prot, acc_articles[acc_num].locID));
      #endif
      if(acc_articles[acc_num].pushed_grn != state) {   // Sende 1 mal Schaltbefehlt für Weichen & Signale
        mcan.sendAccessoryFrame(device, acc_articles[acc_num].locID, color, false, state);
        acc_articles[acc_num].pushed_grn = 1;
        #ifdef LED_FEEDBACK
          switchLED(acc_num, color, state);
        #endif
      } else if(acc_articles[acc_num].acc_type == 1) {        // Sende wiederholt Schaltbefehlt für Entkuppler
        mcan.sendAccessoryFrame(device, acc_articles[acc_num].locID, color, false, state);
      }      
    }
    
  } else {
    /*
     * Button NOT pushed
     */
    if (color == 0 ) {
      /*
       * red
       */
      if (acc_articles[acc_num].pushed_red == 1) {
        mcan.sendAccessoryFrame(device, acc_articles[acc_num].locID, color, false, state);
        #ifdef LED_FEEDBACK
          switchLED(acc_num, color, state);
        #endif
      }
      acc_articles[acc_num].pushed_red = 0;

    } else {
      /*
       * green
       */
      if (acc_articles[acc_num].pushed_grn == 1) {
        mcan.sendAccessoryFrame(device, acc_articles[acc_num].locID, color, false, state);
        #ifdef LED_FEEDBACK
          switchLED(acc_num, color, state);
        #endif
      }
      acc_articles[acc_num].pushed_grn = 0;
      
    }
  }
  
  
}


/*
 * Main loop
 */
void loop() {
  /*
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    detachInterrupt(digitalPinToInterrupt(2));
    delay(50);
    attachInterrupt(digitalPinToInterrupt(2), interruptFn, LOW);
  }
  */
  currentMillis = millis();
  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    state_LED = !state_LED;
    digitalWrite(9,state_LED);
  }

  for(int i = 0; i < NUM_ACCs; i++){
    #ifdef LED_FEEDBACK
      if(acc_articles[i].state_is != acc_articles[i].state_set){
        switchLED(i, acc_articles[i].state_set, acc_articles[i].power_set);
        #ifdef DEBUG
          Serial.print("Switching ACC ");
          Serial.print(mcan.getadrs(prot, acc_articles[i].locID));
          if(acc_articles[i].state_set == RED){
            Serial.println(" to RED.");
          } else {
            Serial.println(" to GREEN.");
          }
        #endif
      } else if( (acc_articles[i].acc_type == 1) && (acc_articles[i].power_is != acc_articles[i].power_set) ) {
        switchLED(i, acc_articles[i].state_set, acc_articles[i].power_set);
        #ifdef DEBUG
          Serial.print("Switching ACC ");
          Serial.print(mcan.getadrs(prot, acc_articles[i].locID));
          if(acc_articles[i].state_set == RED){
            Serial.println(" to RED.");
          } else {
            Serial.println(" to GREEN.");
          }
        #endif
      }

    #endif

    //digitalWrite(9,0);
    if(acc_articles[i].Modul == 0) {
      if(digitalRead(acc_articles[i].pin_red) == LOW) {
        /*
         * red button pushed
         */
        button_pushed(i, RED, BUTTON_PRESSED);
      } else {
        /*
         * red button NOT pushed
         */
        button_pushed(i, RED, BUTTON_NOT_PRESSED);
      }
      
      if (digitalRead(acc_articles[i].pin_grn) == LOW) {
        /*
         * red button pushed
         */
        button_pushed(i, GREEN, BUTTON_PRESSED);
      } else {
        /*
         * red button NOT pushed
         */
        button_pushed(i, GREEN, BUTTON_NOT_PRESSED);
      }
      
    } else {
      if(AddOn[acc_articles[i].Modul-1].digitalRead(acc_articles[i].pin_red) == LOW) {
        /*
         * red button pushed
         */
        button_pushed(i, RED, true);
      } else {
        /*
         * red button NOT pushed
         */
        button_pushed(i, RED, false);
      }
      
      if (AddOn[acc_articles[i].Modul-1].digitalRead(acc_articles[i].pin_grn) == LOW) {
        /*
         * red button pushed
         */
        button_pushed(i, GREEN, true);
      } else {
        /*
         * red button NOT pushed
         */
        button_pushed(i, GREEN, false);
      }

    }
    //digitalWrite(9,1);
  }
  if(config_poll){
    if(config_index == 0) mcan.sendDeviceInfo(device, CONFIG_NUM);
    if(config_index == 1) mcan.sendConfigInfoDropdown(device, 1, 2, EEPROM.read(REG_PROT), "Protokoll_DCC_MM");
    if(config_index >= 2){
      /*
      string3 = string1 + ( config_index - start_adrs_channel + 1 ) + string2;
      uint16_t adrs = mcan.getadrs(prot, acc_articles[ config_index - start_adrs_channel ].locID);
      mcan.sendConfigInfoSlider(device, config_index, 1, 2048, adrs, string3);
      */
      //int tmp = config_index / 2;
      if (config_index % 2) {
        // TYP:
        //string1=( config_index / 2 );
        //string2=". Magnetartikeltyp:_Weiche/Signal_Entkuppler";
        string3="Magnetartikeltyp:_Weiche/Signal_Entkuppler";
        mcan.sendConfigInfoDropdown(device, config_index, 2, acc_articles[ ( config_index / 2 ) - 1 ].acc_type, string3);
      } else {
        // ADRESSE:
        string1 = ( config_index / 2 );
        string2 = ". Adresse:_1_2048";
        string3 = ( config_index / 2 ) + string2;
        //string3 = ( config_index / 2 ) + ". Adresse:_1_2048";
        uint16_t adrs = mcan.getadrs(prot, acc_articles[ ( config_index / 2 ) - 1 ].locID);
        mcan.sendConfigInfoSlider(device, config_index, 1, 2048, adrs, string3);
      }

    }
    config_poll = false;
  }

}

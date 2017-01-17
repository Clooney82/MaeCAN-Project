/******************************************************************************
   MäCAN-Stellpult AddOn, Software-Version 0.5

    Created by Maximilian Goldschmidt <maxigoldschmidt@gmail.com>
    Modified by Jochen Kielkopf.
    Do with this whatever you want, but keep thes Header and tell
    the others what you changed!

    Last edited: 2017-01-16
 ******************************************************************************/
/******************************************************************************
   Includes
 ******************************************************************************/
#include <MCAN.h>
#include <EEPROM.h>
#include <Adafruit_MCP23017.h>
/******************************************************************************
   EEPROM-Register
   # 0 - 20 -> CONFIG:
   ------------------------------------------------------------------------
    0 -  4 -> not used
    5 -  8 -> UID
         9 -> PROTOCOL
        10 -> Initial Config ( 0 = unkonfiguriert, 1 = konfiguriert )
   11 - 20 -> reserved / free
   ========================================================================
   #21 - XX -> Adressbereich, Pin, Modul (Max 167 ACCs):
   ------------------------------------------------------------------------
    2 byte -> LocalID       // wird aktuell genutzt
    2 byte -> Adresse       // wird nicht aktuell genutzt
    1 byte -> Status        // wird nicht aktuell genutzt
    1 byte -> Modul ( 0 = Decoder, 1 = AddOn 1, 2 = AddOn 2 ) // wird nicht aktuell genutzt
   ------------------------------------------------------------------------
   ------------------------------------------------------------------------
   Formeln zu Berechnung der Speicherregister
   ------------------------------------------------------------------------
   LocalID = (20+(6*i)-5) (20+(6*i)-4)    - acc_articles[num].reg_locid = (20+(6*(num+1))-5);
   Adresse = (20+(6*i)-3) (20+(6*i)-2)    - not used
   Status  = (20+(6*i)-1)                 - acc_articles[num].reg_state = (20+(6*(num+1))-1);
   Typ     = (20+(6*i))                   - acc_articles[num].reg_type  = (20+(6*(num+1)));
   ========================================================================
 ******************************************************************************/
#define VERS_HIGH 0       // Versionsnummer vor dem Punkt
#define VERS_LOW  5       // Versionsnummer nach dem Punkt
/******************************************************************************
   Allgemeine Setup Daten:
 ******************************************************************************/
#define BOARD_NUM 1       // Identifikationsnummer des Boards (Anzeige in der CS2)
#define UID 0x12345678    // CAN-UID (default) - please change!!!

const uint8_t ANZ_ADDONS = 1;       // Anzahl AddOn Platinen (max 8)
                                    // => Ohne LED_FEEDBACK: ab > 7 AddOns wird der Arbeitsspeicher knapp

#define LED_FEEDBACK                // comment it out if you don´t use leds to show switchstatus
#define use_uncuppler               // comment it out if you don´t use uncupplers - keeps config on CS2/CS3 smaller.

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
   Debugging einschalten
 ******************************************************************************/
//#define DEBUG_SERIAL
//#define DEBUG_SETUP
//#define DEBUG_SETUP_ACC
//#define DEBUG_CONFIG
//#define DEBUG_LED
//#define DEBUG_INPUT
//#define DEBUG_MCAN
//#define DEBUG_CAN
//#define run_fake_acc_commands
/******************************************************************************
   Allgemeine Konstanten:
 ******************************************************************************/
#define RED   0
#define GREEN 1
#define POWER_ON  1
#define POWER_OFF 0
#define BUTTON_PRESSED     1
#define BUTTON_NOT_PRESSED 0
#define TYPE_WEICHE 0
#define TYPE_ENTKUPPLER 1

#ifdef run_fake_acc_commands
  uint8_t tmp = 0;
#endif

const uint8_t REG_UID  = 5;     // Speicheradress für UID - 4 bytes
const uint8_t REG_PROT = 9;     // Speicheradress für Protokoll - Global für alle ACC gleich.
String string1;
String string2;
String string3;
String string4;

int start_adrs_channel = 2;

typedef struct {
  int reg_locid;        // EEPROM-Register der Local-IDs
  uint16_t locID;       // LocalID
  uint8_t Modul;        // 0 = On-Board; 1 = Modul1; 2 = Modul2, ...
  uint8_t pin_grn;      // PIN Grün
  uint8_t pin_red;      // PIN Rot
  int reg_type;         // EEPROM-Register für die ACC Typen
  bool acc_type  = 0;   // 0 = Weiche/Signal ; 1 = Entkuppler
  //int adrs_channel;     // Konfigkanäle für die Adressen
  unsigned long pushed  = 0;
  //bool pushed_red  = 0;
  //bool pushed_grn  = 0;
  #ifdef LED_FEEDBACK
    uint8_t pin_led_grn;  // PIN Grün
    uint8_t pin_led_red;  // PIN Rot
    int reg_state;        // EEPROM-Register zum Speichern des Schaltzustand
    bool state_is  = 0;   // ...
    bool state_set = 0;   // ...
    bool power_is  = 0;   // Strom ein-/aus
    bool power_set = 0;   // Strom ein-/ausschalten
  #endif
} acc_Magnet;

acc_Magnet acc_articles[ (ANZ_ADDONS * ANZ_ACC_PER_ADDON) ]; // = 4 * USE_ONBOARD + ANZ_ACC_PER_ADDON * ANZ_ADDONS

Adafruit_MCP23017 AddOn[ANZ_ADDONS];   // Create AddOn

uint8_t NUM_ACCs;

unsigned long previousMillis = 0;
unsigned long previousMillis_input = 0;
//unsigned long previousMillis_config = 0;
unsigned long currentMillis = 0;
uint16_t interval = 1000;
const uint16_t acc_interval = 500;
const uint8_t input_interval = 100;
//const uint16_t config_interval = 10000;
bool state_LED = false;

/******************************************************************************
   Variablen der Magnetartikel:
 ******************************************************************************/
uint8_t CONFIG_NUM;     //Anzahl der Konfigurationspunkte
bool config_poll = false;
byte uid_mat[4];
uint8_t  config_index = 0;
uint16_t prot;
uint16_t prot_old;
bool pushed_red;
bool pushed_grn;

bool config_sent = false;
bool locked = true;
/******************************************************************************
   Benötigtes:
 ******************************************************************************/
MCAN mcan;
MCANMSG can_frame_out;
MCANMSG can_frame_in;

CanDevice device;

/******************************************************************************
   Setup
 ******************************************************************************/
//#############################################################################
// 
//#############################################################################
void setup() {
  #ifdef DEBUG_SERIAL
    //Serial.begin(9600);
    Serial.begin(111520);

    Serial.println("-----------------------------------");
    Serial.println(" - Setup                         -");
    Serial.println("-----------------------------------");
  #endif
  uid_mat[0] = UID >> 24;
  uid_mat[1] = UID >> 16;
  uid_mat[2] = UID >> 8;
  uid_mat[3] = UID;

  if( (EEPROM.read(5)==uid_mat[0]) && 
      (EEPROM.read(6)==uid_mat[1]) && 
      (EEPROM.read(7)==uid_mat[2]) && 
      (EEPROM.read(8)==uid_mat[3]) ){
    #ifdef DEBUG_SETUP
      Serial.println(" - UID was not changed: No Setup of EEPROM needed.");
    #endif
  } else {
    #ifdef DEBUG_SETUP
      Serial.println(" - UID was changed: Initital Setup of EEPROM");
    #endif
    EEPROM.put(10, 0);
    EEPROM.put(REG_UID  , uid_mat[0]);
    EEPROM.put(REG_UID+1, uid_mat[1]);
    EEPROM.put(REG_UID+2, uid_mat[2]);
    EEPROM.put(REG_UID+3, uid_mat[3]);
    EEPROM.put(REG_PROT, 1);    // 0 = DCC, 1=MM
  }

  pinMode(9, OUTPUT);
  digitalWrite(9, state_LED);


  device.versHigh = VERS_HIGH;
  device.versLow = VERS_LOW;
  device.hash = mcan.generateHash(UID);
  device.uid = UID;
  device.artNum = "00053";   // 8 byte
  device.name = "MäCAN Stellpult";
//  device.artNum = "xxxxx";   // 8 byte
//  device.name = "TEST EK";
  device.boardNum = BOARD_NUM;
  device.type = MCAN_STELLPULT;


  if (!EEPROM.read(REG_PROT)) {
    prot = DCC_ACC;
  } else {
    prot = MM_ACC;
  }
  prot_old = prot;

  NUM_ACCs = ANZ_ACC_PER_ADDON * ANZ_ADDONS;
  
  #ifdef use_uncuppler
    // mit Entkuppler:
    CONFIG_NUM = start_adrs_channel + ( 2 * NUM_ACCs ) - 1;
  #else
    // ohne Entkuppler:
    CONFIG_NUM = start_adrs_channel + NUM_ACCs - 1;
  #endif

  setup_acc();
  #ifdef LED_FEEDBACK
    test_leds();
    restore_last_state();
  #endif

  #ifdef DEBUG_MCAN
    //mcan.initMCAN(true);
    mcan.initMCAN();
  #else
    mcan.initMCAN();
  #endif
  
  #ifdef DEBUG_SERIAL
    Serial.println("-----------------------------------");
    Serial.println(" - Setup completed               -");
    Serial.println("-----------------------------------");
    Serial.print(" - Board-UID:  ");
    Serial.println(UID);
    Serial.print(" - Board-Num:  ");
    Serial.println(BOARD_NUM);
    Serial.print(" - Config Num: ");
    Serial.println(CONFIG_NUM);
    Serial.print(" - Num ACCs:   ");
    Serial.println(NUM_ACCs);
    Serial.println("-----------------------------------");
    Serial.println(" - Device is now ready...        -");
    Serial.println("-----------------------------------");
  #endif
  attachInterrupt(digitalPinToInterrupt(2), interruptFn, LOW);
  state_LED = 1;
  digitalWrite(9, state_LED);
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
void setup_acc(){
  //**************************************************************************************************
  // START - Setup ACCs
  //**************************************************************************************************
  #ifdef DEBUG_SETUP
    Serial.print(millis());
    Serial.println(" - Setting up Accs");
    Serial.println("-----------------------------------");
  #endif
  //--------------------------------------------------------------------------------------------------
  // Load Default Values
  //--------------------------------------------------------------------------------------------------
  if (EEPROM.read(10) == 0) {
    EEPROM.put(10, 1);
    #ifdef DEBUG_SETUP
      Serial.print(millis());
      Serial.println(" - ACC Setup not done.");
      Serial.print(millis());
      Serial.println(" - Loading default values.");
    #endif
    //for (int i = 20; i < 1024; i++) EEPROM.put(i,0);
    config_own_adresses_manual();
  }
  //--------------------------------------------------------------------------------------------------
  // Setup PINs
  //--------------------------------------------------------------------------------------------------
  int num = 0;
  if (ANZ_ADDONS > 0) {
    for (int m = 0; m < ANZ_ADDONS; m++) {
      AddOn[m].begin(m);
      int pin = 0;
      for (int i = 0; i < ANZ_ACC_PER_ADDON; i++) {
        acc_articles[num].Modul = m;
        acc_articles[num].pin_grn = pin;
        pin++;
        acc_articles[num].pin_red = pin;
        pin++;

        AddOn[m].pinMode(acc_articles[num].pin_grn, INPUT);
        AddOn[m].pullUp(acc_articles[num].pin_grn, HIGH);   // Activate Internal Pull-Up Resistor
        AddOn[m].pinMode(acc_articles[num].pin_red, INPUT);
        AddOn[m].pullUp(acc_articles[num].pin_red, HIGH);   // Activate Internal Pull-Up Resistor
        
        acc_articles[num].reg_locid = (20 + (6 * (num + 1)) - 5);
        acc_articles[num].locID     = (EEPROM.read( acc_articles[num].reg_locid ) << 8) | (EEPROM.read( acc_articles[num].reg_locid + 1 ));
        acc_articles[num].reg_type  = (20 + (6 * (num + 1))    );
        acc_articles[num].acc_type  = EEPROM.read( acc_articles[num].reg_type );

        //acc_articles[num].adrs_channel = num + start_adrs_channel;
        
        #ifdef LED_FEEDBACK
          acc_articles[num].pin_led_grn = acc_articles[num].pin_grn + 8;
          acc_articles[num].pin_led_red = acc_articles[num].pin_red + 8;
          AddOn[m].pinMode(acc_articles[num].pin_led_grn, OUTPUT);
          AddOn[m].pinMode(acc_articles[num].pin_led_red, OUTPUT);

          acc_articles[num].reg_state = (20 + (6 * (num + 1)) - 1);
          acc_articles[num].state_is = EEPROM.read( acc_articles[num].reg_state );
          acc_articles[num].state_set = acc_articles[num].state_is;
        #endif
        

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
    #ifdef LED_FEEDBACK
      acc_articles[i].reg_state = (20 + (6 * (i + 1)) - 1);
    #endif
    #ifdef use_uncuppler
      acc_articles[i].reg_type  = (20 + (6 * (i + 1))    );
      acc_articles[i].acc_type = 0;
    #endif
    adrsss = base_address + i;
    acc_articles[i].locID = mcan.generateLocId(prot, adrsss );
    byte locid_high = acc_articles[i].locID >> 8;
    byte locid_low = acc_articles[i].locID;
    EEPROM.put(acc_articles[i].reg_locid, locid_high);
    EEPROM.put(acc_articles[i].reg_locid + 1, locid_low);
    EEPROM.put(acc_articles[i].reg_type, acc_articles[i].acc_type);
    #ifdef LED_FEEDBACK
      acc_articles[i].state_is = 0;
      EEPROM.put( acc_articles[i].reg_state, acc_articles[i].state_is);
      acc_articles[i].state_set = acc_articles[i].state_is;
      acc_articles[i].power_is = 0;
      acc_articles[i].power_set = acc_articles[i].power_is;
    #endif
  }

}


//#############################################################################
// Test aller LEDs
//#############################################################################
#ifdef LED_FEEDBACK
void test_leds() {
  for (int i = 0; i < NUM_ACCs; i++) {
    AddOn[acc_articles[i].Modul].digitalWrite(acc_articles[i].pin_led_red, 1);
    AddOn[acc_articles[i].Modul].digitalWrite(acc_articles[i].pin_led_grn, 1);
  }
  #ifndef DEBUG_SERIAL
    delay(10000);
  #endif
  for (int i = 0; i < NUM_ACCs; i++) {
    AddOn[acc_articles[i].Modul].digitalWrite(acc_articles[i].pin_led_red, 0);
    AddOn[acc_articles[i].Modul].digitalWrite(acc_articles[i].pin_led_grn, 0);
  }

}
#endif


//#############################################################################
// Letzten Schaltzustand wiederherstellen.
//#############################################################################
#ifdef LED_FEEDBACK
void restore_last_state() {
  #ifdef DEBUG_SETUP
    Serial.print(millis());
    Serial.println(" - Restoring last LED Status:");
  #endif
  for (int i = 0; i < NUM_ACCs; i++) {
    #ifdef DEBUG_SETUP_ACC
      Serial.print(millis());
      Serial.print(" - Adresse:");
      Serial.print(mcan.getadrs(prot_old, acc_articles[i].locID));
    #endif
    if ( acc_articles[i].acc_type == 1 ) {
      #ifdef DEBUG_SETUP_ACC
        Serial.println(" (Entkuppler) - LEDs off.");
      #endif
      AddOn[acc_articles[i].Modul].digitalWrite(acc_articles[i].pin_led_red, LOW);
      AddOn[acc_articles[i].Modul].digitalWrite(acc_articles[i].pin_led_grn, LOW);
    } else {
      #ifdef DEBUG_SETUP_ACC
        Serial.print(" (Weiche/Signal) - LED: ");
      #endif
      switch (acc_articles[i].state_is) {
        case RED:   // rot
          AddOn[acc_articles[i].Modul].digitalWrite(acc_articles[i].pin_led_grn, LOW);
          AddOn[acc_articles[i].Modul].digitalWrite(acc_articles[i].pin_led_red, HIGH);
          #ifdef DEBUG_SETUP_ACC
            Serial.println("RED: ON / green: off");
          #endif
          break;
        case GREEN:   // grün
          AddOn[acc_articles[i].Modul].digitalWrite(acc_articles[i].pin_led_red, LOW);
          AddOn[acc_articles[i].Modul].digitalWrite(acc_articles[i].pin_led_grn, HIGH);
          #ifdef DEBUG_SETUP_ACC
            Serial.println("red: off / GREEN: ON");
          #endif
          break;
      }
    }
  }
}
#endif


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
  for (int i = 0; i < NUM_ACCs; i++) {
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


//#############################################################################
// Erfolgreiches Setup signalisieren
//#############################################################################
void signal_setup_successfull() {
  for (int i = 0; i < 20; i++) {
    state_LED = !state_LED;
    digitalWrite(9, state_LED);
    delay(100);
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
    //************************************************************************************************
    // START - PING Frame           - Auf Ping Request antworten
    //************************************************************************************************
    #ifdef DEBUG_CAN
      Serial.print(millis());
      Serial.println(" - Sending ping response.");
    #endif
    mcan.sendPingFrame(device, true);
    
    //************************************************************************************************
    // ENDE - PING Frame
    //************************************************************************************************
  } else if ((can_frame_in.cmd == SYS_CMD) && (can_frame_in.resp_bit == 0)) {   //Abhandlung bei gültigem Weichenbefehl
    //************************************************************************************************
    // START - STOP / GO / HALT     - 
    //************************************************************************************************
    uint8_t sub_cmd = can_frame_in.data[4];
    if ( (sub_cmd == SYS_STOP) || (sub_cmd == SYS_HALT) ) {
      //----------------------------------------------------------------------------------------------
      // STOP oder HALT             - Eingaben verhindern.
      //----------------------------------------------------------------------------------------------
      #ifdef DEBUG_CAN
        Serial.print(millis());
        Serial.println(" - System locked.");
      #endif
      locked = true;
      
    } else if (sub_cmd == SYS_GO) {
      //----------------------------------------------------------------------------------------------
      // GO                         - Eingaben zulassen.
      //----------------------------------------------------------------------------------------------
      #ifdef DEBUG_CAN
        Serial.print(millis());
        Serial.println(" - System unlocked.");
      #endif
      locked = false;
      
    }
    //************************************************************************************************
    // ENDE - STOP / GO / HALT
    //************************************************************************************************
  } else if ((can_frame_in.cmd == SWITCH_ACC) && (can_frame_in.resp_bit == 0)) {   //Abhandlung bei gültigem Weichenbefehl
    //************************************************************************************************
    // START - ACC Frame            - Püfen auf Schaltbefehle
    //************************************************************************************************
    #ifdef LED_FEEDBACK
      uint16_t locid = (can_frame_in.data[2] << 8) | can_frame_in.data[3];
      #ifdef DEBUG_CAN
        Serial.print(millis());
        Serial.print(" - Recieved ACC-Frame for ACC: ");
        Serial.println(mcan.getadrs(prot, locid));
      #endif
      for (int i = 0; i < NUM_ACCs; i++) {
        if (locid == acc_articles[i].locID) {                                            //Auf benutzte Adresse überprüfen
          #ifdef DEBUG_CAN
            Serial.print(millis());
            Serial.println("    => match found -> Set target state.");
          #endif
          acc_articles[i].state_set = can_frame_in.data[4];
          acc_articles[i].power_set = can_frame_in.data[5];
          break;
        }
        
      }
    #endif
    //************************************************************************************************
    // ENDE - ACC Frame
    //************************************************************************************************
  } else if ((uid_mat[0] == can_frame_in.data[0]) && (uid_mat[1] == can_frame_in.data[1]) && (uid_mat[2] == can_frame_in.data[2]) && (uid_mat[3] == can_frame_in.data[3])) {
  //==================================================================================================
  // Befehle nur für eine UID
  //==================================================================================================
    if ((can_frame_in.cmd == CONFIG) && (can_frame_in.resp_bit == 0)) {
      //**********************************************************************************************
      // START - Config Frame       - 
      //**********************************************************************************************
      #ifdef DEBUG_CAN
        Serial.print(millis());
        Serial.print(" - Recieved config frame.  - INDEX:");
        Serial.println(can_frame_in.data[4]);
      #endif
      config_poll = true;
      config_index = can_frame_in.data[4];
      if (config_index == 0) locked = true;
      if (config_index == CONFIG_NUM) {
        locked = false;
        config_sent = true;
        Serial.println(" - CONFIG SENT");
        Serial.println(" - System unlocked");
      }

      //**********************************************************************************************
      // ENDE - Config Frame
      //**********************************************************************************************
    } else if ((can_frame_in.cmd == SYS_CMD) && (can_frame_in.resp_bit == 0) && (can_frame_in.data[4] == SYS_STAT)) {
      //**********************************************************************************************
      // START - Status Frame       -
      //**********************************************************************************************
      #ifdef DEBUG_CAN
        Serial.print(millis());
        Serial.print(" - Recieved Status Frame for config_index: ");
        Serial.println(can_frame_in.data[5]);
      #endif
      //----------------------------------------------------------------------------------------------
      // CAN_FRAME_IN.DATA[5]       => CONFIG_INDEX
      //----------------------------------------------------------------------------------------------
      if (can_frame_in.data[5] == 1) {
        //--------------------------------------------------------------------------------------------
        // Protokoll schreiben ( MM oder DCC)
        //--------------------------------------------------------------------------------------------
        prot_old = prot;
        if (!can_frame_in.data[7]) {
          prot = DCC_ACC;
        } else {
          prot = MM_ACC;
        }
        if (prot != prot_old) {
          EEPROM.put(REG_PROT, can_frame_in.data[7]);
          #ifdef DEBUG_CAN
            Serial.print(millis());
            Serial.print(" -  => Changing protocol: ");
            Serial.print(prot_old);
            Serial.print(" -> ");
            Serial.println(prot);
          #endif
          change_prot();
        }
        mcan.statusResponse(device, can_frame_in.data[5]);

      } else if (can_frame_in.data[5] >= 2) {
#ifdef use_uncuppler
        //--------------------------------------------------------------------------------------------
        // mit Entkuppler
        //--------------------------------------------------------------------------------------------
        
        uint8_t acc_num = ( can_frame_in.data[5] / 2 ) - 1;
        #ifdef DEBUG_CAN
          Serial.print(millis());
          Serial.print(" -  => Change ACC Definition of ACC_NUM: #");
          Serial.println(acc_num);
        #endif
        
        if ( (can_frame_in.data[5] % 2) > 0 ) {
          //------------------------------------------------------------------------------------------
          // TYP: 
          //------------------------------------------------------------------------------------------
          #ifdef DEBUG_CAN
            Serial.print(millis());
            Serial.print(" -  => Changing Acc Type: ");
            Serial.print(acc_articles[acc_num].acc_type);
            Serial.print(" -> ");
          #endif
          acc_articles[acc_num].acc_type = can_frame_in.data[7];
          EEPROM.put(acc_articles[acc_num].reg_type, can_frame_in.data[7]);
          #ifdef DEBUG_CAN
            Serial.print(acc_articles[acc_num].acc_type);
            Serial.println("    ==> 0 = Weiche/Signal ; 1 = Entkuppler");
          #endif
        } else {
          //------------------------------------------------------------------------------------------
          // ADRESSE: 
          //------------------------------------------------------------------------------------------
          #ifdef DEBUG_CAN
            Serial.print(millis());
            Serial.print(" -  => Changing address: ");
            Serial.print(mcan.getadrs(prot, acc_articles[acc_num].locID));
            Serial.print(" -> ");
          #endif
          acc_articles[acc_num].locID = mcan.generateLocId(prot, (can_frame_in.data[6] << 8) | can_frame_in.data[7] );
          byte locid_high = acc_articles[acc_num].locID >> 8;
          byte locid_low  = acc_articles[acc_num].locID;
          EEPROM.put(acc_articles[acc_num].reg_locid    , locid_high);
          EEPROM.put(acc_articles[acc_num].reg_locid + 1, locid_low);
          #ifdef DEBUG_CAN
            Serial.println(mcan.getadrs(prot, acc_articles[acc_num].locID));
          #endif
        }
        mcan.statusResponse(device, can_frame_in.data[5]);
#else 
        //--------------------------------------------------------------------------------------------
        // ohne Entkuppler
        //--------------------------------------------------------------------------------------------
        #ifdef DEBUG_CAN
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
        #ifdef DEBUG_CAN
          Serial.println(mcan.getadrs(prot, acc_articles[can_frame_in.data[5]-start_adrs_channel].locID));
        #endif
#endif
      }
      //**********************************************************************************************
      // ENDE - Status Frame
      //**********************************************************************************************
    } 
  //==================================================================================================
  // ENDE - Befehle nur für eine UID
  //==================================================================================================
  }
}

//#############################################################################
// Switch LED on/off
//#############################################################################
#ifdef LED_FEEDBACK
void switchLED(uint8_t acc_num, bool color, bool power) {
  /*
   * acc_num    => Number ACC
   * color      => 0 - red, 1 - green
   * save_state => 0 - do not save status, 1 - save status
   * power      => 0 - off, 1 - on
   */
  #ifdef DEBUG_LED
    Serial.print(millis());
    Serial.print(" -- LED - Adresse: ");
    Serial.print(mcan.getadrs(prot, acc_articles[acc_num].locID));
    Serial.print(" ");
  #endif
  switch (acc_articles[acc_num].acc_type) {
    case TYPE_WEICHE:
      switch (color) {
        case RED:
          #ifdef DEBUG_LED
            Serial.print(" - Weiche/Signal (rot|rund) LED: RED: ON / green: off");
          #endif
          AddOn[acc_articles[acc_num].Modul].digitalWrite(acc_articles[acc_num].pin_led_red, HIGH);
          AddOn[acc_articles[acc_num].Modul].digitalWrite(acc_articles[acc_num].pin_led_grn, LOW);
          break;
        case GREEN:
          #ifdef DEBUG_LED
            Serial.print(" - Weiche/Signal (grün|gerade)  LED: red: off / GREEN: ON");
          #endif
          AddOn[acc_articles[acc_num].Modul].digitalWrite(acc_articles[acc_num].pin_led_grn, HIGH);
          AddOn[acc_articles[acc_num].Modul].digitalWrite(acc_articles[acc_num].pin_led_red, LOW);
          break;
      }
      break;
    case TYPE_ENTKUPPLER:
      switch (color) {
        case RED:
          #ifdef DEBUG_LED
            Serial.print(" - Entkuppler: Red: on");
          #endif
          AddOn[acc_articles[acc_num].Modul].digitalWrite(acc_articles[acc_num].pin_led_red, power);
          break;
        case GREEN:
          #ifdef DEBUG_LED
            Serial.print(" - Entkuppler: Green: on");
          #endif
          AddOn[acc_articles[acc_num].Modul].digitalWrite(acc_articles[acc_num].pin_led_grn, power);
          break;
      }
      break;
  }
  #ifdef DEBUG_LED
    if (power == POWER_ON)  Serial.println(" - Power: ON");
    if (power == POWER_OFF) Serial.println(" - Power: OFF");
  #endif
  acc_articles[acc_num].power_is = power;
  acc_articles[acc_num].state_is = color;
  EEPROM.put(acc_articles[acc_num].reg_state, acc_articles[acc_num].state_is);

}
#endif


//#############################################################################
// push button action
//#############################################################################
void button_pushed(uint8_t acc_num, bool color, bool state) {
  /*
   * color:     0 = red
   *            1 = green
   * state:     0 = not pushed
   *            1 = pushed
   */
  switch (state) {
    case BUTTON_PRESSED:
      if (acc_articles[acc_num].pushed == BUTTON_NOT_PRESSED) {
        
        #ifdef DEBUG_INPUT
          Serial.print(millis());
          if ( color == RED ) {
            Serial.print(" - send switchcommand RED to ACC #");
          }
          if ( color == GREEN ) {
            Serial.print(" - send switchcommand GREEN to ACC #");
          }
          Serial.println(mcan.getadrs(prot, acc_articles[acc_num].locID));
        #endif
        mcan.sendAccessoryFrame(device, acc_articles[acc_num].locID, color, false, state);
        acc_articles[acc_num].pushed = millis();
        
      } else if ( (acc_articles[acc_num].acc_type == TYPE_ENTKUPPLER)
                && (millis() - acc_articles[acc_num].pushed >= acc_interval) ) {
                  
        #ifdef DEBUG_INPUT
          Serial.print(millis());
          if ( color == RED )  {
            Serial.print(" - resend switchcommand RED to ACC #");
          }
          if ( color == GREEN ) {
            Serial.print(" - resend switchcommand GREEN to ACC #");
          }
          Serial.println(mcan.getadrs(prot, acc_articles[acc_num].locID));
        #endif
        acc_articles[acc_num].pushed = millis();
        mcan.sendAccessoryFrame(device, acc_articles[acc_num].locID, color, false, state);
        
      }
      #ifdef LED_FEEDBACK
        acc_articles[acc_num].state_set = color;
        acc_articles[acc_num].power_set = state;
      #endif
      break;

      
    case BUTTON_NOT_PRESSED:
      if (acc_articles[acc_num].pushed > BUTTON_NOT_PRESSED) {
        #ifdef DEBUG_INPUT
          Serial.print(millis());
          Serial.print(" - send power-off command to ACC #");
          Serial.println(mcan.getadrs(prot, acc_articles[acc_num].locID));
        #endif
        acc_articles[acc_num].pushed = state;
        mcan.sendAccessoryFrame(device, acc_articles[acc_num].locID, color, false, state);

        #ifdef LED_FEEDBACK
          acc_articles[acc_num].state_set = color;
          acc_articles[acc_num].power_set = state;
        #endif
        
      }
      break;
  }
  
}


//#############################################################################
// XXXXXXXXXXXXXXXXXXXXXXXX
//#############################################################################


//#############################################################################
// Main loop
//#############################################################################
void loop(){

  #ifdef run_fake_acc_commands
    if(acc_articles[tmp].acc_type==1) button_pushed(tmp, acc_articles[tmp].state_set, BUTTON_PRESSED);
  #endif
  
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    /*
    detachInterrupt(digitalPinToInterrupt(2));
    delay(50);
    attachInterrupt(digitalPinToInterrupt(2), interruptFn, LOW);
    */
    state_LED = !state_LED;
    digitalWrite(9, state_LED);
    if (locked == true) {
      interval = 100;
    } else {
      interval = 1000;
    }

    #ifdef run_fake_acc_commands
      button_pushed(tmp, acc_articles[tmp].state_set, BUTTON_NOT_PRESSED);
      interval = random(1500,3000);
      tmp = random(0,NUM_ACCs);
      acc_articles[tmp].state_set = !acc_articles[tmp].state_set;
      acc_articles[tmp].power_set = !acc_articles[tmp].power_set;
    
      tmp = random(0,NUM_ACCs);
      button_pushed(tmp, !acc_articles[tmp].state_set, BUTTON_PRESSED);
    #endif
  }
  
  //==================================================================================================
  // STELL SCHLEIFE
  //==================================================================================================
  for (int i = 0; i < NUM_ACCs; i++) {
    //================================================================================================
    // LED_FEEDBACK
    //================================================================================================
    #ifdef LED_FEEDBACK
      if ( (acc_articles[i].acc_type == TYPE_WEICHE) 
          && (acc_articles[i].state_is != acc_articles[i].state_set) ) {
            
        switchLED(i, acc_articles[i].state_set, acc_articles[i].power_set);
        
      } else if ( (acc_articles[i].acc_type == TYPE_ENTKUPPLER)
          && (acc_articles[i].power_is != acc_articles[i].power_set) ) {
            
        switchLED(i, acc_articles[i].state_set, acc_articles[i].power_set);
        
      }
    #endif
    //================================================================================================
    // ENDE - LED_FEEDBACK
    //================================================================================================
    //================================================================================================
    // TASTER ABFRAGEN
    //================================================================================================
    if ( (locked == false) && (currentMillis - previousMillis_input >= input_interval) ) {
      if (AddOn[acc_articles[i].Modul].digitalRead(acc_articles[i].pin_red) == LOW) {
        
        button_pushed(i, RED, BUTTON_PRESSED);
        
      } else if (AddOn[acc_articles[i].Modul].digitalRead(acc_articles[i].pin_grn) == LOW) {
        
        button_pushed(i, GREEN, BUTTON_PRESSED);

        #ifdef LED_FEEDBACK
      } else {
        
        button_pushed(i, acc_articles[i].state_is, BUTTON_NOT_PRESSED);

        #endif
      }

    }
    //================================================================================================
    // ENDE - TASTER ABFRAGEN
    //================================================================================================
  }
  //==================================================================================================
  // ENDE - STELL SCHLEIFE
  //==================================================================================================
  //==================================================================================================
  // CONFIG_POLL
  //==================================================================================================
  if(config_poll){
    if(config_index == 0) {
      #ifdef DEBUG_CONFIG
        Serial.print(" - ");
        Serial.print(config_index);
        Serial.println("   - Send Device Infos");
      #endif
      mcan.sendDeviceInfo(device, CONFIG_NUM);
    } else
    if(config_index == 1) {
      #ifdef DEBUG_CONFIG
        Serial.print(" - ");
        Serial.print(config_index);
        Serial.print("   - Send protocoltype: ");
        Serial.print(EEPROM.read(REG_PROT));
        Serial.println(" ( 0 = DCC / 1 = MM )");
      #endif
      mcan.sendConfigInfoDropdown(device, 1, 2, EEPROM.read(REG_PROT), "Protokoll_DCC_MM");
    } else
    if(config_index >= 2){
#ifdef use_uncuppler
      //----------------------------------------------------------------------------------------------
      //mit Entkuppler:
      //----------------------------------------------------------------------------------------------
      uint8_t ci_acc_num = ( config_index / 2 ) - 1;
      if ( (config_index % 2) > 0) {
        //--------------------------------------------------------------------------------------------
        // TYP: 
        //--------------------------------------------------------------------------------------------
        #ifdef DEBUG_CONFIG
          //Serial.print(millis());
          Serial.print(" - ");
          Serial.print(config_index);
          if (config_index < 10) {
            Serial.print("   - Send Config Dropdown for ACC_NUM: ");
          } else if (config_index < 100) {
            Serial.print("  - Send Config Dropdown for ACC_NUM: ");
          } else {
            Serial.print(" - Send Config Dropdown for ACC_NUM: ");
          }
          Serial.print(ci_acc_num);
          Serial.print(" - Magnetartikeltyp: ");
          Serial.println(acc_articles[ci_acc_num].acc_type);
        #endif
        mcan.sendConfigInfoDropdown(device, config_index, 2, acc_articles[ci_acc_num].acc_type, "Artikeltyp_WS_EK");
        
      } else {
        //--------------------------------------------------------------------------------------------
        // ADRESSE: 
        //--------------------------------------------------------------------------------------------
        string1 = "Adrs ";
        string2 = "_1_2048";
        string3 = String(ci_acc_num);
        string4 = string1 + string3 + string2;
        uint16_t adrs = mcan.getadrs(prot, acc_articles[ci_acc_num].locID);
        #ifdef DEBUG_CONFIG
          Serial.print(" - ");
          Serial.print(config_index);
          if (config_index < 10) {
            Serial.print("   - Send Config Slider   for ACC_NUM: ");
          } else if (config_index < 100) {
            Serial.print("  - Send Config Slider   for ACC_NUM: ");
          } else {
            Serial.print(" - Send Config Slider   for ACC_NUM: ");
          }
          Serial.print(ci_acc_num);
          Serial.print(" - Adresse: ");
          Serial.println(adrs);
        #endif
        mcan.sendConfigInfoSlider(device, config_index, 1, 2048, adrs, string4);
        
      }
#else
      //----------------------------------------------------------------------------------------------
      // Ohne Entkuppler
      //----------------------------------------------------------------------------------------------
      string1 = "Adrs ";
      string2 = "_1_2048";
      string3 = String(config_index - 1);
      string4 = string1 + string3 + string2;
      uint16_t adrs = mcan.getadrs(prot, acc_articles[ config_index - start_adrs_channel ].locID);
        #ifdef DEBUG_CONFIG
          Serial.print(" - ");
          Serial.print(config_index);
          if (config_index < 10) {
            Serial.print("   - Send Config Slider   for ACC_NUM: ");
          } else if (config_index < 100) {
            Serial.print("  - Send Config Slider   for ACC_NUM: ");
          } else {
            Serial.print(" - Send Config Slider   for ACC_NUM: ");
          }
          Serial.print(config_index-1);
          Serial.print(" Adresse: ");
          Serial.println(adrs);
          Serial.println(string3);
        #endif
      mcan.sendConfigInfoSlider(device, config_index, 1, 2048, adrs, string4);
#endif      
    }
    config_poll = false;
  }
  //==================================================================================================
  // ENDE - CONFIG_POLL
  //==================================================================================================
  
  if (currentMillis - previousMillis_input >= input_interval ) {
    previousMillis_input = currentMillis;
  }
}


//#############################################################################
// ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE
//#############################################################################

/*
 * MäCAN-S88-GBS-AddOn, Software-Version 0.3
 *
 *  Created by Maximilian Goldschmidt <maxigoldschmidt@gmail.com>
 *  Modified by Jochen Kielkopf.
 *  Do with this whatever you want, but keep thes Header and tell
 *  the others what you changed!
 *
 *  Last edited: 2017-01-09
 */
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
 *       5 -> SWITCHMODE    -- not used
 *  6 -  7 -> SWITCHTIME    -- not used
 *       8 -> FEEDBACK      -- not used
 *       9 -> PROTOKOLL     -- not used
 * 10 - 19 -> reserved / free
 *      20 -> S88_Dev       -- CS2/3 (false) oder Link L88 (true) 
 * 21 - 22 -> S88 Device ID
 * 23 - 25 -> S88 Bus 0
 * 26 - 28 -> S88 Bus 1
 * 29 - 31 -> S88 Bus 2
 * 32 - 34 -> S88 Bus 3
 * ========================================================================
 ******************************************************************************/
#define VERS_HIGH 0       // Versionsnummer vor dem Punkt
#define VERS_LOW  3       // Versionsnummer nach dem Punkt
/******************************************************************************
 * Allgemeine Setup Daten:
 ******************************************************************************/
#define BOARD_NUM 1       // Identifikationsnummer des Boards (Anzeige in der CS2)
#define UID 0x19549995    // CAN-UID - please change. Must be unique

const uint8_t USE_ONBOARD = 0;        // On-Board Ausgänge benutzen: 0 = Nein; 1 = Ja
                                      // USE_ONBOARD aktuell nicht unterstützt.

const uint8_t ANZ_ADDONS = 5;         // Anzahl AddOn Platinen (max 8)
//const uint8_t ANZ_ADDONS = 4;         // Anzahl AddOn Platinen (max 8)
                                      // Ab >6 Modulen wird der Arbeitsspeicher auf den ATMega328 knapp.

const uint8_t ANZ_ACC_PER_ADDON = 16; // Anzahl an Ausgängen pro AddOn Platine: Default = 16
                                      // => Relais = 8
                                      // => Weichen = 8
                                      // => Weichen mit Lagerückmeldung = 5 (aktuelle AddOn-HW unterstützt dies noch nicht.)
                                      // ==>> evtl. ab AddOn-HW Rev. b

bool S88_Dev = false;      // false = CS2/3 ; true = Link L88
uint16_t modulID = 0;   // ID des Link L88

/******************************************************************************
 * Debugging einschalten
 * Debbuging is only possible with max 2 AddOns.
 ******************************************************************************/
//#define DEBUG

/******************************************************************************
 * Allgemeine Konstanten:
 ******************************************************************************/
typedef struct {
  uint8_t Modul;        // 0 = On-Board; 1 = Modul1; 2 = Modul2, ...
  uint16_t kontakt;
  uint8_t pin;          // PIN
  bool state_is;        // aktueller LED Schaltzustand - kann ggf. weggelassen werden, dann werden aber in jedem loop() die LEDs neu angesteuert
  bool state_set;       // LED Soll Zustand
} s88_contacts;

bool use_bus[4];
uint16_t start_bus[4];
uint8_t len_bus[4];
uint16_t last_bus[4];

s88_contacts acc_articles[ (8 * USE_ONBOARD) + (ANZ_ADDONS * ANZ_ACC_PER_ADDON)]; // = 8 + (ANZ_ACC_PER_ADDON * ANZ_ADDONS)

Adafruit_MCP23017 AddOn[ANZ_ADDONS];   // Create AddOn

uint8_t NUM_ACCs;


unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
const long interval = 1000;
bool state_LED = false;

/******************************************************************************
 * Variablen für das initiale abfragen der Rückmeldekontakte
 ******************************************************************************/
//uint8_t checked_S88 = 0;
//unsigned long previousMillis2 = 0;
//const long interval2 = 250;

/******************************************************************************
 * Variablen der Magnetartikel:
 ******************************************************************************/
uint8_t CONFIG_NUM;   // Anzahl der Konfigurationspunkte
uint16_t hash;
bool config_poll = false;
byte uid_mat[4];
uint8_t config_index = 0;

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
    Serial.println("Boot Sequence initiated.");
    Serial.println("");
  #endif

  uid_mat[0] = UID >> 24;
  uid_mat[1] = UID >> 16;
  uid_mat[2] = UID >> 8;
  uid_mat[3] = UID;

  if( (EEPROM.read(1)==uid_mat[0]) && (EEPROM.read(2)==uid_mat[1]) && (EEPROM.read(3)==uid_mat[2]) && (EEPROM.read(4)==uid_mat[3]) ){
    #ifdef DEBUG
      Serial.println("UID is already set or has not changed => no initial Setup needed.");
    #endif
  } else {
    #ifdef DEBUG
      Serial.println("UID not set or has changed.");
      Serial.print("Initital Setup of EEPROM");
    #endif
    EEPROM.put(0, 0);
    EEPROM.put(1, uid_mat[0]);
    EEPROM.put(2, uid_mat[1]);
    EEPROM.put(3, uid_mat[2]);
    EEPROM.put(4, uid_mat[3]);
    EEPROM.put(20, S88_Dev);        // S88_Dev => 0 = USE CS2/3 S88 Bus ; 1 = USE Link L88
    EEPROM.put(21, modulID >> 8);   // modulID => Link L88 ID (High Byte)
    EEPROM.put(22, modulID);        // modulID => Link L88 ID (Low Byte)

    EEPROM.put(23, 1);    // use_bus0     ; Link L88 Interner BUS0 (1-16) oder CS2/3
    EEPROM.put(24, 1);    // len_bus0     ; Link L88 Interner BUS0 (1-16) oder CS2/3
    EEPROM.put(25, 1);    // start_bus0   ; Link L88 Interner BUS0 (1-16) oder CS2/3

    EEPROM.put(26, 1);    // use_bus1     ; Link L88 BUS1 (1001-1512)
    EEPROM.put(27, 1);    // len_bus1     ; Link L88 BUS1 (1001-1512)
    EEPROM.put(28, 1);    // start_bus1   ; Link L88 BUS1 (1001-1512)

    EEPROM.put(29, 0);    // use_bus2     ; Link L88 BUS2 (2001-2512)
    EEPROM.put(30, 1);    // len_bus2     ; Link L88 BUS2 (2001-2512)
    EEPROM.put(31, 1);    // start_bus2   ; Link L88 BUS2 (2001-2512)

    EEPROM.put(32, 0);    // use_bus3     ; Link L88 BUS3 (3001-3512)
    EEPROM.put(33, 1);    // len_bus3     ; Link L88 BUS3 (3001-3512)
    EEPROM.put(34, 1);    // start_bus3   ; Link L88 BUS3 (3001-3512)
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
  device.artNum = "MäCAN-0054";
  device.name = "MäCAN S88-GBS";
  device.boardNum = BOARD_NUM;
  device.type = MCAN_S88_GBS;

  NUM_ACCs = ANZ_ACC_PER_ADDON * ANZ_ADDONS;
  #ifdef DEBUG
    Serial.print("Number of RMs: ");
    Serial.println(NUM_ACCs);
  #endif

  setup_s88();
  signal_setup_successfull();
  #ifndef DEBUG
    test_leds();
  #endif
  #ifdef DEBUG
    Serial.println("Initiate CAN-Bus with debugging option");
    mcan.initMCAN(true);       // debugging
  #else
    mcan.initMCAN();       // no debugging
  #endif
  attachInterrupt(digitalPinToInterrupt(2), interruptFn, LOW);
  #ifdef DEBUG
    Serial.println("...completed.");
  #endif

  signal_setup_successfull();
  state_LED = 1;
  digitalWrite(9,state_LED);
  #ifdef DEBUG
    Serial.println("Setup completed.");
    Serial.println("Device is now ready...");
    Serial.println("-----------------------------------");
  #endif

}


/*
 * Setup Funktion zum Einrichten der S88
 */
void setup_s88() {
  #ifdef DEBUG
    Serial.println("Setting up S88");
  #endif

  S88_Dev = EEPROM.read(20);
  modulID = EEPROM.read(21) << 8 | EEPROM.read(22);

  if (S88_Dev) {
  /*
   * Link L88 Konfiguration
   */
    #ifdef DEBUG
      Serial.println("... will be setup with Link L88 S88 Bus configuration.");
    #endif
    CONFIG_NUM = 12;

    use_bus[0] = EEPROM.read(23);
    len_bus[0] = 1;
    start_bus[0] = 1;

    use_bus[1] = EEPROM.read(26);
    len_bus[1] = EEPROM.read(27);
    start_bus[1] = 1001 + ( 16 * ( EEPROM.read(28) - 1 ) );

    use_bus[2] = EEPROM.read(29);
    len_bus[2] = EEPROM.read(30);
    start_bus[2] = 2001 + ( 16 * ( EEPROM.read(31) - 1 ) );

    use_bus[3] = EEPROM.read(32);
    len_bus[3] = EEPROM.read(33);
    start_bus[3] = 3001 + ( 16 * ( EEPROM.read(34) - 1 ) );

    for(int b = 0; b < 4; b++) {
      if(use_bus[b]) {
        last_bus[b] = start_bus[b] + ( 16 * len_bus[b] ) - 1;
      } else {
        len_bus[b] = 0;
        last_bus[b] = 0;
        start_bus[b] = 0;
      }
    }

    int num = 0;
    for(int m = 0; m < ANZ_ADDONS; m++){
      AddOn[m].begin(m);
      int pin = 0;
      if(use_bus[0]) {
        #ifdef DEBUG
          if (m == 0) {
            Serial.println("BUS 0 (1-16) will be configured");
            Serial.print(" * starting with Modul ");
            Serial.println(start_bus[0]);
            Serial.print(" * using ");
            Serial.print(len_bus[0]);
            Serial.println(" Modul(s).");
            delay(25);
          }
        #endif
        for(int i = 0; i < ANZ_ACC_PER_ADDON; i++){
          if (start_bus[0] <= last_bus[0]) {
            #ifdef DEBUG
              if (i == 0) {
                Serial.println(" ");
                Serial.print("AddOn #");
                Serial.print(m+1);
                Serial.print(" :");
              }
              Serial.print(start_bus[0]);
              Serial.print(" ");
            #endif
            acc_articles[num].Modul = m + 1;
            acc_articles[num].kontakt = start_bus[0];
            acc_articles[num].pin = pin;
            AddOn[m].pinMode(acc_articles[num].pin, OUTPUT);
            pin++;
            num++;
            start_bus[0]++;
          }
        }
      }
      if(use_bus[1]) {
        #ifdef DEBUG
          if (m == 0) {
            Serial.print("BUS 1 (1001-1512) will be configured for RMs ");
            Serial.print(start_bus[1]);
            Serial.print("-");
            Serial.println(last_bus[1]);
            Serial.print(" * starting with Modul ");
            Serial.println(EEPROM.read(28));
            Serial.print(" * using ");
            Serial.print(len_bus[1]);
            Serial.println(" Modul(s).");
            delay(25);
          }
        #endif
        for(int i = 0; i < ANZ_ACC_PER_ADDON; i++){
          if (start_bus[1] <= last_bus[1]) {
            #ifdef DEBUG
              if (i == 0) {
                Serial.println(" ");
                Serial.print("AddOn #");
                Serial.print(m+1);
                Serial.print(" :");
              }
              Serial.print(start_bus[1]);
              Serial.print(" ");
            #endif
            acc_articles[num].Modul = m + 1;
            acc_articles[num].kontakt = start_bus[1];
            acc_articles[num].pin = pin;
            AddOn[m].pinMode(acc_articles[num].pin, OUTPUT);
            pin++;
            num++;
            start_bus[1]++;
          }
        }
      }
      if(use_bus[2]) {
        #ifdef DEBUG
          if (m == 0) {
            Serial.print("BUS 2 (2001-2512) will be configured for RMs ");
            Serial.print(start_bus[2]);
            Serial.print("-");
            Serial.println(last_bus[2]);
            Serial.print(" * starting with Modul ");
            Serial.println(EEPROM.read(31));
            Serial.print(" * using ");
            Serial.print(len_bus[2]);
            Serial.println(" Modul(s).");
            delay(25);
          }
        #endif
        if (start_bus[2] <= last_bus[2]) {
          for(int i = 0; i < ANZ_ACC_PER_ADDON; i++){
            #ifdef DEBUG
              if (i == 0) {
                Serial.println(" ");
                Serial.print("AddOn #");
                Serial.print(m+1);
                Serial.print(" :");
              }
              Serial.print(start_bus[2]);
              Serial.print(" ");
            #endif
            acc_articles[num].Modul = m + 1;
            acc_articles[num].kontakt = start_bus[2];
            acc_articles[num].pin = pin;
            AddOn[m].pinMode(acc_articles[num].pin, OUTPUT);
            pin++;
            num++;
            start_bus[2]++;
          }
        }
      }
      if(use_bus[3]) {
        #ifdef DEBUG
          if (m == 0) {
            Serial.print("BUS 3 (3001-3512) will be configured for RMs ");
            Serial.print(start_bus[3]);
            Serial.print("-");
            Serial.println(last_bus[3]);
            Serial.print(" * starting with Modul ");
            Serial.println(EEPROM.read(34));
            Serial.print(" * using ");
            Serial.print(len_bus[3]);
            Serial.println(" Modul(s).");
            delay(25);
          }
        #endif
        if (start_bus[3] <= last_bus[3]) {
          for(int i = 0; i < ANZ_ACC_PER_ADDON; i++){
            #ifdef DEBUG
              if (i == 0) {
                Serial.println(" ");
                Serial.print("AddOn #");
                Serial.print(m+1);
                Serial.print(" :");
              }
              Serial.print(start_bus[3]);
              Serial.print(" ");
            #endif
            acc_articles[num].Modul = m + 1;
            acc_articles[num].kontakt = start_bus[3];
            acc_articles[num].pin = pin;
            AddOn[m].pinMode(acc_articles[num].pin, OUTPUT);
            pin++;
            num++;
            start_bus[3]++;
          }
        }
      }
    }

  } else {
  /*
   * CS2/CS3plus S88 Bus Konfiguration
   */
    #ifdef DEBUG
      Serial.println("... will be setup with CS2 / CS3plus S88 Bus configuration.");
    #endif
    CONFIG_NUM = 4;

    use_bus[0] = 1;
    len_bus[0] = EEPROM.read(24);
    start_bus[0] = 1 + ( 16 * ( EEPROM.read(25) - 1 ) );

    use_bus[1] = 0;
    len_bus[1] = 0;
    start_bus[1] = 0;

    use_bus[2] = 0;
    len_bus[2] = 0;
    start_bus[2] = 0;

    use_bus[3] = 0;
    len_bus[3] = 0;
    start_bus[3] = 0;


    last_bus[0] = start_bus[0] + ( 16 * len_bus[0] ) - 1;

    int num = 0;
    for(int m = 0; m < ANZ_ADDONS; m++){
      AddOn[m].begin(m);
      int pin = 0;
      if(use_bus[0]) {
        #ifdef DEBUG
          if (m == 0) {
            Serial.print("CS2 / CS3plus Bus (1-512) will be configured for RMs ");
            Serial.print(start_bus[0]);
            Serial.print("-");
            Serial.println(last_bus[0]);
            Serial.print(" * starting with Modul ");
            Serial.println(start_bus[0]);
            Serial.print(" * using ");
            Serial.print(len_bus[0]);
            Serial.println(" Modul(s).");
            delay(25);
          }
        #endif
        for(int i = 0; i < ANZ_ACC_PER_ADDON; i++){
          if (start_bus[0] <= last_bus[0]) {
            #ifdef DEBUG
              if (i == 0) {
                Serial.println(" ");
                Serial.print("AddOn #");
                Serial.print(m+1);
                Serial.print(" :");
              }
              Serial.print(start_bus[0]);
              Serial.print(" ");
            #endif
            acc_articles[num].Modul = m + 1;
            acc_articles[num].kontakt = start_bus[0];
            acc_articles[num].pin = pin;
            AddOn[m].pinMode(acc_articles[num].pin, OUTPUT);
            pin++;
            num++;
            start_bus[0]++;
          }
        }
      }
    }
  }
  #ifdef DEBUG
    Serial.println("...completed");
  #endif
}


/*
 *  Aufleuchten aller Rückmelde-LEDs für 10 Sekunden
 */
void test_leds(){
    for(int num = 0; num < NUM_ACCs; num++){
      AddOn[acc_articles[num].Modul - 1].digitalWrite(acc_articles[num].pin, HIGH);   // switch all LEDs on
    }
    delay(10000);                                                                     // wait 10 seconds
    for(int num = 0; num < NUM_ACCs; num++){
      AddOn[acc_articles[num].Modul - 1].digitalWrite(acc_articles[num].pin, LOW);    // switch all LEDs off
    }
      
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
 * Funktion zum schalten der LED Ausgänge (nur auf AddOn-Platinen)
 */
void switchLED(int acc_num, bool set_state){
  if(set_state){                   // rot
    AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin, HIGH);
    #ifdef DEBUG
      Serial.print("LED for contact ");
      Serial.print(acc_articles[acc_num].kontakt);
      Serial.println(" is on.");
    #endif
  } else {
    AddOn[acc_articles[acc_num].Modul-1].digitalWrite(acc_articles[acc_num].pin, LOW);
    #ifdef DEBUG
      Serial.print("LED for contact ");
      Serial.print(acc_articles[acc_num].kontakt);
      Serial.println(" is off.");
    #endif
  }
  acc_articles[acc_num].state_is = acc_articles[acc_num].state_set;

}


/*
 * Ausführen, wenn eine Nachricht verfügbar ist.
 * Nachricht wird geladen und anhängig vom CAN-Befehl verarbeitet.
 */
void interruptFn(){
  can_frame_in = mcan.getCanFrame();
  s88Frame();
  pingFrame();
  configFrame();
  statusFrame();

}


/*
 * Prüfen auf S88_EVENT.
 */
void s88Frame(){
  if((can_frame_in.cmd == S88_EVENT) && (can_frame_in.resp_bit == 1)){                //Abhandlung bei gültigem S88 Event
    uint16_t incoming_modulID = (can_frame_in.data[0] << 8 | can_frame_in.data[1]);   // bsp. 0x00b3 => 179 (L88)
    uint16_t kontakt = (can_frame_in.data[2] << 8 | can_frame_in.data[3]);            // bsp. 0x03fe => 1022 => Bus 1, Modul 2, Kontakt 6
    //uint8_t old_state = can_frame_in.data[4];                                         // bsp. 0x01   => 1
    uint8_t curr_state = can_frame_in.data[5];                                        // bsp. 0x00   => 0
    //uint16_t s_time = (can_frame_in.data[6] << 8 | can_frame_in.data[7]);             // bsp. 0x0190 => 400
    #ifdef DEBUG
      Serial.print("Recieved S88-Frame for modul: ");
      Serial.print(incoming_modulID);
      Serial.print(" / contact: ");
      Serial.println(kontakt);
    #endif
    for(int i = 0; i < NUM_ACCs; i++){
      if(incoming_modulID == modulID) {
        if(kontakt == acc_articles[i].kontakt) {
          #ifdef DEBUG
            Serial.println(" => match found -> Set target state.");
          #else
            Serial.println(" => no for me.");
          #endif
          acc_articles[i].state_set = curr_state;
        }
      }
    }
  }

}


/*
 * Alle S88 Kontakte abfragen  // unpraktisch, da hierfür viel Zeit verstreicht und nicht während setup() möglich.
 */
void check_S88(){
  for(int i = 0; i < NUM_ACCs; i++){
    mcan.checkS88StateFrame(device, modulID, acc_articles[i].kontakt);
  }
  
}


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
 * Config Frame
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
 * Status Frame
 */
void statusFrame(){
  if((can_frame_in.cmd == SYS_CMD) && (can_frame_in.resp_bit == 0) && (can_frame_in.data[4] == SYS_STAT)){
    if((uid_mat[0] == can_frame_in.data[0])&&(uid_mat[1] == can_frame_in.data[1])&&(uid_mat[2] == can_frame_in.data[2])&&(uid_mat[3] == can_frame_in.data[3])){
/*
if(config_index == 1)  mcan.sendConfigInfoDropdown(device,  1,      2, S88_Dev, "Welchen S88 Anschluss nutzen?_CS2/3_Link L88");
if(config_index == 2)  mcan.sendConfigInfoSlider(  device,  2,  0, 16383, modulID, "Gerätekennung_0_16383");

if (!S88_Dev){
  if(config_index == 3)  mcan.sendConfigInfoSlider(  device,  3,  0, 31, EEPROM.read(24), "Bus Länge_0_31");
  if(config_index == 4)  mcan.sendConfigInfoSlider(  device,  4,  1, 31, EEPROM.read(25), "Start Modul_0_31");
} else {
  if(config_index == 3)  mcan.sendConfigInfoDropdown(device,  3,      2, EEPROM.read(23), "Bus 0 nutzen?_Nein_Ja");

  if(config_index == 4)  mcan.sendConfigInfoDropdown(device,  3,      2, EEPROM.read(23), "Bus 1 nutzen?_Nein_Ja");
  if(config_index == 5)  mcan.sendConfigInfoSlider(  device,  4,  1, 31, EEPROM.read(24), "Länge Bus 1_0_31");
  if(config_index == 6)  mcan.sendConfigInfoSlider(  device,  5,  1, 31, EEPROM.read(25), "Start Modul Bus 1_0_31");

  if(config_index == 7)  mcan.sendConfigInfoDropdown(device,  6,      2, EEPROM.read(26), "Bus 2 nutzen?_Nein_Ja");
  if(config_index == 8)  mcan.sendConfigInfoSlider(  device,  7,  1, 31, EEPROM.read(27), "Länge Bus 2_0_31");
  if(config_index == 9)  mcan.sendConfigInfoSlider(  device,  8,  1, 31, EEPROM.read(28), "Start Modul Bus 2_0_31");

  if(config_index == 10)  mcan.sendConfigInfoDropdown(device,  9,      2, EEPROM.read(29), "Bus 3 nutzen?_Nein_Ja");
  if(config_index == 11) mcan.sendConfigInfoSlider(  device, 10,  1, 31, EEPROM.read(30), "Länge Bus 3_0_31");
  if(config_index == 12) mcan.sendConfigInfoSlider(  device, 11,  1, 31, EEPROM.read(31), "Start Modul Bus 3_0_31");
}
bool use_bus[3];
int start_bus[3];
int len_bus[3];
int last_bus[3];

*/
      if(CONFIG_NUM == 4) {
      /*
       * CS2/CS3plus S88 Bus Konfiguration
       */
        switch (can_frame_in.data[5]) {
        case 1:       // S88_Dev       -- CS2/3 (false) oder Links L88 (true) 
          mcan.statusResponse(device, can_frame_in.data[5]);
          if(S88_Dev != can_frame_in.data[7]) {
            EEPROM.put(20, can_frame_in.data[7]);
            if(can_frame_in.data[7] == 0){
              EEPROM.put(23, 1);
            }
            setup_s88();
          }
          break;
        case 2:       // Gerätekennung
          if(modulID != (can_frame_in.data[6] << 8 | can_frame_in.data[7]) ) {
            EEPROM.put(21, can_frame_in.data[6]);
            EEPROM.put(22, can_frame_in.data[7]);
            setup_s88();
          }
          mcan.statusResponse(device, can_frame_in.data[5]);
          break;
        case 3:       // Länge Bus
          if(len_bus[0] != can_frame_in.data[7]) {
            EEPROM.put(24, can_frame_in.data[7]);
            setup_s88();
          }
          mcan.statusResponse(device, can_frame_in.data[5]);
          break;
        case 4:       // Start Bus
          if(start_bus[0] != can_frame_in.data[7]){
            EEPROM.put(25, can_frame_in.data[7]);
            setup_s88();
          }
          mcan.statusResponse(device, can_frame_in.data[5]);
          break;
        }
      } else if (CONFIG_NUM == 12) {
        /*
         * Link L88 Konfiguration
         */
        switch (can_frame_in.data[5]) {
        case 1:       // S88_Dev       -- CS2/3 (false) oder Links L88 (true) 
          mcan.statusResponse(device, can_frame_in.data[5]);
          if(S88_Dev != can_frame_in.data[7]) {
            EEPROM.put(20, can_frame_in.data[7]);
            if(can_frame_in.data[7] == 0){
              EEPROM.put(23, 1);
            }
            setup_s88();
          }
          break;
        case 2:       // Gerätekennung
          if(modulID != (can_frame_in.data[6] << 8 | can_frame_in.data[7]) ) {
            EEPROM.put(21, can_frame_in.data[6]);
            EEPROM.put(22, can_frame_in.data[7]);
            setup_s88();
          }
          mcan.statusResponse(device, can_frame_in.data[5]);
          break;
        case 3:       // Use L88 Bus 0
          EEPROM.put(23, can_frame_in.data[7]);     // use   bus0
          EEPROM.put(24, can_frame_in.data[7]);     // len   bus0
          EEPROM.put(25, can_frame_in.data[7]);     // start bus0
          setup_s88();
          mcan.statusResponse(device, can_frame_in.data[5]);
          break;
        case 4:       // Use Link L88 BUS1 (1001-1512)
          if (use_bus[1] != can_frame_in.data[7]) {
            EEPROM.put(26, can_frame_in.data[7]);
            setup_s88();
          }
          mcan.statusResponse(device, can_frame_in.data[5]);
          break;
        case 5:       // Länge Link L88 BUS1
          if (len_bus[1] != can_frame_in.data[7]) {
            EEPROM.put(27, can_frame_in.data[7]);
            setup_s88();
          }
          mcan.statusResponse(device, can_frame_in.data[5]);
          break;
        case 6:       // Start Link L88 BUS1
          if (start_bus[1] != can_frame_in.data[7]) {
            EEPROM.put(28, can_frame_in.data[7]);
            setup_s88();
          }
          mcan.statusResponse(device, can_frame_in.data[5]);
          break;
        case 7:       // Use Link L88 BUS2 (2001-2512)
          if (use_bus[2] != can_frame_in.data[7]) {
            EEPROM.put(29, can_frame_in.data[7]);
            setup_s88();
          }
          mcan.statusResponse(device, can_frame_in.data[5]);
          break;
        case 8:       // Länge Link L88 BUS2
          if (len_bus[2] != can_frame_in.data[7]) {
            EEPROM.put(30, can_frame_in.data[7]);
            setup_s88();
          }
          mcan.statusResponse(device, can_frame_in.data[5]);
          break;
        case 9:       // Start Link L88 BUS2
          if (start_bus[2] != can_frame_in.data[7]) {
            EEPROM.put(31, can_frame_in.data[7]);
            setup_s88();
          }
          mcan.statusResponse(device, can_frame_in.data[5]);
          break;
        case 10:       // Use Link L88 BUS3 (3001-3512)
          if (use_bus[3] != can_frame_in.data[7]) {
            EEPROM.put(32, can_frame_in.data[7]);
            setup_s88();
          }
          mcan.statusResponse(device, can_frame_in.data[5]);
          break;
        case 11:       // Länge Link L88 BUS3
          if (len_bus[3] != can_frame_in.data[7]) {
            EEPROM.put(33, can_frame_in.data[7]);
            setup_s88();
          }
          mcan.statusResponse(device, can_frame_in.data[5]);
          break;
        case 12:       // Start Link L88 BUS3
          if (start_bus[3] != can_frame_in.data[7]) {
            EEPROM.put(34, can_frame_in.data[7]);
            setup_s88();
          }
          mcan.statusResponse(device, can_frame_in.data[5]);
          break;
        }

      }
    }
  }

}


void loop() {
  //detachInterrupt(digitalPinToInterrupt(2));
  /*
   * Unterbreche Interrup für loop() Abarbeitung
   */
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
  /*
   * Blinks Status LED on Decoder
   */
    previousMillis = currentMillis;
    state_LED = !state_LED;
    digitalWrite(9,state_LED);
  }
  
  /*
   * Alle S88 Kontakte einmal initial abfragen.
   * Alle 200ms wird 1 Kontakt abgefragt. 
   * Dauert für 128 Kontakte ca 32 sek)
   * Eigentlich unnötig, das die CS2 zum booten wesentlich länger braucht,
   * und das Modul eigentlich mit der CS2 eingschaltet wird.
   */
  /*
  if (checked_S88 < NUM_ACCs) {
    if(currentMillis - previousMillis2 >= interval2) {
      previousMillis2 = currentMillis;
      mcan.checkS88StateFrame(device, modulID, acc_articles[checked_S88].kontakt);
      checked_S88++;
    }
  }
  */


  for(int i = 0; i < NUM_ACCs; i++){
    if(acc_articles[i].state_is != acc_articles[i].state_set){
      /*
       * Switch RM LEDs
       */
      switchLED(i, acc_articles[i].state_set);
      #ifdef DEBUG
        Serial.print("Switching LED for contact ");
        Serial.print(acc_articles[i].kontakt);
        if(acc_articles[i].state_set == 1) {
          Serial.println(" on.");
        } else {
          Serial.println(" off.");
        }
      #endif

    }
  }

  if(config_poll){
    if(config_index == 0)  mcan.sendDeviceInfo(device, CONFIG_NUM);

    if(config_index == 1)  mcan.sendConfigInfoDropdown(device,  1,      2, S88_Dev, "Welchen S88 nutzen?_CS2/3plus_Link L88");
    if(config_index == 2)  mcan.sendConfigInfoSlider(  device,  2,  0, 16383, modulID, "Gerätekennung_0_16383");

    if (!S88_Dev){
      if(config_index == 3)  mcan.sendConfigInfoSlider(  device,  3,  0, 31, len_bus[0],   "Bus Länge_0_31");
      if(config_index == 4)  mcan.sendConfigInfoSlider(  device,  4,  1, 31, start_bus[0], "Start Modul_0_31");
    } else {
      if(config_index == 3)  mcan.sendConfigInfoDropdown(device,  3,      2, use_bus[0],   "Bus 0 nutzen?_Nein_Ja");

      if(config_index == 4)  mcan.sendConfigInfoDropdown(device,  4,      2, use_bus[1],   "Bus 1 nutzen?_Nein_Ja");
      if(config_index == 5)  mcan.sendConfigInfoSlider(  device,  5,  1, 31, len_bus[1],   "Länge Bus 1_0_31");
      if(config_index == 6)  mcan.sendConfigInfoSlider(  device,  6,  1, 31, start_bus[1], "Start Modul Bus 1_0_31");

      if(config_index == 7)  mcan.sendConfigInfoDropdown(device,  7,      2, use_bus[2],   "Bus 2 nutzen?_Nein_Ja");
      if(config_index == 8)  mcan.sendConfigInfoSlider(  device,  8,  1, 31, len_bus[2],   "Länge Bus 2_0_31");
      if(config_index == 9)  mcan.sendConfigInfoSlider(  device,  9,  1, 31, start_bus[2], "Start Modul Bus 2_0_31");

      if(config_index == 10) mcan.sendConfigInfoDropdown(device, 10,      2, use_bus[3],   "Bus 3 nutzen?_Nein_Ja");
      if(config_index == 11) mcan.sendConfigInfoSlider(  device, 11,  1, 31, len_bus[3],   "Länge Bus 3_0_31");
      if(config_index == 12) mcan.sendConfigInfoSlider(  device, 12,  1, 31, start_bus[3], "Start Modul Bus 3_0_31");
    }
    config_poll = false;
  }
  //attachInterrupt(digitalPinToInterrupt(2), interruptFn, LOW);

}

/******************************************************************************
 * DO NOT CHANGE
 * >>>>>>>>>>>>
 ******************************************************************************/
#include <MCAN.h>
#include <EEPROM.h>

#if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))  // teensy 3.0/3.1-3.2/LC/3.5/3.6
  #include <FlexCAN.h>
#endif
MCAN mcan;
CanDevice device;
/******************************************************************************
 * <<<<<<<<<<<<<
 * DO NOT CHANGE
 ******************************************************************************
 * Debug options:
 ******************************************************************************/
#ifdef DEBUG
  #define DEBUG_CONFIG
  //#define DEBUG_CAN
  #define DEBUG_SETUP
  //#define DEBUG_MCAN
#endif
/******************************************************************************
 * Allgemeine Setup Daten:
 ******************************************************************************/
#define VERS_HIGH 0       // Versionsnummer vor dem Punkt
#define VERS_LOW  1       // Versionsnummer nach dem Punkt
#define BOARD_NUM 1       // Identifikationsnummer des Boards (Anzeige in der CS2)
#define UID 0x10143978    // CAN-UID - please change. Must be unique
/******************************************************************************
 * DO NOT CHANGE
 * >>>>>>>>>>>>
 ******************************************************************************/

/******************************************************************************
 * Allgemeine Konstanten:
 ******************************************************************************/
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

/******************************************************************************
   Variablen der Magnetartikel:
 ******************************************************************************/
uint8_t  CONFIG_NUM_INPUT = 3;   // Anzahl der Konfigurationspunkte
uint16_t prot = MM_ACC;
uint16_t prot_old = prot;

#include "22_OwnSetup.h"

/******************************************************************************
 ******************************************************************************
 ******************************************************************************
 ******************************************************************************/
//#############################################################################
// Change LocalID when protocol has changed (gloabl)
//#############################################################################
void change_global_prot() {
  for (int i = 0; i < NUM_ACCs; i++) {
    uint16_t adrsss = mcan.getadrs(prot_old, acc_articles[i].locID);
    acc_articles[i].locID = mcan.generateLocId(prot, adrsss );
  }
}
/******************************************************************************
 * Hintergrundklasse:
 * - prüft eingehende CAN Daten
 ******************************************************************************/
#if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))  // teensy 3.0/3.1-3.2/LC/3.5/3.6
class BackgroundClass : public CANListener {
#else
class BackgroundClass {
#endif
private:
  MCANMSG mcan_frame;
public:
  void incomingFrame(MCANMSG &mcan_frame_in);
  #if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))  // teensy 3.0/3.1-3.2/LC/3.5/3.6
   bool frameHandler(CAN_message_t &frame, int mailbox, uint8_t controller); //overrides the parent version so we can actually do something
  #endif
};


void BackgroundClass::incomingFrame(MCANMSG &mcan_frame_in)//CAN_message_t &frame)
{
  //==================================================================================================
  // Frames ohne UID
  //==================================================================================================
  if ((mcan_frame_in.cmd == CAN_PING) && (mcan_frame_in.resp_bit == 0)) {
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
   } else if ((mcan_frame_in.cmd == SWITCH_ACC) && (mcan_frame_in.resp_bit == 0)) {
    //------------------------------------------------------------------------------------------------
    // START - ACC Frame            - Püfen auf Schaltbefehle
    //------------------------------------------------------------------------------------------------
    uint16_t locid = (mcan_frame_in.data[2] << 8) | mcan_frame_in.data[3];
    #ifdef DEBUG_CAN
      Serial.print(millis());
      Serial.print(" - Recieved ACC-Frame for ACC (local ID): ");
      Serial.println(locid);
    #endif

    for (int i = 0; i < NUM_ACCs; i++) {
      if (locid == acc_articles[i].locID) {
        #ifdef DEBUG_CAN
          Serial.print(millis());
          Serial.println("    => match found -> Set target state.");
        #endif
        acc_articles[i].state_set = mcan_frame_in.data[4];
        //acc_articles[i].power_set = mcan_frame_in.data[5];
        acc_articles[i].power_set = ON;
        break;
      }

    }
    //------------------------------------------------------------------------------------------------
    // ENDE - ACC Frame
    //------------------------------------------------------------------------------------------------
  } else if ((uid_mat[0] == mcan_frame_in.data[0]) && (uid_mat[1] == mcan_frame_in.data[1]) && (uid_mat[2] == mcan_frame_in.data[2]) && (uid_mat[3] == mcan_frame_in.data[3])) {
  //==================================================================================================
  // Befehle nur für eine UID
  //==================================================================================================
    #ifdef DEBUG_CAN
      Serial.print(millis());
      Serial.println(" - Recieved frame only for me:");
    #endif
    if ((mcan_frame_in.cmd == CONFIG) && (mcan_frame_in.resp_bit == 0)) {
      //----------------------------------------------------------------------------------------------
      // START - Config Frame       -
      //----------------------------------------------------------------------------------------------
      #ifdef DEBUG_CAN
        Serial.print(millis());
        Serial.print(" - Recieved config frame.  - INDEX:");
        Serial.println(mcan_frame_in.data[4]);
      #endif
      config_poll = true;
      config_index = mcan_frame_in.data[4];

      //----------------------------------------------------------------------------------------------
      // ENDE - Config Frame
      //----------------------------------------------------------------------------------------------
    } else if ((mcan_frame_in.cmd == SYS_CMD) && (mcan_frame_in.resp_bit == 0) && (mcan_frame_in.data[4] == SYS_STAT)) {
      //----------------------------------------------------------------------------------------------
      // START - Status Frame       -
      //----------------------------------------------------------------------------------------------
      #ifdef DEBUG_CAN
        Serial.print(millis());
        Serial.print(" - Recieved Status Frame for config_index: ");
        Serial.println(mcan_frame_in.data[5]);
      #endif
      //----------------------------------------------------------------------------------------------
      // mcan_frame_in.DATA[5]       => CONFIG_INDEX
      //----------------------------------------------------------------------------------------------
      if (mcan_frame_in.data[5] == 1) {
        //--------------------------------------------------------------------------------------------
        // Protokoll schreiben ( MM oder DCC)
        //--------------------------------------------------------------------------------------------
        prot_old = prot;
        if (!mcan_frame_in.data[7]) {
          prot = DCC_ACC;
        } else {
          prot = MM_ACC;
        }
        if (prot != prot_old) {
          #ifdef DEBUG_CAN
            Serial.print(millis());
            Serial.print(" -  => Changing protocol: ");
            Serial.print(prot_old);
            Serial.print(" -> ");
            Serial.println(prot);
          #endif
          change_global_prot();
        }
        mcan.statusResponse(device, mcan_frame_in.data[5]);

      } else if (mcan_frame_in.data[5] == 2) {
        //------------------------------------------------------------------------------------------
        // ADRESSE:
        //------------------------------------------------------------------------------------------
        #ifdef DEBUG_CAN
          Serial.print(millis());
          Serial.print(" -  => Changing base_address: ");
          Serial.print(mcan.getadrs(prot, acc_articles[0].locID));
          Serial.print(" -> ");
        #endif
        acc_articles[0].locID = mcan.generateLocId(prot, (mcan_frame_in.data[6] << 8) | mcan_frame_in.data[7] );
        base_address = mcan.getadrs(prot, acc_articles[0].locID);
        setup_zza();
        #ifdef DEBUG_CAN
          Serial.println(base_address);
        #endif
        mcan.statusResponse(device, mcan_frame_in.data[5]);

      }
      //----------------------------------------------------------------------------------------------
      // ENDE - Status Frame
      //----------------------------------------------------------------------------------------------
    }
  //==================================================================================================
  // ENDE - Befehle nur für eine UID
  //==================================================================================================
  } else if ((mcan_frame_in.cmd == SYS_CMD) && (mcan_frame_in.resp_bit == 0)) {
      if ( ( (uid_mat[0] == mcan_frame_in.data[0]) &&
             (uid_mat[1] == mcan_frame_in.data[1]) &&
             (uid_mat[2] == mcan_frame_in.data[2]) &&
             (uid_mat[3] == mcan_frame_in.data[3])
           ) || (
            (0 == mcan_frame_in.data[0]) &&
            (0 == mcan_frame_in.data[1]) &&
            (0 == mcan_frame_in.data[2]) &&
            (0 == mcan_frame_in.data[3])
          ) ){
      //----------------------------------------------------------------------------------------------
      // START - STOP / GO / HALT     -
      //----------------------------------------------------------------------------------------------
      uint8_t sub_cmd = mcan_frame_in.data[4];
      if ( (sub_cmd == SYS_STOP) || (sub_cmd == SYS_HALT) ) {
        //--------------------------------------------------------------------------------------------
        // STOP oder HALT             - Eingaben verhindern.
        //--------------------------------------------------------------------------------------------
        #ifdef DEBUG_CAN
          Serial.print(millis());
          Serial.println(" - System locked.");
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
  }

}
#if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))  // teensy 3.0/3.1-3.2/LC/3.5/3.6
bool BackgroundClass::frameHandler(CAN_message_t &frame, int mailbox, uint8_t controller)
{
    mcan_frame = mcan.getCanFrame(frame);
    incomingFrame(mcan_frame);
    return true;
}
#endif

BackgroundClass BackgroundClass;


//#############################################################################
// Ausführen, wenn eine Nachricht verfügbar ist.
// Nachricht wird geladen und anhängig vom CAN-Befehl verarbeitet.
//#############################################################################
void interruptFn() {
  MCANMSG mcan_frame_in = mcan.getCanFrame();
  BackgroundClass.incomingFrame(mcan_frame_in);
}



void fnc_config_poll() {
  //================================================================================================
  // CONFIG_POLL
  //================================================================================================
  if(config_index == 0) {
    #ifdef DEBUG_CONFIG
      Serial.print(millis());
      Serial.print(" - ");
      Serial.print(config_index);
      Serial.print("   - Send Device Infos - CONFIG_NUM_INPUT: ");
      Serial.println(CONFIG_NUM_INPUT);
    #endif
    mcan.sendDeviceInfo(device, CONFIG_NUM_INPUT);
  } else
  if(config_index == 1) {
    #ifdef DEBUG_CONFIG
      Serial.print(millis());
      Serial.print(" - ");
      Serial.print(config_index);
      Serial.print("   - Send protocoltype: ");
      Serial.print(prot);
      Serial.println(" ( 0 = DCC / 1 = MM )");
    #endif
    mcan.sendConfigInfoDropdown(device, 1, 2, prot, "Protokoll_DCC_MM");
  } else
  if(config_index == 2){
    uint16_t adrs = mcan.getadrs(prot, acc_articles[0].locID);
    mcan.sendConfigInfoSlider(device, config_index, 1, 2048, adrs, "Basis_Adresse: ");

  }
  config_poll = false;
  //================================================================================================
  // ENDE - CONFIG_POLL
  //================================================================================================

}


//#############################################################################
// Setup MäCAN
//#############################################################################
void setup_can() {
#ifdef DEBUG_SETUP
  Serial.println("-----------------------------------");
  Serial.println(" - Setup                         - ");
  Serial.println("-----------------------------------");
#endif

  uid_mat[0] = byte(UID >> 24);
  uid_mat[1] = byte(UID >> 16);
  uid_mat[2] = byte(UID >> 8);
  uid_mat[3] = byte(UID);

  //Geräteinformationen:
  device.versHigh = VERS_HIGH;
  device.versLow = VERS_LOW;
  device.hash = mcan.generateHash(UID);;
  device.uid = UID;
  device.artNum = "00057";
  device.name = "MäCAN Zugzielanzeiger";
  device.boardNum = BOARD_NUM;
  device.type = MCAN_ZZA;

#if (defined(__MK20DX256__) || defined(__MK64FX512__)|| defined(__MK66FX1M0__))
  pinMode(28, OUTPUT);
  digitalWrite(28, LOW);
#endif

  setup_zza();


#ifdef DEBUG_MCAN
  mcan.initMCAN(true);
#else
  mcan.initMCAN();
#endif

  CONFIG_NUM = 3;

#if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))  // teensy 3.0/3.1-3.2/LC/3.5/3.6
  Can0.attachObj(&BackgroundClass);
  BackgroundClass.attachGeneralHandler();
#else
  attachInterrupt(digitalPinToInterrupt(21), interruptFn, LOW);
#endif

}

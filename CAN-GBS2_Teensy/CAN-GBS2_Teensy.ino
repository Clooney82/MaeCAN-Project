unsigned long run = 0 ;
/*
   MäCAN-GBS2-AddOn , Software-Version 0.1

    Created by Maximilian Goldschmidt <maxigoldschmidt@gmail.com>
    Modified by Jochen Kielkopf.
    Do with this whatever you want, but keep thes Header and tell
    the others what you changed!

    Last edited: 2019-02-15

    Changelog:
 *  * v0.1:
      - Combined Version of MäCAN-S88-GBS and MäCAN-INPUT Addon
      - Modified for teensy support
      - Own Library for Multiple I2C Bus Support with MCP23017 Multiplexer
      - Code Split into multiple Files
      - Files:
        + 01-Configuration_01_global.h  -> Config file for global things
        + 01-Configuration_02_S88.h     -> Config file for S88 part
        + 01-Configuration_03_input.h   -> Config file for INPUT part
        + 02-CAN-Stuff.h                -> CAN related code
        + 03-Functions_01_global.h      -> Global functions
        + 03-Functions_02_S88.h         -> Functions only needed for S888 part
        + 03-Functions_03_input.h       -> Functions only needed for INPUT part
        + 04-OwnSetup.h                 -> Manual Config for INPUT ACC configuration
      - Combined use of Input and Output (S88) Moduls possible
      - S88 part can be setup via CS2/3 or manually in source code
      - INPUT part must be configured in source code!!!
        + configure in 04-OwnSetup.h
        + INFO: more than 32 config fields are difficult to handle on a Central Station 2 or 3
      - only CS2/CS3plus local S88 BUS or 1x Link S88 with 1 CAN-Decoder possible
        + if you want to Monitor CS2/CS3plus local S88 BUS and 1x Link S88 you´ll need 2 CAN-Decoders
        + if you want to Monitor multiple Link S88 you´ll need multiple CAN-Decoders
      - S88 part will be on the first, then INPUT part
      - S88 and INPUT modules can be on the same I2C bus
 *  * v0.2:
      -
      -

*/
#include <MCAN.h>
#include <EEPROM.h>
#if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))  // teensy 3.0/3.1-3.2/LC/3.5/3.6
#include <mcp23017.h>
#include <FlexCAN.h>
#else
#include <Adafruit_MCP23017.h>
#endif
MCAN mcan;
CanDevice device;

#include "01-Configuration_01_global.h" // This file contains the configuration of the program (Pin numbers, Options, ...)
#include "01-Configuration_02_S88.h"    // This file contains the configuration of the S88 Part 
#include "01-Configuration_03_input.h"  // This file contains the configuration of the Input Part
#include "02-CAN-Stuff.h"               // This file contains the complete CAN handling stuff
#include "03-Functions_01_global.h"     // This file contains the global used functions
#include "03-Functions_02_S88.h"        // This file contains the s88 functions
#include "04-OwnSetup.h"                // This file contains the manual config for the ACCs
#include "03-Functions_03_input.h"      // This file contains the input functions
//#include "05-other.h"                 // This file contains the other stuff


//#############################################################################
// Setup
//#############################################################################
void setup() {
#ifdef DEBUG_SERIAL
  Serial.begin(111520);
  delay(3000);
#endif
#ifdef DEBUG_SETUP
  Serial.println("-----------------------------------");
  Serial.println(" - Setup                         - ");
  Serial.println("-----------------------------------");
#endif

  uid_mat[0] = byte(UID >> 24);
  uid_mat[1] = byte(UID >> 16);
  uid_mat[2] = byte(UID >> 8);
  uid_mat[3] = byte(UID);

#ifdef DEBUG_SETUP
  Serial.print(" - UID:    ");
  Serial.println(UID);
  Serial.print(" - PARTS:  ");
  Serial.print(uid_mat[0]);
  Serial.print(" ");
  Serial.print(uid_mat[1]);
  Serial.print(" ");
  Serial.print(uid_mat[2]);
  Serial.print(" ");
  Serial.println(uid_mat[3]);
  Serial.print(" - EEPROM: ");
  Serial.print(EEPROM.read(REG_UID));
  Serial.print(" ");
  Serial.print(EEPROM.read(REG_UID + 1));
  Serial.print(" ");
  Serial.print(EEPROM.read(REG_UID + 2));
  Serial.print(" ");
  Serial.println(EEPROM.read(REG_UID + 3));
  delay(100);
#endif

  if ( (EEPROM.read(REG_UID) == uid_mat[0]) &&
       (EEPROM.read(REG_UID + 1) == uid_mat[1]) &&
       (EEPROM.read(REG_UID + 2) == uid_mat[2]) &&
       (EEPROM.read(REG_UID + 3) == uid_mat[3]) ) {
#ifdef DEBUG_SETUP
    Serial.println(" - UID was not changed: No Setup of EEPROM needed.");
#endif
  } else {
#ifdef DEBUG_SETUP
    Serial.println(" - UID was changed: Initital Setup of EEPROM");
#endif

    EEPROM.put(REG_INPUT_SETUP, 0);
    EEPROM.put(REG_UID, uid_mat[0]);
    EEPROM.put(REG_UID + 1, uid_mat[1]);
    EEPROM.put(REG_UID + 2, uid_mat[2]);
    EEPROM.put(REG_UID + 3, uid_mat[3]);

    init_s88_registers();
  }

  pinMode(STATUS_LED_PIN, OUTPUT);
  blink_LED();


  //Geräteinformationen:
  device.versHigh = VERS_HIGH;
  device.versLow = VERS_LOW;
  device.hash = mcan.generateHash(UID);;
  device.uid = UID;
  device.artNum = "00056";
  device.name = "MäCAN GBS2";
  device.boardNum = BOARD_NUM;
  device.type = MCAN_GBS2;

#if (defined(__MK20DX256__) || defined(__MK64FX512__)|| defined(__MK66FX1M0__))
  pinMode(28, OUTPUT);
  digitalWrite(28, LOW);
#endif
#ifdef DEBUG_SERIAL
  Serial.println("-----------------------------------");
  Serial.print(" - Board-UID:  ");
  Serial.println(UID);
  Serial.print(" - Board-Num:  ");
  Serial.println(BOARD_NUM);
  Serial.print(" - Config Num: ");
  Serial.println(CONFIG_NUM_S88);
  Serial.print(" - Num RMs:   ");
  Serial.println(NUM_S88);
  Serial.println("-----------------------------------");
  Serial.println(" - Device will now prepared...     -");
  Serial.println("-----------------------------------");
  delay(100);
#endif


  setup_s88();
  test_s88_leds();

  setup_input();


#ifdef DEBUG_MCAN
  mcan.initMCAN(true);
#else
  mcan.initMCAN();
#endif

  signal_setup_successfull();

  CONFIG_NUM = CONFIG_NUM_S88;
  // CONFIG_NUM = CONFIG_NUM_INPUT;

#if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))  // teensy 3.0/3.1-3.2/LC/3.5/3.6
  Can0.attachObj(&BackgroundClass);
  BackgroundClass.attachGeneralHandler();
#else
  attachInterrupt(digitalPinToInterrupt(2), interruptFn, LOW);
#endif

  blink_LED();

}


//#############################################################################
// Main loop
//#############################################################################
void loop() {
  /*
    Serial.print("loop:");
    Serial.println(run);
    run++;
  */
#ifdef run_fake_acc_commands
  if (acc_articles[tmp].acc_type == 1) button_pushed(tmp, acc_articles[tmp].state_set, BUTTON_PRESSED);
#endif
  //==================================================================================================
  // Blink Status LED
  //==================================================================================================
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    blink_LED();
    if (locked == true) {
      interval = 100;
    } else {
      interval = 1000;
    }

#ifdef run_fake_s88_events
    int randoms88 = random(0, NUM_S88);
    interval = random(100, 500);
    //s88_contacts[randoms88].state_set = random(0, 1);
    s88_contacts[randoms88].state_set = !s88_contacts[randoms88].state_set;
    /*
    Serial.print("...");
    Serial.print(randoms88);
    Serial.print(" - RM: ");
    Serial.print(s88_contacts[randoms88].rm);
    Serial.print(" IST:");
    Serial.print(s88_contacts[randoms88].state_is);
    Serial.print(" SOLL:");
    Serial.print(s88_contacts[randoms88].state_set);
    Serial.println();
    */
#endif
#ifdef run_fake_acc_commands
    button_pushed(tmp, acc_articles[tmp].state_set, BUTTON_NOT_PRESSED);
    interval = random(1500, 3000);
    tmp = random(0, NUM_ACCs);
    button_pushed(tmp, !acc_articles[tmp].state_set, BUTTON_PRESSED);
#endif
  }

  //================================================================================================
  // START - CONFIG_POLL
  //================================================================================================
  //------------------------------------------------------------------------------------------------
  // Wenn Konfig angefragt keine Abarbeitung von S88 Events oder sonstigem.
  // Grund: Die Abarbeitung der Konfig abfrage, sollte nicht zu lange dauern
  //        Es gehen dabei keine S88 Events verloren.
  //------------------------------------------------------------------------------------------------
  if (config_poll) {
    config_poll_s88();
    //config_poll_input();

  } else {
    //================================================================================================
    // LEDs schalten für S88 Anzeige
    //================================================================================================
    if (new_s88_setup_needed) {
      setup_s88();
      //read_all_s88_contact();   // Liest alle S88 Kontakte aus, dauert je nach Anzahl sehr lange
    }
    s88_loop();
    //================================================================================================
    // INPUT STELL SCHLEIFE
    //================================================================================================
    input_loop();
  }
}


//#############################################################################
// ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE
//#############################################################################

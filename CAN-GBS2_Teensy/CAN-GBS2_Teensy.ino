/*
   MäCAN-GBS2-AddOn

    Created by Maximilian Goldschmidt <maxigoldschmidt@gmail.com>
    Modified by Jochen Kielkopf.
    Do with this whatever you want, but keep thes Header and tell
    the others what you changed!

    Last edited: 2019-04-29

    Changelog:
 *  * v0.3:
      - fixes for turntable control
      - fixes for SINGLE_BUTTON inputs
      - fix wire0 to wire3 handly on teensy boards (new library needed)
      - DEFAULT: only wire0 is enabled
      - DEFAULT: 1x S88 and 1x Input (with LED_FEEDBACK) enabled
 *  * v0.2:
      - Cheange some switch cases into if states
      - added onboard handles for DS and GBS locking
      - added File:
        + 05-onboard.h                 -> functions to handle onboard stuff like GBS Locking.
      - added ACC Type: SINGLE_BUTTON input. Toggles between red and green
      - removed onboard functions for MäCAN Decoder, only AddOn Modules supported.
      - started English translation of code comments
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
      - max S88 RMs = 512 ( 8 Moduls x 4 I2C Busses x 16 RMs per Modul )
      - max INPUTs ( with LED_FEEDBACK )    = 128 ( 8 Moduls x 4 I2C Busses x 4 Inputs per Modul )
      - max INPUTs ( without LED_FEEDBACK ) = 256 ( 8 Moduls x 4 I2C Busses x 8 Inputs per Modul )

*/
#include <MCAN.h>
#include <EEPROM.h>

MCAN mcan;
CanDevice device;

#include "01-Configuration_01_global.h" // This file contains the configuration of the program (Pin numbers, Options, ...)
#include "01-Configuration_02_S88.h"    // This file contains the configuration of the S88 Part 
#include "01-Configuration_03_input.h"  // This file contains the configuration of the Input Part
#include "05-onboard.h"                 // This file handles teensy onboard INPUT/OUTPUT
#include "02-CAN-Stuff.h"               // This file contains the complete CAN handling stuff
#include "03-Functions_01_global.h"     // This file contains the global used functions
#include "03-Functions_02_S88.h"        // This file contains the s88 functions
#include "04-OwnSetup.h"                // This file contains the manual config for the ACCs
#include "03-Functions_03_input.h"      // This file contains the input functions


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
  Serial.println("-- Setup                         --");
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


  // Deviceinformations:
  device.versHigh = VERS_HIGH;
  device.versLow  = VERS_LOW;
  device.hash = mcan.generateHash(UID);
  device.uid  = UID;
  device.artNum = "00054";
  device.name = "MäCAN GBS v2";
  device.boardNum = BOARD_NUM;
  device.type = MCAN_GBS2;

#if (defined(__MK20DX256__) || defined(__MK64FX512__)|| defined(__MK66FX1M0__))
  pinMode(28, OUTPUT);
  digitalWrite(28, LOW);
#endif
#ifdef DEBUG_SERIAL
  Serial.println("-----------------------------------");
  Serial.print(" - Board:      ");
  Serial.println(device.name);
  Serial.print(" - Board-UID:  ");
  Serial.println(UID);
  Serial.print(" - Version:    ");
  Serial.print(VERS_HIGH);
  Serial.print(".");
  Serial.println(VERS_LOW);
  Serial.print(" - Board-Num:  ");
  Serial.println(BOARD_NUM);
  Serial.print(" - Num RMs:    ");
  Serial.println(NUM_S88);
  Serial.print(" - Num ACCS:   ");
  Serial.println(NUM_ACCs);
  Serial.println("-----------------------------------");
  Serial.println("-- preparing device...           --");
  Serial.println("-----------------------------------");
  delay(100);
#endif


  setup_s88();
  test_s88_leds();
  
  setup_input();
  #ifdef LED_FEEDBACK
    test_acc_leds();
    restore_last_state();
  #endif

  setup_onboard();


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
#ifdef DEBUG_SERIAL
  Serial.println("-----------------------------------");
  Serial.println(" - Setup finished");
  Serial.println("-----------------------------------");
  Serial.println("");
#endif  
}


//#############################################################################
// Main loop
//#############################################################################
void loop() {
#ifdef run_fake_acc_commands
  // only if uncoupler is active ( WARNING !!! IF CS2/3 is connected )
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
    s88_contacts[randoms88].state_set = !s88_contacts[randoms88].state_set;
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
  // EN:
  // If config is requestet no S88 events or so will be procesed.
  // Cause: Sending config should not take to much time
  //        No S88 Events are lost.
  // DE: 
  // Wenn Konfig angefragt keine Abarbeitung von S88 Events oder sonstigem.
  // Grund: Die Abarbeitung der Konfig abfrage, sollte nicht zu lange dauern
  //        Es gehen dabei keine S88 Events verloren.
  //------------------------------------------------------------------------------------------------
  if (config_poll) {
    config_poll_s88();
    //config_poll_input();

  } else {
    //================================================================================================
    // switch LEDs for S88
    //================================================================================================
    if (new_s88_setup_needed) {
      setup_s88();
      //read_all_s88_contact();   // reads status of all S88 contacts. Runtime could be very long, depends on number of S88 contacts.
    }
    s88_loop();
    //================================================================================================
    // Lock GBS INPUT
    //================================================================================================
    check_lock();

    //================================================================================================
    // INPUT & TURNTABLE loop
    //================================================================================================
    input_loop();
    if (!GBS_locked) {
      
      // TURNTABLE
      check_DS_input();
    }
    
  }
}


//#############################################################################
// END END END END END END END END END END END END END END END END END END END
//#############################################################################

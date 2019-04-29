//#############################################################################
// Setup function for ACCs
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
    Serial.println("-----------------------------------");
    Serial.println(" - Setting up Input ACCs         - ");
    Serial.println("-----------------------------------");
    Serial.println("");
    delay(100);
  #endif
  //--------------------------------------------------------------------------------------------------
  // Load Default Values
  //--------------------------------------------------------------------------------------------------
  if (!EEPROM.read(REG_INPUT_SETUP)) {
    EEPROM.put(REG_INPUT_SETUP, ON);
    #ifdef DEBUG_SETUP
      Serial.println(" - ACC Setup not done.");
      Serial.println(" - Loading default values.");
      Serial.println("");
    #endif
    //for (int i = 20; i < 1024; i++) EEPROM.put(i,0);
    config_own_adresses_manual();
  }
  //--------------------------------------------------------------------------------------------------
  // Setup PINs
  //--------------------------------------------------------------------------------------------------
  #ifdef DEBUG_SETUP
    Serial.println(" - Loading configuration from EEPROM");
    Serial.println("");
  #endif
  int num = 0;

  if ( ANZ_S88_ADDONS <= 8 ) {
    board_num = ANZ_S88_ADDONS;
  } else if ( ANZ_S88_ADDONS <= 16 ) {
    board_num = ANZ_S88_ADDONS - 8;
  } else if ( ANZ_S88_ADDONS <= 24 ) {
    board_num = ANZ_S88_ADDONS - 16;
  } else if ( ANZ_S88_ADDONS <= 32 ) {
    board_num = ANZ_S88_ADDONS - 24;
  }
  
  
  if (ANZ_ACC_ADDONS > 0) {
    for (int m = ANZ_S88_ADDONS; m < ANZ_ADDONS; m++) {
      #if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))  // teensy 3.0/3.1-3.2/LC/3.5/3.6
        if ( m < i2c_bus0_len ) {                                                           // i2c_bus0_len = 2 -> m = 0..1
          i2c_bus_con = 0;
        } else if ( m < ( i2c_bus0_len + i2c_bus1_len ) ) {                                 // i2c_bus1_len = 2 -> m = 2..4
          i2c_bus_con = 1;
        } else if ( m < ( i2c_bus0_len + i2c_bus1_len + i2c_bus2_len ) ) {                  // i2c_bus2_len = 2 -> m = 5..6
          i2c_bus_con = 2;
        } else if ( m < ( i2c_bus0_len + i2c_bus1_len + i2c_bus2_len + i2c_bus3_len ) ) {   // i2c_bus3_len = 2 -> m = 7..8
          i2c_bus_con = 3;
        } 
        #ifdef DEBUG_SETUP
          Serial.println("");
          Serial.print("I2C-BUS: ");
          Serial.print(i2c_bus_con);
          Serial.print(" | Board: ");
          Serial.print(board_num);
          Serial.print(" | Modul: ");
          Serial.print(m);
          Serial.print(" | AddOn #");
          Serial.println(m+1);
        #endif
        
        if ( i2c_bus_con == 0 ) {
          AddOn[board_num].begin(board_num, i2c_bus_con);
        #ifdef USE_WIRE1
        } else if ( i2c_bus_con == 1 ) {
          AddOn_W1[board_num].begin(board_num, i2c_bus_con);
        #endif
        #ifdef USE_WIRE2
        } else if (i2c_bus_con == 2 ) {
          AddOn_W2[board_num].begin(board_num, i2c_bus_con);
        #endif
        #ifdef USE_WIRE3
        } else if ( i2c_bus_con == 3 ) {
          AddOn_W3[board_num].begin(board_num, i2c_bus_con);
        #endif
        }

      #else
        AddOn[m].begin(m);
      #endif
      int pin = 0;
      for (int i = 0; i < ANZ_ACC_PER_ADDON; i++) {
        acc_articles[num].reg_locid = (200 + (6 * (num + 1)) - 5);
        acc_articles[num].locID     = (EEPROM.read( acc_articles[num].reg_locid ) << 8) | (EEPROM.read( acc_articles[num].reg_locid + 1 ));
        acc_articles[num].reg_type  = (200 + (6 * (num + 1))    );
        acc_articles[num].acc_type  = EEPROM.read( acc_articles[num].reg_type );
        acc_articles[num].reg_prot  = (200 + (6 * (num + 1)) - 2);
        acc_articles[num].prot      = EEPROM.read( acc_articles[num].reg_prot );

        acc_articles[num].i2c_bus = i2c_bus_con;
        acc_articles[num].board_num = board_num;
        acc_articles[num].Modul = m;
        acc_articles[num].pin_grn = pin;
        pin++;
        if ( acc_articles[num].acc_type == TYPE_SINGLE_BUTTON ) { 
          acc_articles[num].pin_red = acc_articles[num].pin_grn;
        } else {
          acc_articles[num].pin_red = pin;
        }
        pin++;

        if ( acc_articles[num].i2c_bus == 0 ) {
          AddOn[acc_articles[num].board_num].pinMode(acc_articles[num].pin_grn, INPUT);
          AddOn[acc_articles[num].board_num].pullUp(acc_articles[num].pin_grn, HIGH);   // Activate Internal Pull-Up Resistor
          AddOn[acc_articles[num].board_num].pinMode(acc_articles[num].pin_red, INPUT);
          AddOn[acc_articles[num].board_num].pullUp(acc_articles[num].pin_red, HIGH);   // Activate Internal Pull-Up Resistor
        #ifdef USE_WIRE1
        } else if ( acc_articles[num].i2c_bus == 1 ) {
          AddOn_W1[acc_articles[num].board_num].pinMode(acc_articles[num].pin_grn, INPUT);
          AddOn_W1[acc_articles[num].board_num].pullUp(acc_articles[num].pin_grn, HIGH);   // Activate Internal Pull-Up Resistor
          AddOn_W1[acc_articles[num].board_num].pinMode(acc_articles[num].pin_red, INPUT);
          AddOn_W1[acc_articles[num].board_num].pullUp(acc_articles[num].pin_red, HIGH);   // Activate Internal Pull-Up Resistor
        #endif
        #ifdef USE_WIRE2
        } else if ( acc_articles[num].i2c_bus == 2 ) {
          AddOn_W2[acc_articles[num].board_num].pinMode(acc_articles[num].pin_grn, INPUT);
          AddOn_W2[acc_articles[num].board_num].pullUp(acc_articles[num].pin_grn, HIGH);   // Activate Internal Pull-Up Resistor
          AddOn_W2[acc_articles[num].board_num].pinMode(acc_articles[num].pin_red, INPUT);
          AddOn_W2[acc_articles[num].board_num].pullUp(acc_articles[num].pin_red, HIGH);   // Activate Internal Pull-Up Resistor
        #endif
        #ifdef USE_WIRE3
        } else if ( acc_articles[num].i2c_bus == 3 ) {
          AddOn_W3[acc_articles[num].board_num].pinMode(acc_articles[num].pin_grn, INPUT);
          AddOn_W3[acc_articles[num].board_num].pullUp(acc_articles[num].pin_grn, HIGH);   // Activate Internal Pull-Up Resistor
          AddOn_W3[acc_articles[num].board_num].pinMode(acc_articles[num].pin_red, INPUT);
          AddOn_W3[acc_articles[num].board_num].pullUp(acc_articles[num].pin_red, HIGH);   // Activate Internal Pull-Up Resistor
        #endif
        } 

        #ifdef LED_FEEDBACK
          
          if ( acc_articles[num].acc_type == TYPE_SINGLE_BUTTON ) { 
            acc_articles[num].pin_led_grn = acc_articles[num].pin_red + 9;
            acc_articles[num].pin_led_red = acc_articles[num].pin_grn + 8;
          } else {
            acc_articles[num].pin_led_grn = acc_articles[num].pin_grn + 8;
            acc_articles[num].pin_led_red = acc_articles[num].pin_red + 8;
          }
          if ( acc_articles[num].i2c_bus == 0 ) {
            AddOn[acc_articles[num].board_num].pinMode(acc_articles[num].pin_led_grn, OUTPUT);
            AddOn[acc_articles[num].board_num].pinMode(acc_articles[num].pin_led_red, OUTPUT);
          #ifdef USE_WIRE1
          } else if ( acc_articles[num].i2c_bus == 1 ) {
            AddOn_W1[acc_articles[num].board_num].pinMode(acc_articles[num].pin_led_grn, OUTPUT);
            AddOn_W1[acc_articles[num].board_num].pinMode(acc_articles[num].pin_led_red, OUTPUT);
          #endif
          #ifdef USE_WIRE2
          } else if ( acc_articles[num].i2c_bus == 2 ) {
            AddOn_W2[acc_articles[num].board_num].pinMode(acc_articles[num].pin_led_grn, OUTPUT);
            AddOn_W2[acc_articles[num].board_num].pinMode(acc_articles[num].pin_led_red, OUTPUT);
          #endif
          #ifdef USE_WIRE3
          } else if ( acc_articles[num].i2c_bus == 3 ) {
            AddOn_W3[acc_articles[num].board_num].pinMode(acc_articles[num].pin_led_grn, OUTPUT);
            AddOn_W3[acc_articles[num].board_num].pinMode(acc_articles[num].pin_led_red, OUTPUT);
          #endif
          }

          acc_articles[num].reg_state = (200 + (6 * (num + 1)) - 1);
          acc_articles[num].state_is = EEPROM.read( acc_articles[num].reg_state );
          acc_articles[num].state_set = acc_articles[num].state_is;
        #endif


        #ifdef DEBUG_SETUP_ACC
          Serial.print("Setup ACC #");
          Serial.print(num);
          Serial.print(" -> Modul: ");
          Serial.print(acc_articles[num].Modul);
          Serial.print(" -> Adresse: ");
          Serial.print(mcan.getadrs(acc_articles[num].prot, acc_articles[num].locID));
          Serial.print(" -> Protocol: ");
          Serial.print(acc_articles[num].prot);
          Serial.print(" -> Local-ID: ");
          Serial.println(acc_articles[num].locID);
          Serial.print(" reg_state: ");
          Serial.print(acc_articles[num].reg_state);
          Serial.print(" | State stored: ");
          Serial.print(acc_articles[num].state_is);
          Serial.println(" ( 0 = OFF/RED | 1 = ON/GREEN )");
          Serial.print(" PIN GREEN:  ");
          Serial.print(acc_articles[num].pin_grn);
          Serial.print(" | PIN RED:  ");
          Serial.println(acc_articles[num].pin_red);
          Serial.print(" LED GREEN:  ");
          Serial.print(acc_articles[num].pin_led_grn);
          Serial.print(" | LED RED:  ");
          Serial.println(acc_articles[num].pin_led_red);
          Serial.println("-----------------------------------");
          delay(5);
        #endif
        num++;
      }
      board_num++;
      if ( board_num == 8 ) {
        board_num = 0;
      }

    }
  }
  #ifdef DEBUG_SETUP
    Serial.println(" - ...completed");
    delay(100);
  #endif
  signal_setup_successfull();

}

//#############################################################################
// Test all LEDs
//#############################################################################
#ifdef LED_FEEDBACK
void test_acc_leds() {
  for (int i = 0; i < NUM_ACCs; i++) {
    #ifdef DEBUG_LED_TEST
      Serial.print("I2C-BUS: ");
      Serial.print(acc_articles[i].i2c_bus);
      Serial.print(" | Board: ");
      Serial.print(acc_articles[i].board_num);
      Serial.print(" | Modul: ");
      Serial.print(acc_articles[i].Modul);
      Serial.print(" | RED LED: ");
      Serial.print(acc_articles[i].pin_led_red);
      Serial.print(" ON - Address: ");
      Serial.println(mcan.getadrs(acc_articles[i].locID));
    #endif
    
    if ( acc_articles[i].i2c_bus == 0 ) {
      AddOn[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, ON);
    #ifdef USE_WIRE1
    } else if ( acc_articles[i].i2c_bus == 1 ) {
      AddOn_W1[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, ON);
    #endif
    #ifdef USE_WIRE2
    } else if ( acc_articles[i].i2c_bus == 2 ) {
      AddOn_W2[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, ON);
    #endif
    #ifdef USE_WIRE3
    } else if ( acc_articles[i].i2c_bus == 3 ) {
      AddOn_W3[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, ON);
    #endif
    }
    
    #ifdef DEBUG_LED_TEST
      Serial.println("");
      delay(25);
    #endif
    #ifdef DEBUG_LED_TEST
      Serial.print("I2C-BUS: ");
      Serial.print(acc_articles[i].i2c_bus);
      Serial.print(" | Board: ");
      Serial.print(acc_articles[i].board_num);
      Serial.print(" | Modul: ");
      Serial.print(acc_articles[i].Modul);
      Serial.print(" GREEN LED: ");
      Serial.print(acc_articles[i].pin_led_grn);
      Serial.print(" ON - Address: ");
      Serial.println(mcan.getadrs(acc_articles[i].locID));
    #endif
    
    if ( acc_articles[i].i2c_bus == 0 ) {
      AddOn[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, ON);
    #ifdef USE_WIRE1
    } else if ( acc_articles[i].i2c_bus == 1 ) {
      AddOn_W1[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, ON);
    #endif
    #ifdef USE_WIRE2
    } else if ( acc_articles[i].i2c_bus == 2 ) {
      AddOn_W2[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, ON);
    #endif
    #ifdef USE_WIRE3
    } else if ( acc_articles[i].i2c_bus == 3 ) {
      AddOn_W3[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, ON);
    #endif
    }
    #ifdef DEBUG_LED_TEST
      Serial.println("");
      delay(10);
    #endif
    
  }
  
  #ifndef DEBUG_LED_TEST
    delay(5000);
  #else
    delay(5000);
    delay(60000);
  #endif

  for (int i = 0; i < NUM_ACCs; i++) {
    if ( acc_articles[i].i2c_bus == 0 ) {
      AddOn[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, OFF);
      AddOn[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, OFF);
    #ifdef USE_WIRE1
    } else if ( acc_articles[i].i2c_bus == 1 ) {
      AddOn_W1[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, OFF);
      AddOn_W1[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, OFF);
    #endif
    #ifdef USE_WIRE2
    } else if ( acc_articles[i].i2c_bus == 2 ) {
      AddOn_W2[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, OFF);
      AddOn_W2[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, OFF);
    #endif
    #ifdef USE_WIRE3
    } else if ( acc_articles[i].i2c_bus == 3 ) {
      AddOn_W3[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, OFF);
      AddOn_W3[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, OFF);
    #endif
    }

  }

}
#endif


//#############################################################################
// restore last State
//#############################################################################
#ifdef LED_FEEDBACK
void restore_last_state() {
  #ifdef DEBUG_SETUP
    Serial.println();
    Serial.println(" - Restoring last LED Status:");
  #endif
  for (int i = 0; i < NUM_ACCs; i++) {
    #ifdef DEBUG_SETUP_ACC
      Serial.print(millis());
      Serial.print(" - Address: ");
      Serial.print(mcan.getadrs(acc_articles[i].prot, acc_articles[i].locID));
    #endif
    if ( acc_articles[i].acc_type == TYPE_UNCOUPLER ) {
      #ifdef DEBUG_SETUP_ACC
        Serial.println(" (Uncoupler) - LEDs off.");
      #endif
      if ( acc_articles[i].i2c_bus == 0 ) {
        AddOn[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, LOW);
        AddOn[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, LOW);
      #ifdef USE_WIRE1
      } else if ( acc_articles[i].i2c_bus == 1 ) {
        AddOn_W1[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, LOW);
        AddOn_W1[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, LOW);
      #endif
      #ifdef USE_WIRE2
      } else if ( acc_articles[i].i2c_bus == 2 ) {
        AddOn_W2[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, LOW);
        AddOn_W2[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, LOW);
      #endif
      #ifdef USE_WIRE3
      } else if ( acc_articles[i].i2c_bus == 3 ) {
        AddOn_W3[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, LOW);
        AddOn_W3[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, LOW);
      #endif
      }
    } else {
      #ifdef DEBUG_SETUP_ACC
        Serial.print(" (Turnout/Signal) - LED: ");
      #endif
      if (acc_articles[i].state_is == RED) {
        if ( acc_articles[i].i2c_bus == 0 ) {
          AddOn[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, LOW);
          AddOn[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, HIGH);
        #ifdef USE_WIRE1
        } else if ( acc_articles[i].i2c_bus == 1 ) {
          AddOn_W1[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, LOW);
          AddOn_W1[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, HIGH);
        #endif
        #ifdef USE_WIRE2
        } else if ( acc_articles[i].i2c_bus == 2 ) {
          AddOn_W2[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, LOW);
          AddOn_W2[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, HIGH);
        #endif
        #ifdef USE_WIRE3
        } else if ( acc_articles[i].i2c_bus == 3 ) {
          AddOn_W3[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, LOW);
          AddOn_W3[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, HIGH);
        #endif
        }
        #ifdef DEBUG_SETUP_ACC
          Serial.println("RED: ON / green: off");
        #endif
      }  
      else if (acc_articles[i].state_is == GREEN) {
        if ( acc_articles[i].i2c_bus == 0 ) {
          AddOn[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, LOW);
          AddOn[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, HIGH);
        #ifdef USE_WIRE1
        } else if ( acc_articles[i].i2c_bus == 1 ) {
          AddOn_W1[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, LOW);
          AddOn_W1[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, HIGH);
        #endif
        #ifdef USE_WIRE2
        } else if ( acc_articles[i].i2c_bus == 2 ) {
          AddOn_W2[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, LOW);
          AddOn_W2[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, HIGH);
        #endif
        #ifdef USE_WIRE3
        } else if ( acc_articles[i].i2c_bus == 3 ) {
          AddOn_W3[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_red, LOW);
          AddOn_W3[acc_articles[i].board_num].digitalWrite(acc_articles[i].pin_led_grn, HIGH);
        #endif
        }
        #ifdef DEBUG_SETUP_ACC
          Serial.println("red: off / GREEN: ON");
        #endif
      }
    }
  }
}
#endif


//#############################################################################
// Change LocalID when protocol has changed (gloabl)
//#############################################################################
void change_global_prot() {
  #ifdef DEBUG_CONFIG
    Serial.print(millis());
    Serial.print("Changing protocol from ");
    if (prot_old == ACC_DCC ) {
      Serial.print("DCC to ");
    } else {
      Serial.print("MM to ");
    }
    if (prot == ACC_DCC ) {
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
// Change LocalID when protocol has changed
//#############################################################################
void change_prot() {
  signal_setup_successfull();

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
    Serial.print(mcan.getadrs(acc_articles[acc_num].prot, acc_articles[acc_num].locID));
    Serial.print(" ");
  #endif
  if ( ( acc_articles[acc_num].acc_type == TYPE_TURNOUT ) || ( acc_articles[acc_num].acc_type == TYPE_SINGLE_BUTTON ) ) {
    if ( color == RED ) {
      #ifdef DEBUG_LED
        Serial.print(" - Turnout/Signal (red|round) LED: RED: ON / green: off");
      #endif
      if ( acc_articles[acc_num].i2c_bus == 0 ) {
        AddOn[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_grn, LOW);
        AddOn[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_red, HIGH);
      #ifdef USE_WIRE1
      } else if ( acc_articles[acc_num].i2c_bus == 1 ) {
        AddOn_W1[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_grn, LOW);
        AddOn_W1[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_red, HIGH);
      #endif
      #ifdef USE_WIRE2
      } else if ( acc_articles[acc_num].i2c_bus == 2 ) {
        AddOn_W2[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_grn, LOW);
        AddOn_W2[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_red, HIGH);
      #endif
      #ifdef USE_WIRE3
      } else if ( acc_articles[acc_num].i2c_bus == 3 ) {
        AddOn_W3[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_grn, LOW);
        AddOn_W3[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_red, HIGH);
      #endif
      }
    } else if ( color == GREEN ) {
      #ifdef DEBUG_LED
        Serial.print(" - Turnout/Signal (green|straight)  LED: red: off / GREEN: ON");
      #endif
      if ( acc_articles[acc_num].i2c_bus == 0 ) {
        AddOn[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_grn, HIGH);
        AddOn[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_red, LOW);
      #ifdef USE_WIRE1
      } else if ( acc_articles[acc_num].i2c_bus == 1 ) {
        AddOn_W1[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_red, LOW);
        AddOn_W1[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_grn, HIGH);
      #endif
      #ifdef USE_WIRE2
      } else if ( acc_articles[acc_num].i2c_bus == 2 ) {
        AddOn_W2[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_red, LOW);
        AddOn_W2[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_grn, HIGH);
      #endif
      #ifdef USE_WIRE3
      } else if ( acc_articles[acc_num].i2c_bus == 3 ) {
        AddOn_W3[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_grn, HIGH);
        AddOn_W3[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_red, LOW);
      #endif
      }
    }
  } else if ( acc_articles[acc_num].acc_type == TYPE_UNCOUPLER) {
    if ( color == RED ) {
      #ifdef DEBUG_LED
        Serial.print(" - Uncoupler: RED: ON");
      #endif
      if ( acc_articles[acc_num].i2c_bus == 0 ) {
        AddOn[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_red, power);
      #ifdef USE_WIRE1
      } else if ( acc_articles[acc_num].i2c_bus == 1 ) {
        AddOn_W1[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_red, power);
      #endif
      #ifdef USE_WIRE2
      } else if ( acc_articles[acc_num].i2c_bus == 2 ) {
        AddOn_W2[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_red, power);
      #endif
      #ifdef USE_WIRE3
      } else if ( acc_articles[acc_num].i2c_bus == 3 ) {
        AddOn_W3[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_red, power);
      #endif
      }
    } else if ( color == GREEN ) {
      #ifdef DEBUG_LED
        Serial.print(" - Uncoupler: GREEN: ON");
      #endif
      if ( acc_articles[acc_num].i2c_bus == 0 ) {
        AddOn[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_grn, power);
      #ifdef USE_WIRE1
      } else if ( acc_articles[acc_num].i2c_bus == 1 ) {
        AddOn_W1[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_grn, power);
      #endif
      #ifdef USE_WIRE2
      } else if ( acc_articles[acc_num].i2c_bus == 2 ) {
        AddOn_W2[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_grn, power);
      #endif
      #ifdef USE_WIRE3
      } else if ( acc_articles[acc_num].i2c_bus == 3 ) {
        AddOn_W3[acc_articles[acc_num].board_num].digitalWrite(acc_articles[acc_num].pin_led_grn, power);
      #endif
      }
    }
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
  if ( state == BUTTON_PRESSED ) {
    if (acc_articles[acc_num].pushed == BUTTON_NOT_PRESSED) {
      if ( acc_articles[acc_num].acc_type == TYPE_SINGLE_BUTTON ) {
        #ifdef DEBUG_INPUT
          Serial.print(millis());
          Serial.println(" - PUSH Button - toggle color");
        #endif
        color = !acc_articles[acc_num].state_is;
        #ifdef DEBUG_INPUT
          Serial.print(millis());
          Serial.print(" - State IS: ");
          Serial.print(acc_articles[acc_num].state_is);
          Serial.print(" => State SET: ");
          Serial.print(acc_articles[acc_num].state_set);
          Serial.print(" => Target State: ");
          Serial.println(color);          
        #endif
      }
      #ifdef DEBUG_INPUT
        Serial.print(millis());
        if ( color == RED ) {
          Serial.print(" - send switchcommand RED to ACC #");
        }
        if ( color == GREEN ) {
          Serial.print(" - send switchcommand GREEN to ACC #");
        }
        Serial.println(mcan.getadrs(acc_articles[acc_num].prot, acc_articles[acc_num].locID));
      #endif
      mcan.sendAccessoryFrame(device, acc_articles[acc_num].locID, color, false, state);
      acc_articles[acc_num].pushed = millis();

    } else if ( (acc_articles[acc_num].acc_type == TYPE_UNCOUPLER)
              && (millis() - acc_articles[acc_num].pushed >= acc_interval) ) {

      #ifdef DEBUG_INPUT
        Serial.print(millis());
        if ( color == RED )  {
          Serial.print(" - resend switchcommand RED to ACC #");
        }
        if ( color == GREEN ) {
          Serial.print(" - resend switchcommand GREEN to ACC #");
        }
        Serial.println(mcan.getadrs(acc_articles[acc_num].prot, acc_articles[acc_num].locID));
      #endif
      acc_articles[acc_num].pushed = millis();
      mcan.sendAccessoryFrame(device, acc_articles[acc_num].locID, color, false, state);

    }
    #ifdef LED_FEEDBACK
      acc_articles[acc_num].state_set = color;
      acc_articles[acc_num].power_set = state;
    #endif
    #ifdef DEBUG_INPUT
      Serial.print(millis());
      Serial.print(" - State IS: ");
      Serial.print(acc_articles[acc_num].state_is);
      Serial.print(" => State SET: ");
      Serial.print(acc_articles[acc_num].state_set);
      Serial.print(" => Target State: ");
      Serial.println(color);          
    #endif

    
  } else if ( state == BUTTON_NOT_PRESSED ) {
    if (acc_articles[acc_num].pushed > BUTTON_NOT_PRESSED) {
      #ifdef DEBUG_INPUT
        Serial.print(millis());
        Serial.print(" - send power-off command to ACC #");
        Serial.println(mcan.getadrs(acc_articles[acc_num].prot, acc_articles[acc_num].locID));
      #endif
      acc_articles[acc_num].pushed = state;
      mcan.sendAccessoryFrame(device, acc_articles[acc_num].locID, color, false, state);

      #ifdef LED_FEEDBACK
        acc_articles[acc_num].state_set = color;
        acc_articles[acc_num].power_set = state;
      #endif

    }
  }

}

void config_poll_input() {
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
      Serial.print(EEPROM.read(REG_PROT));
      Serial.println(" ( 0 = DCC / 1 = MM )");
    #endif
    mcan.sendConfigInfoDropdown(device, 1, 2, EEPROM.read(REG_PROT), "Protocol_DCC_MM");
  } else
  if(config_index >= 2){
    //----------------------------------------------------------------------------------------------
    // with Uncoupler:
    //----------------------------------------------------------------------------------------------
    uint8_t ci_acc_num = ( config_index / 2 ) - 1;
    if ( (config_index % 2) > 0) {
      //--------------------------------------------------------------------------------------------
      // TYP:
      //--------------------------------------------------------------------------------------------
      #ifdef DEBUG_CONFIG
        Serial.print(millis());
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
      // ADDRESSE:
      //--------------------------------------------------------------------------------------------
      string1 = "Adrs ";
      string2 = "_1_2048";
      string3 = String(ci_acc_num+1);
      string4 = string1 + string3 + string2;
      uint16_t adrs = mcan.getadrs(prot, acc_articles[ci_acc_num].locID);
      #ifdef DEBUG_CONFIG
        //Serial.print(millis());
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
        Serial.print(" - Address: ");
        Serial.println(adrs);
      #endif
      mcan.sendConfigInfoSlider(device, config_index, 1, 2048, adrs, string4);

    }

  }
  config_poll = false;
  //================================================================================================
  // END - CONFIG_POLL
  //================================================================================================

}

void setup_input() {
  if (!EEPROM.read(REG_PROT)) {
    prot = ACC_DCC;
  } else {
    prot = ACC_MM;
  }
  prot_old = prot;

  // with Uncoupler:
  CONFIG_NUM_INPUT = start_adrs_channel + ( 2 * NUM_ACCs ) - 1;

  setup_acc();

  #ifdef DEBUG_SERIAL
    Serial.println("-----------------------------------");
    Serial.println(" - INPUT Setup completed.         -");
    Serial.println("-----------------------------------");
    delay(100);
  #endif
}

void input_loop() {
  //================================================================================================
  // SWITCH LOOP
  //================================================================================================
  for (int i = 0; i < NUM_ACCs; i++) {
    //==============================================================================================
    // LED_FEEDBACK
    //==============================================================================================
    #ifdef LED_FEEDBACK
      if ( ( (acc_articles[i].acc_type == TYPE_TURNOUT) || (acc_articles[i].acc_type == TYPE_SINGLE_BUTTON) )
          && (acc_articles[i].state_is != acc_articles[i].state_set) ) {

        switchLED(i, acc_articles[i].state_set, acc_articles[i].power_set);

      } else if ( (acc_articles[i].acc_type == TYPE_UNCOUPLER)
          && (acc_articles[i].power_is != acc_articles[i].power_set) ) {

        switchLED(i, acc_articles[i].state_set, acc_articles[i].power_set);

      }
    #endif
    //==============================================================================================
    // END - LED_FEEDBACK
    //==============================================================================================
    //==============================================================================================
    // SCAN BUTTONS
    //==============================================================================================
    if ( (locked == false) && (GBS_locked == false) && (currentMillis - previousMillis_input >= input_interval) ) {



      if ( acc_articles[i].i2c_bus == 0 ) {

        if (AddOn[acc_articles[i].board_num].digitalRead(acc_articles[i].pin_red) == LOW) {
          button_pushed(i, RED, BUTTON_PRESSED);
  
        } else if (AddOn[acc_articles[i].board_num].digitalRead(acc_articles[i].pin_grn) == LOW) {
          button_pushed(i, GREEN, BUTTON_PRESSED);
  
        } else {
          #ifdef LED_FEEDBACK
          button_pushed(i, acc_articles[i].state_is, BUTTON_NOT_PRESSED);
          #endif
  
        }
        
      #ifdef USE_WIRE1
      
      } else if ( acc_articles[i].i2c_bus == 1 ) {

        if (AddOn_W1[acc_articles[i].board_num].digitalRead(acc_articles[i].pin_red) == LOW) {
          button_pushed(i, RED, BUTTON_PRESSED);
  
        } else if (AddOn_W1[acc_articles[i].board_num].digitalRead(acc_articles[i].pin_grn) == LOW) {
          button_pushed(i, GREEN, BUTTON_PRESSED);
  
        } else {
          #ifdef LED_FEEDBACK
          button_pushed(i, acc_articles[i].state_is, BUTTON_NOT_PRESSED);
          #endif
  
        }

      #endif
      #ifdef USE_WIRE2
      
      } else if ( acc_articles[i].i2c_bus == 2 ) {

        if (AddOn_W2[acc_articles[i].board_num].digitalRead(acc_articles[i].pin_red) == LOW) {
          button_pushed(i, RED, BUTTON_PRESSED);
  
        } else if (AddOn_W2[acc_articles[i].board_num].digitalRead(acc_articles[i].pin_grn) == LOW) {
          button_pushed(i, GREEN, BUTTON_PRESSED);
  
        } else {
          #ifdef LED_FEEDBACK
          button_pushed(i, acc_articles[i].state_is, BUTTON_NOT_PRESSED);
          #endif
  
        }

      #endif
      #ifdef USE_WIRE3
      
      } else if ( acc_articles[i].i2c_bus == 3 ) {

        if (AddOn_W3[acc_articles[i].board_num].digitalRead(acc_articles[i].pin_red) == LOW) {
          button_pushed(i, RED, BUTTON_PRESSED);
  
        } else if (AddOn_W3[acc_articles[i].board_num].digitalRead(acc_articles[i].pin_grn) == LOW) {
          button_pushed(i, GREEN, BUTTON_PRESSED);
  
        } else {
          #ifdef LED_FEEDBACK
          button_pushed(i, acc_articles[i].state_is, BUTTON_NOT_PRESSED);
          #endif
  
        }

        #endif
      
      }

    }
    //==============================================================================================
    // END - SCAN BUTTONS
    //==============================================================================================
  }
  //================================================================================================
  // END - SWITCH LOOP
  //================================================================================================
  if (currentMillis - previousMillis_input >= input_interval ) {
    previousMillis_input = currentMillis;
  }

}

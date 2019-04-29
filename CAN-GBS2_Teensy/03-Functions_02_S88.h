void init_s88_registers() {
  EEPROM.put(REG_use_L88,   use_L88);              // use_L88 => 0 = USE CS2/3 S88 Bus ; 1 = USE Link L88
  EEPROM.put(REG_modulID,   byte(modulID >> 8));   // modulID => Link L88 ID (High Byte)
  EEPROM.put(REG_modulID+1, byte(modulID));        // modulID => Link L88 ID (Low Byte)

  EEPROM.put(REG_use_bus0,   use_bus[0]);          // Link L88 Interner BUS0 (1-16) oder CS2/3plus
  EEPROM.put(REG_len_bus0,   len_bus[0]);          // Link L88 Interner BUS0 (1-16) oder CS2/3plus
  EEPROM.put(REG_start_bus0, start_bus[0]);        // Link L88 Interner BUS0 (1-16) oder CS2/3plus

  EEPROM.put(REG_use_bus1,   use_bus[1]);          // Link L88 BUS1 (1001-1512)
  EEPROM.put(REG_len_bus1,   len_bus[1]);          // Link L88 BUS1 (1001-1512)
  EEPROM.put(REG_start_bus1, start_bus[1]);        // Link L88 BUS1 (1001-1512)

  EEPROM.put(REG_use_bus2,   use_bus[2]);          // Link L88 BUS2 (2001-2512)
  EEPROM.put(REG_len_bus2,   len_bus[2]);          // Link L88 BUS2 (2001-2512)
  EEPROM.put(REG_start_bus2, start_bus[2]);        // Link L88 BUS2 (2001-2512)

  EEPROM.put(REG_use_bus3,   use_bus[3]);          // Link L88 BUS3 (3001-3512)
  EEPROM.put(REG_len_bus3,   len_bus[3]);          // Link L88 BUS3 (3001-3512)
  EEPROM.put(REG_start_bus3, start_bus[3]);        // Link L88 BUS3 (3001-3512)
  
}
//#############################################################################
// Setup function for S88
//#############################################################################
void setup_s88() {
  //--------------------------------------------------------------------------------------------------
  // START - Setup S88
  //--------------------------------------------------------------------------------------------------
  #ifdef DEBUG_SETUP
    Serial.println("-----------------------------------");
    Serial.println(" - Setting up S88");
    Serial.println("-----------------------------------");
    for(int s = 0; s < 4; s++){
      Serial.print(" - ");
      Serial.print(use_bus[s]);
      Serial.print(" - ");
      Serial.print(start_bus[s]);
      Serial.print(" - ");
      Serial.println(len_bus[s]);
    }
    Serial.println("");
    delay(100);
  #endif

  use_L88 = EEPROM.read(REG_use_L88);
  modulID = EEPROM.read(REG_modulID) << 8 | EEPROM.read(REG_modulID+1);
  int num = 0;

  if (!use_L88) {
    //----------------------------------------------------------------------------------------------
    // CS2/CS3plus S88 Bus configuration
    //----------------------------------------------------------------------------------------------
    #ifdef DEBUG_SETUP
      Serial.println("... will be setup with CS2 / CS3plus S88 Bus configuration.");
    #endif
    //----------------------------------------------------------------------------------------------
    // setup variables
    //----------------------------------------------------------------------------------------------
    CONFIG_NUM_S88 = 4;
    
    use_bus[0]   = 1;
    len_bus[0]   = EEPROM.read(REG_len_bus0);
    start_bus[0] = EEPROM.read(REG_start_bus0);
    curr_bus[0]  = 1 + ( 16 * ( start_bus[0] - 1 ) );
    last_bus[0]  = curr_bus[0] + ( 16 * len_bus[0] ) - 1;
    
    use_bus[1]   = 0;
    len_bus[1]   = 0;
    start_bus[1] = 0;
    
    use_bus[2]   = 0;
    len_bus[2]   = 0;
    start_bus[2] = 0;
    
    use_bus[3]   = 0;
    len_bus[3]   = 0;
    start_bus[3] = 0;

    //----------------------------------------------------------------------------------------------
    // S88 Feedback configuration
    //----------------------------------------------------------------------------------------------
    for(int m = 0; m < ANZ_S88_ADDONS; m++){
      // ANZ_S88_ADDONS = 8 -> m = 0..7
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
          Serial.print("teensy | starting AddOn ");
          Serial.print(m+1);
          Serial.print(" I2C bus: ");
          Serial.println(i2c_bus_con);
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
        #ifdef DEBUG_SETUP
          Serial.println("running on arduino hardware.");
        #endif
        AddOn[m].begin(m);
      #endif
      if(use_bus[0]) {
        #ifdef DEBUG_SETUP_S88
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
            delay(100);
          }
        #endif
        for(int i = 0; i < ANZ_S88_PER_ADDON; i++){
          if (curr_bus[0] <= last_bus[0]) {
            #ifdef DEBUG_SETUP_S88
              if (i == 0) {
                Serial.println(" ");
                Serial.print("AddOn #");
                Serial.print(m+1);
                Serial.print(" :");
              }
              Serial.print(curr_bus[0]);
              Serial.print(" ");
            #endif
            s88_contacts[num].rm = curr_bus[0];
            s88_contacts[num].i2c_bus = i2c_bus_con;
            s88_contacts[num].board_num = board_num;
            
            if ( i2c_bus_con == 0 ) {
              AddOn[board_num].pinMode(i, OUTPUT);
            #ifdef USE_WIRE1       
            } else if ( i2c_bus_con == 1 ) {
              AddOn_W1[board_num].pinMode(i, OUTPUT);
            #endif
            #ifdef USE_WIRE2
            } else if ( i2c_bus_con == 2 ) {
              AddOn_W2[board_num].pinMode(i, OUTPUT);
            #endif
            #ifdef USE_WIRE3
            } else if ( i2c_bus_con == 3 ) {
              AddOn_W3[board_num].pinMode(i, OUTPUT);
            #endif
            }
            num++;
            curr_bus[0]++;
          }
        }
        board_num++;
        if ( board_num == 8 ) {
          board_num = 0;
        }
      }
    }
    #ifdef DEBUG_SETUP_S88
      Serial.println(" ");
      Serial.println(" ");
    #endif

    //----------------------------------------------------------------------------------------------
    // ENDE - CS2/CS3plus S88 Bus configuration
    //----------------------------------------------------------------------------------------------
  } else {
    //----------------------------------------------------------------------------------------------
    // Link L88 - S88 BUS configuration
    //----------------------------------------------------------------------------------------------
    #ifdef DEBUG_SETUP
      Serial.println("... will be setup with Link L88 S88 Bus configuration.");
    #endif
    //----------------------------------------------------------------------------------------------
    // setup variables
    //----------------------------------------------------------------------------------------------
    CONFIG_NUM_S88 = 12;
    
    use_bus[0] = EEPROM.read(REG_use_bus0);
    if (use_bus[0]) {
      len_bus[0]   = 1;
      start_bus[0] = 1;
      curr_bus[0]  = 1;
    }
    
    use_bus[1] = EEPROM.read(REG_use_bus1);
    if (use_bus[1]) {
      len_bus[1]   = EEPROM.read(REG_len_bus1);
      start_bus[1] = EEPROM.read(REG_start_bus1);
      curr_bus[1]  = 1001 + ( 16 * ( start_bus[1] - 1 ) );
    }
    
    use_bus[2] = EEPROM.read(REG_use_bus2);
    if (use_bus[2]) {
      len_bus[2]   = EEPROM.read(REG_len_bus2);
      start_bus[2] = EEPROM.read(REG_start_bus2);
      curr_bus[2]  = 2001 + ( 16 * ( start_bus[2] - 1 ) );
    }
    
    use_bus[3] = EEPROM.read(REG_use_bus3);
    if (use_bus[3]) {
      len_bus[3]   = EEPROM.read(REG_len_bus2);
      start_bus[3] = EEPROM.read(REG_start_bus3);
      curr_bus[3]  = 3001 + ( 16 * ( start_bus[3] - 1 ) );
    }
    
    for(int b = 0; b < 4; b++) {
      if(use_bus[b]) {
        last_bus[b] = curr_bus[b] + ( 16 * len_bus[b] ) - 1;
    #ifdef DEBUG_SETUP
      Serial.print("... Bus");
      Serial.print(b);
      Serial.print(" Länge: ");
      Serial.print(len_bus[b]);
      Serial.print(" || Start: ");
      Serial.print(curr_bus[b]);
      Serial.print(", Ende:");
      Serial.println(last_bus[b]);
      delay(100);
    #endif
      } else {
        len_bus[b] = 0;
        last_bus[b] = 0;
        start_bus[b] = 0;
      }
    }
    
    //----------------------------------------------------------------------------------------------
    // S88 Feedback configuration
    //----------------------------------------------------------------------------------------------
    for(int m = 0; m < ANZ_S88_ADDONS; m++) {
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
          Serial.print("teensy | starting ");
          Serial.print(" I2C-BUS: ");
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
        } else if ( i2c_bus_con == 2 ) {
          AddOn_W2[board_num].begin(board_num, i2c_bus_con);
        #endif
        #ifdef USE_WIRE3
        } else if ( i2c_bus_con == 3 ) {
          AddOn_W3[board_num].begin(board_num, i2c_bus_con);
        #endif
        }
      #else
        #ifdef DEBUG_SETUP
          Serial.println("running on arduino hardware.");
        #endif
        AddOn[m].begin(m);
      #endif
      board_num++;
      if ( board_num == 8 ) {
        board_num = 0;
      }
    }
    
    int m = 0;
    board_num = 0;

    if(use_bus[0]) {
      //------------------------------------------------------------------------------------------
      // S88 BUS 0 configuration
      //------------------------------------------------------------------------------------------
      #ifdef DEBUG_SETUP_S88
        Serial.println("");
        Serial.println("BUS 0 (1-16) will be configured");
        Serial.print(" * starting with Modul ");
        Serial.println(start_bus[0]);
        Serial.print(" * using ");
        Serial.print(len_bus[0]);
        Serial.print(" Modul(s).");
        delay(100);
      #endif
      while (curr_bus[0] <= last_bus[0]) {
        for(int i = 0; i < ANZ_S88_PER_ADDON; i++){
          if (curr_bus[0] <= last_bus[0]) {
            #ifdef DEBUG_SETUP_S88
              Serial.println(" ");
              Serial.print("I2C-BUS: ");
              Serial.print(m/16);
              Serial.print(" | Board: ");
              Serial.print(board_num);
              Serial.print(" | Modul: ");
              Serial.print(m);
              Serial.print(" | AddOn #");
              Serial.print(m+1);
              Serial.print(" : ");
              Serial.print(curr_bus[0]);
              Serial.print(" (");
              Serial.print(i);
              Serial.println(") ");
            #endif
            s88_contacts[num].rm = curr_bus[0];
            
            s88_contacts[num].i2c_bus = m / 8;
            s88_contacts[num].board_num = board_num;
            
            if ( s88_contacts[num].i2c_bus == 0 ) {
              AddOn[board_num].pinMode(i, OUTPUT);
            #ifdef USE_WIRE1
            } else if ( s88_contacts[num].i2c_bus == 1 ) {
              AddOn_W1[board_num].pinMode(i, OUTPUT);
            #endif
            #ifdef USE_WIRE2
            } else if ( s88_contacts[num].i2c_bus == 2 ) {
              AddOn_W2[board_num].pinMode(i, OUTPUT);
            #endif
            #ifdef USE_WIRE3
            } else if ( s88_contacts[num].i2c_bus == 3 ) {
              AddOn_W3[board_num].pinMode(i, OUTPUT);
            #endif
            }
            num++;
            curr_bus[0]++;
          }
        }
        m++;
        board_num++;
        if ( board_num == 8 ) {
          board_num = 0;
        }

      }
      delay(100);
    }
    if(use_bus[1]) {
      //------------------------------------------------------------------------------------------
      // S88 BUS 1 configuration
      //------------------------------------------------------------------------------------------
      #ifdef DEBUG_SETUP_S88
        Serial.println(" ");
        Serial.println(" ");
        Serial.print("BUS 1 (1001-1512) will be configured for RMs ");
        Serial.print(curr_bus[1]);
        Serial.print("-");
        Serial.println(last_bus[1]);
        Serial.print(" * starting with Modul ");
        Serial.println(start_bus[1]);
        Serial.print(" * using ");
        Serial.print(len_bus[1]);
        Serial.print(" Modul(s).");
      #endif
      while (curr_bus[1] <= last_bus[1]) {
        for(int i = 0; i < ANZ_S88_PER_ADDON; i++){
          if (curr_bus[1] <= last_bus[1]) {
            #ifdef DEBUG_SETUP_S88
              Serial.println(" ");
              Serial.print("I2C-BUS: ");
              Serial.print(m/8);
              Serial.print(" | Board: ");
              Serial.print(board_num);
              Serial.print(" | Modul: ");
              Serial.print(m);
              Serial.print(" | AddOn #");
              Serial.print(m+1);
              Serial.print(" : ");
              Serial.print(curr_bus[1]);
              Serial.print(" (");
              Serial.print(i);
              Serial.println(") ");
            #endif
            s88_contacts[num].rm = curr_bus[1];         // DEFAULT
            s88_contacts[num].i2c_bus = m / 8;
            s88_contacts[num].board_num = board_num;
            
            if ( s88_contacts[num].i2c_bus == 0 ) {
              AddOn[board_num].pinMode(i, OUTPUT);
            #ifdef USE_WIRE1
            } else if ( s88_contacts[num].i2c_bus == 1 ) {
              AddOn_W1[board_num].pinMode(i, OUTPUT);
            #endif
            #ifdef USE_WIRE2
            } else if ( s88_contacts[num].i2c_bus == 2 ) {
              AddOn_W2[board_num].pinMode(i, OUTPUT);
            #endif
            #ifdef USE_WIRE3
            } else if ( s88_contacts[num].i2c_bus == 3 ) {
              AddOn_W3[board_num].pinMode(i, OUTPUT);
            #endif
            }
            num++;
            curr_bus[1]++;
          }
        }
        m++;
        board_num++;
        if ( board_num == 8 ) {
          board_num = 0;
        }
      }
      delay(100);
    }
    if(use_bus[2]) {
      //------------------------------------------------------------------------------------------
      // S88 BUS 2 configuration
      //------------------------------------------------------------------------------------------
      #ifdef DEBUG_SETUP_S88
        Serial.println(" ");
        Serial.println(" ");
        Serial.print("BUS 2 (2001-2512) will be configured for RMs ");
        Serial.print(curr_bus[2]);
        Serial.print("-");
        Serial.println(last_bus[2]);
        Serial.print(" * starting with Modul ");
        Serial.println(start_bus[2]);
        Serial.print(" * using ");
        Serial.print(len_bus[2]);
        Serial.print(" Modul(s).");
      #endif
      while (curr_bus[2] <= last_bus[2]) {
        for(int i = 0; i < ANZ_S88_PER_ADDON; i++){
          if (curr_bus[2] <= last_bus[2]) {
            #ifdef DEBUG_SETUP_S88
              Serial.println(" ");
              Serial.print("I2C-BUS: ");
              Serial.print(m/8);
              Serial.print(" | Board: ");
              Serial.print(board_num);
              Serial.print(" | Modul: ");
              Serial.print(m);
              Serial.print(" | AddOn #");
              Serial.print(m+1);
              Serial.print(" : ");
              Serial.print(curr_bus[2]);
              Serial.print(" (");
              Serial.print(i);
              Serial.println(") ");
            #endif
            s88_contacts[num].rm = curr_bus[2];
            
            s88_contacts[num].i2c_bus = m / 8;
            s88_contacts[num].board_num = board_num;
            
            if ( s88_contacts[num].i2c_bus == 0 ) {
              AddOn[board_num].pinMode(i, OUTPUT);
            #ifdef USE_WIRE1
            } else if ( s88_contacts[num].i2c_bus == 1 ) {
              AddOn_W1[board_num].pinMode(i, OUTPUT);
            #endif
            #ifdef USE_WIRE2
            } else if ( s88_contacts[num].i2c_bus == 2 ) {
              AddOn_W2[board_num].pinMode(i, OUTPUT);
            #endif
            #ifdef USE_WIRE3
            } else if ( s88_contacts[num].i2c_bus == 3 ) {
              AddOn_W3[board_num].pinMode(i, OUTPUT);
            #endif
            }
            num++;
            curr_bus[2]++;
          }
        }
        m++;
        board_num++;
        if ( board_num == 8 ) {
          board_num = 0;
        }
      }
      delay(100);
    }
    if(use_bus[3]) {
      //------------------------------------------------------------------------------------------
      // S88 BUS 3 configuration
      //------------------------------------------------------------------------------------------
      #ifdef DEBUG_SETUP_S88
        Serial.println(" ");
        Serial.println(" ");
        Serial.print("BUS 3 (3001-3512) will be configured for RMs ");
        Serial.print(curr_bus[3]);
        Serial.print("-");
        Serial.println(last_bus[3]);
        Serial.print(" * starting with Modul ");
        Serial.println(start_bus[3]);
        Serial.print(" * using ");
        Serial.print(len_bus[3]);
        Serial.print(" Modul(s).");
      #endif
      while (curr_bus[3] <= last_bus[3]) {
        for(int i = 0; i < ANZ_S88_PER_ADDON; i++){
          if (curr_bus[3] <= last_bus[3]) {
            #ifdef DEBUG_SETUP_S88
              Serial.println(" ");
              Serial.print("I2C-BUS: ");
              Serial.print(m/8);
              Serial.print(" | Board: ");
              Serial.print(board_num);
              Serial.print(" | Modul: ");
              Serial.print(m);
              Serial.print(" | AddOn #");
              Serial.print(m+1);
              Serial.print(" :");
              Serial.print(curr_bus[3]);
              Serial.print(" (");
              Serial.print(i);
              Serial.println(") ");
            #endif
            s88_contacts[num].rm = curr_bus[3];
            
            s88_contacts[num].i2c_bus = m / 8;
            s88_contacts[num].board_num = board_num;
            
            if ( s88_contacts[num].i2c_bus == 0 ) {
              AddOn[board_num].pinMode(i, OUTPUT);
            #ifdef USE_WIRE1
            } else if ( s88_contacts[num].i2c_bus == 1 ) {
              AddOn_W1[board_num].pinMode(i, OUTPUT);
            #endif
            #ifdef USE_WIRE2
            } else if ( s88_contacts[num].i2c_bus == 2 ) {
              AddOn_W2[board_num].pinMode(i, OUTPUT);
            #endif
            #ifdef USE_WIRE3
            } else if ( s88_contacts[num].i2c_bus == 3 ) {
              AddOn_W3[board_num].pinMode(i, OUTPUT);
            #endif
            }
            num++;
            curr_bus[3]++;
          }
        }
        m++;
        board_num++;
        if ( board_num == 8 ) {
          board_num = 0;
        }
      }
      delay(100);
    }      
    //----------------------------------------------------------------------------------------------
    // Link L88 - S88 BUS configuration
    //----------------------------------------------------------------------------------------------
  }
  
  #ifdef DEBUG_SERIAL
    Serial.println("");
    Serial.println("-----------------------------------");
    Serial.println(" - S88 Setup completed           -");
    Serial.println("-----------------------------------");
    
  #endif
  #ifdef DEBUG_SETUP_S88
    Serial.println(" ");
    delay(100);
  #endif
  new_s88_setup_needed = false;
}


//#############################################################################
// Test aller LEDs
//#############################################################################
void test_s88_leds(){
  int s88_i2c_bus;
  board_num = 0;
  #ifdef DEBUG_LED_TEST
    Serial.println("");
    Serial.println("-----------------------------------");
    Serial.println(" - S88: Testing LEDS              -");
    Serial.println("-----------------------------------");
  #endif
  for (int m = 0; m < ANZ_S88_ADDONS; m++) {
    for(int i = 0; i < ANZ_S88_PER_ADDON; i++){
      s88_i2c_bus = m / 8;
      #ifdef DEBUG_LED_TEST
        Serial.print("I2C-BUS: ");
        Serial.print(s88_i2c_bus);
        Serial.print(" | Modul: ");
        Serial.print(m);
        Serial.print(" | AddOn: ");
        Serial.print(m+1);
        Serial.print(" LED: ");
        Serial.print(i+1);
        Serial.println(" ON");
      #endif
      
      if ( s88_i2c_bus == 0 ) {
        AddOn[board_num].digitalWrite(i, HIGH);   // switch all LEDs on
      #ifdef USE_WIRE1
      } else if ( s88_i2c_bus == 1 ) {
        AddOn_W1[board_num].digitalWrite(i, HIGH);   // switch all LEDs on
      #endif
      #ifdef USE_WIRE2
      } else if ( s88_i2c_bus == 2 ) {
        AddOn_W2[board_num].digitalWrite(i, HIGH);   // switch all LEDs on
      #endif
      #ifdef USE_WIRE3
      } else if ( s88_i2c_bus == 3 ) {
        AddOn_W3[board_num].digitalWrite(i, HIGH);   // switch all LEDs on
      #endif
      }
      #ifdef DEBUG_LED_TEST
        Serial.println("");
        delay(25);
      #endif
    }
    board_num++;
    if ( board_num == 8 ) {
      board_num = 0;
    }
    
  }
  #ifndef DEBUG_LED_TEST
    delay(5000);
  #else
    delay(5000);
    delay(60000);
  #endif
  board_num = 0;
  for (int m = 0; m < ANZ_S88_ADDONS; m++) {
    for(int i = 0; i < ANZ_S88_PER_ADDON; i++){
      s88_i2c_bus = m / 8;
      #ifdef DEBUG_LED_TEST
        Serial.print("I2C-BUS: ");
        Serial.print(s88_i2c_bus);
        Serial.print(" | Modul: ");
        Serial.print(m);
        Serial.print(" | AddOn: ");
        Serial.print(m+1);
        Serial.print(" LED: ");
        Serial.print(i+1);
        Serial.println(" OFF");
      #endif

      if ( s88_i2c_bus == 0 ) {
        AddOn[board_num].digitalWrite(i, LOW);   // switch all LEDs on
      #ifdef USE_WIRE1
      } else if ( s88_i2c_bus == 1 ) {
        AddOn_W1[board_num].digitalWrite(i, LOW);   // switch all LEDs on
      #endif
      #ifdef USE_WIRE2
      } else if ( s88_i2c_bus == 2 ) {
        AddOn_W2[board_num].digitalWrite(i, LOW);   // switch all LEDs on
      #endif
      #ifdef USE_WIRE3
      } else if ( s88_i2c_bus == 3 ) {
        AddOn_W3[board_num].digitalWrite(i, LOW);   // switch all LEDs on
      #endif
      }
      #ifdef DEBUG_LED_TEST
        Serial.println("");
        delay(10);
      #endif

    }
    board_num++;
    if ( board_num == 8 ) {
      board_num = 0;
    }

  }
      
}


//#############################################################################
// read all S88 feedback contacts
//#############################################################################
void read_all_s88_contact() {
  /*
   * EN:
   * initial reading of all s88 feedback contacts
   * 200ms for 1 contacts
   * 128 contacts needs about 32 seconds
   * normally not needed, because CS2/3 needs longer to boot
   * 
   * DE:
   * Alle S88 Kontakte einmal initial abfragen.
   * Alle 200ms wird 1 Kontakt abgefragt. 
   * Dauert für 128 Kontakte ca 32 sek)
   * Eigentlich unnötig, das die CS2 zum booten wesentlich länger braucht,
   * und das Modul eigentlich mit der CS2 eingschaltet wird.
   */
  while (checked_S88 <= NUM_S88) {
    currentMillis_s88read = millis();
    if(currentMillis_s88read - previousMillis_s88read >= interval_s88read) {
      previousMillis_s88read = currentMillis_s88read;
      mcan.checkS88StateFrame(device, modulID, s88_contacts[checked_S88].rm);
      #ifdef DEBUG_SETUP_S88
        Serial.print(millis());
        Serial.print(" - requesting RM (");
        Serial.print(checked_S88);
        Serial.print("): ");
        Serial.println(s88_contacts[checked_S88].rm);
      #endif
      checked_S88++;
    }
  }
  checked_S88 = 0;
  
}


//#############################################################################
// Set LEDs for S88
//#############################################################################
void s88_loop() {
  if (!use_L88) {
    //--------------------------------------------------------------------------------------------
    // CS2/CS3plus - S88 BUS
    //--------------------------------------------------------------------------------------------
    for(int i = 0; i < NUM_S88; i++){
      if(s88_contacts[i].state_is != s88_contacts[i].state_set){
        addon_modul = ( s88_contacts[i].rm - 1 ) / 16;                      // ( 66 - 1) / 16 = 4        
        addon_pin   = s88_contacts[i].rm - ( s88_modul * 16 ) - 1;          // 66 - ( 1 * 16 ) - 1 = 1

        if ( s88_contacts[i].i2c_bus == 0 ) {
          AddOn[s88_contacts[i].board_num].digitalWrite(addon_pin, s88_contacts[i].state_set);
        #ifdef USE_WIRE1
        } else if ( s88_contacts[i].i2c_bus == 1 ) {
          AddOn_W1[s88_contacts[i].board_num].digitalWrite(addon_pin, s88_contacts[i].state_set);
        #endif
        #ifdef USE_WIRE2
        } else if ( s88_contacts[i].i2c_bus == 2 ) {
          AddOn_W2[s88_contacts[i].board_num].digitalWrite(addon_pin, s88_contacts[i].state_set);
        #endif
        #ifdef USE_WIRE3
        } else if ( s88_contacts[i].i2c_bus == 3 ) {
          AddOn_W3[s88_contacts[i].board_num].digitalWrite(addon_pin, s88_contacts[i].state_set);
        #endif
        }
        s88_contacts[i].state_is = s88_contacts[i].state_set;
        #ifdef DEBUG_S88
          Serial.print(millis());
          Serial.print(" - switching RM - ");
          Serial.print(s88_contacts[i].rm);
          Serial.print(" - AddOn: ");
          Serial.print(addon_modul);
          Serial.print(" PIN: ");
          Serial.print(addon_pin);
          Serial.print(" -> ");
          if(s88_contacts[i].state_set == POWER_ON)  Serial.println("ON");
          if(s88_contacts[i].state_set == POWER_OFF) Serial.println("OFF");
          delay(100);
        #endif
      }
    }
    //--------------------------------------------------------------------------------------------
    // END - CS2/CS3plus - S88 BUS
    //--------------------------------------------------------------------------------------------
  } else {
    //--------------------------------------------------------------------------------------------
    // Link L88 - S88 BUS
    //--------------------------------------------------------------------------------------------
    for(int i = 0; i < NUM_S88; i++){
      if(s88_contacts[i].state_is != s88_contacts[i].state_set){
        if (s88_contacts[i].rm > 1000) {
          bus       = s88_contacts[i].rm / 1000;
          contact   = s88_contacts[i].rm - ( bus * 1000 );
          s88_modul = ( ( contact - 1 ) / 16 );
          addon_pin = contact - ( s88_modul * 16 ) - 1;
          switch (bus) {
            case 3:
              addon_modul = 1 + s88_modul - start_bus[3] + len_bus[2] + len_bus[1] + use_bus[0];
              break;
            case 2:
              addon_modul = 1 + s88_modul - start_bus[2] + len_bus[1] + use_bus[0];
              break;
            case 1:
              addon_modul = 1 + s88_modul - start_bus[1] + use_bus[0];
              break;
          }
        } else {
          addon_modul = 0;
          addon_pin   = s88_contacts[i].rm - 1;
        }
        #ifdef DEBUG_S88
          Serial.print(millis());
          Serial.print(" - switching RM - ");
          Serial.print(s88_contacts[i].rm);
          Serial.print(" - AddOn: ");
          Serial.print(addon_modul);
          Serial.print(" PIN: ");
          Serial.print(addon_pin);
          Serial.print(" -> ");
          if(s88_contacts[i].state_set == POWER_ON)  Serial.println("ON");
          if(s88_contacts[i].state_set == POWER_OFF) Serial.println("OFF");
          delay(100);
        #endif
        if ( s88_contacts[i].i2c_bus == 0 ) {
          AddOn[s88_contacts[i].board_num].digitalWrite(addon_pin, s88_contacts[i].state_set);
        #ifdef USE_WIRE1
        } else if ( s88_contacts[i].i2c_bus == 1 ) {
          AddOn_W1[s88_contacts[i].board_num].digitalWrite(addon_pin, s88_contacts[i].state_set);
        #endif
        #ifdef USE_WIRE2
        } else if ( s88_contacts[i].i2c_bus == 2 ) {
          AddOn_W2[s88_contacts[i].board_num].digitalWrite(addon_pin, s88_contacts[i].state_set);
        #endif
        #ifdef USE_WIRE3
        } else if ( s88_contacts[i].i2c_bus == 3 ) {
          AddOn_W3[s88_contacts[i].board_num].digitalWrite(addon_pin, s88_contacts[i].state_set);
        #endif
        }
        s88_contacts[i].state_is = s88_contacts[i].state_set;
      }
    }
    //--------------------------------------------------------------------------------------------
    // END - Link L88 - S88 BUS
    //--------------------------------------------------------------------------------------------
  }

}



void config_poll_s88() {
  Serial.println(" ");
  Serial.println("----------------------------------------------------------------------------------------------");
  Serial.print(millis());
  Serial.print(" - config_poll = ");
  Serial.print(config_poll);
  Serial.print(" / config_index = ");
  Serial.println(config_index);
  //================================================================================================
  // Send Config
  //================================================================================================
  if(config_index == 0)  mcan.sendDeviceInfo(device, CONFIG_NUM_S88);

  if(config_index == 1)  mcan.sendConfigInfoDropdown(device,  1,      2, use_L88, "Welchen S88 nutzen?_CS2/3plus_Link L88");
  if(config_index == 2)  mcan.sendConfigInfoSlider(  device,  2,  0, 16383, modulID, "Gerätekennung_0_16383");

  if (!use_L88){
    //----------------------------------------------------------------------------------------------
    // START CS2/CS3plus - S88 BUS
    //----------------------------------------------------------------------------------------------
    if(config_index == 3)  mcan.sendConfigInfoSlider(  device,  3,  0, 31, len_bus[0],   "Bus Länge_0_31");
    if(config_index == 4)  mcan.sendConfigInfoSlider(  device,  4,  0, 31, start_bus[0], "Start Modul_0_31");
    //----------------------------------------------------------------------------------------------
    // END - CS2/CS3plus - S88 BUS
    //----------------------------------------------------------------------------------------------
  } else {
    //----------------------------------------------------------------------------------------------
    // START Link L88 - S88 BUS
    //----------------------------------------------------------------------------------------------
    if(config_index == 3)  mcan.sendConfigInfoDropdown(device,  3,      2, use_bus[0],   "Bus 0 nutzen?_Nein_Ja");

    if(config_index == 4)  mcan.sendConfigInfoDropdown(device,  4,      2, use_bus[1],   "Bus 1 nutzen?_Nein_Ja");
    if(config_index == 5)  mcan.sendConfigInfoSlider(  device,  5,  0, 31, len_bus[1],   "Bus Länge_0_31");
    if(config_index == 6)  mcan.sendConfigInfoSlider(  device,  6,  0, 31, start_bus[1], "Start Modul_0_31");

    if(config_index == 7)  mcan.sendConfigInfoDropdown(device,  7,      2, use_bus[2],   "Bus 2 nutzen?_Nein_Ja");
    if(config_index == 8)  mcan.sendConfigInfoSlider(  device,  8,  0, 31, len_bus[2],   "Bus Länge_0_31");
    if(config_index == 9)  mcan.sendConfigInfoSlider(  device,  9,  0, 31, start_bus[2], "Start Modul_0_31");

    if(config_index == 10) mcan.sendConfigInfoDropdown(device, 10,      2, use_bus[3],   "Bus 3 nutzen?_Nein_Ja");
    if(config_index == 11) mcan.sendConfigInfoSlider(  device, 11,  0, 31, len_bus[3],   "Bus Länge_0_31");
    if(config_index == 12) mcan.sendConfigInfoSlider(  device, 12,  0, 31, start_bus[3], "Start Modul_0_31");
    //----------------------------------------------------------------------------------------------
    // END - Link L88 - S88 BUS
    //----------------------------------------------------------------------------------------------
  }
  config_poll = false;
  Serial.print(millis());
  Serial.print(" - config_poll = ");
  Serial.println(config_poll);
  Serial.println("----------------------------------------------------------------------------------------------");
  Serial.println(" ");
  //================================================================================================
  // END - CONFIG_POLL
  //================================================================================================

}

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
  if ((mcan_frame_in.cmd == PING) && (mcan_frame_in.resp_bit == 0)) {
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
      //Serial.println(mcan.getadrs(prot, locid));
      Serial.println(locid);
    #endif
    
    for (int i = 0; i < NUM_ACCs; i++) {
      if (locid == acc_articles[i].locID) {
        #ifdef DEBUG_CAN
          Serial.print(millis());
          Serial.println("    => match found -> Set target state.");
        #endif
        acc_articles[i].state_set = mcan_frame_in.data[4];
        acc_articles[i].power_set = mcan_frame_in.data[5];
        break;
      }

    }
    //------------------------------------------------------------------------------------------------
    // ENDE - ACC Frame
    //------------------------------------------------------------------------------------------------
  } else if((mcan_frame_in.cmd == S88_EVENT) && (mcan_frame_in.resp_bit == 1)){
    // ------------------------------------------------------------------------------------------------
    // START - S88 Frame            - Püfen auf S88 Events
    // ------------------------------------------------------------------------------------------------
    uint16_t incoming_modulID = (mcan_frame_in.data[0] << 8 | mcan_frame_in.data[1]);   // bsp. 0x00b3 => 179 (L88)
    uint16_t kontakt = (mcan_frame_in.data[2] << 8 | mcan_frame_in.data[3]);            // bsp. 0x03fe => 1022 => Bus 1, Modul 2, Kontakt 6
    //uint8_t old_state = mcan_frame_in.data[4];                                        // bsp. 0x01   => 1
    uint8_t curr_state = mcan_frame_in.data[5];                                         // bsp. 0x00   => 0
    //uint16_t s_time = (mcan_frame_in.data[6] << 8 | mcan_frame_in.data[7]);           // bsp. 0x0190 => 400
    #ifdef DEBUG_CAN
      Serial.print("Recieved S88-Frame for modul: ");
      Serial.print(incoming_modulID);
      Serial.print(" / contact: ");
      Serial.println(kontakt);
    #endif
    for(int i = 0; i < NUM_S88; i++){
      if(incoming_modulID == modulID) {
        if(s88_contacts[i].rm == kontakt) {
          #ifdef DEBUG_CAN
            Serial.println(" => match found -> Set target state.");
          #endif
          s88_contacts[i].state_set = curr_state;
          break;
        }
      }
    }
    // ------------------------------------------------------------------------------------------------
    // ENDE - S88 Frame
    // ------------------------------------------------------------------------------------------------
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
      if(CONFIG_NUM_S88 == 4) {
        //--------------------------------------------------------------------------------------------
        // CS2/CS3plus - S88 Bus Konfiguration
        //--------------------------------------------------------------------------------------------
        switch (mcan_frame_in.data[5]) {
        case 1:       // use_L88       -- CS2/3 (false) oder Links L88 (true) 
          if(use_L88 != mcan_frame_in.data[7]) {
            EEPROM.put(REG_use_L88, mcan_frame_in.data[7]);
            if(mcan_frame_in.data[7] == 0){
              EEPROM.put(REG_use_bus0, 1);
            }
            new_s88_setup_needed = true;
          }
          break;
        case 2:       // Gerätekennung
          if(modulID != (mcan_frame_in.data[6] << 8 | mcan_frame_in.data[7]) ) {
            EEPROM.put(REG_modulID,   mcan_frame_in.data[6]);
            EEPROM.put(REG_modulID+1, mcan_frame_in.data[7]);
            new_s88_setup_needed = true;
          }
          break;
        case 3:       // Länge Bus
          if(len_bus[0] != mcan_frame_in.data[7]) {
            EEPROM.put(REG_len_bus0, mcan_frame_in.data[7]);
            new_s88_setup_needed = true;
          }
          break;
        case 4:       // Start Bus
          if(start_bus[0] != mcan_frame_in.data[7]){
            EEPROM.put(REG_start_bus0, mcan_frame_in.data[7]);
            new_s88_setup_needed = true;
          }
          break;
        }
      } else if (CONFIG_NUM_S88 == 12) {
        //--------------------------------------------------------------------------------------------
        // Link L88 - S88 Bus Konfiguration
        //--------------------------------------------------------------------------------------------
        switch (mcan_frame_in.data[5]) {
        case 1:       // use_L88       -- CS2/3 (false) oder Links L88 (true) 
          if(use_L88 != mcan_frame_in.data[7]) {
            EEPROM.put(REG_use_L88, mcan_frame_in.data[7]);
            if(mcan_frame_in.data[7] == 0){
              EEPROM.put(REG_use_bus0, 1);
            }
            new_s88_setup_needed = true;
          }
          break;
        case 2:       // Gerätekennung
          if(modulID != (mcan_frame_in.data[6] << 8 | mcan_frame_in.data[7]) ) {
            EEPROM.put(REG_modulID,   mcan_frame_in.data[6]);
            EEPROM.put(REG_modulID+1, mcan_frame_in.data[7]);
            new_s88_setup_needed = true;
          }
          break;
        case 3:       // Use L88 Bus 0
          if (use_bus[0] != mcan_frame_in.data[7]) {
            EEPROM.put(REG_use_bus0,   mcan_frame_in.data[7]);     // use   bus0
            EEPROM.put(REG_len_bus0,   mcan_frame_in.data[7]);     // len   bus0
            EEPROM.put(REG_start_bus0, mcan_frame_in.data[7]);     // start bus0
            new_s88_setup_needed = true;
          }
          break;
        case 4:       // Use Link L88 BUS1 (1001-1512)
          if (use_bus[1] != mcan_frame_in.data[7]) {
            EEPROM.put(REG_use_bus1, mcan_frame_in.data[7]);
            new_s88_setup_needed = true;
          }
          break;
        case 5:       // Länge Link L88 BUS1
          if (len_bus[1] != mcan_frame_in.data[7]) {
            EEPROM.put(REG_len_bus1, mcan_frame_in.data[7]);
            new_s88_setup_needed = true;
          }
          break;
        case 6:       // Start Link L88 BUS1
          if (start_bus[1] != mcan_frame_in.data[7]) {
            EEPROM.put(REG_start_bus1, mcan_frame_in.data[7]);
            new_s88_setup_needed = true;
          }
          break;
        case 7:       // Use Link L88 BUS2 (2001-2512)
          if (use_bus[2] != mcan_frame_in.data[7]) {
            EEPROM.put(REG_use_bus2, mcan_frame_in.data[7]);
            new_s88_setup_needed = true;
          }
          break;
        case 8:       // Länge Link L88 BUS2
          if (len_bus[2] != mcan_frame_in.data[7]) {
            EEPROM.put(REG_len_bus2, mcan_frame_in.data[7]);
            new_s88_setup_needed = true;
          }
          break;
        case 9:       // Start Link L88 BUS2
          if (start_bus[2] != mcan_frame_in.data[7]) {
            EEPROM.put(REG_start_bus2, mcan_frame_in.data[7]);
            new_s88_setup_needed = true;
          }
          break;
        case 10:       // Use Link L88 BUS3 (3001-3512)
          if (use_bus[3] != mcan_frame_in.data[7]) {
            EEPROM.put(REG_use_bus3, mcan_frame_in.data[7]);
            new_s88_setup_needed = true;
          }
          break;
        case 11:       // Länge Link L88 BUS3
          if (len_bus[3] != mcan_frame_in.data[7]) {
            EEPROM.put(REG_len_bus3, mcan_frame_in.data[7]);
            new_s88_setup_needed = true;
          }
          break;
        case 12:       // Start Link L88 BUS3
          if (start_bus[3] != mcan_frame_in.data[7]) {
            EEPROM.put(REG_start_bus3, mcan_frame_in.data[7]);
            new_s88_setup_needed = true;
          }
          break;
        }
      }
      mcan.statusResponse(device, mcan_frame_in.data[5]);
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


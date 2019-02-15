// OLD FILE
// normally not needed
// only if you want to configure INPUT part instead of S88 part via CS2/3



//#############################################################################
// Eingehene CAN Frame überprüfen
//#############################################################################
void CONFIG_FRAME_INPUT() {
  if ((uid_mat[0] == can_frame_in.data[0]) && (uid_mat[1] == can_frame_in.data[1]) && (uid_mat[2] == can_frame_in.data[2]) && (uid_mat[3] == can_frame_in.data[3])) {
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
      /*
      if (config_index == 0) locked = true;
      if (config_index == CONFIG_NUM) {
        locked = false;
        config_sent = true;
        Serial.println(" - CONFIG SENT");
        Serial.println(" - System unlocked");
      }
      */
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
          change_global_prot();
        }
        mcan.statusResponse(device, can_frame_in.data[5]);

      } else if (can_frame_in.data[5] >= 2) {
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
// ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE ENDE
//#############################################################################

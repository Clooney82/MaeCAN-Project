//#############################################################################
// manual config, when UID changed
//#############################################################################
void config_own_adresses_manual() {
  uint16_t adrsss;
  for (int i = 0; i < NUM_ACCs; i++) {
    acc_articles[i].reg_locid = (200 + (6 * (i + 1)) - 5);
    acc_articles[i].reg_prot  = (200 + (6 * (i + 1)) - 2);
    #ifdef LED_FEEDBACK
      acc_articles[i].reg_state = (200 + (6 * (i + 1)) - 1);
    #endif
    acc_articles[i].reg_type  = (200 + (6 * (i + 1))    );
    switch (i) {
      case 0:
        acc_articles[i].acc_type = TYPE_UNCOUPLER;   // 0 = Turnout/Signal ; 1 = Uncoupler
        acc_articles[i].prot = ACC_MM;  // 0 = ACC_DCC oder 1 = ACC_MM;
        adrsss = 44;
        break;
      case 1:
        acc_articles[i].acc_type = TYPE_TURNOUT;   // 0 = Turnout/Signal ; 1 = Uncoupler
        acc_articles[i].prot = ACC_DCC;  // 0 = ACC_DCC oder 1 = ACC_MM;
        adrsss = 43;
        break;
      case 2:
        acc_articles[i].acc_type = TYPE_SINGLE_BUTTON;   // 0 = Turnout/Signal ; 1 = Uncoupler
        acc_articles[i].prot = ACC_MM;  // 0 = ACC_DCC oder 1 = ACC_MM;
        adrsss = 42;
        break;
      case 3:
        acc_articles[i].acc_type = TYPE_UNCOUPLER;   // 0 = Turnout/Signal ; 1 = Uncoupler
        acc_articles[i].prot = ACC_MM;  // 0 = ACC_DCC oder 1 = ACC_MM;
        adrsss = 41;
        break;
        
      default:
        acc_articles[i].acc_type = TYPE_TURNOUT;   // 0 = Turnout/Signal ; 1 = Uncoupler
        acc_articles[i].prot = ACC_MM;  // 0 = ACC_DCC oder 1 = ACC_MM;
        adrsss = base_address + i;
        break;
    }
    //acc_articles[i].locID = mcan.generateLocId(prot, adrsss );      //GLOBAL
    acc_articles[i].locID = mcan.generateLocId(acc_articles[i].prot, adrsss );
    byte locid_high = acc_articles[i].locID >> 8;
    byte locid_low = acc_articles[i].locID;
    EEPROM.put(acc_articles[i].reg_locid, locid_high);
    EEPROM.put(acc_articles[i].reg_locid + 1, locid_low);
    EEPROM.put(acc_articles[i].reg_type, acc_articles[i].acc_type);
    EEPROM.put(acc_articles[i].reg_prot, acc_articles[i].prot);
    #ifdef LED_FEEDBACK
      acc_articles[i].state_is = 0;
      EEPROM.put( acc_articles[i].reg_state, acc_articles[i].state_is);
      acc_articles[i].state_set = acc_articles[i].state_is;
      acc_articles[i].power_is = 0;
      acc_articles[i].power_set = acc_articles[i].power_is;
    #endif
    #ifdef DEBUG_SETUP_ACC
      Serial.print("Init ACC #");
      Serial.print(i);
      Serial.print(" -> Adresse: ");
      Serial.print(adrsss);
      Serial.print(" -> Protocol: ");
      Serial.print(acc_articles[i].prot);
      Serial.print(" -> Local-ID: ");
      Serial.println(acc_articles[i].locID);
      //Serial.println("-----------------------------------");
      delay(100);
    #endif

  }

}

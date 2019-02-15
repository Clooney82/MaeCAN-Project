//#############################################################################
// Manuelles configurieren der Adressen, wenn sich UID ändert.
//#############################################################################
void config_own_adresses_manual() {
  uint16_t adrsss;
  for (int i = 0; i < NUM_ACCs; i++) {
    acc_articles[i].reg_locid = (200 + (6 * (i + 1)) - 5);
    #ifdef LED_FEEDBACK
      acc_articles[i].reg_state = (200 + (6 * (i + 1)) - 1);
    #endif
    acc_articles[i].reg_type  = (200 + (6 * (i + 1))    );
    switch (i) {
      case 0:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 1:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 2:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 3:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 4:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 5:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 6:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 7:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 8:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 9:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 10:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 11:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 12:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 13:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 14:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 15:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 16:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 33:
        acc_articles[i].acc_type = TYPE_ENTKUPPLER;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 41:
        acc_articles[i].acc_type = TYPE_ENTKUPPLER;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      case 44:
        acc_articles[i].acc_type = TYPE_ENTKUPPLER;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
        adrsss = base_address + i;
        break;
      default:
        acc_articles[i].acc_type = TYPE_WEICHE;   // 0 = Weiche/Signal ; 1 = Entkuppler
        acc_articles[i].prot = MM_ACC;  // 0 = DCC_ACC oder 1 = MM_ACC;
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
    #ifdef LED_FEEDBACK
      acc_articles[i].state_is = 0;
      EEPROM.put( acc_articles[i].reg_state, acc_articles[i].state_is);
      acc_articles[i].state_set = acc_articles[i].state_is;
      acc_articles[i].power_is = 0;
      acc_articles[i].power_set = acc_articles[i].power_is;
    #endif
  }

}



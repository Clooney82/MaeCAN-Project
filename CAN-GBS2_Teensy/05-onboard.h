const uint8_t LOCK_BUTTON = 17;
const uint8_t RGB_R = 16;
const uint8_t RGB_G = 15;
const uint8_t RGB_B = 14;

unsigned long buttonTimer = 0;
unsigned long longPressTime = 2000; // ms
bool buttonActive = false;
bool longPressActive = false;

bool GBS_locked = true;

const bool    RIGHT = 0;
const bool    LEFT  = 1;
uint8_t DS_rec_turn_dir;
int DS_new_pos;
unsigned long DS_rectime = 0;
unsigned long DS_rectimeout = 1000;
#define REG_DS_pos    110



// Teensy PINS for GL
const uint8_t DS_GL01 =  5;
const uint8_t DS_GL02 =  6;
const uint8_t DS_GL03 =  7;
const uint8_t DS_GL04 =  8;
const uint8_t DS_GL05 =  9;
const uint8_t DS_GL06 = 10;
const uint8_t DS_GL07 = 11;
const uint8_t DS_GL08 = 12;
const uint8_t DS_GL09 = 23;
const uint8_t DS_GL10 = 22;
const uint8_t DS_GL11 = 31;
const uint8_t DS_GL12 = 32;
bool DS_buttonActive = false;

typedef struct  {
  int base_address = 225;
  int RM_active = 1097;
  int RM_num;
  int Step_address      = base_address +  2;
  int Turn_180_address  = base_address +  1;
  int Turn_side_address = base_address +  3;
  int POS_address[12]   = {
    base_address +  4,
    base_address +  5,
    base_address +  6,
    base_address +  7,
    base_address +  8,
    base_address +  9,
    base_address + 10,
    base_address + 11,
    base_address + 12,
    base_address + 13,
    base_address + 14,
    base_address + 15
  };
  int POS_target  = 0;
  int POS_current = 0;
  uint16_t locID[12];       // LocalID
  uint16_t locID_turn_dir;
  uint16_t locID_turn180;
  uint8_t prot = ACC_MM;

} turntable;

turntable DSD;


void setup_ds_locid() {
  #ifdef DEBUG_DS
    Serial.print(millis());
    Serial.println(" - TURNABLE Setup: ");
    Serial.print(millis());
    Serial.print(" - bade_address: ");
    Serial.println(DSD.base_address);
  #endif
  DSD.locID_turn_dir = mcan.generateLocId(DSD.prot, DSD.Turn_side_address);
  #ifdef DEBUG_DS
    Serial.print(millis());
    Serial.print(" - TURNABLE: Turn side (right/left) address: ");
    Serial.print(DSD.Turn_side_address);
    Serial.print(" localID: ");
    Serial.println(DSD.locID_turn_dir);
  #endif
  DSD.locID_turn180 = mcan.generateLocId(DSD.prot, DSD.Turn_180_address);
  #ifdef DEBUG_DS
    Serial.print(millis());
    Serial.print(" - TURNABLE: Turn 180 degree address: ");
    Serial.print(DSD.Turn_side_address);
    Serial.print(" localID: ");
    Serial.println(DSD.locID_turn_dir);
  #endif
  for (int i = 0; i < 12; i++) {
    DSD.locID[i] = mcan.generateLocId(DSD.prot, DSD.POS_address[i]);
    #ifdef DEBUG_DS
      Serial.print(millis());
      Serial.print(" - TURNABLE: address(");
      Serial.print(i+1);
      Serial.print("): ");
      Serial.print(DSD.POS_address[i]);
      Serial.print(" localID: ");
      Serial.println(DSD.locID[i]);
    #endif
  }
  
}


void setup_ds_rm_num() {
  for(int i = 0; i < NUM_S88; i++){
    if ( s88_contacts[i].rm == DSD.RM_active ) {
      DSD.RM_num = i;
      break;
      
    }
    
  }
  
}


void set_current_DS_pos(uint16_t locid, bool state) {
  if ((millis() - DS_rectime ) > DS_rectimeout) {
    if (locid == DSD.locID_turn_dir) {
      #ifdef DEBUG_DS
        Serial.print(millis());
        Serial.print(" - current DS position: ");
        Serial.println(DSD.POS_current);
      #endif
      if ( state == RIGHT ) {     // turn right
        #ifdef DEBUG_DS
          Serial.print(millis());
          Serial.println(" - recieved DS turn right.");
        #endif
        DS_rec_turn_dir = RIGHT;
        
      } else {                  // turn left
        #ifdef DEBUG_DS
          Serial.print(millis());
          Serial.println(" - recieved DS turn left.");
        #endif
        DS_rec_turn_dir = LEFT;
        
      }
      
    } else if (locid == DSD.locID_turn180) {
      DS_new_pos = DSD.POS_current + 24;
      if ( DS_new_pos > 48 ) {
        DS_new_pos = DS_new_pos - 48;
      }
      
      #ifdef DEBUG_DS
        Serial.print(millis());
        Serial.println(" - recieved turn 180 degree.");
        Serial.print(millis());
        Serial.print(" - new DS position: ");
        Serial.println(DS_new_pos);
      #endif
      DSD.POS_current = DS_new_pos;
      DS_rectime = millis();  
      
    } else {
      for (int i = 0; i < 12; i++) {
        if (locid == DSD.locID[i]) {
          DS_new_pos = 1 + ( i * 2 ) + state;
          #ifdef DEBUG_DS
            Serial.print(millis());
            Serial.print(" - recieved new DS position (temp): ");
            Serial.println(DS_new_pos);
          #endif
          break;
          
        }
        
      }
      if ( DS_rec_turn_dir == RIGHT ) {     // turn right
        if ( DSD.POS_current > DS_new_pos ) {
          if ( ( DS_new_pos + 24 ) > DSD.POS_current ) {
            DS_new_pos = DS_new_pos + 24; 
          }
        }
  
      } else {                              // turn left
        if ( DSD.POS_current < DS_new_pos ) {
          DS_new_pos = DS_new_pos + 24;
        } else {
          if ( DS_new_pos + 24 < 24 ) {
            DS_new_pos = DS_new_pos + 24;
          }
        }
        
      }
  
      #ifdef DEBUG_DS
        Serial.print(millis());
        Serial.print(" - recieved new DS position (final): ");
        Serial.println(DS_new_pos);
      #endif
  
      DSD.POS_current = DS_new_pos;
      DS_rectime = millis();
      
    }
  } 
  EEPROM.put(REG_DS_pos, DSD.POS_current);
}


void send_ds_acc(int pos) {
  if (pos > 24) pos = pos - 24;
  int i = (pos - 1) / 2;
  int state = (pos + 1) %2;
  int turn_steps_right = 0;
  int turn_steps_left  = 0;
  if ((millis() - DS_rectime ) > DS_rectimeout) {
    if ( ( DSD.POS_current == 1 && DSD.POS_target == 25 ) || ( DSD.POS_current == 25 && DSD.POS_target == 1 ) ||
          ( DSD.POS_current == 2 && DSD.POS_target == 26 ) || ( DSD.POS_current == 26 && DSD.POS_target == 2 ) ||
          ( DSD.POS_current == 47 && DSD.POS_target == 23 ) || ( DSD.POS_current == 23 && DSD.POS_target == 47 ) ||
          (DSD.POS_current == DSD.POS_target) ) {
    //if ( (DSD.POS_current == DSD.POS_target) ) {
      #ifdef DEBUG_DS
        Serial.print(millis());
        Serial.print(" - send 180 degree turn command to DS: ");
        Serial.println(DSD.base_address);
      #endif
      mcan.sendAccessoryFrame(device, mcan.generateLocId(DSD.prot, DSD.Turn_side_address), GREEN, false);
      delay(10);
      mcan.sendAccessoryFrame(device, mcan.generateLocId(DSD.prot, DSD.Turn_180_address), GREEN, false);
      if (DSD.POS_current == DSD.POS_target) {
        DSD.POS_target = DSD.POS_target + 24;
        if ( DSD.POS_target > 48 ) {
          DSD.POS_target = DSD.POS_target - 48;
          
        } 
      }
      #ifdef DEBUG_DS
        Serial.print(millis());
        Serial.print(" - new Position: ");
        Serial.println(DSD.POS_target);
      #endif
    
    } else {
      if ( DSD.POS_target > DSD.POS_current ) {
        turn_steps_right = DSD.POS_target - DSD.POS_current;
        
      } else {
        turn_steps_right = 48 - DSD.POS_current + DSD.POS_target;
        
      }
      
      if ( DSD.POS_current > DSD.POS_target ) {
        turn_steps_left = DSD.POS_current - DSD.POS_target;
        
      } else {
        turn_steps_left = 48 - turn_steps_right;
        
      }
      
      if ( turn_steps_right < turn_steps_left) {
        // Turn right
        #ifdef DEBUG_DS
          Serial.print(millis());
          Serial.print(" - send turn right command to DS: ");
          Serial.println(DSD.base_address);
        #endif
        mcan.sendAccessoryFrame(device, mcan.generateLocId(DSD.prot, DSD.Turn_side_address), RED, false);
        
      } else { 
        // Turn left
      #ifdef DEBUG_DS
          Serial.print(millis());
          Serial.print(" - send turn left command to DS: ");
          Serial.println(DSD.base_address);
        #endif
        mcan.sendAccessoryFrame(device, mcan.generateLocId(DSD.prot, DSD.Turn_side_address), GREEN, false);
        
      } 
      
      delay(25);            
    
      #ifdef DEBUG_DS
        Serial.print(millis());
        Serial.print(" - i: ");
        Serial.print(i);
        Serial.print(" - state: ");
        Serial.print(state);
        Serial.print(" - Current DS Position: ");
        Serial.println(DSD.POS_current);
    
        Serial.print(millis());
        Serial.print(" - send turn to position ");
        Serial.print(DSD.POS_target);
        Serial.print(" (");
        Serial.print(pos);
        Serial.print(") command to DS: ");
        Serial.println(DSD.base_address);
    
        Serial.print(millis());
        Serial.print(" - sending DS command via address: ");
        Serial.print(DSD.POS_address[i]);
        if (state) {
          Serial.println(" - GREEN");
        } else {
          Serial.println(" - RED");
        }
        Serial.println("");
      #endif
      mcan.sendAccessoryFrame(device, mcan.generateLocId(DSD.prot, DSD.POS_address[i]), state, false);
      
    }
    DSD.POS_current = DSD.POS_target;
    EEPROM.put(REG_DS_pos, DSD.POS_current);
    DS_rectime = millis();
  }
//  DSD.POS_target = 0;
  
}


void check_DS_input() {
  if ( s88_contacts[DSD.RM_num].state_is == false) {
    if ( DS_buttonActive == false && digitalRead(DS_GL01) == LOW ) {
      DSD.POS_target = 1;
      DS_buttonActive = true;
      send_ds_acc(DSD.POS_target);
      
    } else if ( DS_buttonActive == false && digitalRead(DS_GL02) == LOW ) {
      DSD.POS_target = 2;
      DS_buttonActive = true;
      send_ds_acc(DSD.POS_target);
      
    } else if ( DS_buttonActive == false && digitalRead(DS_GL03) == LOW ) {
      DSD.POS_target = 20;
      DS_buttonActive = true;
      send_ds_acc(DSD.POS_target);
      
    } else if ( DS_buttonActive == false && digitalRead(DS_GL04) == LOW ) {
      DSD.POS_target = 21;
      DS_buttonActive = true;
      send_ds_acc(DSD.POS_target);
      
    } else if ( DS_buttonActive == false && digitalRead(DS_GL05) == LOW ) {
      DSD.POS_target = 22;
      DS_buttonActive = true;
      send_ds_acc(DSD.POS_target);
      
    } else if ( DS_buttonActive == false && digitalRead(DS_GL06) == LOW ) {
      DSD.POS_target = 23;
      DS_buttonActive = true;
      send_ds_acc(DSD.POS_target);
      
    } else if ( DS_buttonActive == false && digitalRead(DS_GL07) == LOW ) {
      DSD.POS_target = 24;
      DS_buttonActive = true;
      send_ds_acc(DSD.POS_target);
      
    } else if ( DS_buttonActive == false && digitalRead(DS_GL08) == LOW ) {
      DSD.POS_target = 25;  // >> 1
      DS_buttonActive = true;
      send_ds_acc(DSD.POS_target);
      
    } else if ( DS_buttonActive == false && digitalRead(DS_GL09) == LOW ) {
      DSD.POS_target = 26;  // >> 2
      DS_buttonActive = true;
      send_ds_acc(DSD.POS_target);
      
    } else if ( DS_buttonActive == false && digitalRead(DS_GL10) == LOW ) {
      DSD.POS_target = 28;  // >> 4
      DS_buttonActive = true;
      send_ds_acc(DSD.POS_target);
      
    } else if ( DS_buttonActive == false && digitalRead(DS_GL11) == LOW ) {
      DSD.POS_target = 30;  // >> 6
      DS_buttonActive = true;
      send_ds_acc(DSD.POS_target);
      
    } else if ( DS_buttonActive == false && digitalRead(DS_GL12) == LOW ) {
      DSD.POS_target = 47;  // >> 23
      DS_buttonActive = true;
      send_ds_acc(DSD.POS_target);
      
    } else {
      if ( DS_buttonActive == true ) {
        if ( digitalRead(DS_GL01) == HIGH && 
              digitalRead(DS_GL02) == HIGH && 
              digitalRead(DS_GL03) == HIGH && 
              digitalRead(DS_GL04) == HIGH &&
              digitalRead(DS_GL05) == HIGH &&
              digitalRead(DS_GL06) == HIGH &&
              digitalRead(DS_GL07) == HIGH &&
              digitalRead(DS_GL08) == HIGH &&
              digitalRead(DS_GL09) == HIGH &&
              digitalRead(DS_GL10) == HIGH &&
              digitalRead(DS_GL11) == HIGH &&
              digitalRead(DS_GL12) == HIGH ) {
          DS_buttonActive = false;
        }
        
      }
      
    }

//    if ( DSD.POS_target > 0 ) {
//    if ( DSD.POS_target != DSD.POS_current ) {
//      send_ds_acc(DSD.POS_target);
//      DSD.POS_target  = 0;
//    } 
    
  }

}


void setup_onboard() {
  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);
  pinMode(LOCK_BUTTON, INPUT_PULLUP);

  pinMode(DS_GL01, INPUT_PULLUP);
  pinMode(DS_GL02, INPUT_PULLUP);
  pinMode(DS_GL03, INPUT_PULLUP);
  pinMode(DS_GL04, INPUT_PULLUP);
  pinMode(DS_GL05, INPUT_PULLUP);
  pinMode(DS_GL06, INPUT_PULLUP);
  pinMode(DS_GL07, INPUT_PULLUP);
  pinMode(DS_GL08, INPUT_PULLUP);
  pinMode(DS_GL09, INPUT_PULLUP);
  pinMode(DS_GL10, INPUT_PULLUP);
  pinMode(DS_GL11, INPUT_PULLUP);
  pinMode(DS_GL12, INPUT_PULLUP);
  
  setup_ds_locid();
  setup_ds_rm_num();
  DSD.POS_current = EEPROM.read(REG_DS_pos);
}


void set_GBS_LED() {
  if ( locked ) {
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, LOW);
    digitalWrite(RGB_R, HIGH);
    
  } else if ( GBS_locked ) {
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, LOW);
    digitalWrite(RGB_B, HIGH);
    
  } else {
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_B, LOW);
    digitalWrite(RGB_G, HIGH);
    
  }

}


void check_lock() {
  if (digitalRead(LOCK_BUTTON) == LOW) {
    if (buttonActive == false) {
      buttonActive = true;
      buttonTimer = millis();
      
    }
    if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) {
      longPressActive = true;
      GBS_locked = !GBS_locked;
    }
    
  } else {
    if (buttonActive == true) {
      if (longPressActive == true) {
        longPressActive = false;
        
      }
      buttonActive = false;
      
    }
    
  }
  set_GBS_LED();
  
}

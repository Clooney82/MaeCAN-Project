//#############################################################################
// Manuelles configurieren der Adressen, wenn sich UID ändert.
//#############################################################################
/*
 * Example:
 * BASE_ADRESS = 200
 * MSG_COUNT = 11
 * NO ADRESSES = 11 % 2 = 6
 * 
 * --------   ---   -----------   - - --- RED GREEN
 * ADRESS 1 = 200 ( BASE_ADRESS     - MSG  0,  1 )
 * ADRESS 2 = 201 ( BASE_ADRESS + 1 - MSG  2,  3 )
 * ADRESS 3 = 202 ( BASE_ADRESS + 2 - MSG  4,  5 )
 * ADRESS 4 = 203 ( BASE_ADRESS + 3 - MSG  6,  7 )
 * ADRESS 5 = 204 ( BASE_ADRESS + 4 - MSG  8,  9 )
 * ADRESS 6 = 205 ( BASE_ADRESS + 5 - MSG 10, 11 )
 * ...
 * ------------------------------------------------
 * 
 * RECIEVED:
 * ADRESS 202, GREEN
 * -> 
 * 
 * SETUP:
 * uint16_t .locID = ADRESS
 * char     .RailNr[4]
 * bool     .msg_RED
 * bool     .msg_GREEN
 *
 * 
 * 
 */

typedef struct {
  #ifdef USE_MACAN
    uint16_t locID;       // LocalID
  #endif
  #ifdef USE_DCC
    uint16_t address;
  #endif
  char    RailNr[4];
  uint8_t msg_RED;
  uint8_t msg_GREEN;
  bool    state_is  = 0;   // ...
  bool    state_set = 0;   // ...
//  bool    power_is  = 0;   // current status
  bool    power_set = 0;   // target status
//  unsigned long Millis_set = 0 // time when activated

} ZZA_T;

const uint8_t adrss_per_acc = (MSG_COUNT / 2)+0.5;
const uint8_t NUM_ACCs = adrss_per_acc * RAIL_COUNT;

ZZA_T acc_articles[NUM_ACCs];

#ifdef USE_DCC
  const uint16_t FIRST_DCC_ADDR = base_address;      // First used DCC accessory message
  const uint16_t LAST_DCC_ADDR  = FIRST_DCC_ADDR + (MSG_COUNT * 2) - 1;     // Last    "         "
#endif
void setup_zza()
{
  // CALC NEEDED ADRESSES:
  //const uint8_t tot_adrss     = RAIL_COUNT * adrss_per_acc;
  #ifdef DEBUG
    Serial.print("Number of Rails: ");
    Serial.println(RAIL_COUNT);
    Serial.print("Adresses per Rail: ");
    Serial.println(adrss_per_acc);
    #endif
  uint8_t tmp_adrs = base_address;
  uint8_t acc_num = 0;
  for ( uint8_t rails = 0; rails < RAIL_COUNT; rails++)//i < ADRS_SPACE; i++ )
  {
    uint8_t tmp_msg = 0;
    for ( uint8_t adrs = 0; adrs < adrss_per_acc; adrs++)
    {
      #ifdef USE_MACAN
        acc_articles[acc_num].locID = mcan.generateLocId( prot, tmp_adrs );
      #endif
      #ifdef USE_DCC
        acc_articles[acc_num].address = tmp_adrs;
      #endif
      strcpy(acc_articles[acc_num].RailNr, rail_definition[rails].RailNr);
      #ifdef DEBUG
        Serial.print("Setup Adresse #: ");
        Serial.println(tmp_adrs);
      #endif
      acc_articles[acc_num].msg_RED = tmp_msg;
      #ifdef DEBUG
        Serial.print("GLEIS: ");
        Serial.print(rail_definition[rails].RailNr);
        Serial.print(" | ");
        Serial.print(tmp_msg);
        Serial.print(" | ROT  | ");
        Serial.print(Text_Messages[tmp_msg].zugnummer);
        Serial.print(" | ");
        Serial.print(Text_Messages[tmp_msg].ziel);
        Serial.print(" | ");
        Serial.print(Text_Messages[tmp_msg].zuglauf1);
        Serial.print(" | ");
        Serial.println(Text_Messages[tmp_msg].zuglauf2);
      #endif
      tmp_msg++;
      acc_articles[acc_num].msg_GREEN = tmp_msg;
      #ifdef DEBUG
        Serial.print("GLEIS: ");
        Serial.print(rail_definition[rails].RailNr);
        Serial.print(" | ");
        Serial.print(tmp_msg);
        Serial.print(" | GRÜN | ");
        Serial.print(Text_Messages[tmp_msg].zugnummer);
        Serial.print(" | ");
        Serial.print(Text_Messages[tmp_msg].ziel);
        Serial.print(" | ");
        Serial.print(Text_Messages[tmp_msg].zuglauf1);
        Serial.print(" | ");
        Serial.println(Text_Messages[tmp_msg].zuglauf2);
        Serial.println();
      #endif
      tmp_msg++;
      acc_num++;
      tmp_adrs = tmp_adrs +1;
    }
  }
}


 

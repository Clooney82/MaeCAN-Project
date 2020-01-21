#include <U8g2lib.h>
#include "99_4x6_t_german.h"   // Special fonts which have only the german "umlauts" instead of all
#include "99_5x8_t_german.h"   // extendet characters (>127). Use "4x6_tf.h" if you need all ANSI characters >127
#include "99_6x13B_t_german.h"
#include "99_tpss_t_german.h"

#include "10_Text_Messages.h"
#include "00_GLOBAL_CONFIG.h"
#include "02_DISPLAY.h"
#ifdef USE_MACAN
  #include "20_CAN.h"
#endif
#ifdef USE_DCC
  #include "21_DCC.h"
#endif

uint8_t tst_msg = 0;
unsigned long currentMillis  = 0;
unsigned long previousMillis_loop   = 0;
unsigned long previousMillis_force  = 0;
unsigned long previousMillis_clock  = 0;
unsigned long previousMillis_late   = 0;
unsigned long loop_interval  =    100;
unsigned long clock_interval =   6000;
unsigned long late_interval  =  60000;
unsigned long force_interval = 120000;

void zza_loop() {
  //================================================================================================
  // STELL SCHLEIFE
  //================================================================================================
  for (int i = 0; i < NUM_ACCs; i++) {
    if ( acc_articles[i].power_set == 1 )
    {
      if ( acc_articles[i].state_set == RED ) Change_Display_on_RailNr(acc_articles[i].RailNr, acc_articles[i].msg_RED);
      else Change_Display_on_RailNr(acc_articles[i].RailNr, acc_articles[i].msg_GREEN);
      acc_articles[i].state_is = acc_articles[i].state_set;
      acc_articles[i].power_set = 0;
    }
    /* OLD
    // if ( acc_articles[i].state_is != acc_articles[i].state_set ) 
    } if ( acc_articles[i].power_set == 1 || ( acc_articles[i].state_is != acc_articles[i].state_set ) )
    {

      if ( acc_articles[i].state_set == RED ) Change_Display_on_RailNr(acc_articles[i].RailNr, acc_articles[i].msg_RED);
      else Change_Display_on_RailNr(acc_articles[i].RailNr, acc_articles[i].msg_GREEN);
      acc_articles[i].state_is = acc_articles[i].state_set;
      acc_articles[i].power_is = acc_articles[i].power_set;
    }
    */
  }
  //================================================================================================
  // ENDE - STELL SCHLEIFE
  //================================================================================================
  // RANDOM VERSPÄTUNG
  
  if (currentMillis - previousMillis_late >= late_interval) {
    previousMillis_late = currentMillis;
    uint8_t be_late = random(LATE_COUNT);
    uint8_t who = random(OLED_COUNT);
    if ( oleds[who].Msg_No_Displayed >= 2 ) {
      if ( be_late > 0 ) {
        oleds[who].Late = Text_Late[be_late].verspaetung;
        oleds[who].UpdateDisplay = UPD_DISP_ROLL;
        oleds[who].oled->clearDisplay();
        for (uint8_t secOLED = 0; secOLED < OLED_COUNT; secOLED++) // Initialize the OLEDs
        {
          if ( ( secOLED != who ) &&( strcmp(oleds[secOLED].RailNr, oleds[who].RailNr) == 0 ) )
          {
            oleds[secOLED].oled->clearDisplay();
            oleds[secOLED].Late = Text_Late[be_late].verspaetung;
            oleds[secOLED].UpdateDisplay = UPD_DISP_ROLL;
          }
        }
      }
    }
  }

}

void setup() {
//  #ifdef DEBUG
    Serial.begin(SERIAL_BAUDRATE);
    Serial.print("OLED Anzahl: ");
    Serial.println(OLED_COUNT);
//  #endif
  Setup_LAUFTEXT();
  Setup_OLEDs();            // Calls u8g.begin() for all displays and displays the first screens
  #ifdef USE_MACAN
    setup_can();
  #endif
  #ifdef USE_DCC
    setup_dcc();
  #endif
  #ifdef DEBUG
    Serial.println("Setup done.");
  #endif

}

void loop() {
  currentMillis = millis();
  if (currentMillis - previousMillis_loop >= loop_interval) {
    #ifdef USE_DCC
      Dcc.process(); // You MUST call the NmraDcc.process() method frequently from the Arduino loop() function for correct library operation
    #endif
    for (uint8_t OLED_No = 0; OLED_No < OLED_COUNT; OLED_No++) // Initialize the OLEDs
    {
      if (oleds[OLED_No].UpdateDisplay != DONT_UPD_DISP)
      {
        Write_to_OLED(OLED_No,oleds[OLED_No].Msg_No_Displayed); // Update the display and set UpdateDisplay
        
        // re-Calculate position for rolling text
        oleds[OLED_No].subset++;
        if (oleds[OLED_No].subset > 3)
        {
          oleds[OLED_No].subset = 0;
          oleds[OLED_No].offset++;
        }
      } else { // refresh Displays every 2min
        if (currentMillis - previousMillis_force >= force_interval) 
        {
          oleds[OLED_No].UpdateDisplay = UPD_DISP_ONCE;           // Update the display on next loop
          //Write_to_OLED(OLED_No,oleds[OLED_No].Msg_No_Displayed); // Update the display and set UpdateDisplay
        }
      }
    }



    previousMillis_loop = currentMillis;
  }
  
  #if defined(RAND_CHANGE_MINTIME) && defined(RAND_CHANGE_MAXTIME)
    Change_Display_ramdomly();
  #endif

  #ifdef USE_MACAN
  if (config_poll) {
    fnc_config_poll();

  } else
  #endif
  {
    zza_loop();
  }

  #ifdef DYNAMIC_CLOCK
  if (currentMillis - previousMillis_clock >= clock_interval) {
    previousMillis_clock = currentMillis;
    change_clock();
  }
  #endif  
}

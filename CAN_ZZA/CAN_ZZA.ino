/******************************************************************************
 * Zugzielanzeiger
 */
#define VERSION "v0.5"
/*******************************************************************************
 * Config files:
 * ---------------
 * 00_GLOBAL_CONFIG.h   <= GLOBAL CONFIG FILE
 * 10_Text_Message.h    <= Message definition for OLEDs
 * 20_CAN.h             <= CAN configuration file
 * 23_wifi.h            <= WiFi configuration file
 ******************************************************************************
 * Version History:
 * 0.5:
 * * Implementation of HTTP Webserver for controling Displays
 * * -> see 25_http.h
 * * Bugfix (Telnet): Wrong display of ABSCHNITT & WAGENSTAND
 * 0.4:
 * * I2C OLED Display implemention
 * 0.3:
 * * telnet integartion:
 ******************************************************************************
 * INFORMATIONS:
 ******************************************************************************
 * the sketch receives the text to be displayed at Telnet port 23
 ******************************************************************************
 * USING WIFI CLOCK:
 * ----------------------------------------------------------------------------
 * if "-T xx:xx" received: 
 * xx:xx is converted to analog clock image and drawn at OLED display  
 * example:
 * C:\Program Files (x86)\Nmap>echo -T 13:48 | ncat 10.0.0.57 23
 * ----------------------------------------------------------------------------
 * if "-S" received, then clock counts up by 1 minute
 * example:
 * C:\Program Files (x86)\Nmap>echo -S | ncat 10.0.0.57 23
 * ----------------------------------------------------------------------------
 * if "-D <GLEIS>" received, then display will show "Zugdurchfahrt"
 * ANALOG_CLOCK will be displayed if enabled
 * example:
 * C:\Program Files (x86)\Nmap>echo -D 1 | ncat 10.0.0.57 23
 * ----------------------------------------------------------------------------
 * if "-C <GLEIS>" received, then display will be cleared
 * ANALOG_CLOCK will be displayed if enabled
 * example:
 * C:\Program Files (x86)\Nmap>echo -C 1 | ncat 10.0.0.57 23
 ******************************************************************************
 * Setting of Traindata via telnet
 * ----------------------------------------------------------------------------
 * -G <GLEIS> -U <ABFAHRTSZEIT> -N <ZUGNUMMER> -Z <ZUGZIEL> -1 <ZUGLAUF1> -2 <ZUGLAUF2> -W <WAGENSTAND> -A <HALTEPOSITION> -L <LAUFTEXT>
 * ----------------------------------------------------------------------------
 * Parameter definition:
 * ---------------------
 * -G <GLEIS>         -> rail of departure        -> max   3 digits
 * -U <ABFAHRTSZEIT>  -> time of departure        -> max   5 digits
 * -N <ZUGNUMMER>     -> train number             -> max   7 digits
 * -Z <ZUGZIEL>       -> train target             -> max  16 digits
 * -1 <ZUGLAUF1>      -> next stop (1)            -> max  20 digits
 * -2 <ZUGLAUF2>      -> next stop (2)            -> max  20 digits
 * -W <WAGENSTAND>    ->                          -> max   7 digits
 * -A <HALTEPOSITION> ->                          -> max   7 digits
 * -L <LAUFTEXT>      -> rolling text             -> max 100 digits
 * ----------------------------------------------------------------------------
 * German "Umlaute" need to be send as their representative eg:
 * Ü=Ue, ü=ue, Ä=Ae, ä=ae and so on.  
 * ----------------------------------------------------------------------------
 * example:
 * C:\Program Files (x86)\Nmap>echo "-G 1a -U 07:24 -N ICE 153 -Z Mainz Hbf -1 Schlier ueber -2  Karlsruhe nach -W ABCDEFG -A -222F-- -L +++ Vorsicht: STUMMI-ICE faehrt durch +++" | ncat 10.0.0.57 23
 ******************************************************************************
 * 0.2:
 * * bugfix: not all messages are always shown
 * 0.1 initial Verion:
 * * basic DCC and CAN functionality
 * * Message need to be define in 10_Text_Message.h
 * * ------------------------------------------------
 * * Adress Calculation Example:
 * * BASE_ADRESS = 200
 * * MSG_COUNT = 11
 * * RAILS = 2
 * * NO ADRESSES = 11 % 2 = 6
 * * ------------------------------------------------
 * * --------   ---   -----------   - - --- RED GREEN
 * * RAIL 1:
 * * ADRESS 1 = 200 ( BASE_ADRESS     - MSG  0,  1 )
 * * ADRESS 2 = 201 ( BASE_ADRESS + 1 - MSG  2,  3 )
 * * ADRESS 3 = 202 ( BASE_ADRESS + 2 - MSG  4,  5 )
 * * ADRESS 4 = 203 ( BASE_ADRESS + 3 - MSG  6,  7 )
 * * ADRESS 5 = 204 ( BASE_ADRESS + 4 - MSG  8,  9 )
 * * ADRESS 6 = 205 ( BASE_ADRESS + 5 - MSG 10, 11 )
 * * RAIL 2:
 * * ADRESS 1 = 206 ( BASE_ADRESS     - MSG  0,  1 )
 * * ADRESS 2 = 207 ( BASE_ADRESS + 1 - MSG  2,  3 )
 * * ADRESS 3 = 208 ( BASE_ADRESS + 2 - MSG  4,  5 )
 * * ADRESS 4 = 209 ( BASE_ADRESS + 3 - MSG  6,  7 )
 * * ADRESS 5 = 210 ( BASE_ADRESS + 4 - MSG  8,  9 )
 * * ADRESS 6 = 211 ( BASE_ADRESS + 5 - MSG 10, 11 )
 * * ...
 * * ------------------------------------------------
 ******************************************************************************
 * DO NOT CHANGE
 * >>>>>>>>>>>>
 ******************************************************************************/
/******************************************************************************
 * DEFINE FONTS
 ******************************************************************************/
#include <U8g2lib.h>
#include "99_4x6_t_german.h"   // Special fonts which have only the german "umlauts" instead of all
#include "99_5x8_t_german.h"   // extendet characters (>127). Use "4x6_tf.h" if you need all ANSI characters >127
#include "99_6x13B_t_german.h"
#include "99_tpss_t_german.h"

#include "00_GLOBAL_CONFIG.h"
#include "10_Text_Messages.h"
#include "02_DISPLAY.h"
#ifdef USE_MACAN
  #include "20_CAN.h"
#endif
#ifdef USE_DCC
  #include "21_DCC.h"
#endif
#ifdef USE_WIFI
  #include "23_wifi.h"
#endif
#ifdef USE_TELNET
  #include "24_telnet.h"
#endif
#ifdef USE_HTTP
  #include "25_http.h"
#endif

uint8_t tst_msg = 0;
unsigned long currentMillis  = 0;
unsigned long previousMillis_loop   = 0;
unsigned long previousMillis_force  = 0;
unsigned long previousMillis_clock  = 0;
unsigned long previousMillis_late   = 0;
unsigned long loop_interval  =    100;
unsigned long clock_interval =  20000;
unsigned long late_interval  =  60000;
unsigned long force_interval = 120000;

void zza_loop() {
  #ifndef USE_WIFI
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
  }
  //================================================================================================
  // ENDE - STELL SCHLEIFE
  //================================================================================================
  #endif
  // RANDOM VERSPÄTUNG
  #ifdef RANDOM_LATE
  if (currentMillis - previousMillis_late >= late_interval) {
    previousMillis_late = currentMillis;
    //uint8_t be_late = random(LATE_COUNT);
    uint8_t who = random(OLED_COUNT);
    if ( oleds[who].Msg_No_Displayed >= 2 ) {
      uint8_t be_late = oleds[who].Late + 1;
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
  #endif
}

void setup() {
  #ifdef DEBUG
    delay(500);
    Serial.begin(SERIAL_BAUDRATE);
    delay(500);
    Serial.println();

    Serial.print("Running Sketch: ");
    Serial.println(F(VERSION "  " __DATE__ "  " __TIME__));
    Serial.println();

    // SHOW BOARD INFORMATION
    #if (defined(__MK20DX256__) || defined(__MK64FX512__)|| defined(__MK66FX1M0__))
      Serial.println("TEENSY 3.x HARDWARE");
    #endif
    #if defined(__AVR_ATmega328P__)
      Serial.println("Arduino UNO, NANO");
    #endif
    #if defined(__AVR_ATmega2560__)
      Serial.println("Arduino MEGA");
    #endif
    #ifdef ARDUINO_ESP8266_WEMOS_D1MINI
      Serial.println("ARDUINO_ESP8266_WEMOS_D1MINI");
    #endif
    #ifdef ARDUINO_LOLIN_D32
      Serial.println("ARDUINO_LOLIN_D32");
    #endif
    #ifdef ARDUINO_LOLIN_D32_PRO
      Serial.println("ARDUINO_LOLIN_D32_PRO");
    #endif

    Serial.print("OLED Anzahl: ");
    Serial.println(OLED_COUNT);

  #endif
  Setup_LAUFTEXT();
  Setup_OLEDs();            // Calls u8g.begin() for all displays and displays the first screens
  for (uint8_t OLED_No = 0; OLED_No < OLED_COUNT; OLED_No++) // Initialize the OLEDs
  { oleds[OLED_No].oled->clearBuffer();
    oleds[OLED_No].oled->setFont(FONT_5x8);
    oleds[OLED_No].oled->drawStr(1, 15, "Zugzielanzeiger");
    oleds[OLED_No].oled->drawStr(1, 25, VERSION);
    oleds[OLED_No].oled->sendBuffer();
  }
  delay(1000);

  #ifdef USE_MACAN
    setup_can();
  #endif
  #ifdef USE_DCC
    setup_dcc();
  #endif
  #ifdef USE_WIFI
    setup_wifi();
  #endif
  #ifdef USE_TELNET
    setup_telnet();
  #endif
  #ifdef USE_HTTP
    setup_http();
  #endif
  #ifdef DEBUG
    Serial.println("Setup done.");
    for (uint8_t OLED_No = 0; OLED_No < OLED_COUNT; OLED_No++) // Initialize the OLEDs
    { oleds[OLED_No].oled->clearBuffer();
      oleds[OLED_No].oled->setFont(FONT_5x8);
      //oleds[OLED_No].oled->drawStr(1, 15, "Setup done.");
      oleds[OLED_No].oled->drawStr(1, 25, "Setup done.");
      oleds[OLED_No].oled->sendBuffer();
    }
    delay(1000);
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
    #ifdef USE_TELNET
      telnet_loop();
    #endif
    #ifdef USE_HTTP
      http_loop();
    #endif
  }

  #ifdef DYNAMIC_CLOCK
  if (currentMillis - previousMillis_clock >= clock_interval) {
    previousMillis_clock = currentMillis;
    change_clock();
  }
  #endif  
}

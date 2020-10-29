/******************************************************************************
 * INFORMATIONS:
 ******************************************************************************
 * the sketch receives the text to be displayed at Telnet port 23
 ******************************************************************************
 * USING WIFI CLOCK:
 * ----------------------------------------------------------------------------
 * if "-R" received, then it will reset OLED displays
 * example:
 * Windows:     C:\Program Files (x86)\Nmap> echo -R | ncat 10.0.0.57 23
 * Linux/MacOS: echo -R | netcat 10.0.0.57 23
 * ----------------------------------------------------------------------------
 * if "-T xx:xx" received: 
 * xx:xx is converted to analog clock image and drawn at OLED display  
 * example:
 * Windows:     C:\Program Files (x86)\Nmap> echo -T 13:48 | ncat 10.0.0.57 23
 * Linux/MacOS: echo -T 13:48 | netcat 10.0.0.57 23
 * ----------------------------------------------------------------------------
 * if "-S" received, then clock counts up by 1 minute
 * example:
 * Windows:     C:\Program Files (x86)\Nmap> echo -S | ncat 10.0.0.57 23
 * Linux/MacOS: echo -S | netcat 10.0.0.57 23
 * ----------------------------------------------------------------------------
 * if "-D <GLEIS>" received, then display will show "Zugdurchfahrt"
 * ANALOG_CLOCK will be displayed if enabled
 * example:
 * Windows:     C:\Program Files (x86)\Nmap> echo -D 1 | ncat 10.0.0.57 23
 * Linux/MacOS: echo -D 1 | nnetcat 10.0.0.57 23
 * ----------------------------------------------------------------------------
 * if "-C <GLEIS>" received, then display will be cleared
 * ANALOG_CLOCK will be displayed if enabled
 * example:
 * Windows:     C:\Program Files (x86)\Nmap> echo -C 1 | ncat 10.0.0.57 23
 * Linux/MacOS: echo -C 1 | netcat 10.0.0.57 23
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
 * Windows:     C:\Program Files (x86)\Nmap> echo "-G 1a -U 07:24 -N ICE 153 -Z Mainz Hbf -1 Schlier ueber -2  Karlsruhe nach -W ABCDEFG -A -222F-- -L +++ Vorsicht: STUMMI-ICE faehrt durch +++" | ncat 10.0.0.57 23
 * Linux/MacOS: echo "-G 1a -U 07:24 -N ICE 153 -Z Mainz Hbf -1 Schlier ueber -2  Karlsruhe nach -W ABCDEFG -A -222F-- -L +++ Vorsicht: STUMMI-ICE faehrt durch +++" | netcat 10.0.0.57 23
 ******************************************************************************/
/******************************************************************************
 * DO NOT CHANGE
 * >>>>>>>>>>>>
 ******************************************************************************/
void setup_telnet() {
  // init Telent Server
  server.begin();
  delay(100);


}

void processCommand(String cmdString) {
  // parse the incoming text and split the payload in hour and minute
  #ifdef USE_WIFI_CLOCK
    String tmp_string;
    if (cmdString.charAt(1) == 'T') {
      int iDDot = cmdString.indexOf(":");
      hh = cmdString.substring(3, iDDot).toInt();
      mm = cmdString.substring(iDDot + 1).toInt();
      
      if (hh < 10) tmp_string = "0";
      tmp_string = tmp_string + hh + ":";
      if ( mm < 10) tmp_string = tmp_string + "0";
      tmp_string = tmp_string + mm;
      tmp_string.toCharArray(uhrzeit, 6);
      #ifdef DEBUG
        Serial.print("New Time recieved: ");
        Serial.print(uhrzeit);
        Serial.print(" | ");
        Serial.print(hh);
        Serial.print(":");
        Serial.println(mm);
      #endif
      for (uint8_t OLED_No = 0; OLED_No < OLED_COUNT; OLED_No++) // Initialize the OLEDs
      { if ( oleds[OLED_No].UpdateDisplay == DONT_UPD_DISP ) oleds[OLED_No].UpdateDisplay = UPD_DISP_ONCE;
      }

    } else if (cmdString.charAt(1) == 'S') {
      mm = mm + 1;
      if (mm == 60) {
        mm = 0;
        hh++;
      }
      if (hh == 24) hh = 0;
      
      if (hh < 10) tmp_string = "0";
      tmp_string = tmp_string + hh + ":";
      if ( mm < 10) tmp_string = tmp_string + "0";
      tmp_string = tmp_string + mm;
    
      tmp_string.toCharArray(uhrzeit, 6);
      #ifdef DEBUG
        Serial.print("New Time recieved: ");
        Serial.print(uhrzeit);
        Serial.print(" | ");
        Serial.print(hh);
        Serial.print(":");
        Serial.println(mm);
      #endif
      for (uint8_t OLED_No = 0; OLED_No < OLED_COUNT; OLED_No++) // Initialize the OLEDs
      { if ( oleds[OLED_No].UpdateDisplay == DONT_UPD_DISP ) oleds[OLED_No].UpdateDisplay = UPD_DISP_ONCE;
      }
    } else 
  #endif
  if (cmdString.charAt(1) == 'C') {
    String RAIL           = cmdString.substring(2);
    char RailNo[4];
    RAIL.trim();
    RAIL.toCharArray(RailNo, 4);
    Change_Display_on_RailNr(RailNo, 0);

  } else if (cmdString.charAt(1) == 'D') {
    String RAIL           = cmdString.substring(2);
    char RailNo[4];
    RAIL.trim();
    RAIL.toCharArray(RailNo, 4);
    Change_Display_on_RailNr(RailNo, 1);
    
    } else if (cmdString.charAt(1) == 'R') {
      Setup_OLEDs();
      for (uint8_t OLED_No = 0; OLED_No < OLED_COUNT; OLED_No++) // Initialize the OLEDs
      {
        load_Display_defaults(OLED_No,oleds[OLED_No].Msg_No_Displayed);
      }

  } else {
    // since german umlauts are always nasty in different OS the sketch expect to receive their representive eg. "ae" instead of "ä"
    // here we convert it to octal code for proper display    
    cmdString.replace("ae", "\344"); // replace ä
    cmdString.replace("oe", "\366"); // replace ö
    cmdString.replace("ue", "\374"); // replace ü
    cmdString.replace("Ae", "\304"); // replace Ä
    cmdString.replace("Oe", "\326"); // replace Ö
    cmdString.replace("Ue", "\334"); // repalce Ü
    cmdString.replace("s?", "\337"); // replace ß
  
    // parse the incoming text and split the payload in lines 
    uint8_t pos_G  = cmdString.indexOf("-G");
    uint8_t pos_U  = cmdString.indexOf("-U");
    uint8_t pos_N  = cmdString.indexOf("-N");
    uint8_t pos_Z  = cmdString.indexOf("-Z");
    uint8_t pos_Z1 = cmdString.indexOf("-1");
    uint8_t pos_Z2 = cmdString.indexOf("-2");
    uint8_t pos_W  = cmdString.indexOf("-W");
    uint8_t pos_A  = cmdString.indexOf("-A");
    uint8_t pos_L  = cmdString.indexOf("-L");
  
    String RAIL           = cmdString.substring(pos_G  + 3, pos_U);
    String DEPARTURE_TIME = cmdString.substring(pos_U  + 3, pos_N);
    String TRAIN_NUMBER   = cmdString.substring(pos_N  + 3, pos_Z);
    String DESTINATION    = cmdString.substring(pos_Z  + 3, pos_Z1);
    String DESTINATION1   = cmdString.substring(pos_Z1 + 3, pos_Z2);
    String DESTINATION2   = cmdString.substring(pos_Z2 + 3, pos_W);
    String WAGONPOSITION  = cmdString.substring(pos_W  + 3, pos_A);
    String HALTEPOSITION  = cmdString.substring(pos_A  + 3, pos_L);
    String ROLLINGTEXT    = cmdString.substring(pos_L  + 3);

    // Remove emty characters
    RAIL.trim();
    DEPARTURE_TIME.trim();
    TRAIN_NUMBER.trim();
    DESTINATION.trim();
    DESTINATION1.trim();
    DESTINATION2.trim();
    //WAGONPOSITION.trim();
    //HALTEPOSITION.trim();
    ROLLINGTEXT.trim();

  
    // convert the string to array of char which is expected by the u8g2 library
    char RailNo[4];
    RAIL.toCharArray(RailNo, 4);
  
    for (int i = 0; i < RAIL_COUNT; i++) {
      if ( strcmp(rail_definition[i].RailNr, RailNo) == 0 )
      {
        #ifdef DYNAMIC_DEPARTURE_TIME
          change_departure_time();
          new_dep_time.toCharArray(Text_Messages[i+2].uhrzeit, 6);
        #else
          DEPARTURE_TIME.toCharArray(Text_Messages[i+2].uhrzeit, 6);
        #endif
        TRAIN_NUMBER.toCharArray(Text_Messages[i+2].zugnummer, 8);
        DESTINATION.toCharArray(Text_Messages[i+2].ziel, 17);
        DESTINATION1.toCharArray(Text_Messages[i+2].zuglauf1, 21);
        DESTINATION2.toCharArray(Text_Messages[i+2].zuglauf2, 21);
        HALTEPOSITION.toCharArray(Text_Messages[i+2].abschnitt, 8);
        WAGONPOSITION.toCharArray(Text_Messages[i+2].wagenstand, 8);
        ROLLINGTEXT.toCharArray(Text_Messages[i+2].lauftext, 100);
  
        Change_Display_on_RailNr(RailNo, i+2);
        
      }
    }
  }
}


void telnet_loop()
{
  WiFiClient client = server.available();
  client.setTimeout(100);
  if (client)
  {
    while (client.connected())
    {
      if (client.available())
      {
        String commandStr = client.readStringUntil('\r');  // Read char until linefeed
        //String commandStr = client.readStringUntil('\n');  // Read char until linefeed  

        #ifdef DEBUG
          Serial.println("recieved command:");
          Serial.println(commandStr);
        #endif
        client.print("recieved command: ");
        client.println(commandStr);
        processCommand(commandStr);                        // Process the received command
        delay(1); 
        client.stop();
      }
    }
  }

}

#ifdef  ESP32
  #include <WebServer.h>
  WebServer web_server(80);
#else
  #include <ESP8266WebServer.h>
  ESP8266WebServer web_server(80);
#endif

String displaycurrentZZA() {
  String tmphtml = String("  \r\n  <hr>\r\n  Bahnsteiganzeigen LIVEVIEW:<br>\r\n  <br>\r\n");
  for (int i = 0; i < RAIL_COUNT; i++) {
    String tlauftext = String(Text_Messages[i+2].lauftext);
    tlauftext.trim();
    String bg;
    if (tlauftext != "")  bg = String("class=\"invert\"");
    tmphtml = tmphtml + String("  <table class=\"DISPLAY\">\r\n") +
           "    <tr>\r\n" +
           "      <td align=\"CENTER\"                width=\" 88px\" id=\"R" + String(i) + "_TIME\">"     + String(Text_Messages[i+2].uhrzeit) + "</td>\r\n" +
           "      <td align=\"LEFT\"  valign=\"CENTER\" width=\"368px\" " + bg + "><marquee id=\"R" + String(i) + "_LAUFTEXT\" class=\"lauftext\">" + tlauftext  + "</marquee></td>\r\n" +
           "      <td align=\"RIGHT\" valign=\"TOP\"    width=\" 44px\" id=\"R" + String(i) + "_GLEIS\"  rowspan=\"4\" class=\"big\">" + String(rail_definition[i].RailNr) + "</td>\r\n" +
           "    </tr>\r\n" +
           "    <tr>\r\n" +
           "      <td align=\"CENTER\" id=\"R" + String(i) + "_ZUG\">"      + String(Text_Messages[i+2].zugnummer)  + "</td>\r\n" +
           "      <td align=\"LEFT\"   id=\"R" + String(i) + "_ZUGLAUF1\">" + String(Text_Messages[i+2].zuglauf1)  + "</td>\r\n" +
           "    </tr>\r\n" +
           "    <tr>\r\n" +
           "      <td align=\"CENTER\" id=\"R" + String(i) + "_ABSCHNITT\">" + String(Text_Messages[i+2].abschnitt)  + "</td>\r\n" +
           "      <td align=\"LEFT\" id=\"R" + String(i) + "_ZUGLAUF2\">"  + String(Text_Messages[i+2].zuglauf2)  + "</td>\r\n" +
           "    </tr>\r\n" +
           "    <tr>\r\n";
    if (String(Text_Messages[i+2].wagenstand) != "") {
      bg = String("class=\"invert\"");
    } else {
      bg = "";
    }
    tmphtml = tmphtml + String("      <td align=\"CENTER\" id=\"R" + String(i) + "_WAGENSTAND\" " + bg + ">" + String(Text_Messages[i+2].wagenstand)  + "</td>\r\n") +
           "      <td align=\"LEFT\" id=\"R" + String(i) + "_ZUGZIEL\">"    + String(Text_Messages[i+2].ziel)  + "</td>\r\n" +
           "    </tr>\r\n" +
           "  </table>\r\n";
    if ( RAIL_COUNT > 1 && i < RAIL_COUNT) {
      tmphtml = tmphtml + String("  <hr>\r\n");
    }
    tmphtml = tmphtml + String("  <br>\r\n");
  }
  return tmphtml;

}

String prepareHtmlPage(bool refresh=false) {
  String htmlPage =
    String("\r\n") +
           "<!DOCTYPE HTML>\r\n" +
           "<html lang=\"de-DE\"\r\n" +
           "<head>\r\n" +
           "  <title>Bahnsteiganzeige " + String(VERSION) + "</title>\r\n";
  if (refresh) htmlPage = htmlPage + String("  <meta http-equiv=\"refresh\" content=\"60\">\r\n");
  htmlPage = htmlPage + String("  <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\r\n") +
           "  <style>\r\n" +
           "    body {\r\n" +
           "      background-color: darkgrey;\r\n" +
           "    }\r\n" +
           "    input {\r\n" +
           "      font-family: \"Courier New\", Courier, monospace;\r\n" +
           "    }\r\n" +
           "    select {\r\n" +
           "      font-family: \"Courier New\", Courier, monospace;\r\n" +
           "      white-space: PRE;\r\n" +
           "    }\r\n" +
           "    div.divbody {\r\n" +
           "      margin:0 auto;\r\n" +
           "      width: 500px;\r\n" +
           "      align: center;\r\n" +
           "    }\r\n" +
           "    table.DISPLAY {\r\n" +
           "      background-color: darkblue;\r\n" +
           "      color: white;\r\n" +
           "      white-space: PRE;\r\n" +
           "      font-family: \"Courier New\", Courier, monospace;\r\n" +
           "      border: 3px solid black;\r\n" +
           "      font-size: 16px;\r\n" +
           "      height: 96px\r\n" +
           "    }\r\n" +
           "    td.big {\r\n" +
           "      font-size: 24px;\r\n" +
           "    }\r\n" +
           "    td.invert {\r\n" +
           "      background-color: white;\r\n" +
           "      color: darkblue;\r\n" +
           "    }\r\n" +
           "    marquee.lauftext {\r\n" +
           "      margin: 0 auto;\r\n" +
           "      height: 13px;\r\n" +
           "      font-size: 16px;\r\n" +
           "      border: 0;\r\n" +
           "    }\r\n" +
           "  </style>\r\n" +
           "</head>\r\n" +
           "<body>\r\n" +
           "  <div class=\"divbody\">\r\n" +
           "  <font face=\"Arial\">\r\n" +
           "  <h1 align=\"center\">Bahnsteiganzeige " + String(VERSION) + "</h1>\r\n" +
           "\r\n";
  return htmlPage;
}

const String endHtmlPage = R"=====(
  </div>
</body>
</html>
)=====";

const String htmlInput1 = R"=====(
  <hr>Vorschau:
  <table class="DISPLAY">
    <tr>
      <td align="CENTER"                width=" 88px" id="TIME">XX:xx</td>
      <td align="LEFT"  valign="CENTER" width="368px" style="background-color: white; color: darkblue"><marquee class="lauftext" id="LAUFTEXT" ></marquee></td>
      <td align="RIGHT" valign="TOP"    width=" 44px" id="GLEIS" rowspan="4" class="big">X</td>
    </tr>
    <tr>
     <td align="CENTER" id="ZUG">XXXYYY</td>
      <td align="LEFT"   id="ZUGLAUF1">ABCDEF - GHIFJKLM</td>
    </tr>
   <tr>
     <td align="CENTER" id="ABSCHNITT">ABCDEFG</td>
      <td align="LEFT"   id="ZUGLAUF2">OPQRSTUVWXYZ</td>
    </tr>
    <tr>
      <td align="CENTER" id="WAGENSTAND" class="invert">-22211-</td>
      <td align="LEFT" id="ZUGZIEL">ZIEL</td>
    </tr>
  </table>
  <hr>
  Eingabe:
  <form method="POST">
  <table class="DISPLAY">
    <tr>
      <td align="CENTER"                width=" 88px"><input type="time" id="DEPARTURE_TIME" name="DEPARTURE_TIME" value="15:21"><label for="DELAYED_TIME"> +<input id="DELAYED_TIME" name="DELAYED_TIME" type="number" min="0" max="240" step="5" value="0">Min</td>
      <td align="LEFT"  valign="CENTER" width="368px"><input type="text" size="20" maxlength="100" id="ROLLINGTEXT" name="ROLLINGTEXT" value="Lauftext"></td>
      <td align="RIGHT" valign="TOP"    width=" 44px"><select id="RAIL" name="RAIL">
)=====";


const String htmlInput2 = R"=====(
        </select></td>
    </tr>
    <tr>
      <td align="CENTER"><input type="text" size="7" maxlength="7" id="TRAIN" name="TRAIN" value="ICE 592"></td>
      <td align="LEFT"><input type="text" size="20" maxlength="20" id="TARGET1" name="TARGET1" value="Fulda - Kassel -"></td>
    </tr>
    <tr>
      <td align="CENTER"><select id="SECTION_A" name="SECTION_A">
        <option value="A">A</option>
        <option value=" "> </option>
      </select><select id="SECTION_B" name="SECTION_B">
        <option value="B">B</option>
        <option value=" "> </option>
        <option value="A">A</option>
      </select><select id="SECTION_C" name="SECTION_C">
        <option value="C">C</option>
        <option value=" "> </option>
        <option value="A">A</option>
        <option value="B">B</option>
      </select><select id="SECTION_D" name="SECTION_D">
        <option value="D">D</option>
        <option value=" "> </option>
        <option value="A">A</option>
        <option value="B">B</option>
        <option value="C">C</option>
      </select><select id="SECTION_E" name="SECTION_E">
        <option value="E">E</option>
        <option value=" "> </option>
        <option value="A">A</option>
        <option value="B">B</option>
        <option value="C">C</option>
        <option value="D">D</option>
      </select><select id="SECTION_F" name="SECTION_F">
        <option value="F">F</option>
        <option value=" "> </option>
        <option value="A">A</option>
        <option value="B">B</option>
        <option value="C">C</option>
        <option value="D">D</option>
        <option value="E">E</option>
      </select><select id="SECTION_G" name="SECTION_G">
        <option value="G">G</option>
        <option value=" "> </option>
        <option value="A">A</option>
        <option value="B">B</option>
        <option value="C">C</option>
        <option value="D">D</option>
        <option value="E">E</option>
        <option value="F">F</option>
      </select></td>
      <td align="LEFT"><input type="text" size="20" maxlength="20" id="TARGET2" name="TARGET2" value="Braunschweig Hbf"></td>
    </tr>
    <tr>
      <td align="CENTER" class="invert"><select id="WAGON_A" name="WAGON_A">
        <option value="1">1</option>
        <option value=" "> </option>
        <option value="-">-</option>
        <option value="2">2</option>
      </select><select id="WAGON_B" name="WAGON_B">
        <option value="1">1</option>
        <option value=" "> </option>
        <option value="-">-</option>
        <option value="2">2</option>
      </select><select id="WAGON_C" name="WAGON_C">
        <option value="1">1</option>
        <option value=" "> </option>
        <option value="-">-</option>
        <option value="2">2</option>
      </select><select id="WAGON_D" name="WAGON_D">
        <option value="2">2</option>
        <option value=" "> </option>
        <option value="-">-</option>
        <option value="1">1</option>
      </select><select id="WAGON_E" name="WAGON_E">
        <option value="2">2</option>
        <option value=" "> </option>
        <option value="-">-</option>
        <option value="1">1</option>
      </select><select id="WAGON_F" name="WAGON_F">
        <option value="2">2</option>
        <option value=" "> </option>
        <option value="-">-</option>
        <option value="1">1</option>
      </select><select id="WAGON_G" name="WAGON_G">
        <option value=" "> </option>
        <option value="-">-</option>
        <option value="1">1</option>
        <option value="2">2</option>
      </select></td>
      <td align="LEFT"><input type="text" size="20" maxlength="16" id="TARGET" name="TARGET" value="Berlin Ostbf"></td>
    </tr>
  </table>
  <input type="reset" value="Reset"> <input type="submit" name="submit" value="Submit"> <input type="submit" name="submit" value="Clear"> <input type="submit" name="submit" value="Zugdurchfahrt"></form>
  <hr>
  <button onclick="myFunction()">Vorschau</button>
)=====";

String htmlInput() {
  String tmphtml;// = htmlInput1;
  for (int i = 0; i < RAIL_COUNT; i++) {
    tmphtml = tmphtml + "          <option value=\"" + String(rail_definition[i].RailNr) + "\">" + String(rail_definition[i].RailNr) + "</option>\r\n";
  }
  //tmphtml = tmphtml + htmlInput2;
  return tmphtml;
}

const String htmlInputScript = R"=====(
  <script>
  function myFunction() {
    document.getElementById("TIME").innerHTML       = document.forms[0].elements[0].value;
    if (document.forms[0].elements[1].value != "0") {
      document.getElementById("LAUFTEXT").innerHTML = "--- Verspätung ca. "+ document.forms[0].elements[1].value + " Minuten --- " + document.forms[0].elements[2].value;
    } else {
      document.getElementById("LAUFTEXT").innerHTML = document.forms[0].elements[2].value;
    }
    document.getElementById("GLEIS").innerHTML      = document.forms[0].elements[3].value;
    document.getElementById("ZUG").innerHTML        = document.forms[0].elements[4].value;
    document.getElementById("ZUGLAUF1").innerHTML   = document.forms[0].elements[5].value;
    document.getElementById("ABSCHNITT").innerHTML  = document.forms[0].elements[6].value
                                                     +document.forms[0].elements[7].value
                                                     +document.forms[0].elements[8].value
                                                     +document.forms[0].elements[9].value
                                                     +document.forms[0].elements[10].value
                                                     +document.forms[0].elements[11].value
                                                     +document.forms[0].elements[12].value;
    document.getElementById("ZUGLAUF2").innerHTML   = document.forms[0].elements[13].value;
    document.getElementById("WAGENSTAND").innerHTML = document.forms[0].elements[14].value
                                                     +document.forms[0].elements[15].value
                                                     +document.forms[0].elements[16].value
                                                     +document.forms[0].elements[17].value
                                                     +document.forms[0].elements[18].value
                                                     +document.forms[0].elements[19].value
                                                     +document.forms[0].elements[20].value;
    document.getElementById("ZUGZIEL").innerHTML    = document.forms[0].elements[21].value;
  }
  </script>
)=====";


void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += web_server.uri();
  message += "\nMethod: ";
  message += (web_server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += web_server.args();
  message += "\n";
  for (uint8_t i = 0; i < web_server.args(); i++) {
    message += " " + web_server.argName(i) + ": " + web_server.arg(i) + "\n";
  }
  web_server.send(404, "text/plain", message);
}

void handleClose() {
  String message =
    String("<!DOCTYPE HTML>\r\n") +
           "<html lang=\"de-DE\"\r\n" +
           "<head>\r\n" +
           "  <title>Bahnsteiganzeige " + String(VERSION) + "</title>\r\n" +
           "  <meta http-equiv=\"refresh\" content=\"5; URL=http://" + WiFi.localIP().toString() + "\">\r\n" +
           "  <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\r\n" +
           "</head>\r\n" +
           "<body>\r\n" +
           "Page Not Found\r\n" +
           "</body>\r\n" +
           "</html>\r\n";
  web_server.send(404, "text/html",  message);
}

void handleRoot() {
  if (web_server.method() == HTTP_POST)  {
    String RAIL = web_server.arg(3);
    char RailNo[4];
    RAIL.toCharArray(RailNo, 4);
    /*
    for (int i = 0; i < RAIL_COUNT; i++) {
      if ( strcmp(rail_definition[i].RailNr, RailNo) == 0 )
      {
        Text_Messages[i+2].uhrzeit[0] = '\0';
        Text_Messages[i+2].zugnummer[0] = '\0';
        Text_Messages[i+2].ziel[0] = '\0';
        Text_Messages[i+2].zuglauf1[0] = '\0';
        Text_Messages[i+2].zuglauf2[0] = '\0';
        Text_Messages[i+2].abschnitt[0] = '\0';
        Text_Messages[i+2].wagenstand[0] = '\0';
        Text_Messages[i+2].lauftext[0] = '\0';
      }
    }
    */
    
    if ( web_server.arg(22) == "Clear" ) {
      for (int i = 0; i < RAIL_COUNT; i++) {
        if ( strcmp(rail_definition[i].RailNr, RailNo) == 0 )
        {
          Text_Messages[i+2] = Text_Messages[0];
          strcpy(Text_Messages[i+2].uhrzeit,uhrzeit);
        }
      }
      Change_Display_on_RailNr(RailNo, 0);
    } else if ( web_server.arg(22) == "Zugdurchfahrt" ) {
      for (int i = 0; i < RAIL_COUNT; i++) {
        if ( strcmp(rail_definition[i].RailNr, RailNo) == 0 )
        {
          Text_Messages[i+2] = Text_Messages[1];
          strcpy(Text_Messages[i+2].uhrzeit,uhrzeit);
        }
      }
      Change_Display_on_RailNr(RailNo, 1);
    } else if ( web_server.arg(22) == "Submit" ) {
      // Read POST Data and send to OLED
      // then proceed with loading Webpage
      /*
      String message = "POST form was:\n";
      for (uint8_t i = 0; i < web_server.args(); i++) {
        message += " " + web_server.argName(i) + ": " + web_server.arg(i) + "\n";
      }
      web_server.send(200, "text/plain", message);
      */
      String DEPARTURE_TIME;
      String DELAYED;
      String TRAIN_NUMBER;
      String DESTINATION;
      String DESTINATION1;
      String DESTINATION2;
      String H_ABSCHNITT;
      String H_WAGENSTAND;
      String ROLLINGTEXT;
  
      DEPARTURE_TIME = web_server.arg(0);
      DELAYED        = web_server.arg(1);
      ROLLINGTEXT    = web_server.arg(2);
      if ( DELAYED != "0" ) ROLLINGTEXT = "--- Verspätung ca. " + DELAYED + " Minuten  --- " + ROLLINGTEXT;
      //RAIL           = web_server.arg(3);
      TRAIN_NUMBER   = web_server.arg(4);
      DESTINATION1   = web_server.arg(5);
      //SECTION_A      = web_server.arg(6);
      if ( web_server.arg(6) == "" ) {
        H_ABSCHNITT = " ";
      } else {
        H_ABSCHNITT = web_server.arg(6);
      }
      //SECTION_B      = web_server.arg(7);
      if ( web_server.arg(7) == "" ) {
        H_ABSCHNITT = H_ABSCHNITT + " ";
      } else {
        H_ABSCHNITT = H_ABSCHNITT + web_server.arg(7);
      }
      //SECTION_C      = web_server.arg(8);
      if ( web_server.arg(8) == "" ) {
        H_ABSCHNITT = H_ABSCHNITT + " ";
      } else {
        H_ABSCHNITT = H_ABSCHNITT + web_server.arg(8);
      }
      //SECTION_D      = web_server.arg(9);
      if ( web_server.arg(9) == "" ) {
        H_ABSCHNITT = H_ABSCHNITT + " ";
      } else {
        H_ABSCHNITT = H_ABSCHNITT + web_server.arg(9);
      }
      //SECTION_E      = web_server.arg(10);
      if ( web_server.arg(10) == "" ) {
        H_ABSCHNITT = H_ABSCHNITT + " ";
      } else {
        H_ABSCHNITT = H_ABSCHNITT + web_server.arg(10);
      }
      //SECTION_F      = web_server.arg(11);
      if ( web_server.arg(11) == "" ) {
        H_ABSCHNITT = H_ABSCHNITT + " ";
      } else {
        H_ABSCHNITT = H_ABSCHNITT + web_server.arg(11);
      }
      //SECTION_G      = web_server.arg(12);
      if ( web_server.arg(12) == "" ) {
        H_ABSCHNITT = H_ABSCHNITT + " ";
      } else {
        H_ABSCHNITT = H_ABSCHNITT + web_server.arg(12);
      }
      DESTINATION2   = web_server.arg(13);
      //WAGON_A        = web_server.arg(14);
      if ( web_server.arg(14) == "" ) {
        H_WAGENSTAND = H_WAGENSTAND + " ";
      } else {
        H_WAGENSTAND = H_WAGENSTAND + web_server.arg(14);
      }
      //WAGON_B        = web_server.arg(15);
      if ( web_server.arg(15) == "" ) {
        H_WAGENSTAND = H_WAGENSTAND + " ";
      } else {
        H_WAGENSTAND = H_WAGENSTAND + web_server.arg(15);
      }
      //WAGON_C        = web_server.arg(16);
      if ( web_server.arg(16) == "" ) {
        H_WAGENSTAND = H_WAGENSTAND + " ";
      } else {
        H_WAGENSTAND = H_WAGENSTAND + web_server.arg(16);
      }
      //WAGON_D        = web_server.arg(17);
      if ( web_server.arg(17) == "" ) {
        H_WAGENSTAND = H_WAGENSTAND + " ";
      } else {
        H_WAGENSTAND = H_WAGENSTAND + web_server.arg(17);
      }
      //WAGON_E        = web_server.arg(18);
      if ( web_server.arg(18) == "" ) {
        H_WAGENSTAND = H_WAGENSTAND + " ";
      } else {
        H_WAGENSTAND = H_WAGENSTAND + web_server.arg(18);
      }
      //WAGON_F        = web_server.arg(19);
      if ( web_server.arg(19) == "" ) {
        H_WAGENSTAND = H_WAGENSTAND + " ";
      } else {
        H_WAGENSTAND = H_WAGENSTAND + web_server.arg(19);
      }
      //WAGON_G        = web_server.arg(20);
      if ( web_server.arg(20) == "" ) {
        H_WAGENSTAND = H_WAGENSTAND + " ";
      } else {
        H_WAGENSTAND = H_WAGENSTAND + web_server.arg(20);
      }
      DESTINATION    = web_server.arg(21);
      // convert the string to array of char which is expected by the u8g2 library
  
      for (int i = 0; i < RAIL_COUNT; i++) {
        if ( strcmp(rail_definition[i].RailNr, RailNo) == 0 )
        {
          DEPARTURE_TIME.toCharArray(Text_Messages[i+2].uhrzeit, 6);
          TRAIN_NUMBER.toCharArray(Text_Messages[i+2].zugnummer, 8);
          DESTINATION.toCharArray(Text_Messages[i+2].ziel, 17);
          DESTINATION1.toCharArray(Text_Messages[i+2].zuglauf1, 21);
          DESTINATION2.toCharArray(Text_Messages[i+2].zuglauf2, 21);
          H_ABSCHNITT.toCharArray(Text_Messages[i+2].abschnitt, 8);
          H_WAGENSTAND.toCharArray(Text_Messages[i+2].wagenstand, 8);
          ROLLINGTEXT.toCharArray(Text_Messages[i+2].lauftext, 100);
  
          Change_Display_on_RailNr(RailNo, i+2);
  
        }
      }
      //web_server.send(200, "text/html", prepareHtmlPage(false) + htmlInput() + htmlInputScript + endHtmlPage);
      //web_server.send(200, "text/html", prepareHtmlPage(false) + htmlInput() + displaycurrentZZA() + htmlInputScript + endHtmlPage);
    }
  }
  web_server.sendContent(prepareHtmlPage(false));
  web_server.sendContent(htmlInput1);
  web_server.sendContent(htmlInput());
  web_server.sendContent(htmlInput2);
  web_server.sendContent(displaycurrentZZA());
  web_server.sendContent(htmlInputScript);
  web_server.sendContent(endHtmlPage);
}


void handleZZA() {
  web_server.send(200, "text/html", prepareHtmlPage(true) + displaycurrentZZA() + endHtmlPage);
}












void handlePlain() {
  if (web_server.method() != HTTP_POST) {
    web_server.send(405, "text/plain", "Method Not Allowed");
  } else {
    web_server.send(200, "text/plain", "POST body was:\n" + web_server.arg("plain"));
  }
}

void handleForm() {
  if (web_server.method() != HTTP_POST) {
    web_server.send(405, "text/plain", "Method Not Allowed");
  } else {
    String message = "POST form was:\n";
    for (uint8_t i = 0; i < web_server.args(); i++) {
      message += " " + web_server.argName(i) + ": " + web_server.arg(i) + "\n";
    }
    web_server.send(200, "text/plain", message);
  }
}






void handle_setup_wifi() {
  String htmlPage =
    String("\r\n") +
           "<!DOCTYPE HTML>\r\n" +
           "<html lang=\"de-DE\"\r\n" +
           "<head>\r\n" +
           "  <title>Bahnsteiganzeige " + String(VERSION) + "</title>\r\n" +
           "  <meta http-equiv=\"refresh\" content=\"5; URL=http://" + WiFi.localIP().toString() + "\">\r\n" +
           "  <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\r\n" +
           "</head>\r\n" +
           "<body>\r\n" +
           "\r\n" +
           "</body>\r\n" +
           "</html>\r\n";

  web_server.send(200, "text/html", htmlPage);
  delay(500);
  web_server.stop();
  delay(1000);
  #ifdef DEBUG_HTTP
    Serial.println("##### START CONFIG #####");
  #endif

  setup_wifi2();
  #ifdef DEBUG_HTTP
    Serial.println("##### CONFIG FINISHED #####");
  #endif
  htmlPage =
    String("\r\n") +
           "<!DOCTYPE HTML>\r\n" +
           "<html lang=\"de-DE\"\r\n" +
           "<head>\r\n" +
           "  <title>Bahnsteiganzeige " + String(VERSION) + "</title>\r\n" +
           "  <meta http-equiv=\"refresh\" content=\"5; URL=http://" + WiFi.localIP().toString() + "\">\r\n" +
           "  <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\r\n" +
           "</head>\r\n" +
           "<body>\r\n" +
           "\r\n" +
           "</body>\r\n" +
           "</html>\r\n";

  web_server.send(200, "text/html", htmlPage);
  web_server.on("/", handleRoot);
  web_server.on("/ZZA",  handleZZA);
  web_server.on("/ZZA/", handleZZA);
  web_server.on("/config",  handle_setup_wifi);
  web_server.on("/close",  handleClose);
  web_server.onNotFound(handleNotFound);

  web_server.begin();
  #ifdef DEBUG_HTTP
    Serial.println("HTTP web_server restarted");
  #endif


  //ESP.restart();

}





void setup_http() {
  web_server.on("/", handleRoot);

  web_server.on("/ZZA",  handleZZA);
  web_server.on("/ZZA/", handleZZA);

  /*
  web_server.on("/postplain/", handlePlain);

  web_server.on("/postform/", handleForm);
  */
  web_server.on("/config",  handle_setup_wifi);
  web_server.on("/close",  handleClose);

  web_server.onNotFound(handleNotFound);

  web_server.begin();
  #ifdef DEBUG_HTTP
    Serial.println("HTTP web_server started");
  #endif

}

void http_loop(void) {
  web_server.handleClient();
}

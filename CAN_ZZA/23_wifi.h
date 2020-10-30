/******************************************************************************
 * Wireless Network setup:
 ******************************************************************************/
/*
    const char* KNOWN_SSID[] = {"SSID0", "SSID1", "SSID2", "SSID3"};
    const char* KNOWN_PASSWORD[] = {"Password0", "Password1", "Password2", "Password3"};
*/
// DEFINE HERE THE KNOWN NETWORKS
const char* KNOWN_SSID[] = {"SSID0", "SSID1", "SSID2", "SSID3"};
const char* KNOWN_PASSWORD[] = {"Password0", "Password1", "Password2", "Password3"};

/******************************************************************************
 * DO NOT CHANGE
 * >>>>>>>>>>>>
 ******************************************************************************/
// ESP Wifi:
//#include <ESP8266WiFi.h>
#ifdef ESP32
//  #include <esp_wifi.h>
  #include <WiFi.h>
  //#include <WiFiClient.h>
  //#define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())
#else
  #include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#endif
// Wifi Shield:
// #include <WiFi.h>
#ifdef USE_TELNET
  WiFiServer telnet_server(23);
#endif

const int   KNOWN_SSID_COUNT = sizeof(KNOWN_SSID) / sizeof(KNOWN_SSID[0]); // number of known networks

void setup_wifi() {
  boolean wifiFound = false;
  int i, n;

  // ----------------------------------------------------------------
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  // ----------------------------------------------------------------
  //WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // ----------------------------------------------------------------
  // WiFi.scanNetworks will return the number of networks found
  // ----------------------------------------------------------------
  #ifdef DEBUG_WIFI
    Serial.println(F("scan start"));
  #endif
  int nbVisibleNetworks = WiFi.scanNetworks();
  #ifdef DEBUG_WIFI
    Serial.println(F("scan done"));
  #endif
  if (nbVisibleNetworks == 0) {
    #ifdef DEBUG_WIFI
      Serial.println(F("no networks found. Reset to try again"));
    #endif
    while (true); // no need to go further, hang in there, will auto launch the Soft WDT reset
  }

  // ----------------------------------------------------------------
  // if you arrive here at least some networks are visible
  // ----------------------------------------------------------------
  #ifdef DEBUG_WIFI
    Serial.print(nbVisibleNetworks);
    Serial.println(" network(s) found");
  #endif
  // ----------------------------------------------------------------
  // check if we recognize one by comparing the visible networks
  // one by one with our list of known networks
  // ----------------------------------------------------------------
  for (i = 0; i < nbVisibleNetworks; ++i) {
    #ifdef DEBUG_WIFI
      Serial.println(WiFi.SSID(i)); // Print current SSID
    #endif
    for (n = 0; n < KNOWN_SSID_COUNT; n++) { // walk through the list of known SSID and check for a match
// ESP Wifi:
      if (strcmp(KNOWN_SSID[n], WiFi.SSID(i).c_str())) {
// Wifi Shield:
//      if (strcmp(KNOWN_SSID[n], WiFi.SSID(i))) {
        #ifdef DEBUG_WIFI
          Serial.print(F("\tNot matching "));
          Serial.println(KNOWN_SSID[n]);
        #endif
      } else { // we got a match
        wifiFound = true;
        break; // n is the network index we found
      }
    } // end for each known wifi SSID
    if (wifiFound) break; // break from the "for each visible network" loop
  } // end for each visible network

  if (!wifiFound) {
    #ifdef DEBUG_WIFI
      Serial.println(F("no known network identified. Reset to try again"));
    #endif
    for (uint8_t OLED_No = 0; OLED_No < OLED_COUNT; OLED_No++) // Initialize the OLEDs
    { oleds[OLED_No].oled->clearBuffer();
      oleds[OLED_No].oled->setFont(FONT_5x8);
      oleds[OLED_No].oled->drawStr(1, 15, "no known network found.");
      oleds[OLED_No].oled->drawStr(1, 25, "Reset to try again!");
      oleds[OLED_No].oled->sendBuffer();
    }
    delay(5000);
    while (true); // no need to go further, hang in there, will auto launch the Soft WDT reset
  }

  // ----------------------------------------------------------------
  // if you arrive here you found 1 known SSID
  // ----------------------------------------------------------------
  #ifdef DEBUG_WIFI
    Serial.print(F("\nConnecting to "));
    Serial.println(KNOWN_SSID[n]);
  #endif
  for (uint8_t OLED_No = 0; OLED_No < OLED_COUNT; OLED_No++) // Initialize the OLEDs
  { oleds[OLED_No].oled->clearBuffer();
    oleds[OLED_No].oled->setFont(FONT_5x8);
    oleds[OLED_No].oled->drawStr(1, 15, "Connecting to");
    oleds[OLED_No].oled->drawStr(1, 25, KNOWN_SSID[n]);
    oleds[OLED_No].oled->sendBuffer();
  }
  // ----------------------------------------------------------------
  // We try to connect to the WiFi network we found
  // ----------------------------------------------------------------
  WiFi.begin(KNOWN_SSID[n], KNOWN_PASSWORD[n]);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #ifdef DEBUG_WIFI
      Serial.print(".");
    #endif
  }
  #ifdef DEBUG_WIFI
    Serial.println("");

    // ----------------------------------------------------------------
    // SUCCESS, you are connected to the known WiFi network
    // ----------------------------------------------------------------
    Serial.println(F("WiFi connected, your IP address is "));
    Serial.println(WiFi.localIP());
  #endif
  String tmp_strng = WiFi.localIP().toString();
  char tmp_char[15];
  tmp_strng.toCharArray(tmp_char, 15);
  for (uint8_t OLED_No = 0; OLED_No < OLED_COUNT; OLED_No++) // Initialize the OLEDs
  { oleds[OLED_No].oled->clearBuffer();
    oleds[OLED_No].oled->setFont(FONT_5x8);
    oleds[OLED_No].oled->drawStr(1, 15, "My IP is");
    oleds[OLED_No].oled->drawStr(1, 25, tmp_char);
    oleds[OLED_No].oled->sendBuffer();
  }

  #ifdef DEBUG_WIFI
    Serial.println("WiFi Setup done");
  #endif

  delay(5000);
}

void wifi_loop() {
  if (WiFi.status() != WL_CONNECTED) setup_wifi();
}

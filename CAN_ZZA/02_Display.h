/******************************************************************************
 * DO NOT CHANGE
 * >>>>>>>>>>>>
 ******************************************************************************/
typedef struct
{
  uint8_t ColorIndex    : 1; // 1= Normal, 0 = Invers
  uint8_t Rightaligned  : 1; // Text is right alligned if gleisSeite = Rechts
  uint8_t DynInvLen     : 1; // Invers box has only the length of the text
  uint8_t NoFontDescent : 1; // Pixel below baseline (in 'g') not shown in invers mode. Text shifted down
  uint8_t DelMark       : 1; // Delete this entry with the 'X' command
  uint8_t DispIfPrivious: 1; // Display only if the privious line is also shown
} Flags_T;

typedef struct
{
  uint8_t        xR, xL, y; // x position if gleisSeite is R or L, y position
  uint8_t        PixWidth;  // output length in pixels
  uint8_t        RolTextLen;// Visible characters of the rolling text
  Flags_T        Flags;     // single bits (see above)
  const uint8_t *font;      // Pointer to the used font
//  uint8_t        size;      // Length of the Txt
} Disp_T;


// Numbers of the Disp[] entries
#define LAUFTEXT     0
#define GLEIS        1
#define UHRZEIT      2
#define ZUGNUMMER    3
#define ZIEL         4
#define ZUGLAUF1     5
#define ZUGLAUF2     6
#define WAGENSTAND   7
#define ABSCHNITT    8
#define ANALOG_CLOCK 9

// Changed layout ("ziel" is left alligned if "GleisSeite_Rechts" is active. Letters like 'g' are shown with descent pixel)
const  Disp_T Disp[] =
{ //       xR,  xL,  y, Pix   RolT. Color Right DynInv    NoFont  Del  DispIf            font   Activ Define variable length
  //                    Width  Len  Index alig.    Len   Descent Mark  Privi.                    Char
/* 0 */  { 30,  20,  5,   78,  20,  {  0,    0,     0,        0,   0,      0 },      FONT_4x6},//  'L',  DEFVAR(lauftext  ) }, // lauftext  Should be the first entry because it overwrites the areas on the left and right side with Black pixels
/* 1 */  {127,   0, 13,   18,   0,  {  1,    1,     0,        0,   0,      0 },    FONT_6x13B},//  'G',  DEFVAR(gleis     ) }, // gleis
/* 2 */  {  1, 101,  6,   27,   0,  {  1,    0,     0,        0,   1,      0 },      FONT_5x8},//  'U',  DEFVAR(uhrzeit   ) }, // uhrzeit
/* 3 */  {  0, 100, 13,   28,   0,  {  1,    0,     0,        0,   1,      0 },      FONT_4x6},//  'N',  DEFVAR(zugnummer ) }, // zugnummer
/* 4 */  { 30,   0, 28,   98,   0,  {  1,    0,     0,        0,   1,      0 }, FONT_PS_11X17},//  'Z',  DEFVAR(ziel      ) }, // ziel      proportional font 11x17 ? https://github.com/olikraus/u8glib/wiki/fontgrouporgdot
/* 5 */  { 30,  20, 11,   78,   0,  {  1,    0,     0,        0,   1,      0 },      FONT_4x6},//  '1',  DEFVAR(zuglauf1  ) }, // zuglauf1
/* 6 */  { 30,  20, 17,   78,   0,  {  1,    0,     0,        0,   1,      0 },      FONT_4x6},//  '2',  DEFVAR(zuglauf2  ) }, // zuglauf2
/* 7 */  {  0, 100, 27,   28,   0,  {  0,    0,     1,        0,   1,      0 },      FONT_4x6},//  'W',  DEFVAR(wagenstand) }, // wagenstand
/* 8 */  {  0, 100, 21,   28,   0,  {  1,    0,     0,        0,   0,      1 },      FONT_4x6},//  'A',  DEFVAR(abschnitt ) }, // haltepositionen ABCDEFG
/* 9 */  { 16, 112, 16,   32,   0,  {  1,    0,     0,        0,   0,      1 },      FONT_4x6},//  'C',  DEFVAR(analog_clock) }, // analog clock
};

char    uhrzeit[6];
uint8_t hh = 8;
uint8_t mm = 0;

#ifdef DYNAMIC_CLOCK
uint8_t clock_step = 1;

//----------------------
void change_clock()
//----------------------
{
  String tmp_string;
  
  mm = mm + clock_step;
  if (mm >= 60) 
  {
    mm = 0;
    hh++;
  }
  if (hh == 24) hh = 0;
  
  if (hh < 10) tmp_string = "0";
  tmp_string = tmp_string + hh + ":";
  if ( mm < 10) tmp_string = tmp_string + "0";
  tmp_string = tmp_string + mm;

  tmp_string.toCharArray(uhrzeit, 6);

  for (uint8_t OLED_No = 0; OLED_No < OLED_COUNT; OLED_No++) // Initialize the OLEDs
  {
    if ( oleds[OLED_No].UpdateDisplay == DONT_UPD_DISP ) oleds[OLED_No].UpdateDisplay = UPD_DISP_ONCE;
  }
  //return uhrzeit;
}
#endif

#ifdef DYNAMIC_DEPARTURE_TIME
String new_dep_time;
void change_departure_time()
{
  String tmp_string;

  mm = mm + add_dep_time;
  if (mm >= 60) 
  { 
    //mm = 0;
    mm = mm - 60;
    hh++;
  }
  if (hh == 24) hh = 0;
  
  if (hh < 10) tmp_string = "0";
  tmp_string = tmp_string + hh + ":";
  if ( mm < 10) tmp_string = tmp_string + "0";
  tmp_string = tmp_string + mm;

  //tmp_string.toCharArray(new_dep_time, 6);
  new_dep_time = tmp_string;
  //return new_dep_time;
    
}
#endif

// draw analog clock
//void Draw_analog_clock(uint8_t OLED_No, uint8_t Hour, uint8_t Minute) {
void Draw_analog_clock(uint8_t OLED_No, char l_uhrzeit[6]) {
  String tmpstrng;
  tmpstrng = l_uhrzeit[0];
  tmpstrng += l_uhrzeit[1];
  tmpstrng += l_uhrzeit[2];
  tmpstrng += l_uhrzeit[3];
  tmpstrng += l_uhrzeit[4];
  uint8_t Hour = tmpstrng.substring(0,2).toInt();;
  uint8_t Minute = tmpstrng.substring(3,5).toInt();;
  uint8_t clockX;
  if ( oleds[OLED_No].RailSide == GleisSeite_Rechts ) {
    clockX = Disp[ANALOG_CLOCK].xR;
  } else {
    clockX = Disp[ANALOG_CLOCK].xL;
  }

  uint8_t clockY = Disp[ANALOG_CLOCK].y;

  // now send the stuff to the display
  oleds[OLED_No].oled->drawCircle(clockX, clockY, 15);
  //hour ticks
  for ( int z = 0; z < 360; z = z + 30 ) {
    //Begin at 0° and stop at 360°
    float angle = z ;
    angle = (angle / 57.29577951) ; //Convert degrees to radians
    int x2 = (clockX + (sin(angle) * 15));
    int y2 = (clockY - (cos(angle) * 15));
    int x3 = (clockX + (sin(angle) * (15 - 4)));
    int y3 = (clockY - (cos(angle) * (15 - 4)));
    oleds[OLED_No].oled->drawLine(x2, y2, x3, y3);
  }

  // display minute hand
  float angle = Minute * 6 ;
  angle = (angle / 57.29577951) ; //Convert degrees to radians
  int x3 = (clockX + (sin(angle) * (15 - 3)));
  int y3 = (clockY - (cos(angle) * (15 - 3)));
  oleds[OLED_No].oled->drawLine(clockX, clockY, x3, y3);

  // display hour hand
  angle = Hour * 30 + int((Minute / 12) * 6 )   ;
  angle = (angle / 57.29577951) ; //Convert degrees to radians
  x3 = (clockX + (sin(angle) * (20 - 11)));
  y3 = (clockY - (cos(angle) * (20 - 11)));
  oleds[OLED_No].oled->drawLine(clockX, clockY, x3, y3);

}

#ifdef USE_I2C
//------------------------------
void Enable_OLED_Pin(uint8_t Nr)
//------------------------------
// Activates the enable pin for one OLED.
// The enable pin drives a FET (BS170) which
// switches the SDA line to the OLED display.
{
  for (uint8_t i = 0; i < OLED_COUNT; i++)
    digitalWrite(oleds[i].OLED_Enable_Pin, 0);
  digitalWrite(oleds[Nr].OLED_Enable_Pin, 1);
  //Active_OLED = Nr;
}
#endif

//----------------------
void Draw_Element(uint8_t OLED_No, uint8_t Txt_Type, char *Txt)
//----------------------
{
  oleds[OLED_No].oled->setFont(Disp[Txt_Type].font);

  uint8_t x;
  if ( oleds[OLED_No].RailSide == GleisSeite_Rechts ) {
    x = Disp[Txt_Type].xR;
    if ( Disp[Txt_Type].Flags.Rightaligned ) x -= oleds[OLED_No].oled->getStrWidth(Txt);
  } else {
    x = Disp[Txt_Type].xL;
  }

  uint8_t y = Disp[Txt_Type].y;

  if ( Disp[Txt_Type].Flags.ColorIndex == 0 ) {       // Invers => draw a box 
    oleds[OLED_No].oled->setColorIndex(1);

    if ( *Txt != '\0' && ( Disp[Txt_Type].RolTextLen == 0 || oleds[OLED_No].UpdateDisplay != UPD_DISP_STOP ) ) {  // Don't draw box if text is empty or RolTextLen enabled and UPD_DISP_STOP
      uint8_t PixWidth;
      if ( Disp[Txt_Type].Flags.DynInvLen )
          PixWidth = oleds[OLED_No].oled->getStrWidth(Txt);
      else PixWidth = Disp[Txt_Type].PixWidth;
      oleds[OLED_No].oled->drawBox(x, y - oleds[OLED_No].oled->getFontAscent(), PixWidth, oleds[OLED_No].oled->getFontAscent() + oleds[OLED_No].oled->getFontDescent() + 2);
    }

    if ( Disp[Txt_Type].Flags.NoFontDescent )
        y += (oleds[OLED_No].oled->getFontDescent() + 2);
  }
  oleds[OLED_No].oled->setColorIndex(Disp[Txt_Type].Flags.ColorIndex);

  if ( Disp[Txt_Type].RolTextLen && *Txt && oleds[OLED_No].UpdateDisplay != UPD_DISP_STOP ) { // rolling text used ?
    uint8_t Len = strlen(Txt);
    if ( oleds[OLED_No].offset > Len ) 
        oleds[OLED_No].offset = 0;
    int remaining = Len - oleds[OLED_No].offset;
    if ( remaining > Disp[Txt_Type].RolTextLen )
        remaining = Disp[Txt_Type].RolTextLen;
    if ( oleds[OLED_No].offset < Len ) {
      char Old;
      uint8_t End = oleds[OLED_No].offset + remaining;
      if ( End < Len ) {
        Old = Txt[End];
        Txt[End] = '\0';
      }
      oleds[OLED_No].oled->drawStr(x - oleds[OLED_No].subset, y, Txt + oleds[OLED_No].offset);
      if (End < Len) 
          Txt[End] = Old;
    }
  }
  else oleds[OLED_No].oled->drawStr(x, y, Txt); // Normal text

}


//----------------------
void Draw_All_Elements(uint8_t OLED_No, uint8_t Msg_No)
//----------------------
{
  // LAUFTEXT     0
  if ( oleds[OLED_No].Late ) {
    for (uint8_t i = 0 ; i < LATE_COUNT; i++) {
      if ( Text_Late[i].verspaetung == oleds[OLED_No].Late ) {
        Draw_Element(OLED_No, LAUFTEXT, Text_Late[i].text);
      }
    }
  } else Draw_Element(OLED_No, LAUFTEXT, Text_Messages[Msg_No].lauftext);

  // GLEIS        1
  Draw_Element(OLED_No, GLEIS, oleds[OLED_No].RailNr);

  // UHRZEIT      2
  if ((Msg_No == 0)||(Msg_No == 1)) {    
    #if ( defined (USE_WIFI_CLOCK) || defined(DYNAMIC_CLOCK) ) &&  defined(USE_ANALOG_CLOCK)
      Draw_analog_clock(OLED_No, uhrzeit);
    #else
    #if ( defined (USE_WIFI_CLOCK) || defined(DYNAMIC_CLOCK) ) && !defined(USE_ANALOG_CLOCK)
      Draw_Element(OLED_No, UHRZEIT, uhrzeit);
    #else
    #if ( defined (USE_WIFI_CLOCK) || !defined(DYNAMIC_CLOCK) ) &&  defined(USE_ANALOG_CLOCK)
      Draw_analog_clock(OLED_No, Text_Messages[Msg_No].uhrzeit);
    #else 
      Draw_Element(OLED_No, UHRZEIT, Text_Messages[Msg_No].uhrzeit);
    #endif
    #endif
    #endif
  } else {
    /*
    #if defined(DYNAMIC_CLOCK)
      Draw_Element(OLED_No, UHRZEIT, uhrzeit);
    #else 
    */
      Draw_Element(OLED_No, UHRZEIT, Text_Messages[Msg_No].uhrzeit);
    //#endif
  }
  
  // ZUGNUMMER    3
  Draw_Element(OLED_No, ZUGNUMMER, Text_Messages[Msg_No].zugnummer);
  
  // ZIEL         4
  Draw_Element(OLED_No, ZIEL, Text_Messages[Msg_No].ziel);
    
  // ZUGLAUF1     5
  Draw_Element(OLED_No, ZUGLAUF1, Text_Messages[Msg_No].zuglauf1);
  
  // ZUGLAUF2     6
  Draw_Element(OLED_No, ZUGLAUF2, Text_Messages[Msg_No].zuglauf2);
  
  // WAGENSTAND   7
  Draw_Element(OLED_No, WAGENSTAND, Text_Messages[Msg_No].wagenstand);
  
  // ABSCHNITT    8
  if ( strlen(Text_Messages[Msg_No].wagenstand) > 0 ) {
    Draw_Element(OLED_No, ABSCHNITT, Text_Messages[Msg_No].abschnitt);
  }


  
  if ( oleds[OLED_No].UpdateDisplay == UPD_DISP_ONCE ) oleds[OLED_No].UpdateDisplay = DONT_UPD_DISP;

}


//--------------------------
void Write_to_OLED(uint8_t OLED_No, uint8_t Msg_No = 0)
//--------------------------
{ 
  /* Page buffer mode (Picture Loop) */
  /*
  oleds[OLED_No].oled->firstPage();
  do {
    Draw_All_Elements(OLED_No, Msg_No);
  } while ( oleds[OLED_No].oled->nextPage() );
  */
  /* Full screen buffer mode */
  #ifdef USE_I2C
    Enable_OLED_Pin(OLED_No);
  #endif
  
  oleds[OLED_No].oled->clearBuffer();
  Draw_All_Elements(OLED_No, Msg_No);
  oleds[OLED_No].oled->sendBuffer();
  
  if ( strlen(Text_Messages[Msg_No].lauftext) > 0 ) {
    oleds[OLED_No].subset++;
    if (oleds[OLED_No].subset > 3)
    {
      oleds[OLED_No].subset = 0;
      oleds[OLED_No].offset++;
    }
  }

}


//--------------------------
void load_Display_defaults(uint8_t OLED_No, uint8_t Msg_No = 0)
//--------------------------
{
  oleds[OLED_No].offset = oleds[OLED_No].subset = 0; // new start of the rolling text
  if ( strlen(Text_Messages[Msg_No].lauftext) > 0 ) oleds[OLED_No].UpdateDisplay = UPD_DISP_ROLL;
  else oleds[OLED_No].UpdateDisplay = UPD_DISP_ONCE;
  oleds[OLED_No].Msg_No_Displayed = Msg_No;
  oleds[OLED_No].Late = 0;
  #ifdef DEBUG
  Serial.print("aktiviere OLED No.: ");
  Serial.print(OLED_No);
  Serial.print(" Gleis: ");
  Serial.print(oleds[OLED_No].RailNr);
  Serial.print(" Anzeige Nr.: ");
  Serial.println(oleds[OLED_No].Msg_No_Displayed);
  // -G <GLEIS> -U <ABFAHRTSZEIT> -N <ZUGNUMMER> -Z <ZUGZIEL> -1 <ZUGLAUF1> -2 <ZUGLAUF2> -W <WAGENSTAND> -A <HALTEPOSITION> -L <LAUFTEXT>
  Serial.print("-G: ");
  Serial.print(oleds[OLED_No].RailNr);
  Serial.print(" -U: ");
  Serial.print(Text_Messages[Msg_No].uhrzeit);
  Serial.print(" -N: ");
  Serial.print(Text_Messages[Msg_No].zugnummer);
  Serial.print(" -Z: ");
  Serial.print(Text_Messages[Msg_No].ziel);
  Serial.print(" -1: ");
  Serial.print(Text_Messages[Msg_No].zuglauf1);
  Serial.print(" -2: ");
  Serial.print(Text_Messages[Msg_No].zuglauf2);
  Serial.print(" -W: ");
  Serial.print(Text_Messages[Msg_No].wagenstand);
  Serial.print(" -A: ");
  Serial.print(Text_Messages[Msg_No].abschnitt);
  Serial.print(" -L: ");
  Serial.println(Text_Messages[Msg_No].lauftext);
  delay(20);
  #endif
  

}


//--------------------------
void Change_Display_on_RailNr(char RailNo[4], uint8_t Msg_No = random(MSG_COUNT))
//--------------------------
{
  for (uint8_t OLED_No = 0; OLED_No < OLED_COUNT; OLED_No++)
  {
    if ( strcmp(oleds[OLED_No].RailNr, RailNo) == 0 )
    {
      load_Display_defaults(OLED_No, Msg_No);
      #ifdef DEBUG
        Serial.print("Ändere OLED No.: ");
        Serial.print(OLED_No);
        Serial.print(" Gleis: ");
        Serial.print(oleds[OLED_No].RailNr);
        Serial.print(" Anzeige Nr.: ");
        Serial.println(oleds[OLED_No].Msg_No_Displayed);
      #endif

    }
  }
  
}


#if defined(RAND_CHANGE_MINTIME) && defined(RAND_CHANGE_MAXTIME)
//----------------------------
void Change_Display_ramdomly(uint8_t OLED_No = random(OLED_COUNT) , uint8_t Msg_No = random(MSG_COUNT))
//----------------------------
{
  static uint32_t Next_Rand_Change = 0;
  if (Next_Rand_Change == 0) randomSeed(analogRead(UNUSED_AIN_PIN));
  if (Next_Rand_Change <= 1) Next_Rand_Change = millis() + random(RAND_CHANGE_MINTIME * 1000L, RAND_CHANGE_MAXTIME * 1000L);

  if (millis() >= Next_Rand_Change)
  {
    Next_Rand_Change = 1;
    load_Display_defaults(OLED_No, Msg_No);
    for (uint8_t secOLED = 0; secOLED < OLED_COUNT; secOLED++) // Initialize the OLEDs
    { 
      if ( ( secOLED != OLED_No ) &&( strcmp(oleds[secOLED].RailNr, oleds[OLED_No].RailNr) == 0 ) )
      {
        load_Display_defaults(secOLED, Msg_No);
      }
    }    
  }
  
}
#endif


//----------------
void Setup_LAUFTEXT()
//----------------
{
  // Text Messages
  for (uint8_t i = 0 ; i < MSG_COUNT; i++)
  {
    if (Text_Messages[i].lauftext)
    {
      uint8_t ActLen = strlen(Text_Messages[i].lauftext);
      if (Disp[0].RolTextLen > 0 && ActLen > 0)    // Special treatement for rolling text
      {
        memmove(Text_Messages[i].lauftext + Disp[0].RolTextLen, Text_Messages[i].lauftext, ActLen);
        memset(Text_Messages[i].lauftext, ' ', Disp[0].RolTextLen); // add leading space characters
      }
    }
  }

  // LATE MESSAGES
  for (uint8_t i = 0 ; i < LATE_COUNT; i++)
  {
    if (Text_Late[i].text)
    {
      uint8_t ActLen = strlen(Text_Late[i].text);
      if (Disp[0].RolTextLen > 0 && ActLen > 0)    // Special treatement for rolling text
      {
        memmove(Text_Late[i].text + Disp[0].RolTextLen, Text_Late[i].text, ActLen);
        memset(Text_Late[i].text, ' ', Disp[0].RolTextLen); // add leading space characters
      }
    }
  }

}


//----------------
void Setup_OLEDs()
//----------------
{ 
  delay(500);
  for (uint8_t OLED_No = 0; OLED_No < OLED_COUNT; OLED_No++) // Initialize the OLEDs
  {
    #ifdef DEBUG
      Serial.print("Setup OLED No.: ");
      Serial.println(OLED_No);
    #endif
    oleds[OLED_No].oled->begin();
    oleds[OLED_No].oled->setContrast(255);  // Kontrast Anpassen
    oleds[OLED_No].oled->setFontMode(1);    // Fonts in the new library draw also the background pixels => The invers background is visible on the left side
    oleds[OLED_No].oled->setBusClock(1000000);
    delay(100);
  }
  /*
  for (uint8_t OLED_No = 0; OLED_No < OLED_COUNT; OLED_No++) // Initialize the OLEDs
  {
    load_Display_defaults(OLED_No);
    //Write_Display_to_OLED(OLED_No);
  }
  */

}

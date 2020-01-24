// Abhaengig vom dem Zeichensatz des Editors werden die Deutschen Umlaute richtig dardestellt oder nicht ;-(
// Es ist es besser wenn hier immer die folgenden Codes anstelle der Umlaute verwendtet werden.
// Dann kommt jeder Editor damit zurecht.
//
// Umlaute ersetzten laut folgender Chiffre. Dazu koennen Hex Codes (0xE4) oder Octal Codes (\344) verwendet werden
// Die oktalen Codes enthalten immer 3 Ziffern. Dadurch koennen diese besser zusammen mit darauf folgenden
// Ziffern verwendet werden.
//   ae  = \344
//   oe  = \366
//   ue  = \374
//   ss  = \337
//   Ae  = \304
//   Oe  = \326
//   Ue  = \334
//   3/4 = \276
// Wuerzburg ist daher W\374rzburg
// Verspaetung ist daher Versp\344tung

//#define G934 "#G9\276|11:00" // Gleis '9 3/4' at '11:00'

// Attention: The sequence of the entries must match with the sequence in the array Dat[]
//            "lauftext" is an exception to this sequence. This is handled in function
//            Short_Mode_Next_ProcState()

typedef struct 
{
  char uhrzeit[6]   ;
  char zugnummer[8] ;
  char ziel[17]     ;
  char zuglauf1[21] ;
  char zuglauf2[21] ;
  char abschnitt[8] ;
  char wagenstand[8];
  char lauftext[100];
} MESSAGE_T;

 MESSAGE_T Text_Messages[] =
{
//         Uhrzeit  Zugnummer  Ziel                Zuglauf1                Zuglauf2                abschnitt  Wagenstand  lauftext  1         2         3         4         5         6         7         8         9        10
//         "12345", "1234567", "1234567890123456", "12345678901234567890", "12345678901234567890", "1234567", "1234567",  "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
/*  0 */ { "00:00", "",        "",                 "",                     "",                     "",        "",         "" },
/*  1 */ { "01:00", "",        " Zugdurchfahrt",   "",                     "",                     "",        "",         "+++ Vorsicht am Bahnsteig +++" },
#if !defined USE_DCC || !defined USE_MACAN
/*  2 */ { "02:00", "MPZ 01",  "Oberdorf",         "Unterberg",            "",                     "  CDE  ", "  22-  ",  "" },
/*  3 */ { "02:15", "MPZ 02",  "Oberdorf",         "Unterberg",            "",                     "   DE  ", "   22  ",  "" },
/*  4 */ { "02:30", "MPZ 03",  "Oberdorf",         "Unterberg",            "",                     "  CDE  ", "  22-  ",  "" },
/*  5 */ { "02:45", "MPZ 04",  "Stuttgart",        "Plochingen - ",        "Esslingen",            "  CDE  ", "  22-  ",  "+++ Sonderzug +++" },
/*  6 */ { "02:50", "MPZ 05",  "Ulm",              "Gleislinge a.d.S.",    "",                     "  CDE  ", "  22-  ",  "+++ Sonderzug +++" },
/*  7 */ { "03:00", "RB 01",   "Oberdorf",         "Unterberg",            "",                     "   DE  ", "   22  ",  "" },
/*  8 */ { "03:15", "VT95",    "Oberdorf",         "Unterberg",            "",                     "   DE  ", "   22  ",  "" },
/*  9 */ { "03:30", "RB95.9",  "Oberdorf",         "Unterberg",            "",                     "   D-  ", "   2-  ",  "" },
/* 10 */ { "04:00", "RE 1234", "Stuttgart",        "Plochingen - ",        "Esslingen",            " -CDEF ", " -2221 ",  "" },
/* 11 */ { "04:15", "RE 2345", "Ulm",              "Gleislinge a.d.S.",    "",                     " -CDEF ", " -2221 ",  "" },
#if (defined(__MK20DX256__) || defined(__MK64FX512__)|| defined(__MK66FX1M0__))
/* xx */ { "07:24", "ICE 153", "Mainz Hbf",        "Schlier \374ber",      " Karlsruhe nach",      "ABCDEFG", "-222F--",  "+++ Vorsicht: STUMMI-ICE f\344hrt durch +++            +++ Danke an Tobias, Klaus & Fredddy,... +++" },
/* xx */ { "09:34", "RB 1521", "Aschaffenburg",    "Gro\337auheim - Kahl", "- Gro\337krotzenburg", "ABCDEFG", "",         "" },
/* xx */ { "10:04", "RB 3237", "Plattling",        "Freising - Moosburg",  "- Landshut",           "ABCDEFG", "",         "" },
/* xx */ { "12:53", "EC 172",  "Hamburg - Altona", "Berlin Hbf - ",        "Hamburg Hbf",          "ABCDEFG", "-222211",  "Versp\344tung ca 10 Min" },
/* xx */ { "15:21", "ICE 592", "Berlin Ostbf",     "Fulda - Kassel -",     "Braunschweig Hbf",     "ABCDEFG", "11111  ",  "" },
/* xx */ { "17:02", "IC 602",  "Puttgarden",       "Wuppertal - Dortmund", "Bremen - Hamburg",     "ABCDEFG", " 22111 ",  "" },
/* xx */ { "18:30", "RE 7",    "Kiel / Flensburg", "Elmshorn -",           "Neum\374nster",        "ABCDEFG", "   2121",  "Zugteilung in Neum\374nster - Vorderer Zugteil f\344hrt bis Flensburg" },
/* xx */ { "21:45", "ICE 651", "Leipzig Hbf",      "Fulda - Eisenach",     "",                     "",        "",         "Achtung: Heute auf Gleis 7" },
#endif
///* xx */ { "", "", "", "", "", "ABCDEFG", "", "" },
///* xx */ { "", "", "", "", "", "ABCDEFG", "", "" },
///* xx */ { "", "", "", "", "", "ABCDEFG", "", "" },
///* xx */ { "", "", "", "", "", "ABCDEFG", "", "" },
#endif
#ifdef USE_WIFI
/* EL */ { "",     "",        "",                 "",                     "",                     "",        "",         "" }, // EMPTY ENTRY TO WIFI
/* EL */ { "",     "",        "",                 "",                     "",                     "",        "",         "" }, // EMPTY ENTRY TO WIFI
/* EL */ { "",     "",        "",                 "",                     "",                     "",        "",         "" }, // EMPTY ENTRY TO WIFI
/* EL */ { "",     "",        "",                 "",                     "",                     "",        "",         "" }, // EMPTY ENTRY TO WIFI
/* EL */ { "",     "",        "",                 "",                     "",                     "",        "",         "" }, // EMPTY ENTRY TO WIFI
/* EL */ { "",     "",        "",                 "",                     "",                     "",        "",         "" }, // EMPTY ENTRY TO WIFI
/* EL */ { "",     "",        "",                 "",                     "",                     "",        "",         "" }, // EMPTY ENTRY TO WIFI
/* EL */ { "",     "",        "",                 "",                     "",                     "",        "",         "" }, // EMPTY ENTRY TO WIFI
/* EL */ { "",     "",        "",                 "",                     "",                     "",        "",         "" }, // EMPTY ENTRY TO WIFI
#endif
};
 // End of the Text_Messages string

#define MSG_COUNT (sizeof(Text_Messages)/sizeof(MESSAGE_T))

typedef struct 
{
  uint8_t verspaetung   ;
  char text[100] ;
} LATE_T;

LATE_T Text_Late[] = 
{
    {   0, "" },
    {   5, "Versp\344tung ca 5 Min" },
    {  10, "Versp\344tung ca 10 Min" },
    {  15, "Versp\344tung ca 15 Min" },
    {  20, "Versp\344tung ca 20 Min" },
    {  30, "Versp\344tung ca 30 Min" },
    {  60, "Versp\344tung ca 60 Min" },
    { 120, "Versp\344tung ca 120 Min" },
    { 180, "Versp\344tung ca 180 Min" },
};

#define LATE_COUNT (sizeof(Text_Late)/sizeof(LATE_T))

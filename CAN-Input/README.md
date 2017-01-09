# MäCAN-CAN Input / Stellpult

Der nachfolgende Teil bezieht sich ausschließlich auf die Hardware-Revision A und Software-Version 0.3. Es ist zu beachten, welche Software-Version mit welcher Hardware-Revision kompatibel ist! Alle Hardware-Versionen der gleichen Revision sind zuheinander kompatibel und unterscheiden sich nicht in ihrer Funktionsweise.

Das Projekt ist noch im Anfangsstadium. Es ist also mit Veränderungen in relativ kurzen abständen zu rechnen und weder Hard- noch Software sind final! Hier kann sich jederzeit einiges verändern. Es wird versucht diese Dokumentation so aktuell wie möglich zu halten. Hierfür besteht aber keine Gewähr.

## Changelog

##### V0.3

 + new feature: Unterscheidung Weiche/Signal & Entkuppler

##### V0.2

  + erst Implementierung zur Unterscheidung Weiche/Signal & Entkuppler (nicht veröffentlicht)

##### V0.1

 + initiale Version.
 + pro Decoder können maximal 8 AddOn Platinen angeschlossen werden.
 + es können bis zu 64 Magnetartikel angesteuert werden (mit LED_FEEDBACK die Hälfte)
 + Konfiguration über CS2 (getestet) / CS3plus (ungetestet) möglich.
 + Protokoll ( MM oder DCC ) global für alle Magnetartikel die am Decoder angeschlossen sind identisch.

## Dokumentation

#### Inhalt

* [Bestückung der AddOn Platinen](#bestückung-der-addon-platinen)
* [Erstkonfiguration](#erstkonfiguration)
* [Screenshots](#screenshots)
* [Wichtige Hinweise](#wichtige-hinweise)

#### Bestückung der AddOn Platinen

![img_pcb](/CAN-Input/board/MäCAN-I-O-AddOn-PCB.png)

* Bestückung ohne LED_FEEDBACK
  * IC2 wird nicht bestückt.
  * Die LEDs werden mit dem Minuspol an IC3 (GPB0 - GPB7) angeschlossen (Stiftleiste über IC3)
  * Die Betriebsspannung kann über den Jumper unterhalb IC3 gewählt werden.
     * \+5V & Mitte: 5V Spannungsversorgung über den Spannungsregler des Decoders
     * oder Mitte & \+18V: Externe Spannungsversorgung (18V Gleispannung oder beliebig wählbar über externes Netzteil)
  * Die Taster werden an der Stiftleiste unterhalb von IC2 (GPA0 - GPA7) angeschlossen, ein seperater PullUp Widerstand wird nicht benötigt.

* Bestückung ohne LED_FEEDBACK
  * IC2 & IC3 werden nicht bestückt.
  * Die Taster werden an der Stiftleiste unterhalb von IC2 & IC3 angeschlossen, ein seperater PullUp Widerstand wird nicht benötigt.

* Die Hardware-Adresse der AddOn Platinen wird über die Jumper A0, A1 und A2 festgelegt. Der Status (GND = 0 / +5V = 1) der Hardware Address Pins ergeben folgende Adressen:

| A0 | A1 | A2 | Adresse | AddOn-Platine# |
| :---: | :---: | :---: | :---: | :---: |
|0|0|0|0x20|1|
|1|0|0|0x21|2|
|0|1|0|0x22|3|
|1|1|0|0x23|4|
|0|0|1|0x24|5|
|1|0|1|0x25|6|
|0|1|1|0x26|7|
|1|1|1|0x27|8|

#### Erstkonfiguration

* UID anpassen (darf nur einmal vorhanden sein).
* Wenn mehrere Decoder verwendet werden, muss auch BOARD_NUM angepasst werden.
* mittels ANZ_ADDONS festlegen, wieviele AddOn Platinen angeschlossen sind.
* #define LED_FEEDBACK auskommentieren, fall keine LED Anzeige angeschlossen / gewünscht.
* ggf. base_address anpassen.
* in void config_own_adresses_manual() kann die Konfiguration der Adressen & Magnetartikeltypen manuell angepasst werden.

##### Screenshots

![img1](/images/Input_config_1.png)
![img2](/images/Input_config_2.png)

##### Wichtige Hinweise

 * Eine Änderung des Protokoll wirkt sich auf alle angeschlossenen Magnetartikel aus.

# MäCAN-CAN GBS

Der nachfolgende Teil bezieht sich ausschließlich auf die Hardware-Revision A und Software-Version 0.3. Es ist zu beachten, welche Software-Version mit welcher Hardware-Revision kompatibel ist! Alle Hardware-Versionen der gleichen Revision sind zuheinander kompatibel und unterscheiden sich nicht in ihrer Funktionsweise.

Das Projekt ist noch im Anfangsstadium. Es ist also mit Veränderungen in relativ kurzen abständen zu rechnen und weder Hard- noch Software sind final! Hier kann sich jederzeit einiges verändern. Es wird versucht diese Dokumentation so aktuell wie möglich zu halten. Hierfür besteht aber keine Gewähr.

## Changelog

##### V0.3

 + initiale Version.
 + Pro Decoder können maximal 6 AddOn Platinen angeschlossen werden.
 + Es können 96 Rückmelder dadurch angezeigt werden.
 + Konfiguration über CS2 (getestet) / CS3plus (ungetestet) möglich.

## Dokumentation

#### Inhalt

* [Bestückung der AddOn Platinen](#bestückung-der-addon-platinen)
* [Erstkonfiguration](#erstkonfiguration)
* [Screenshots](#screenshots)
* [Wichtige Hinweise](#wichtige-hinweise)

#### Bestückung der AddOn Platinen

![img_pcb](/CAN-GBS/board/MäCAN-I-O-AddOn-PCB.png)

* IC2 wird nicht bestückt.
* Die LEDs werden mit dem Minuspol an IC3 (GPB0 - GPB7) angeschlossen (Stiftleiste über IC3)
* Die Betriebsspannung kann über den Jumper unterhalb IC3 gewählt werden.
  * \+5V & Mitte: 5V Spannungsversorgung über den Spannungsregler des Decoders
  * oder Mitte & \+18V: Externe Spannungsversorgung (18V Gleispannung oder beliebig wählbar über externes Netzteil)
* Die Taster werden an der Stiftleiste unterhalb von IC2 (GPA0 - GPA7) angeschlossen, ein seperater PullUp Widerstand wird nicht benötigt.

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

* UID anpassen (darf nur einmal vorhanden sein)
* Wenn mehrere Decoder verwendet werden, muss auch BOARD_NUM angepasst werden.
* S88_Dev anpassen:
** S88_Dev = false;  bedeutet, dass die S88 Rückmelder direkt an der CS2 / CS3plus angeschlossen sind
** S88_Dev = true;   bedeutet, dass die S88 Rückmelder via Link L88 an die CS2 / CS3 / CS3plus angeschlossen sind.
   in diesem Fall muss auch die Variable modulID angepasst werden.

##### Screenshots

* Konfigruration 1: S88 Rückmelder direkt an der CS2/CS3plus angeschlossen
![img1](/images/GBS_config_1.png)

* Konfigruration 2: S88 Rückmelder an Link L88 angeschlossen
![img2](/images/GBS_config_2.0.png)
![img3](/images/GBS_config_2.1.png)
![img4](/images/GBS_config_2.2.png)

##### Wichtige Hinweise

 * Das Ändern des S88 Anschluss ( CS2/CS3plus bzw. Link L88 ) funktioniert erst nach einem Neustart der CS2/CS3plus

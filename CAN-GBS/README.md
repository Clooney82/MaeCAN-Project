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

* [Erstkonfiguration](#Erstkonfiguration)
* [Screenshots](#Screenshots)
* [Wichtige Hinweise](#Wichtige-Hinweise)

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

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

* [Erstkonfiguration](#Erstkonfiguration)
* [Screenshots](#Screenshots)
* [Wichtige Hinweise](#Wichtige-Hinweise)

#### Erstkonfiguration

* UID anpassen (darf nur einmal vorhanden sein).
* Wenn mehrere Decoder verwendet werden, muss auch BOARD_NUM angepasst werden.
* mittels ANZ_ADDONS festlegen, wieviele AddOn Platinen angeschlossen sind.
* #define LED_FEEDBACK auskommentieren, fall keine LED Anzeige angeschlossen / gewünscht.
* ggf. base_address anpassen.
* in void config_own_adresses_manual() kann die Konfiguration der Adressen & Magnetartikeltypen manuell angepasst werden.

##### Screenshots

![img1](/images/Input_config_1.png)
![img2](/images/Input_config_1.png)

##### Wichtige Hinweise

 * Eine Änderung des Protokoll wirkt sich auf alle angeschlossenen Magnetartikel aus.

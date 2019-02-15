//#############################################################################
// Blink Status LED
//#############################################################################
void blink_LED() {
  state_LED = !state_LED;
  digitalWrite(STATUS_LED_PIN, state_LED);
}


//#############################################################################
// Erfolgreiches Setup signalisieren
//#############################################################################
void signal_setup_successfull(){
  for(int i=0; i < 20; i++){
    blink_LED();
    delay(100);
  }
}


//#############################################################################
// Ausführen, wenn eine Nachricht verfügbar ist.
// Nachricht wird geladen und anhängig vom CAN-Befehl verarbeitet.
//#############################################################################
void interruptFn() {
  blink_LED();
  MCANMSG mcan_frame_in = mcan.getCanFrame();
  BackgroundClass.incomingFrame(mcan_frame_in);
}


//#############################################################################
// XXXXXXXXXXXXXXXXXXXXXXXX
//#############################################################################

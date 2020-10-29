#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"     // Disable the warning: warning: 'EEPROM' defined but not used [-Wunused-variable]
#include <NmraDcc.h>
NmraDcc  Dcc;        // Instance of the NmraDcc class
// #pragma GCC diagnostic pop                          // Unfortunately I can't turn on the warning again for the following code for some reasons ;-(
// #pragma GCC diagnostic warning "-Wunused-variable"  // Is also enabling the EEPROM warning

#include "22_OwnSetup.h"

//-------------------------------------------------------------------------------------
void setup_dcc()
//-------------------------------------------------------------------------------------
{
    Dcc.pin(0, DCC_SIGNAL_PIN, 1); // Setup which External Interrupt, the Pin it's associated with that we're using and enable the Pull-Up

    // Call the main DCC Init function to enable the DCC Receiver
    Dcc.init( MAN_ID_DIY, 10, CV29_ACCESSORY_DECODER | CV29_OUTPUT_ADDRESS_MODE, 0 );  // ToDo: Was bedeuten die Konstanten ?
}


//-------------------------------------------------------------------------------------
void notifyDccAccTurnoutOutput( uint16_t Addr, uint8_t Direction, uint8_t OutputPower )
//-------------------------------------------------------------------------------------
// This function is called whenever a normal DCC Turnout Packet is received
//
// Unfortunately not all DCC commands are received if the I2C is used to update the OLED display.
// => We don't allways get the "Buuom pressed" and "Button released" messages.
// As a work arround a timeout is used.
// The first message sets the timeout and triggers the action independant from the "Button" state.
// The following messages within the timeout are ignored.
// This methode is used because it's possible that we miss the "Button pressed" message but
// receive the "Button released" signal. In this case we assume thet the button must be pressed before.
// The OutputPower parameter is ignored.
// 6-7 DCC messages from a MS2 are received if a button is pressed and released again. This takes
// about 300 ms (Depending on the time the button is pressed). Therefor the timeout is set to 500 ms.
{
  const int DCC_BUTTON_TIMEOUT = 500;
  static uint32_t Timeout = 0;

  if (Addr >= FIRST_DCC_ADDR && Addr <= LAST_DCC_ADDR)
  {
    if (millis() > Timeout)
    {
      if (Direction>0) Direction = 1; // In case some controller sends an other value
      Timeout = millis() + DCC_BUTTON_TIMEOUT;
      for (int i = 0; i < NUM_ACCs; i++) {
        if (Addr == acc_articles[i].address) {
          acc_articles[i].state_set = Direction;
          acc_articles[i].power_set = ON;
          break;
        }

      }
    }
  }
}

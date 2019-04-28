
#ifndef _mcp23017_w1_H_
#define _mcp23017_w1_H_

#include <i2c_t3.h>

//#define DEBUG

class MCP23017_W1 {
public:
	void begin(uint8_t addr = 0, uint8_t wirebus = 1);

  void pinMode(uint8_t pin, uint8_t direction);
  void digitalWrite(uint8_t pin, uint8_t value);
  void pullUp(uint8_t pin, uint8_t state);
  uint8_t digitalRead(uint8_t pin);

 private:
  uint8_t i2caddr;

  uint8_t bitForPin(uint8_t pin);
  uint8_t regForPin(uint8_t pin, uint8_t portAaddr, uint8_t portBaddr);

  uint8_t readRegister(uint8_t addr);
  void writeRegister(uint8_t addr, uint8_t value);

  void updateRegisterBit(uint8_t pin, uint8_t pinValue, uint8_t portAaddr, uint8_t portBaddr);

};

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

#define MCP23017_W1_ADDRESS  0x20

// registers
#define MCP23017_W1_IODIRA   0x00
#define MCP23017_W1_IPOLA    0x02
#define MCP23017_W1_GPINTENA 0x04
#define MCP23017_W1_DEFVALA  0x06
#define MCP23017_W1_INTCONA  0x08
#define MCP23017_W1_IOCONA   0x0A
#define MCP23017_W1_GPPUA    0x0C
#define MCP23017_W1_INTFA    0x0E
#define MCP23017_W1_INTCAPA  0x10
#define MCP23017_W1_GPIOA    0x12
#define MCP23017_W1_OLATA    0x14

#define MCP23017_W1_IODIRB   0x01
#define MCP23017_W1_IPOLB    0x03
#define MCP23017_W1_GPINTENB 0x05
#define MCP23017_W1_DEFVALB  0x07
#define MCP23017_W1_INTCONB  0x09
#define MCP23017_W1_IOCONB   0x0B
#define MCP23017_W1_GPPUB    0x0D
#define MCP23017_W1_INTFB    0x0F
#define MCP23017_W1_INTCAPB  0x11
#define MCP23017_W1_GPIOB    0x13
#define MCP23017_W1_OLATB    0x15

#define MCP23017_W1_INT_ERR  255

#endif

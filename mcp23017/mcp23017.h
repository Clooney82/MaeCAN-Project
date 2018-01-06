
#ifndef _mcp23017_H_
#define _mcp23017_H_

#if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))	// teensy 3.0/3.1-3.2/LC/3.5/3.6
	#include <i2c_t3.h>
#else
	#include <Wire.h>
#endif

class MCP23017 {
public:
	void begin(uint8_t addr = 0, uint8_t wirebus = 0);
	//void begin(uint8_t addr);
  //void begin(void);

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

#define MCP23017_ADDRESS  0x20

// registers
#define MCP23017_IODIRA   0x00
#define MCP23017_IPOLA    0x02
#define MCP23017_GPINTENA 0x04
#define MCP23017_DEFVALA  0x06
#define MCP23017_INTCONA  0x08
#define MCP23017_IOCONA   0x0A
#define MCP23017_GPPUA    0x0C
#define MCP23017_INTFA    0x0E
#define MCP23017_INTCAPA  0x10
#define MCP23017_GPIOA    0x12
#define MCP23017_OLATA    0x14

#define MCP23017_IODIRB   0x01
#define MCP23017_IPOLB    0x03
#define MCP23017_GPINTENB 0x05
#define MCP23017_DEFVALB  0x07
#define MCP23017_INTCONB  0x09
#define MCP23017_IOCONB   0x0B
#define MCP23017_GPPUB    0x0D
#define MCP23017_INTFB    0x0F
#define MCP23017_INTCAPB  0x11
#define MCP23017_GPIOB    0x13
#define MCP23017_OLATB    0x15

#define MCP23017_INT_ERR  255

#endif

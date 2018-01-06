#include <mcp23017.h>
#include <Arduino.h>


/**
 * Bit number associated to a give Pin
 */
uint8_t MCP23017::bitForPin(uint8_t pin){
	return pin%8;
}

/**
 * Register address, port dependent, for a given PIN
 */
uint8_t MCP23017::regForPin(uint8_t pin, uint8_t portAaddr, uint8_t portBaddr){
	return(pin<8) ?portAaddr:portBaddr;
}

/**
 * Reads a given register
 */
uint8_t MCP23017::readRegister(uint8_t addr){
	Wire.beginTransmission(MCP23017_ADDRESS | i2caddr);
	Wire.write(addr);
	Wire.endTransmission();
	Wire.requestFrom(MCP23017_ADDRESS | i2caddr, 1);
	return Wire.read();
}

/**
 * Writes a given register
 */
void MCP23017::writeRegister(uint8_t regAddr, uint8_t regValue){
	Wire.beginTransmission(MCP23017_ADDRESS | i2caddr);
	Wire.write(regAddr);
	Wire.write(regValue);
	Wire.endTransmission();
}

/**
 * Helper to update a single bit of an A/B register.
 * - Reads the current register value
 * - Writes the new register value
 */
void MCP23017::updateRegisterBit(uint8_t pin, uint8_t pinValue, uint8_t portAaddr, uint8_t portBaddr) {
	uint8_t regValue;
	uint8_t regAddr=regForPin(pin,portAaddr,portBaddr);
	uint8_t bit=bitForPin(pin);
	regValue = readRegister(regAddr);

	bitWrite(regValue,bit,pinValue);

	writeRegister(regAddr,regValue);
}

/**
 * Initializes the MCP23017 given its HW selected address, see datasheet for Address selection.
 */
/*
void MCP23017::begin(uint8_t addr) {
	if (addr > 7) {
		addr = 7;
	}
	i2caddr = addr;

	Wire.begin();

	// set defaults!
	// all inputs on port A and B
	writeRegister(MCP23017_IODIRA, 0xff);
	writeRegister(MCP23017_IODIRB, 0xff);
}
*/
/**
 * Initializes the default MCP23017, with 000 for the configurable part of the address
 */
/*
void MCP23017::begin(void) {
	begin(0);
}
*/
/**
 * Initializes MCP23017, on given I2C interface (only teensy, needs i2c_t3 library)
 * default pin setting SCL/SDA:
 *  * Wire : 19/18
 *  * Wire1: 29/30 (3.1/3.2), 22/23 (LC), 37/38 (3.5/3.6)
 *  * Wire2: 3/4 (3.5/3.6)
 *  * Wire3: 57/56 (3.6)
 */
void MCP23017::begin(uint8_t addr, uint8_t wirebus) {
	#if ( defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))	// teensy 3.0/3.1-3.2/LC/3.5/3.6
		if ( wirebus == 1 ) {
			Wire = Wire1;
		#if ( defined(__MK64FX512__) || defined(__MK66FX1M0__))
			} else if ( wirebus == 2 ) {
				Wire = Wire2;
		#endif
		#if defined(__MK66FX1M0__)	// teensy 3.0/3.1-3.2/LC/3.5/3.6
			} else if ( wirebus == 3 ) {
				Wire = Wire3;
		#endif
		}
	#endif
	if (addr > 7) {
		addr = 7;
	}
	i2caddr = addr;

	Wire.begin();

	writeRegister(MCP23017_IODIRA, 0xff);
	writeRegister(MCP23017_IODIRB, 0xff);
}

/**
 * Sets the pin mode to either INPUT or OUTPUT
 */
void MCP23017::pinMode(uint8_t pin, uint8_t direction) {
	updateRegisterBit(pin,(direction==INPUT),MCP23017_IODIRA,MCP23017_IODIRB);
}

void MCP23017::digitalWrite(uint8_t pin, uint8_t value) {
	uint8_t gpio;
	uint8_t bit=bitForPin(pin);

	// read the current GPIO output latches
	uint8_t regAddr=regForPin(pin,MCP23017_OLATA,MCP23017_OLATB);
	gpio = readRegister(regAddr);

	// set the pin and direction
	bitWrite(gpio,bit,value);

	// write the new GPIO
	regAddr=regForPin(pin,MCP23017_GPIOA,MCP23017_GPIOB);
	writeRegister(regAddr,gpio);
}

void MCP23017::pullUp(uint8_t pin, uint8_t state) {
	updateRegisterBit(pin,state,MCP23017_GPPUA,MCP23017_GPPUB);
}

uint8_t MCP23017::digitalRead(uint8_t pin) {
	uint8_t bit=bitForPin(pin);
	uint8_t regAddr=regForPin(pin,MCP23017_GPIOA,MCP23017_GPIOB);
	return (readRegister(regAddr) >> bit) & 0x1;
}

// ---------------------------------------------------------------------------

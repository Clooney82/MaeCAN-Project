#include <mcp23017_w1.h>
#include <Arduino.h>


/**
 * Bit number associated to a give Pin
 */
uint8_t MCP23017_W1::bitForPin(uint8_t pin){
	return pin%8;
}

/**
 * Register address, port dependent, for a given PIN
 */
uint8_t MCP23017_W1::regForPin(uint8_t pin, uint8_t portAaddr, uint8_t portBaddr){
	return(pin<8) ?portAaddr:portBaddr;
}

/**
 * Reads a given register
 */
uint8_t MCP23017_W1::readRegister(uint8_t addr){
	Wire1.beginTransmission(MCP23017_W1_ADDRESS | i2caddr);
	Wire1.write(addr);
	Wire1.endTransmission();
	Wire1.requestFrom(MCP23017_W1_ADDRESS | i2caddr, 1);
	return Wire1.read();
}

/**
 * Writes a given register
 */
void MCP23017_W1::writeRegister(uint8_t regAddr, uint8_t regValue){
	Wire1.beginTransmission(MCP23017_W1_ADDRESS | i2caddr);

	#ifdef DEBUG
		if (( Wire1.status() != 0 ) || ( Wire1.getError() != 0 ) ) {
			Serial.print("mcp23017 | writeRegister | beginTransmission");
			Serial.print(" | Status: ");
			Serial.print(Wire1.status());
			Serial.print(" | Error: ");
			Serial.println(Wire1.getError());
		}
	#endif

	Wire1.write(regAddr);

	#ifdef DEBUG
		if (( Wire1.status() != 0 ) || ( Wire1.getError() != 0 ) ) {
			Serial.print("mcp23017 | writeRegister | write(regAddr) ");
			Serial.print(regAddr);
			Serial.print(" | Status: ");
			Serial.print(Wire1.status());
			Serial.print(" | Error: ");
			Serial.println(Wire1.getError());
		}
	#endif

	Wire1.write(regValue);

	#ifdef DEBUG
		if (( Wire1.status() != 0 ) || ( Wire1.getError() != 0 ) ) {
			Serial.print("mcp23017 | writeRegister | write(regValue) ");
			Serial.print(regValue);
			Serial.print(" | Status: ");
			Serial.print(Wire1.status());
			Serial.print(" | Error: ");
			Serial.println(Wire1.getError());
		}
	#endif

	Wire1.endTransmission();

	#ifdef DEBUG
		if (( Wire1.status() != 0 ) || ( Wire1.getError() != 0 ) ) {
			Serial.print("mcp23017 | writeRegister | endTransmission");
			Serial.print(" | Status: ");
			Serial.print(Wire1.status());
			Serial.print(" | Error: ");
			Serial.println(Wire1.getError());
		}
	#endif
}

/**
 * Helper to update a single bit of an A/B register.
 * - Reads the current register value
 * - Writes the new register value
 */
void MCP23017_W1::updateRegisterBit(uint8_t pin, uint8_t pinValue, uint8_t portAaddr, uint8_t portBaddr) {
	uint8_t regValue;
	uint8_t regAddr=regForPin(pin,portAaddr,portBaddr);
	uint8_t bit=bitForPin(pin);
	regValue = readRegister(regAddr);

	bitWrite(regValue,bit,pinValue);

	writeRegister(regAddr,regValue);
}

/**
 * Initializes MCP23017_W1, on given I2C interface (only teensy, needs i2c_t3 library)
 * default pin setting SCL/SDA:
 *  * Wire : 19/18
 *  * Wire1: 29/30 (3.1/3.2), 22/23 (LC), 37/38 (3.5/3.6)
 *  * Wire2: 3/4 (3.5/3.6)
 *  * Wire3: 57/56 (3.6)
 */
void MCP23017_W1::begin(uint8_t addr, uint8_t wirebus) {
	/*#if ( defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))	// teensy 3.0/3.1-3.2/LC/3.5/3.6
		if ( wirebus == 1 ) {
			Wire = i2c_t3(1);
		#if ( defined(__MK64FX512__) || defined(__MK66FX1M0__))	// teensy 3.5/3.6
			} else if ( wirebus == 2 ) {
				Wire = i2c_t3(2);
		#endif
		#if ( defined(__MK66FX1M0__))	// teensy 3.6
			} else if ( wirebus == 3 ) {
				Wire = i2c_t3(3);
			} else if ( wirebus == 0 ) {
				Wire = i2c_t3(0);
		#endif
		}
	#endif
	*/
	if (addr > 7) {
		addr = 7;
	}

	i2caddr = addr;

	#ifdef DEBUG
		Serial.print("mcp23017 | begin | I2C-Bus: ");
		Serial.print(wirebus);
		Serial.print(" | SCL Pin: ");
		Serial.print(Wire1.getSCL());
		Serial.print(" | SDA Pin: ");
		Serial.println(Wire1.getSDA());
	#endif
	/*
	#if ( defined(__MK20DX256__) || defined(__MKL26Z64__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))	// teensy 3.0/3.1-3.2/LC/3.5/3.6
		Wire1.setDefaultTimeout(5000);
	#endif
	*/
	Wire1.begin();

	#ifdef DEBUG
		if (( Wire1.status() != 0 ) || ( Wire1.getError() != 0 ) ) {
			Serial.print("mcp23017 | begin");
			Serial.print(" | Status: ");
			Serial.print(Wire1.status());
			Serial.print(" | Error: ");
			Serial.println(Wire1.getError());
		}
	#endif

	writeRegister(MCP23017_W1_IODIRA, 0xff);
	writeRegister(MCP23017_W1_IODIRB, 0xff);

	#ifdef DEBUG
		Serial.println("mcp23017 | begin | finished");
	#endif
}

/**
 * Sets the pin mode to either INPUT or OUTPUT
 */
void MCP23017_W1::pinMode(uint8_t pin, uint8_t direction) {
	#ifdef DEBUG
		Serial.print("mcp23017 | pinMode | START");
		Serial.print(" | SCL Pin: ");
		Serial.print(Wire1.getSCL());
		Serial.print(" | SDA Pin: ");
		Serial.println(Wire1.getSDA());
	#endif
	updateRegisterBit(pin,(direction==INPUT),MCP23017_W1_IODIRA,MCP23017_W1_IODIRB);
	#ifdef DEBUG
		Serial.println("mcp23017 | pinMode | END");
	#endif
}

void MCP23017_W1::digitalWrite(uint8_t pin, uint8_t value) {
	uint8_t gpio;
	uint8_t bit=bitForPin(pin);

	// read the current GPIO output latches
	uint8_t regAddr=regForPin(pin,MCP23017_W1_OLATA,MCP23017_W1_OLATB);
	gpio = readRegister(regAddr);

	// set the pin and direction
	bitWrite(gpio,bit,value);

	// write the new GPIO
	regAddr=regForPin(pin,MCP23017_W1_GPIOA,MCP23017_W1_GPIOB);
	writeRegister(regAddr,gpio);
}

void MCP23017_W1::pullUp(uint8_t pin, uint8_t state) {
	#ifdef DEBUG
		Serial.print("mcp23017 | pullUP | START");
		Serial.print(" | SCL Pin: ");
		Serial.print(Wire1.getSCL());
		Serial.print(" | SDA Pin: ");
		Serial.println(Wire1.getSDA());
	#endif
	updateRegisterBit(pin,state,MCP23017_W1_GPPUA,MCP23017_W1_GPPUB);
	#ifdef DEBUG
		Serial.println("mcp23017 | pullUP | END");
	#endif
}

uint8_t MCP23017_W1::digitalRead(uint8_t pin) {
	uint8_t bit=bitForPin(pin);
	uint8_t regAddr=regForPin(pin,MCP23017_W1_GPIOA,MCP23017_W1_GPIOB);
	return (readRegister(regAddr) >> bit) & 0x1;
}

// ---------------------------------------------------------------------------

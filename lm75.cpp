/*
 Copyright © 2014 José Luis Zabalza  License LGPLv3+: GNU
 LGPL version 3 or later <http://www.gnu.org/copyleft/lgpl.html>.
 This is free software: you are free to change and redistribute it.
 There is NO WARRANTY, to the extent permitted by law.
*/


#include "lm75.h"
#include <Arduino.h>


//-------------------------------------------------------------------------------
TempI2C_LM75::TempI2C_LM75( uint8_t i2c_addr,  uint8_t _sda, uint8_t _scl)
{
    if (_sda == 21) {
        wire=&Wire1;
    } else {
        wire=&Wire;
    }
    wire->begin(_sda,_scl);
    wire->setClock(100000);

    wire->beginTransmission(i2c_addr);
    wire->endTransmission();
    if (wire->lastError() != I2C_ERROR_OK) {
        m_u8I2CAddr=0;
        return;
    }

    m_u8I2CAddr = i2c_addr;
}

//-------------------------------------------------------------------------------
float TempI2C_LM75::getTemp()
{
	union {
		unsigned short tempX;
		short tempS;
	} temperature;

	temperature.tempX = getReg(temp_reg);
	return (temperature.tempS / 256.0F);
}

//-------------------------------------------------------------------------------
unsigned short TempI2C_LM75::getReg(LM75Register reg)
{
    unsigned short Result=0;
    if ( m_u8I2CAddr == 0) {
        return 0;
    }
#define DEBUG
#ifdef DEBUG
    Serial.print("getReg(");
    Serial.print(m_u8I2CAddr,HEX);
    Serial.print(",");
    Serial.print(uint8_t(reg),HEX);
    Serial.print(")=");
#endif


    wire->beginTransmission(m_u8I2CAddr);
    Serial.print(m_u8I2CAddr,HEX); Serial.println(wire->lastError());
    wire->write(reg); // pointer reg
    wire->endTransmission();

    uint8_t c=0;

    wire->requestFrom(m_u8I2CAddr, uint8_t(2));
    if(wire->available())
    {
        c = wire->read();
        Result = c;
        if(reg != config_reg)
        {
            Result = (unsigned(c))<<8;
            if(wire->available())
            {
                c = wire->read();
                Result |= (unsigned(c));
            }
            else
            {
#ifdef DEBUG
                Serial.println("Error 1");
#endif
                Result = 0;
            }
        }
    }
#ifdef DEBUG
    else
        Serial.println("Error 2");
#endif

    Serial.println(Result);
    return(Result);
}
//-------------------------------------------------------------------------------
bool TempI2C_LM75::setReg(LM75Register reg, unsigned newValue)
{
#ifdef DEBUG
    Serial.print("setReg(");
    Serial.println(uint8_t(reg),HEX);
    Serial.print(",");
    Serial.println(newValue,HEX);
    Serial.println(")");
#endif
    wire->beginTransmission(m_u8I2CAddr);
    wire->write(reg); // pointer reg
    if(reg != config_reg)
    {
        wire->write((newValue>>8) & 0xFF);
    }
    wire->write(newValue & 0xFF);

    wire->endTransmission();
    return wire->lastError == I2C_ERROR_OK;
}

//-------------------------------------------------------------------------------
void TempI2C_LM75::setTHyst(float newTHyst)
{
    setReg(THyst_reg,int(newTHyst * 256));
}

//-------------------------------------------------------------------------------
void TempI2C_LM75::setTOS(float newTOS)
{
    setReg(TOS_reg,int(newTOS * 256) );
}

//-------------------------------------------------------------------------------
float TempI2C_LM75::getTHyst(void)
{
    return(int(getReg(THyst_reg)) / 256.0F);
}

//-------------------------------------------------------------------------------
float TempI2C_LM75::getTOS(void)
{
    return(int(getReg(TOS_reg)) / 256.0F);
}

//-------------------------------------------------------------------------------
TempI2C_LM75::ThermostatMode TempI2C_LM75::getThermostatMode()
{
    CfgRegister regv;

    regv.mbyte = getReg(config_reg);

	return (ThermostatMode(regv.mbits.thermostat_mode));
}

//-------------------------------------------------------------------------------
void TempI2C_LM75::setThermostatMode(TempI2C_LM75::ThermostatMode newMode)
{
    CfgRegister regv;

    regv.mbyte = getReg(config_reg);
	regv.mbits.thermostat_mode = newMode;
	regv.mbits.reserved = 0;
    setReg(config_reg,unsigned(regv.mbyte));
}

//-------------------------------------------------------------------------------
TempI2C_LM75::ThermostatFaultTolerance TempI2C_LM75::getThermostatFaultTolerance()
{
    CfgRegister regv;

    regv.mbyte = getReg(config_reg);

	return (ThermostatFaultTolerance(regv.mbits.thermostat_fault_tolerance));
}

//-------------------------------------------------------------------------------
void TempI2C_LM75::setThermostatFaultTolerance(ThermostatFaultTolerance newFaultTolerance)
{
    CfgRegister regv;

    regv.mbyte = getReg(config_reg);
	regv.mbits.thermostat_fault_tolerance = newFaultTolerance;
	regv.mbits.reserved = 0;
    setReg(config_reg,unsigned(regv.mbyte));
}

bool TempI2C_LM75::getShutdown()
{
    CfgRegister regv;

    regv.mbyte = getReg(config_reg);

	return (regv.mbits.shutdown);
}

void TempI2C_LM75::setShutdown(bool newShutdown)
{
    CfgRegister regv;

    regv.mbyte = getReg(config_reg);
	regv.mbits.shutdown = newShutdown;
	regv.mbits.reserved = 0;
    setReg(config_reg,unsigned(regv.mbyte));
}

//-------------------------------------------------------------------------------
TempI2C_LM75::OSPolarity TempI2C_LM75::getOSPolarity()
{
    CfgRegister regv;

    regv.mbyte = getReg(config_reg);

	return (OSPolarity(regv.mbits.thermostat_output_polarity));
}

//-------------------------------------------------------------------------------
void TempI2C_LM75::setOSPolarity(OSPolarity newOSPolarity)
{
    CfgRegister regv;

    regv.mbyte = getReg(config_reg);
	regv.mbits.thermostat_output_polarity = newOSPolarity;
	regv.mbits.reserved = 0;
    setReg(config_reg,unsigned(regv.mbyte));
}

bool TempI2C_LM75::isActive()
{
    return m_u8I2CAddr != 0;
}

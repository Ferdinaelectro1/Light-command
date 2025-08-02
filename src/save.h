#pragma once

#include <EEPROM.h>
#include "def.h"


const int EEPROM_SIZE = 512;
const int CONFIG_ADDRESS = 0;
const int OLD_STATE_ADRESS = 1;

void saveTimeConfigToEEPROM(const FourConfig &config)
{
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(CONFIG_ADDRESS,config);
    EEPROM.commit();
    EEPROM.end();
}

FourConfig loadTimeConfigsToEEPROM()
{
    FourConfig config;
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(CONFIG_ADDRESS,config);
    EEPROM.end();
    return config;
}

void saveOldLampStateToEEPROM(const LampStates &lamp_states)
{
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(OLD_STATE_ADRESS,lamp_states);
    EEPROM.commit();
    EEPROM.end();
}

LampStates loadOldLampesState()
{
    LampStates oldState;
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(OLD_STATE_ADRESS,oldState);
    EEPROM.end();
    return oldState;
}
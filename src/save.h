#pragma once

#include <EEPROM.h>
#include "def.h"


const int EEPROM_SIZE = 512;
const int CONFIG_ADDRESS = 0;
const int OLD_STATE_ADRESS = 1;
const int CHECK_ADRESS = 2;
const int HOST_POINT_CONFIG_ADRESS = 3;

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

LampStates loadOldLampesStatetoEEPROM()
{
    LampStates oldState;
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(OLD_STATE_ADRESS,oldState);
    EEPROM.end();
    return oldState;
}

void saveCheckActivitytoEEPROM(const SaveStateActivity &save)
{
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(CHECK_ADRESS,save);
    EEPROM.commit();
    EEPROM.end();
}

SaveStateActivity loadCheckActivitytoEEPROM()
{
    SaveStateActivity save;
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(CHECK_ADRESS,save);
    EEPROM.end();
    return save;
}

void saveHostPointConfigtoEEPROM(const hostPointConfig &config)
{
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(HOST_POINT_CONFIG_ADRESS,config);
    EEPROM.commit();
    EEPROM.end();
}

hostPointConfig loadHostPointConfigtoEEPROM()
{
    hostPointConfig config;
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(HOST_POINT_CONFIG_ADRESS,config);
    EEPROM.end();
    return config;
}
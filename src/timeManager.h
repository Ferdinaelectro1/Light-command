#pragma once
#include <RTClib.h>
#include "def.h"
#include "rtc_horloge_manager.h"
#include "gsm_manager.h"

extern SaveStateActivity save_activity;
extern SoftwareSerial sim800;

void updateState(const TimeConfig& config, const Time& currentTime,uint8_t pinToDeclenche)
{
    if (currentTime == config.onTime) {
          digitalWrite(pinToDeclenche,true);
            LampStates lmpstate;
            lmpstate.oldLamp1State = (save_activity.checkLampe1) ? digitalRead(13) : false;
            lmpstate.oldLamp2State =  (save_activity.ckeckLampe2) ? digitalRead(15) : false;
            if(save_activity.checkLampe1 || save_activity.ckeckLampe2)
            saveOldLampStateToEEPROM(lmpstate);
        return;
    }

    if (currentTime == config.ofTime) {
          digitalWrite(pinToDeclenche,false);
          LampStates lmpstate;
          lmpstate.oldLamp1State = (save_activity.checkLampe1) ? digitalRead(13) : false;
          lmpstate.oldLamp2State =  (save_activity.ckeckLampe2) ? digitalRead(15) : false;
          if(save_activity.checkLampe1 || save_activity.ckeckLampe2)
            saveOldLampStateToEEPROM(lmpstate);
        return;
    }
}
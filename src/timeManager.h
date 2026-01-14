#pragma once
#include <RTClib.h>
#include "def.h"
#include "rtc_horloge_manager.h"
#include "gsm_manager.h"

extern SaveStateActivity save_activity;
extern SoftwareSerial sim800;

//cette fonction permet de changer l'état de la lampe en fonction de l'heure d'allumage et de l'heure d'extinction
/* void updateStateFlexible(const TimeConfig& config, bool& state, const Time& currentTime)
{
/*     if (!config.isvalide || !currentTime.valide)
        return; */


/*    // Vérifie si on a atteint ou dépassé l'heure d'allumage mais pas encore atteint l'heure d'extinction
    if (config.onTime < config.ofTime)
    {
        // Cas normal : allumage et extinction le même jour
        if ((currentTime > config.onTime || currentTime == config.onTime) && currentTime < config.ofTime)
        {
            state = true;
        }
        else
        {
            state = false;
        }
    }
    else
    {
        // Cas spécial : allumage le soir et extinction le matin suivant (ex: 19h -> 7h)
        if ((currentTime > config.onTime || currentTime == config.onTime) || currentTime < config.ofTime)
        {
            state = true;
        }
        else
        {
            state = false;
        }
    }
} */

void updateState(const TimeConfig& config, const Time& currentTime,uint8_t pinToDeclenche)
{
    // Vérifie la validité des temps
/*     if (!config.isvalide || !currentTime.valide)
        return; */
    
    // Allumer à la date/heure exacte
    if (currentTime == config.onTime) {
          digitalWrite(pinToDeclenche,true);
            LampStates lmpstate;
            lmpstate.oldLamp1State = (save_activity.checkLampe1) ? digitalRead(13) : false;
            lmpstate.oldLamp2State =  (save_activity.ckeckLampe2) ? digitalRead(15) : false;
            if(save_activity.checkLampe1 || save_activity.ckeckLampe2)
            saveOldLampStateToEEPROM(lmpstate);
        return;
    }

    // Éteindre à la date/heure exacte
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

/* void updateTimeallTwelve(RTC_DS3231 &rtc, const Time &nowTime,Time &originTime)
{
    date dt;
    if(nowTime - originTime == Time{12,0,0,true})
    {
        setupTimeToRTC(gsm::getNetworkTime(sim800,dt),rtc,dt);
        //remettre l'origine des temps à jour
        originTime = nowTime;
    }
} */

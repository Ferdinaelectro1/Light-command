#pragma once
#define LAMP_PIN 13
#include "def.h"


//cette fonction permet de changer l'état de la lampe en fonction de l'heure d'allumage et de l'heure d'extinction
void updateStateFlexible(const TimeConfig& config, bool& state, const Time& currentTime)
{
/*     if (!config.isvalide || !currentTime.valide)
        return; */


    // Vérifie si on a atteint ou dépassé l'heure d'allumage mais pas encore atteint l'heure d'extinction
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
}

void updateState(const TimeConfig& config, volatile bool& state, const Time& currentTime,volatile bool manuelController)
{
    // Vérifie la validité des temps
    if (!config.isvalide || !currentTime.valide)
        return;

    // Allumer à la date/heure exacte
    if (currentTime == config.onTime) {
          state = true;
          digitalWrite(LAMP_PIN,true);
        return;
    }

    // Éteindre à la date/heure exacte
    if (currentTime == config.ofTime) {
          state = false;
          digitalWrite(LAMP_PIN,false);
        return;
    }
}


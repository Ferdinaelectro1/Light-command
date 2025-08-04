#pragma once
#include <string>
#include <ArduinoJson.h>

//structure pour garder l'état passé des lampes
struct LampStates
{
  bool oldLamp1State = false;
  bool oldLamp2State = false;
};

//structure pour représenter le temps
struct Time
{
    long heure;
    long minute ;
    long seconde;    
    bool valide;

    bool operator<(const Time &other) const
    { 
        if (heure < other.heure) return 1;
        if (heure > other.heure) return 0;

        if (minute < other.minute) return 1;
        if (minute > other.minute) return 0;

        if (seconde < other.seconde) return 1;
        return 0;
    }

    bool operator>(const Time &other) const
    { 
        if (heure < other.heure) return 0;
        if (heure > other.heure) return 1;

        if (minute < other.minute) return 0;
        if (minute > other.minute) return 1;

        if (seconde < other.seconde) return 0;
        return 1;
    }

    bool operator==(const Time &other) const
    {
        return (heure == other.heure && minute == other.minute && seconde == other.seconde);
    }

    Time operator-(const Time &other) const
    {
      long h = heure - other.heure;
      long m = minute - other.minute;
      long s = seconde - other.seconde;
      if (s < 0) {
        s += 60;
        m -= 1;
      }
      if (m < 0) {
        m += 60;
        h -= 1;
      }
      if (h < 0) h = 0; 
      return Time{ h , m , s };
    }
} ;


//structure pour représenter une configuration de tâche donnée (allumage , extinction)
struct TimeConfig
{
    Time onTime ;
    Time ofTime;
    bool isvalide = false;
};

//structure pour représenter quatre tâche (c'est pour faciliter le stockage dans la mémoire eeprom)
struct FourConfig
{
  TimeConfig tache_1_lamp_1;
  TimeConfig tache_2_lamp_1;
  TimeConfig tache_1_lamp_2;
  TimeConfig tache_2_lamp_2;
  bool isvalide = false;
};

// Fonction qui convertit une chaîne JSON en structure TimeConfig
TimeConfig convertToTimeConfig(const String &str) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, str);


  TimeConfig cfg;

  if (error) {
    Serial.print("Erreur JSON : ");
    Serial.println(error.f_str());
    return cfg;
  }

  //alummage config
  cfg.onTime.heure   = doc["allumage"]["heure"] | 0;
  cfg.onTime.minute  = doc["allumage"]["minute"] | 0;
  cfg.onTime.seconde = doc["allumage"]["seconde"] | 0;
  
  //extinction config
  cfg.ofTime.heure   = doc["extinction"]["heure"] | 0;
  cfg.ofTime.minute  = doc["extinction"]["minute"] | 0;
  cfg.ofTime.seconde = doc["extinction"]["seconde"] | 0;

  return cfg;
}



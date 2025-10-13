#pragma once
#include <string>
#include <ArduinoJson.h>

struct LampStates
{
  bool oldLamp1State = false;
  bool oldLamp2State = false;
};

struct SaveStateActivity
{
  bool checkLampe1 = false;
  bool ckeckLampe2 = false;
};

struct hostPointConfig
{
  String ssid ;
  String password ;
  bool isvalide = false;
};

struct date
{
  int annee;
  int mois;
  int jour;
};

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


struct TimeConfig
{
    Time onTime ;
    Time ofTime;
    bool isvalide = false;
};

struct FourConfig
{
  TimeConfig tache_1_lamp_1;
  TimeConfig tache_2_lamp_1;
  TimeConfig tache_1_lamp_2;
  TimeConfig tache_2_lamp_2;
  bool isvalide = false;
};

TimeConfig convertToTimeConfig(const String &str) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, str);


  TimeConfig cfg;

  if (error) {
    Serial.print("Erreur JSON : ");
    Serial.println(error.f_str());
    return cfg;
  }

  cfg.onTime.heure   = doc["allumage"]["heure"] | 0;
  cfg.onTime.minute  = doc["allumage"]["minute"] | 0;
  cfg.onTime.seconde = doc["allumage"]["seconde"] | 0;
  
  cfg.ofTime.heure   = doc["extinction"]["heure"] | 0;
  cfg.ofTime.minute  = doc["extinction"]["minute"] | 0;
  cfg.ofTime.seconde = doc["extinction"]["seconde"] | 0;

  return cfg;
}

void PrintConfig(const TimeConfig &timeconfig,const uint8_t &numero)
{
  #ifdef DEBUG_MODE
  Serial.print("Tâche ");
  Serial.print(numero);
  Serial.print(" : [");
  Serial.print("Allumage : ");
  Serial.print(timeconfig.onTime.heure);
  Serial.print(" h ");
  Serial.print(timeconfig.onTime.minute);
  Serial.print(" min ");
  Serial.print(timeconfig.onTime.seconde);
  Serial.print(" s || Exinction : ");
  Serial.print(timeconfig.ofTime.heure);
  Serial.print(" h ");
  Serial.print(timeconfig.ofTime.minute);
  Serial.print(" min ");
  Serial.print(timeconfig.ofTime.seconde);
  Serial.println(" s ]");
  #endif
}

void PrintFourConfig(const FourConfig &fourconfig)
{
  PrintConfig(fourconfig.tache_1_lamp_1,1);
  PrintConfig(fourconfig.tache_2_lamp_1,2);   
  PrintConfig(fourconfig.tache_1_lamp_2,3);   
  PrintConfig(fourconfig.tache_2_lamp_2,4);   
}
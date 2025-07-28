#pragma once
#include <string>
struct Time
{
    long heure;
    long minute;
    long seconde;    
    bool valide;
    bool operator==(const Time &other)
    {
        if (other.heure == heure &&
            other.minute == minute &&
            other.seconde == seconde &&
            other.valide == valide)
        {
        return 1;
        } else {
            return 0; 
        }
    }
} ;


struct TimeConfig
{
    Time onTime;
    Time ofTime;
    bool isvalide;
};


void timeInit(const char* t, DS3231& rtc) {
  // Extraire les heures, minutes et secondes
  int heure = atoi(strtok((char*)t, ":"));
  int minute = atoi(strtok(NULL, ":"));
  int seconde = atoi(strtok(NULL, ":"));

  // Date fixe (exemple : 2025-07-27)
  rtc.adjust(DateTime(2025, 7, 27, heure, minute, seconde));
}



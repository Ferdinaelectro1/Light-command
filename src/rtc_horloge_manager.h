#pragma once
#include "def.h"
#include <Wire.h>
#include <RTClib.h>

void setupTimeToRTC(const Time &reelTime,RTC_DS3231 &rtc)
{
  // Extraire les heures, minutes et secondes
  int heure = reelTime.heure;
  int minute = reelTime.minute;
  int seconde = reelTime.seconde;
  int month = reelTime.mois;
  int year = reelTime.annee;
  int day = reelTime.jour;
  // Date fixe (exemple : 2025-07-27)
  rtc.adjust(DateTime(year,month,day, heure, minute, seconde));
}


Time getHeureActuelleToRTC(RTC_DS3231 &rtc) {
    DateTime now = rtc.now();
    Time actuelle;
    actuelle.heure = now.hour();
    actuelle.minute = now.minute();
    actuelle.seconde = now.second();
    actuelle.annee = now.year();
    actuelle.mois = now.month();
    actuelle.jour = now.day();
    actuelle.valide = true;
    return actuelle;
}



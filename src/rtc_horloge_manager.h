#pragma once
#include "def.h"
#include <Wire.h>
#include <RTClib.h>

void setupTimeToRTC(const Time &reelTime,RTC_DS3231 &rtc,const date &date_actuelle)
{
  int heure = reelTime.heure;
  int minute = reelTime.minute;
  int seconde = reelTime.seconde;
  int annee = date_actuelle.annee + 2000;
  int mois = date_actuelle.mois;
  int jour = date_actuelle.jour;
  rtc.adjust(DateTime(annee, mois, jour, heure, minute, seconde));
}


Time getHeureActuelleToRTC(RTC_DS3231 &rtc,date &dt) {
    DateTime now = rtc.now();
    Time actuelle;
    actuelle.heure = now.hour();
    actuelle.minute = now.minute();
    actuelle.seconde = now.second();
    dt.annee = now.year();
    dt.mois = now.month();
    dt.jour = now.day();
    actuelle.valide = true;
    return actuelle;
}



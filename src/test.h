#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

struct Time {
  long heure;
  long minute;
  long seconde;
  bool valide;
};

class RTCTrigger {
  private:
    RTC_DS3231 rtc;
    Time heureCible;
    bool declenche;
    
    bool verifierDeclenchement(Time actuelle, Time cible) {
      return (actuelle.valide && cible.valide &&
              actuelle.heure == cible.heure &&
              actuelle.minute == cible.minute &&
              actuelle.seconde == cible.seconde);
    }
    
  public:
    RTCTrigger() : declenche(false) {
      Wire.begin();
      rtc.begin();
    }
    
    void setHeureCible(long h, long m, long s, bool valide = true) {
      heureCible.heure = h;
      heureCible.minute = m;
      heureCible.seconde = s;
      heureCible.valide = valide;
      declenche = false;
    }
    
    Time getHeureActuelle() {
      DateTime now = rtc.now();
      Time actuelle;
      actuelle.heure = now.hour();
      actuelle.minute = now.minute();
      actuelle.seconde = now.second();
      actuelle.valide = true;
      return actuelle;
    }
    
    void verifierEtDeclencher(void (*callback)()) {
      Time actuelle = getHeureActuelle();
      
      if (!declenche && verifierDeclenchement(actuelle, heureCible)) {
        declenche = true;
        if (callback != nullptr) {
          callback();
        }
      }
    }
    
    void afficherHeure(Time t) {
      Serial.print("Heure actuelle : ");
      Serial.print(t.heure);
      Serial.print(":");
      Serial.print(t.minute);
      Serial.print(":");
      Serial.println(t.seconde);
    }
};

RTCTrigger rtcTrigger;

void monAction() {
  Serial.println("Déclenchement à l'heure cible !");
}

void setup() {
  Serial.begin(115200);
  rtcTrigger.setHeureCible(18, 0, 0);
}

void loop() {
  rtcTrigger.verifierEtDeclencher(monAction);
  delay(1000);
}
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <RTClib.h>
#include "def.h"
#include "gsm_manager.h"
#include "page.h"
#include "rtc_horloge_manager.h"
#include "save.h"
#include "timeManager.h"

#define INTERVALLE 1000
#define LAMP_PIN 16

const char *ssid = "esp8266 wifi";
const char *password = "isniis";

RTC_DS3231 rtc;
unsigned long now = 0;
Time t = {10,20,30,true};
ESP8266WebServer server;
TimeConfig config ;

void printTime(const Time &t)
{
    Serial.print(t.jour);
    Serial.print("-");
    Serial.print(t.mois);
    Serial.print("-");
    Serial.print(t.annee);
    Serial.print(" ");
    Serial.print(t.heure);
    Serial.print(" h ");
    Serial.print(t.minute);
    Serial.print(" min ");
    Serial.print(t.seconde);
    Serial.println(" s");
}

void handleRoot()
{
  server.send_P(200, "text/html", code);
}

void configSetup()
{
  if (server.hasArg("plain"))
  {
    String json = server.arg("plain");
    server.send(200, "application/json", "{\"status\" : \"succes\",\"Données\" : "+json+"}");
    config = convertToTimeConfig(json);
    config.isvalide = false;
    saveTimeConfigToEEPROM(config);
    Serial.println("==================");
    Serial.println("On Time");
    printTime(config.onTime);
    Serial.println("Off Time");
    printTime(config.ofTime);
  }
  else
  {
    server.send(404, "application/json", "{\"status\" : \"echec\"}");
  }
}

void timeGet()
{
  server.send(200, "application/json", "{\"heure\":" + String(t.heure) + ",\"minute\":" + String(t.minute) + ",\"seconde\":" + String(t.seconde) + "}");
}

void command()
{
  if(server.hasArg("plain"))
  {
    String json = server.arg("plain");
    server.send(200,"text/json","{\"status\" : \"succes\"}");
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc,json);
    if(error)
    {
      Serial.println("Erreur de désérialisation");
    }
    digitalWrite(LAMP_PIN,doc["etat"]);
  }
  else
  {
    server.send(404,"text/json", "{\"status\" : \"echec\"}");
  }
}

void setup()
{
  //INIT du module horloge
  if(!rtc.begin())
  {
    Serial.println("Impossible de trouver le module RTC");
    while (1);
  }

  //init de la lampe
  pinMode(LAMP_PIN,OUTPUT);
  digitalWrite(LAMP_PIN,0);
  
  //init du port série
  Serial.begin(115200);
  Serial.println("\n\nStarting setup...");

  // Désactiver temporairement le WiFi
  WiFi.mode(WIFI_OFF);
  delay(1000);

  //Chargement de la configuration si elle est dans l'eeprom
  if(loadTimeConfigToEEPROM().isvalide)
  {
    config = loadTimeConfigToEEPROM();
  }
  else //sinon config par défaut
  {
    config.onTime = Time{20,0,0,true};
    config.ofTime = Time{7,0,0,true};
  }

  // Récupération du temps réel actuel en ligne grâce au gsm
  t = gsm::getNowTime();
  t.valide = true;

  //Réglage manuelle de l'heure du module rtc grâce à l'heure que le module gsm à récupéré en ligne
  setupTimeToRTC(t,rtc);
  printTime(t);

  // Réactiver le WiFi
  Serial.println("Starting WiFi AP...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP IP: ");
  Serial.println(ip);

  //configuration des routes du serveur et initialisation du serveur
  server.on("/", handleRoot);
  server.on("/config", HTTP_POST, configSetup);
  server.on("/time", HTTP_GET, timeGet);
  server.on("/commands",HTTP_POST,command);
  server.begin();
}

void loop()
{
  server.handleClient();
  if(millis() - now > INTERVALLE)
  {
    printTime(t);
    t = getHeureActuelleToRTC(rtc);
    bool lampState = false;
    updateStateFlexible(config,lampState,t);
    digitalWrite(LAMP_PIN,lampState);
    now = millis();
  }
}
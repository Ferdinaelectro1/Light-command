#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <RTClib.h>
#include "def.h"
#include "gsm_manager.h"
#include "page.h"
#include "rtc_horloge_manager.h"
#include "save.h"
#include "timeManager.h"

#define INTERVALLE 1000
#define LAMP_PIN 13 //d7
#define SWITCH_PIN 2 //D4
#define LAMP_PIN_2 15//d8
#define SWITCH_PIN_2 0 //D4

//SCL D1
//sda d2

const char *ssid = "esp8266 wifi";
const char *password = "isniis";

RTC_DS3231 rtc;
unsigned long now = 0;
Time t = {10,20,30,true};
AsyncWebServer server(80);
TimeConfig config ;
volatile bool LampState = false;
volatile bool manuelCommandState = false;

void printTime(const Time &t)
{
    Serial.print(" ");
    Serial.print(t.heure);
    Serial.print(" h ");
    Serial.print(t.minute);
    Serial.print(" min ");
    Serial.print(t.seconde);
    Serial.println(" s");
}

void handleRoot(AsyncWebServerRequest *request)
{
  request->send_P(200, "text/html", code);
}

void configSetup(AsyncWebServerRequest *request)
{
  if (request->hasArg("plain"))
  {
    String json = request->arg("plain");
    request->send(200, "application/json", "{\"status\" : \"succes\",\"Données\" : "+json+"}");
    config = convertToTimeConfig(json);
    config.isvalide = true;
    saveTimeConfigToEEPROM(config);
    Serial.println("==================");
    Serial.println("On Time");
    printTime(config.onTime);
    Serial.println("Off Time");
    printTime(config.ofTime);
    Serial.println(json);
  }
  else
  {
    request->send(404, "application/json", "{\"status\" : \"echec\"}");
  }
}

void timeGet(AsyncWebServerRequest *request)
{
  request->send(200, "application/json", "{\"heure\":" + String(t.heure) + ",\"minute\":" + String(t.minute) + ",\"seconde\":" + String(t.seconde) + "}");
}

void command(AsyncWebServerRequest *request)
{
  if(request->hasArg("plain"))
  {
    String json = request->arg("plain");
    request->send(200,"text/json","{\"status\" : \"succes\"}");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc,json);
    if(error)
    {
      Serial.println("Erreur de désérialisation");
    }
    manuelCommandState = !manuelCommandState;
    digitalWrite(LAMP_PIN,manuelCommandState);
  }
  else
  {
    request->send(404,"text/json", "{\"status\" : \"echec\"}");
  }
}

void getLampeState(AsyncWebServerRequest *request)
{
  request->send(200,"text/json","{\"etat\": "+String(LampState)+"}");
}

void IRAM_ATTR gestionInterruption()
{
  manuelCommandState = !manuelCommandState;
  digitalWrite(LAMP_PIN,manuelCommandState);
}

void getConfig(AsyncWebServerRequest *request)
{
  request->send(200,"text/json",String("{")+
  "\"allumage\":");
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
    config.onTime = Time{19,0,0,true};
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
  server.on("/lampe",HTTP_POST,command);
  server.on("/getState",HTTP_GET,getLampeState);
  server.on("/get-config",HTTP_GET,getConfig);
  server.begin();

  //attach interrupt
  attachInterrupt(digitalPinToInterrupt(SWITCH_PIN), gestionInterruption, CHANGE);
}

void loop()
{
  if(millis() - now > INTERVALLE)
  {
    printTime(t);
    t = getHeureActuelleToRTC(rtc);
    bool lampState = false;
    updateState(config,lampState,t);
    now = millis();
  }
  LampState = digitalRead(LAMP_PIN);
}
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
#define LAMP2_PIN 15//d8 
#define SWITCH2_PIN 0 //D4

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
volatile bool Lamp2State = false;
volatile bool manuelCommandState = false;
volatile bool manuelCommandState2 = false;

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

void configSetup(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  String json = String((char*)data);
  config = convertToTimeConfig(json);
  config.isvalide = true;
  request->send(200, "application/json", "{\"status\" : \"succes\",\"Données\" : "+json+"}");
  saveTimeConfigToEEPROM(config);
  Serial.println("==================");
  Serial.println("On Time");
  printTime(config.onTime);
  Serial.println("Off Time");
  printTime(config.ofTime);
  Serial.println(json);
}

void timeGet(AsyncWebServerRequest *request)
{
  request->send(200, "application/json", "{\"heure\":" + String(t.heure) + ",\"minute\":" + String(t.minute) + ",\"seconde\":" + String(t.seconde) + "}");
}

void command(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  String json = String((char*)data); 
  //Serial.write(data, len);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc,json);
  if(error)
  {
    Serial.println("Erreur de désérialisation");
    request->send(400, "application/json", "{\"status\":\"echec\"}");
      return;
  }

  request->send(200, "application/json", "{\"status\":\"succes\"}");
  manuelCommandState = !manuelCommandState;
  digitalWrite(LAMP_PIN,manuelCommandState);
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

void IRAM_ATTR gestionSwitch2()
{
    manuelCommandState2 = !manuelCommandState2;
    digitalWrite(LAMP2_PIN,manuelCommandState2);
}

void getConfig(AsyncWebServerRequest *request)
{
  request->send(200,"text/json",String("{")+
  "\"allumage\":"+
  "{\"annee\": 2025,\"mois\": 7,\"jour\": 31,\"heure\":"+String(config.onTime.heure)+",\"minute\":"+String(config.onTime.minute)+",\"seconde\":"+String(config.onTime.seconde)+"},"+
  +"\"extinction\": {\"annee\": 2025,\"mois\":7,\"jour\":31,\"heure\":"+String(config.ofTime.heure)+",\"minute\":"+String(config.ofTime.minute)+",\"seconde\":"+String(config.ofTime.seconde)+"}"+
  +"}");
}

void setupManuallyTime(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  String json = String((char*)data);
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
  pinMode(LAMP2_PIN,OUTPUT);
  digitalWrite(LAMP_PIN,0);
  digitalWrite(LAMP2_PIN,0);
  pinMode(0, INPUT_PULLUP); 
  
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
  server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL, configSetup);
  server.on("/time", HTTP_GET, timeGet);
  server.on("/lampe", HTTP_POST,[](AsyncWebServerRequest *request){}, NULL, command);
  server.on("/state",HTTP_GET,getLampeState);
  server.on("/get-config",HTTP_GET,getConfig);
  server.on("/regler-heure",HTTP_POST,[](AsyncWebServerRequest *request){}, NULL, setupManuallyTime);
  server.begin();

  //attach interrupt
  attachInterrupt(digitalPinToInterrupt(SWITCH_PIN), gestionInterruption, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SWITCH2_PIN), gestionSwitch2, CHANGE);
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
  Lamp2State = digitalRead(LAMP2_PIN);
}
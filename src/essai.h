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
#define LAMP1_PIN 13
#define LAMP2_PIN 12
#define SWITCH1_PIN 2
#define SWITCH2_PIN 3

const char *ssid = "esp8266 wifi";
const char *password = "isniis";

RTC_DS3231 rtc;
unsigned long now = 0;
Time t = {10,20,30,true};
ESP8266WebServer server;
TimeConfig config ;
volatile bool Lamp1State = false;
volatile bool Lamp2State = false;
volatile bool manuelCommandState = false;

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
    server.send(404, "application/json", "{\"status\" : \"echec\"}");
  }
}

void timeGet()
{
  server.send(200, "application/json", "{\"heure\":" + String(t.heure) + ",\"minute\":" + String(t.minute) + ",\"seconde\":" + String(t.seconde) + "}");
}

void command() {
  if (server.hasArg("plain")) {
    String json = server.arg("plain");
    server.send(200,"application/json","{\"status\":\"succes\"}");
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
      Serial.println("Erreur JSON");
      return;
    }
    int lamp = doc["lamp"];
    String action = doc["action"].as<String>();

    if (lamp == 1) {
      if (action == "toggle") Lamp1State = !Lamp1State;
      else if (action == "on") Lamp1State = true;
      else if (action == "off") Lamp1State = false;
      digitalWrite(LAMP1_PIN, Lamp1State);
    }
    else if (lamp == 2) {
      if (action == "toggle") Lamp2State = !Lamp2State;
      else if (action == "on") Lamp2State = true;
      else if (action == "off") Lamp2State = false;
      digitalWrite(LAMP2_PIN, Lamp2State);
    }
  }
  else {
    server.send(404,"application/json","{\"status\":\"echec\"}");
  }
void getLampeState() {
  StaticJsonDocument<200> doc;
  doc["lamp1"] = Lamp1State;
  doc["lamp2"] = Lamp2State;
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void IRAM_ATTR gestionSwitch1()
{
 Lamp1State=! Lamp1State;
    digitalWrite(LAMP1_PIN,Lamp1State);
}
void IRAM_ATTR gestionSwitch2()
{
 Lamp2State=! Lamp2State;
    digitalWrite(LAMP2_PIN,Lamp2State);
}
void getConfig()
{
  server.send(200,"text/json",String("{")+
  "\"allumage\":"+
  "{\"annee\":"+String(config.onTime.annee)+",\"mois\":"+String(config.onTime.mois)+",\"jour\":"+String(config.onTime.jour)+",\"heure\":"+String(config.onTime.heure)+",\"minute\":"+String(config.onTime.minute)+",\"seconde\":"+String(config.onTime.seconde)+"},"+
  +"\"extinction\": {\"annee\":"+String(config.ofTime.annee)+",\"mois\":"+String(config.ofTime.mois)+",\"jour\":"+String(config.ofTime.jour)+",\"heure\":"+String(config.ofTime.heure)+",\"minute\":"+String(config.ofTime.minute)+",\"seconde\":"+String(config.ofTime.seconde)+"}"+
  +"}");
}

void setup()
{
  //INIT du module horloge
    Serial.begin(115200);
  if(!rtc.begin())
  {
    Serial.println("Impossible de trouver le module RTC");
    while (1);
  }

  //init de la lampe
  pinMode(LAMP1_PIN,OUTPUT);
    pinMode(LAMP2_PIN,OUTPUT);
  digitalWrite(LAMP1_PIN,0);
    digitalWrite(LAMP2_PIN,0);
    pinMode(SWITCH1,INPUT_PULLUP);
     pinMode(SWITCH2,INPUT_PULLUP);
  
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
  server.on("/state",HTTP_GET,getLampeState);
  server.on("/get-config",HTTP_GET,getConfig);
  server.begin();

  //attach interrupt
 attachInterrupt(digitalPinToInterrupt(SWITCH1_PIN), gestionSwitch1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SWITCH2_PIN), gestionSwitch2, CHANGE);
}

void loop()
{
  server.handleClient();
  if(millis() - now > INTERVALLE)
  {
    printTime(t);
    t = getHeureActuelleToRTC(rtc);
    bool lamp1State = false;
      bool lamp2State = false;
    updateState(config,lamp1State,t);
       updateState(config,lamp2State,t);
    now = millis();
  }
  Lamp1State = digitalRead(LAMP1_PIN);
     Lamp2State = digitalRead(LAMP2_PIN);
}
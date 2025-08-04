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
#define LAMP1_PIN 13 //d7
#define SWITCH1_PIN 2 //D4
#define LAMP2_PIN 15//d8 
#define SWITCH2_PIN 0 //D3

//SCL D1
//sda d2

const char *ssid = "esp8266 wifi";
const char *password = "isniis";

Time originTime;
RTC_DS3231 rtc;
unsigned long now = 0;
Time t = {10,20,30,true};
AsyncWebServer server(80);
FourConfig fourconfig;//les 4 taches
String configJson;
bool Lamp1State = false;
bool Lamp2State = false;
bool saveLampState = false;
volatile bool manuelCommandState1 = false;
volatile bool manuelCommandState2 = false;
volatile bool interruptOnPin1 = false;
volatile bool interruptOnPin2 = false;

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

void configSetup(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total,const char &which_config)
{
  Serial.println("passer");
  for (size_t i = 0; i < len; i++) {
    configJson += (char)data[i];
  }
  //attendre que la lecture complète du json soit faite , parceque le buffer du EspAsycServer post est limité
  //donc il envoie des morcaux, mais nous fournit aussi l'index du morceau actuelle dans le json complet
  if(index + len == total)
  {
    TimeConfig cfg = convertToTimeConfig(configJson);
    cfg.isvalide = true;
    switch (which_config)
    {
      case 'a' :
        fourconfig.tache_1_lamp_1 = cfg;
        break;

      case 'b' :
        fourconfig.tache_2_lamp_1 = cfg;
        break;

      case 'c' :
        fourconfig.tache_1_lamp_2 = cfg;
        break;

      case 'd' :
        fourconfig.tache_2_lamp_2 = cfg;
        break;

      default:
        Serial.println("Config à assigner inconnu");
        return;
        break;
    }
    request->send(200, "application/json", configJson);
    saveTimeConfigToEEPROM(fourconfig);
    Serial.println("==================");
    Serial.println("On Time");
    printTime(cfg.onTime);
    Serial.println("Off Time");
    printTime(cfg.ofTime);
    Serial.println(configJson);
    configJson.clear();//efface le configJson pour pouvoir l'utiliser pour d'autre config
    //originTime = t; //réinitialiser l'origine des temps
  }
}

void getTime(AsyncWebServerRequest *request)
{
  request->send(200, "application/json", "{\"heure\":" + String(t.heure) + ",\"minute\":" + String(t.minute) + ",\"seconde\":" + String(t.seconde) + "}");
}

void getDate(AsyncWebServerRequest *request)
{
  request->send(200, "application/json", "{\"annee\":" + String(t.heure) + ",\"mois\":" + String(t.minute) + ",\"jour\":" + String(t.seconde) + "}");
}

void commandLamp1(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  String json = String((char*)data); 
  if(index + len != total)
  {
    Serial.println("Débordement du buffer !!");
    return;
  }
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc,json);
  if(error)
  {
    Serial.println("Erreur de désérialisation");
    request->send(400, "application/json", "{\"status\":\"echec\"}");
      return;
  }

  request->send(200, "application/json", json);
  manuelCommandState1 = !manuelCommandState1;
  digitalWrite(LAMP1_PIN,manuelCommandState1);
  //Mise à jour du stockage des états des lampes dans l'eeprom si le stockage est activé
  if(saveLampState)
  {
    LampStates lmpstate;
    lmpstate.oldLamp1State = digitalRead(LAMP1_PIN);
    lmpstate.oldLamp2State = digitalRead(LAMP2_PIN);
    saveOldLampStateToEEPROM(lmpstate);
  }
}

void commandLamp2(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  String json = String((char*)data); 
  if(index + len != total)
  {
    Serial.println("Débordement du buffer !!");
    return;
  }
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc,json);
  if(error)
  {
    Serial.println("Erreur de désérialisation");
    request->send(400, "application/json", "{\"status\":\"echec\"}");
      return;
  }

  request->send(200, "application/json", json);
  manuelCommandState2 = !manuelCommandState2;
  digitalWrite(LAMP2_PIN,manuelCommandState2);
  if(saveLampState)
  {
    LampStates lmpstate;
    lmpstate.oldLamp1State = digitalRead(LAMP1_PIN);
    lmpstate.oldLamp2State = digitalRead(LAMP2_PIN);
    saveOldLampStateToEEPROM(lmpstate);
  }
}


void getLampe1State(AsyncWebServerRequest *request)
{
  request->send(200,"text/json","{\"state\": "+String(Lamp1State)+"}");
}

void getLampe2State(AsyncWebServerRequest *request)
{
  request->send(200,"text/json","{\"state\": "+String(Lamp2State)+"}");
}

void IRAM_ATTR gestionInterruption()
{
  manuelCommandState1 = !manuelCommandState1;
  interruptOnPin1 = true;

}

void IRAM_ATTR gestionSwitch2()
{
    manuelCommandState2 = !manuelCommandState2;
    interruptOnPin2 = true;
}

void getConfig(AsyncWebServerRequest *request)
{
  //Serial.println(json);
  request->send(200, "application/json", configJson);
}

void setupManuallyTime(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  String json =  String((char*)data);
  if(index + len != total)
  {
    Serial.println("Débordement du buffer !!");
    return;
  }
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc,json);
  if(error)
  {
    request->send(200,"text/json","{\"ok\" : false}");
    Serial.println("Erreur de deserialisation");
    return;
  }
  Time t = {
    doc["heure"], doc["minute"], doc["seconde"], true
  };
  setupTimeToRTC(t,rtc);
  request->send(200,"text/json",json);
}

void saveOldState(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  String json = String((char *)data);
  if(index + len != total)
  {
    Serial.println("Débordement du buffer !!");
    return;
  }
  DynamicJsonDocument doc(200);
  DeserializationError error = deserializeJson(doc,json);
  if(error)
  {
    request->send(400,"text/json","{\"ok\" : false}");
    Serial.println("Erreur de deserialisation du json");
    return;
  }
  saveLampState = doc["conserver"] ? true : false;
  request->send(200,"text/json",json);
}

void setupHostPoint(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{

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
  pinMode(LAMP1_PIN,OUTPUT);
  pinMode(LAMP2_PIN,OUTPUT);
  //charger l'état de chaque lampes depuis la mémoire eeprom
  digitalWrite(LAMP1_PIN,loadOldLampesState().oldLamp1State);
  digitalWrite(LAMP2_PIN,loadOldLampesState().oldLamp2State);
  pinMode(SWITCH1_PIN, INPUT_PULLUP); 
  pinMode(SWITCH2_PIN, INPUT_PULLUP); 
  
  //init du port série
  Serial.begin(115200);
  Serial.println("\n\nStarting setup...");

  // Désactiver temporairement le WiFi
  WiFi.mode(WIFI_OFF);
  delay(1000);

  //Chargement de la configuration si elle est dans l'eeprom
  FourConfig fcfg = loadTimeConfigsToEEPROM();
  if(fcfg.isvalide)
  {
    fourconfig = fcfg;
  }
  else //sinon config par défaut
  {
    FourConfig default_config;
    default_config.tache_1_lamp_1 = TimeConfig{Time{12,0,0},Time{12,3,0}};
    default_config.tache_1_lamp_1 = TimeConfig{Time{12,5,0},Time{12,7,0}};
    default_config.tache_1_lamp_1 = TimeConfig{Time{12,8,0},Time{12,9,0}};
    default_config.tache_1_lamp_1 = TimeConfig{Time{12,10,0},Time{12,11,0}};
    fourconfig = default_config;
  }

  // Récupération du temps réel actuel en ligne grâce au gsm
  t = gsm::getNowTime();
  t.valide = true;

  //Initialisation de l'origine des temps
  originTime = t;

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
  server.on("/get-config",HTTP_GET,getConfig);
  server.on("/getTime", HTTP_GET, getTime);//
  server.on("/getDate", HTTP_GET, getDate);//
  server.on("/getState/sortie-1",HTTP_GET,getLampe1State);//
  server.on("/getState/sortie-2",HTTP_GET,getLampe2State);//
  server.on("/setState/sortie-1", HTTP_POST,[](AsyncWebServerRequest *request){}, NULL, commandLamp1);//
  server.on("/setState/sortie-2", HTTP_POST,[](AsyncWebServerRequest *request){}, NULL, commandLamp2);//
  server.on("/sortie-1/tache-1", HTTP_POST, 
            [](AsyncWebServerRequest *request){}, NULL, 
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){configSetup(request,data,len,index,total,'a');});//
  server.on("/sortie-1/tache-2", HTTP_POST, 
            [](AsyncWebServerRequest *request){}, NULL, 
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){configSetup(request,data,len,index,total,'b');});//
  server.on("/sortie-2/tache-1", HTTP_POST,
            [](AsyncWebServerRequest *request){}, NULL, 
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){configSetup(request,data,len,index,total,'c');});//
  server.on("/sortie-2/tache-2", HTTP_POST, 
            [](AsyncWebServerRequest *request){}, NULL, 
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){configSetup(request,data,len,index,total,'d');});//
  server.on("/setTime",HTTP_POST,[](AsyncWebServerRequest *request){}, NULL, setupManuallyTime);//
  server.on("/save",HTTP_POST,[](AsyncWebServerRequest *request){}, NULL, saveOldState);//
  server.on("/setInfos-wifi",HTTP_POST,[](AsyncWebServerRequest *request){},NULL,setupHostPoint);//
  server.begin();

  //attach interrupt
  attachInterrupt(digitalPinToInterrupt(SWITCH1_PIN), gestionInterruption, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SWITCH2_PIN), gestionSwitch2, CHANGE);
}

void loop()
{
  if(millis() - now > INTERVALLE)
  {
    printTime(t);
    t = getHeureActuelleToRTC(rtc);
    updateState(fourconfig.tache_1_lamp_1,t,LAMP1_PIN);
    updateState(fourconfig.tache_2_lamp_1,t,LAMP1_PIN);
    updateState(fourconfig.tache_1_lamp_2,t,LAMP2_PIN);
    updateState(fourconfig.tache_2_lamp_2,t,LAMP2_PIN);
    updateTimeallTwelve(rtc,t,originTime);
    now = millis();
  }
  if(interruptOnPin1)
  {
    interruptOnPin1 = false;
    digitalWrite(LAMP1_PIN,manuelCommandState1);
    if(saveLampState)
    {
      LampStates lmpstate;
      lmpstate.oldLamp1State = digitalRead(LAMP1_PIN);
      lmpstate.oldLamp2State = digitalRead(LAMP2_PIN);
      saveOldLampStateToEEPROM(lmpstate);
    }
  }
  if(interruptOnPin2)
  {
    interruptOnPin2 = false;
    digitalWrite(LAMP2_PIN,manuelCommandState2);
    if(saveLampState)
    {
      LampStates lmpstate;
      lmpstate.oldLamp1State = digitalRead(LAMP1_PIN);
      lmpstate.oldLamp2State = digitalRead(LAMP2_PIN);
      saveOldLampStateToEEPROM(lmpstate);
    }
  }
  Lamp1State = digitalRead(LAMP1_PIN);
  Lamp2State = digitalRead(LAMP2_PIN);

}


//cette fonction doit retourner un pointeur null , jusqu'à ce que qu'elle reçoivent la chaine complète
char* recupall(uint8_t index, uint8_t len, uint8_t total,uint8_t *data)
{
  static char *data_ptr = nullptr;
  static bool isFinish = true;

  if(data_ptr == nullptr)
  {
    data_ptr = new char[len];
    memcpy(data_ptr,(char *)data,len);
  }
  else
  {

  }
  if(index + len != total)
  {
    isFinish = false;
    char *new_space = new char[len*2];
    memcpy(new_space,(char *)data,len);
    data_ptr = new_space;
    data_ptr = (char *)data;
  }
  else
  {
    return data_ptr;
  }
}

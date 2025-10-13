#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <RTClib.h>
#include <LittleFS.h>
#include "def.h"
#include "gsm_manager.h"
#include "rtc_horloge_manager.h"
#include "save.h"
#include "timeManager.h"
#define DEBUG_MODE

#define INTERVALLE 1000
#define LAMP1_PIN 16
#define LAMP2_PIN 13

date actuelDate = {2025,8,12};
SoftwareSerial sim800(14, 12);
Time originTime;
RTC_DS3231 rtc;
unsigned long now = 0;
Time t = {10,20,30,true};
AsyncWebServer server(80);
FourConfig fourconfig;//les 4 taches
String configJson;
String hostPointConfigJson;
bool Lamp1State = false;
bool Lamp2State = false;
SaveStateActivity save_activity = {false,false};
hostPointConfig host_point_config;
volatile bool manuelCommandState1 = false;
volatile bool manuelCommandState2 = false;

void printTime(const Time &t)
{
  #ifdef DEBUG_MODE
    Serial.print(" ");
    Serial.print(t.heure);
    Serial.print(" h ");
    Serial.print(t.minute);
    Serial.print(" min ");
    Serial.print(t.seconde);
    Serial.println(" s");
  #endif
}


void configSetup(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total,const char &which_config)
{
  for (size_t i = 0; i < len; i++) {
    configJson += (char)data[i];
  }
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
    request->send(200, "application/json", "{\"data\" : "+configJson+"}");
    fourconfig.isvalide = true;
    saveTimeConfigToEEPROM(fourconfig);
    Serial.println(fourconfig.isvalide);
    Serial.println(configJson);
    Serial.println("=======================");
    PrintFourConfig(fourconfig);
    configJson.clear();
  }
}

void getTime(AsyncWebServerRequest *request)
{
  request->send(200, "application/json", "{ \"data\" : {\"heure\":" + String(t.heure) + ",\"minute\":" + String(t.minute) + ",\"seconde\":" + String(t.seconde) + "}}");
}

void getDate(AsyncWebServerRequest *request)
{
  request->send(200, "application/json", "{ \"data\" : {\"annee\":"+String(actuelDate.annee)+",\"mois\":"+String(actuelDate.mois)+",\"jour\":"+String(actuelDate.jour)+"}}");
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

  request->send(200, "application/json","{\"data\" : {\"state\" : "+String(Lamp1State)+"}}");
  Serial.println("{\"data\" : {\"state\" : "+String(Lamp1State)+"}}");
  manuelCommandState1 = !manuelCommandState1;
  digitalWrite(LAMP1_PIN,manuelCommandState1);
  LampStates lmpstate;
  lmpstate.oldLamp1State = (save_activity.checkLampe1) ? digitalRead(LAMP1_PIN) : false;
  lmpstate.oldLamp2State =  (save_activity.ckeckLampe2) ? digitalRead(LAMP2_PIN) : false;
  if(save_activity.checkLampe1 || save_activity.ckeckLampe2)
      saveOldLampStateToEEPROM(lmpstate);
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

  request->send(200, "application/json", "{\"data\" : {\"state\" : "+String(Lamp2State)+"}}");
  manuelCommandState2 = !manuelCommandState2;
  digitalWrite(LAMP2_PIN,manuelCommandState2);
  LampStates lmpstate;
  lmpstate.oldLamp1State = (save_activity.checkLampe1) ? digitalRead(LAMP1_PIN) : false;
  lmpstate.oldLamp2State =  (save_activity.ckeckLampe2) ? digitalRead(LAMP2_PIN) : false;
    if(save_activity.checkLampe1 || save_activity.ckeckLampe2)
      saveOldLampStateToEEPROM(lmpstate);
}


void getLampe1State(AsyncWebServerRequest *request)
{
  request->send(200,"text/json","{\"data\" : {\"state\": "+String(Lamp1State)+"}}");
}

void getLampe2State(AsyncWebServerRequest *request)
{
  request->send(200,"text/json","{\"data\" : {\"state\": "+String(Lamp2State)+"}}");
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
  String retour;
  serializeJson(doc,retour);
  setupTimeToRTC(t,rtc,actuelDate);
  request->send(200,"text/json","{\"data\" : "+retour+"}");
  Serial.println("{\"data\" : ""}");
}

void saveOldState(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total,uint8_t lamp_nbr)
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
  if(lamp_nbr == 1)
  {
    save_activity.checkLampe1 = doc["save"] ? true : false;
  }
  else if(lamp_nbr == 2)
     save_activity.ckeckLampe2 = doc["save"] ? true : false;
  else 
    Serial.println("lamp a été sauvergarde inconnu");
  
  String retour;
  serializeJson(doc,retour);
  request->send(200,"text/json","{\"data\" : "+retour+"}");
  saveCheckActivitytoEEPROM(save_activity);
}

void setupHostPoint(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  for (size_t i = 0; i < len; i++) {
    hostPointConfigJson += (char)data[i];
  }
   if(index + len == total)
   {
    Serial.println(hostPointConfigJson);
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc,hostPointConfigJson);
    if(error)
    {
      Serial.println("Erreur de désérialisation ");
      return;
    }
    host_point_config.ssid = String(doc["ssid"]);
    host_point_config.password = String(doc["password"]);
    WiFi.softAPdisconnect(true);
    request->send(200,"text/json","{\"data\" : "+hostPointConfigJson+"}");
    delay(1000);
    WiFi.softAP(host_point_config.ssid, host_point_config.password,1,false);
    host_point_config.isvalide = true;
    saveHostPointConfigtoEEPROM(host_point_config);
   }
}

void getSaveState(AsyncWebServerRequest *request,uint8_t nbr_lamp)
{
  if(nbr_lamp == 1)
  {
    String str = "{\"data\" : {\"save\" : ";
    str += (save_activity.checkLampe1) ? String("true") : String("false");
    str += "}}";
    request->send(200,"text/json",str);
  }
  else if(nbr_lamp == 2)
  {
    String str = "{\"data\" : {\"save\" : ";
    str += (save_activity.ckeckLampe2) ? String("true") : String("false");
    str += "}}";
    request->send(200,"text/json",str);
  }
  else
  {
    Serial.println("Etat de la lampe inconnu");
    return;
  }
}

void getCalendarConfigs(AsyncWebServerRequest *request)
{

   String json =   "{"
        "\"success\": true,"
        "\"data\": {"
            "\"sortie-1\": {"
                "\"tache-1\": {"
                    "\"allumage\": {"
                        "\"heure\": "+String(fourconfig.tache_1_lamp_1.onTime.heure)+","
                        "\"minute\": "+String(fourconfig.tache_1_lamp_1.onTime.minute)+","
                        "\"seconde\": "+String(fourconfig.tache_1_lamp_1.onTime.seconde)+""
                    "},"
                    "\"extinction\": {"
                        "\"heure\": "+String(fourconfig.tache_1_lamp_1.ofTime.heure)+","
                        "\"minute\": "+String(fourconfig.tache_1_lamp_1.ofTime.minute)+","
                        "\"seconde\": "+String(fourconfig.tache_1_lamp_1.ofTime.seconde)+""
                    "}"
                "},"
                "\"tache-2\": {"
                    "\"allumage\": {"
                        "\"heure\": "+String(fourconfig.tache_2_lamp_1.onTime.heure)+","
                        "\"minute\": "+String(fourconfig.tache_2_lamp_1.onTime.minute)+","
                        "\"seconde\": "+String(fourconfig.tache_2_lamp_1.onTime.seconde)+""
                    "},"
                    "\"extinction\": {"
                        "\"heure\": "+String(fourconfig.tache_2_lamp_1.ofTime.heure)+","
                        "\"minute\": "+String(fourconfig.tache_2_lamp_1.ofTime.minute)+","
                        "\"seconde\": "+String(fourconfig.tache_2_lamp_1.ofTime.seconde)+""
                    "}"
                "}"
            "},"
            "\"sortie-2\": {"
                "\"tache-1\": {"
                    "\"allumage\": {"
                        "\"heure\": "+String(fourconfig.tache_1_lamp_2.onTime.heure)+","
                        "\"minute\": "+String(fourconfig.tache_1_lamp_2.onTime.minute)+","
                        "\"seconde\": "+String(fourconfig.tache_1_lamp_2.onTime.seconde)+""
                    "},"
                    "\"extinction\": {"
                        "\"heure\": "+String(fourconfig.tache_1_lamp_2.ofTime.heure)+","
                        "\"minute\": "+String(fourconfig.tache_1_lamp_2.ofTime.minute)+","
                        "\"seconde\": "+String(fourconfig.tache_1_lamp_2.ofTime.seconde)+""
                    "}"
                "},"
                "\"tache-2\": {"
                    "\"allumage\": {"
                        "\"heure\": "+String(fourconfig.tache_2_lamp_2.onTime.heure)+","
                        "\"minute\":  "+String(fourconfig.tache_2_lamp_2.onTime.minute)+","
                        "\"seconde\":  "+String(fourconfig.tache_2_lamp_2.onTime.seconde)+""
                    "},"
                    "\"extinction\": {"
                        "\"heure\":  "+String(fourconfig.tache_2_lamp_2.ofTime.heure)+","
                        "\"minute\":  "+String(fourconfig.tache_2_lamp_2.ofTime.minute)+","
                        "\"seconde\":  "+String(fourconfig.tache_2_lamp_2.ofTime.seconde)+""
                    "}"
                "}"
            "}"
        "}"
   " }";

  request->send(200,"text/json",json);
}

void setup()
{
  Serial.begin(115200);

  sim800.begin(9600);
  delay(1000);


  sim800.println("AT+CLTS=1");
  delay(1000);

  sim800.println("AT+CFUN=1,1");
  delay(10000);

  if(!rtc.begin())
  {
    Serial.println("Impossible de trouver le module RTC");
  }
  delay(3000);

  save_activity = loadCheckActivitytoEEPROM();

  pinMode(LAMP1_PIN,OUTPUT);
  pinMode(LAMP2_PIN,OUTPUT);
  bool state1 = loadOldLampesStatetoEEPROM().oldLamp1State;
  bool state2 = loadOldLampesStatetoEEPROM().oldLamp2State;
  digitalWrite(LAMP1_PIN,state1);
  digitalWrite(LAMP2_PIN,state2);

  WiFi.mode(WIFI_OFF);
  delay(1000);

  FourConfig fcfg = loadTimeConfigsToEEPROM();
  if(fcfg.isvalide)
  {
    fourconfig = fcfg;
  }
  else
  { 
    Serial.println("passé dans le else");
    FourConfig default_config;
    default_config.tache_1_lamp_1 = TimeConfig{Time{19,28,0},Time{0,0,0}};
    default_config.tache_2_lamp_1 = TimeConfig{Time{0,0,0},Time{7,0,0}};
    default_config.tache_1_lamp_2 = TimeConfig{Time{19,28,0},Time{0,0,0}};
    default_config.tache_2_lamp_2 = TimeConfig{Time{0,0,0},Time{7,0,0}};
    fourconfig = default_config;
  }

  PrintFourConfig(fourconfig);

  t = gsm::getNetworkTime(sim800,actuelDate);
  if (t.heure == -1)
     Serial.println("Erreur réseau lors de la recup de l'heure");
  t.valide = true;

  originTime = t;

  setupTimeToRTC(t,rtc,actuelDate);
  printTime(t);

  hostPointConfig hostcfg = loadHostPointConfigtoEEPROM();
  if(hostcfg.isvalide)
  {
    host_point_config = hostcfg;
  }
  else
  { 
    host_point_config.ssid = "youpilight_esp";
    host_point_config.password = "123456789A";
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(host_point_config.ssid, host_point_config.password,1,false);
  IPAddress ip = WiFi.softAPIP();

  if(!LittleFS.begin())
  {
    Serial.println("Erreur d'initialisation de little fs");
    return;
  }

  server.on("/",HTTP_GET, [](AsyncWebServerRequest *request){
     Serial.println("Passer dans le get");
     request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/css", LittleFS, "/css");
  server.serveStatic("/js", LittleFS, "/js");
  server.serveStatic("/html", LittleFS, "/html");

  server.on("/getSaveState/sortie-1",HTTP_GET,[](AsyncWebServerRequest *request){getSaveState(request,1);});
  server.on("/getSaveState/sortie-2",HTTP_GET,[](AsyncWebServerRequest *request){getSaveState(request,2);});
  server.on("/getTime", HTTP_GET, getTime);
  server.on("/getDate", HTTP_GET, getDate);
  server.on("/getCalendarConfigs", HTTP_GET, getCalendarConfigs);
  server.on("/sortie-1/getState",HTTP_GET,getLampe1State);
  server.on("/sortie-2/getState",HTTP_GET,getLampe2State);
  server.on("/sortie-1/setState", HTTP_POST,[](AsyncWebServerRequest *request){}, NULL, commandLamp1);
  server.on("/sortie-2/setState", HTTP_POST,[](AsyncWebServerRequest *request){}, NULL, commandLamp2);
  server.on("/sortie-1/tache-1", HTTP_POST, 
            [](AsyncWebServerRequest *request){}, NULL, 
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){configSetup(request,data,len,index,total,'a');});
  server.on("/sortie-1/tache-2", HTTP_POST, 
            [](AsyncWebServerRequest *request){}, NULL, 
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){configSetup(request,data,len,index,total,'b');});
  server.on("/sortie-2/tache-1", HTTP_POST,
            [](AsyncWebServerRequest *request){}, NULL, 
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){configSetup(request,data,len,index,total,'c');});
  server.on("/sortie-2/tache-2", HTTP_POST, 
            [](AsyncWebServerRequest *request){}, NULL, 
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){configSetup(request,data,len,index,total,'d');});
  server.on("/setTime",HTTP_POST,[](AsyncWebServerRequest *request){}, NULL, setupManuallyTime);
  server.on("/save/sortie-1",HTTP_POST,[](AsyncWebServerRequest *request){}, NULL,
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){saveOldState(request,data,len,index,total,1);});
  server.on("/save/sortie-2",HTTP_POST,[](AsyncWebServerRequest *request){}, NULL,
            [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){saveOldState(request,data,len,index,total,2);});
  server.on("/setInfos-wifi",HTTP_POST,[](AsyncWebServerRequest *request){},NULL,setupHostPoint);
  
  server.begin();

}

void loop()
{
  if(millis() - now > INTERVALLE)
  {
    printTime(t);
    t = getHeureActuelleToRTC(rtc,actuelDate);
    updateState(fourconfig.tache_1_lamp_1,t,LAMP1_PIN);
    updateState(fourconfig.tache_2_lamp_1,t,LAMP1_PIN);
    updateState(fourconfig.tache_1_lamp_2,t,LAMP2_PIN);
    updateState(fourconfig.tache_2_lamp_2,t,LAMP2_PIN);
    now = millis();
  }
  Lamp1State = digitalRead(LAMP1_PIN);
  Lamp2State = digitalRead(LAMP2_PIN);

}
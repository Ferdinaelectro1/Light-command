#pragma once
//définition de l'entête indiquand le type de module gsm qui serait utilisé
//ici le SIM800L
#define TINY_GSM_MODEM_SIM800
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include "secret.h"
#include "def.h"

#define RX 14 // GPIO14 = D5
#define TX 12 // GPIO12 = D6

const char apn[] = "internet.mtn.bj";
const  String url ="/v2.1/get-time-zone?key="+API_KEY+"&format=json&by=zone&zone=Africa/Porto-Novo";


namespace gsm
{
    
    Time getNowTime()
    {
        //Ce bloc permet d'initialiser le module gsm
        //D'initialiser un client en se basant sur ce module
        // Et de faire les configurations nécessaires pour faire la requête vers l'api de récup de l'heure
        Time now = {0,0,0,false};
        SoftwareSerial gsm_module(RX, TX);
        TinyGsm modem(gsm_module);
        TinyGsmClient client(modem);
        HttpClient http(client, "api.timezonedb.com", 80);
        gsm_module.begin(9600);
        Serial.println("Initialisation du modem...");
        modem.restart();
        delay(3000);
        Serial.println("Connexion au réseau...");
        if (!modem.gprsConnect(apn, "", "")) {
            Serial.println("Échec GPRS");
            now.valide = false;
            return now;
        }
        
        //lancement de la requête verss l'api
        Serial.println("Requête HTTP...");
        http.get(url);

        int statusCode = http.responseStatusCode();
        String response;
        if(statusCode == 200)
            response = http.responseBody();
        else 
            Serial.println("Erreur lors de la requête");

/*         Serial.print("Code HTTP: ");
        Serial.println(statusCode);
        Serial.println("Réponse JSON:");
        Serial.println(response); */

        //jsonification de la réponse envoyé par l'api
        modem.gprsDisconnect();
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc,response);
        if(error)
        {
            now.valide = false;
            return now;
        }

        //Ce bloc sert à extraire l'heure de la chaine de caractère envoyé par l'api.
        String data = doc["formatted"];
        String date = data.substring(0,10);
        data = data.substring(11,19);
        String para;
        String dataLists[3];
        byte pos = 0;
        for(byte i = 0; i < data.length() ; i++)
        {
            if(data[i] == ':' || (i+1 == data.length())) 
            {
                para += (i+1 == data.length()) ? data[i] : ' ';
                //Serial.println(para);
                dataLists[pos] = para;
                pos ++; 
                para = "";
            }
            else
              para += data[i];
        }

        now = Time{date.substring(0,4).toInt(),date.substring(5,7).toInt(),date.substring(8,10).toInt(),dataLists[0].toInt(),dataLists[1].toInt(),dataLists[2].toInt(),true};
        Serial.println(date.substring(8,9));
        return now;
    }

}//end gsm
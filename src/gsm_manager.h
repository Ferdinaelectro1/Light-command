#pragma once
//définition de l'entête indiquand le type de module gsm qui serait utilisé
//ici le SIM800L
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include "def.h"

#define RX 14 // GPIO14 = D5
#define TX 12 // GPIO12 = D6

/* const char apn[] = "internet.mtn.bj";
const  String url ="/v2.1/get-time-zone?key="+API_KEY+"&format=json&by=zone&zone=Africa/Porto-Novo"; */


namespace gsm
{
    
/*     Time getNowTime()
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
        //3 essai de connexion max , après 3 essai on continue 
        for(byte i = 0; i < 3; i++)
        {
            if(!modem.gprsConnect(apn, "", ""))
            {
                Serial.println("Échec GPRS");
                now.valide = false;
                modem.restart();
                delay(3000);
                Serial.println("Connexion au réseau...");
            }
            else
              return Time{0,0,0,false};
        }
        //lancement de la requête verss l'api
        Serial.println("Requête HTTP...");
        http.get(url);

        int statusCode = http.responseStatusCode();
        String response = http.responseBody();

/*         Serial.print("Code HTTP: ");
        Serial.println(statusCode);
        Serial.println("Réponse JSON:");
        Serial.println(response); */

        //jsonification de la réponse envoyé par l'api
    /*    modem.gprsDisconnect();
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

        now = Time{dataLists[0].toInt(),dataLists[1].toInt(),dataLists[2].toInt(),true};
        Serial.println(date.substring(8,9));
        return now;
    } */

//fonction2
Time getNetworkTime(SoftwareSerial &sim800,date &date) {
  Time currentTime = {-1, -1, -1}; // Valeurs par défaut si erreur
  date = {0,0,0}; 

  sim800.println("AT+CCLK?");
  delay(500);

  String response = "";
  while (sim800.available()) {
    char c = sim800.read();
    response += c;
    //Serial.println(c);
  }

  // Exemple de réponse attendue : +CCLK: "25/08/06,08:45:30+04"
  int index = response.indexOf("\"");
  if (index != -1) {
    String dateTimeStr = response.substring(index + 1, response.indexOf("\"", index + 1));

    // On sépare la partie temps
    int timeSepIndex = dateTimeStr.indexOf(",");
    if (timeSepIndex != -1) {
      String timePart = dateTimeStr.substring(timeSepIndex + 1); // "08:45:30+04"
      int h = timePart.substring(0, 2).toInt();
      int m = timePart.substring(3, 5).toInt();
      int s = timePart.substring(6, 8).toInt();

      currentTime.heure = h;
      currentTime.minute = m;
      currentTime.seconde = s;
    
      String datePart = dateTimeStr.substring(0, timeSepIndex); // "25/08/06"
      int yy = datePart.substring(0, 2).toInt();
      int mm = datePart.substring(3, 5).toInt();
      int dd = datePart.substring(6, 8).toInt();
      date.annee = yy;
      date.mois = mm;
      date.jour = dd;
    }
  }
    return currentTime;
  }

}//end gsm
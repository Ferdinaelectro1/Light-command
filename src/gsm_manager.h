#pragma once
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include "def.h"

#define RX 14
#define TX 12


namespace gsm
{

Time getNetworkTime(SoftwareSerial &sim800,date &date) {
  Time currentTime = {-1, -1, -1};
  date = {0,0,0}; 

  sim800.println("AT+CCLK?");
  delay(500);

  String response = "";
  while (sim800.available()) {
    char c = sim800.read();
    response += c;
  }

  int index = response.indexOf("\"");
  if (index != -1) {
    String dateTimeStr = response.substring(index + 1, response.indexOf("\"", index + 1));

    int timeSepIndex = dateTimeStr.indexOf(",");
    if (timeSepIndex != -1) {
      String timePart = dateTimeStr.substring(timeSepIndex + 1);
      int h = timePart.substring(0, 2).toInt();
      int m = timePart.substring(3, 5).toInt();
      int s = timePart.substring(6, 8).toInt();

      currentTime.heure = h;
      currentTime.minute = m;
      currentTime.seconde = s;
    
      String datePart = dateTimeStr.substring(0, timeSepIndex);
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

}
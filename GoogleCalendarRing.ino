/*
  Diego Avellaneda Torres -> @diego_creates
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  In this file is the main code for the Google Calendar productivity ring created
  to remind daily the tasks and meetings
*/
#include <ring_secrets.h>
#include <HTTPClient.h>
#include "WiFi.h"
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define PIN 13 // Pin for NeopixelRing Din pin 

#define NUMPIXELS 24 // Popular NeoPixel ring size
#define FIXED_TIME_TO_EVENT 30
#define FIRST_MINUTE_TH 20
#define SECOND_MINUTE_TH 10
#define THIRD_MINUTE_TH 5

// NeoPixel ring information before initialization
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = SSID;
const char* password = PASS;

String GOOGLE_SCRIPT_ID = CALENDAR_URL;
String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?read";

HTTPClient http;

DynamicJsonDocument doc(1024);

int requestTimeInterval = 60000; 
unsigned long currentTime = 0;
unsigned long previousTime = 0; 

void setup() {

  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif

  Serial.begin(115200);
  pixels.begin();
  pixels.clear();
  pixels.show();

  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);
  Serial.flush();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
  connectedLoop();

}

void loop() {

  currentTime=millis();

  if((currentTime - previousTime) > requestTimeInterval){
  previousTime=currentTime;
  Serial.println("Making a request");
  http.begin(url.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  int httpCode = http.GET();
  String payload;

  if (httpCode > 0) { //Check for the returning code
    payload = http.getString();
    Serial.println(httpCode);
    Serial.println(payload);
    deserializeJson(doc, payload);
    JsonObject obj = doc.as<JsonObject>();
    bool isNextEvent = obj["nextEvent"]["isNextEvent"];
    bool isCurrentEvent = obj["currentEvent"]["isCurrentEvent"];
    pixels.clear();
      
    if(isCurrentEvent){
      int timeRange = obj["currentEvent"]["timeRange"];
      int spentTime = obj["currentEvent"]["spentTime"];
    }

    if(isNextEvent){
      int timeToEvent = obj["nextEvent"]["timeToEvent"];
      showNextEvent(timeToEvent);
    }

    else{
      pixels.setPixelColor(10, pixels.Color(150, 0, 0));             
    }
  }
  else {
    Serial.println("Error on HTTP request");
  }
  http.end();
  }
  delay(10);  
}

void connectedLoop(){
  pixels.clear(); 
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 150, 0));
    delay(10);
    pixels.show();
  }
  pixels.clear(); 
  pixels.show();
}

void showNextEvent(int timeToEvent) {

  if(timeToEvent < FIXED_TIME_TO_EVENT){

    int closestEvent = FIXED_TIME_TO_EVENT - timeToEvent;
    int mappedNumberOfPixels = map(closestEvent,0,FIXED_TIME_TO_EVENT,0,NUMPIXELS);
    Serial.print("el time to ");
    Serial.print(timeToEvent);
    Serial.print(" ");
    Serial.println(mappedNumberOfPixels);
    pixels.clear(); 
    for(int i=0; i<mappedNumberOfPixels; i++) {
      if(timeToEvent > FIRST_MINUTE_TH ){
        Serial.println("En mayor a 20");
        pixels.setPixelColor(i, pixels.Color(0, 150, 0));
      }
      else if(timeToEvent > SECOND_MINUTE_TH){
        Serial.println("En mayor a 10");
        pixels.setPixelColor(i, pixels.Color(0, 0, 150));  
      }
      else if(timeToEvent > THIRD_MINUTE_TH){
         Serial.println("En mayor a 5");
        pixels.setPixelColor(i, pixels.Color(150, 150, 30));  
      }
      else{
         pixels.setPixelColor(i, pixels.Color(150, 0, 0));  
      }  
    }
    pixels.show();
  }
}

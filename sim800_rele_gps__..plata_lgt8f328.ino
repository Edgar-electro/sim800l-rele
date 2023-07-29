#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <TinyGPS++.h>
#include <SIM800L.h>
#include <TimerOne.h>
TinyGPSPlus gps;
AltSoftSerial serial_gps;  // Rx, Tx for d8 d9
SIM800L sim800l(2, 3); // Rx, Tx for GSM module
#define LED_PIN 7
const int analogPin = A1;
const int voltageThreshold = 900; 
boolean previousState = false;

boolean STATE_LEDPIN = false;

unsigned long previousMillis = 0;   
const unsigned long interval = 600000;






double latitude, longitude; 

String getlocation  ;


const String PHON = "0769888360";
const String PHONE = "+33769888360";


void handleSMS(String number, String message) {
  Serial.println("number: " + number + "\nMessage: " + message);
  if (number == PHONE) {
    if (message == "on") {
      digitalWrite(LED_PIN, LOW);
      sim800l.sendSMS(number, "LED is ON ");
      STATE_LEDPIN = 1;
    } else if (message == "off") {
      digitalWrite(LED_PIN, HIGH);
      sim800l.sendSMS(number, "LED is OFF ");
      STATE_LEDPIN = 0;
    } else if (message == "status") {
      String text = (STATE_LEDPIN) ? "ON" : "OFF";
      sim800l.sendSMS(number, "STATUS IS " + text);
    } else if (message == "location") {
      String text = "YOUR LOCATION ";
      sim800l.sendSMS(number, getlocation + text);
    } else if (message == "batstat") {
      String text = (previousState) ? "ON" : "OFF";
      sim800l.sendSMS(number, "BATTERY STAT " + text);
    }

    
  }
}

void handleCall(String number) {
  Serial.println("New call from " + number);
  if (number == PHON) {
    boolean flag = digitalRead(LED_PIN);
    digitalWrite(LED_PIN, !flag);
    delay(500);
    String text = (flag) ? "ON" : "OFF";
    sim800l.sendSMS("0769888360", "LED STAT " + text);
  } else {
    Serial.println("Unkown phone number.");
  }
}



void setup() {
  delay(5000);
  Serial.begin(9600);
  Serial.println("Arduino serial initialize");
  delay(500);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  pinMode(analogPin, INPUT);
  delay(500);
  serial_gps.begin(9600);
  sim800l.begin(9600);
  sim800l.setSMSCallback(handleSMS);
  sim800l.setCallCallback(handleCall);
  delay(7000);
  sim800l.sendSMS(PHONE, "DEVICE ONLINE ");
 
}

void loop() {
         
         GPS();
       
       sim800l.listen();
       
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      AUTOSENDLOCAL(); 
      }
      

      
      BATTERISTAT();
      
  }

 void GPS(){
    if (serial_gps.available()) {
    gps.encode(serial_gps.read()); 
    }  
      if (gps.location.isUpdated()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        getlocation = "http://maps.google.com/maps?q=loc:" + String(latitude, 6) + "," + String(longitude, 6) ;
        Serial.println(getlocation);
      }
   
    
    }
 
 
 void AUTOSENDLOCAL(){
      sim800l.sendSMS(PHONE, getlocation);
 
   } 
 
 void BATTERISTAT(){
  
  for(int i=0;i<10;i++){
  int sensorValue = analogRead(analogPin);
  
  
  if (sensorValue >= voltageThreshold && !previousState) {
    sim800l.sendSMS(PHONE, "BATTERI ON");
    previousState = true;
  
  } else if (sensorValue < voltageThreshold && previousState) {
    sim800l.sendSMS(PHONE, "BATTERI OFF");
    previousState = false;
 
  } 
  
  
  
 }
}


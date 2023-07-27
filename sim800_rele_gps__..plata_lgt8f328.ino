#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <TinyGPS++.h>
#include <SIM800L.h>
#include <TimerOne.h>
TinyGPSPlus gps;
AltSoftSerial serial_gps;
const int mybaud = 245;

SIM800L sim800l(2, 3); // Rx, Tx for GSM module

#define LED_PIN 7

unsigned long previousMillis = 0;   // Переменная для хранения времени последнего вызова функции
const unsigned long interval = 1800000;


double latitude, longitude; 

String getlocation  ;

const String PHONE = "+33769888360";
boolean STATE_LEDPIN = 0;

void handleSMS(String number, String message) {
  Serial.println("number: " + number + "\nMessage: " + message);
  if (number == PHONE) {
    if (message == "on") {
      digitalWrite(LED_PIN, LOW);
      sim800l.sendSMS(number, "LED is ON");
      STATE_LEDPIN = 1;
    } else if (message == "off") {
      digitalWrite(LED_PIN, HIGH);
      sim800l.sendSMS(number, "LED is OFF");
      STATE_LEDPIN = 0;
    } else if (message == "status") {
      String text = (STATE_LEDPIN) ? "ON" : "OFF";
      sim800l.sendSMS(number, "STATUS IS " + text);
    } else if (message == "location") {
      String text = "YOUR LOCATION";
      sim800l.sendSMS(number, getlocation + text);
    }
  }
}

void handleCall(String number) {
  Serial.println("New call from " + number);
  if (number == PHONE) {
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
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  serial_gps.begin(9600);
  sim800l.begin(9600);
  sim800l.setSMSCallback(handleSMS);
  sim800l.setCallCallback(handleCall);
  
}

void loop() {
  GPS();
  
  sim800l.listen();



 unsigned long currentMillis = millis();
if (currentMillis - previousMillis >= interval) {
    // Сохраняем текущее время как время последнего вызова функции
    previousMillis = currentMillis;
    
    // Вызываем вашу функцию, которую нужно выполнить каждую минуту
    AUTOSENDLOCAL(); // Замените yourFunction() на имя вашей функции
  }
}

void AUTOSENDLOCAL(){
sim800l.sendSMS(PHONE, getlocation);
 } 
void GPS(){
if (serial_gps.available()) {
    gps.encode(serial_gps.read()); 
    }  
      if (gps.location.isUpdated()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
        getlocation = "https://www.google.com/maps?q=" + String(latitude, 6) + "," + String(longitude, 6) ;
        Serial.println(getlocation);
      }
   
    
    }

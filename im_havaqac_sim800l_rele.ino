#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <TinyGPS++.h>
#include <SIM800L.h>

TinyGPSPlus gps;
SoftwareSerial gpsSerial(8, 9); // Rx, Tx for GPS module
SIM800L sim800l(2, 3); // Rx, Tx for GSM module

#define LED_PIN 7
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
    } else if (message == "Get Location") {
      String location = getLocation();
      String googleMapsLink = getGoogleMapsLink(location);
      sim800l.sendSMS(number, googleMapsLink);
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
    sim800l.sendSMS(number, "LED STAT " + text);
  } else {
    Serial.println("Unkown phone number.");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  gpsSerial.begin(9600);
  sim800l.begin(9600);
  sim800l.setSMSCallback(handleSMS);
  sim800l.setCallCallback(handleCall);
}

void loop() {
  sim800l.listen();
}

String getLocation() {
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        float latitude = gps.location.lat();
        float longitude = gps.location.lng();
        String location = String(latitude, 6) + "," + String(longitude, 6);
        return location;
      }
    }
  }
  return "Location not available";
}

String getGoogleMapsLink(String location) {
  return "https://www.google.com/maps?q=" + location;
}

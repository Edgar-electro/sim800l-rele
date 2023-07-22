/*************************************************************************************
 *  Created By: Tauseef Ahmad
 *  Created On: 15 April, 2023
 *  
 *  Tutorial: https://youtu.be/lYN8hqNAKbU
 *  My Channel: https://www.youtube.com/channel/UCOXYfOHgu-C-UfGyDcu5sYw/
 ***********************************************************************************/
 
#include <SIM800L.h>

SIM800L sim800l(2, 3); //Rx, Tx

#define LED_PIN 7
const String PHONE = "+33769888360";
boolean STATE_LEDPIN = 0;



void handleSMS(String number, String message) {
  Serial.println("number: " + number + "\nMessage: " + message);
  if(number == PHONE){
    if(message == "on") {
      digitalWrite(LED_PIN, LOW);
      sim800l.sendSMS(number, "LED is ON");
      STATE_LEDPIN = 1;
    } 
    else if(message == "off") {
      digitalWrite(LED_PIN, HIGH);
      sim800l.sendSMS(number, "LED is OFF");
     STATE_LEDPIN = 0;
    }
  }
  
  if(number == PHONE)  {
  if(message == "status"){
    String text = (STATE_LEDPIN)? "ON" : "OFF";
    sim800l.sendSMS(number,"STATUS IS "+text);
  }
    
}
}
void handleCall(String number) {
  Serial.println("New call from " + number);
  if(number == "0769888360"){
    boolean flag = digitalRead(LED_PIN);
    digitalWrite(LED_PIN, !flag);
  delay (500);
  String text = (flag)? "ON" : "OFF";
    sim800l.sendSMS(number,"LED STAT "+text);
  
  }
  else {
    Serial.println("Unkown phone number.");
  }
}

void setup() {
  Serial.begin(9600);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  sim800l.begin(9600);
  
  sim800l.setSMSCallback(handleSMS);
  sim800l.setCallCallback(handleCall);
}

void loop() {
  sim800l.listen();
}

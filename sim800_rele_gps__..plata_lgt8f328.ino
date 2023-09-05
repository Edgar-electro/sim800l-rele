 


#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <TinyGPS++.h>

//sender phone number with country code
const String PHONE = "+33769888360";


#define RELAY_1 5
#define STATUS_PIN  6
const int analogInputPin = A0;  // Входной пин для измерения напряжения
const float maxVoltage = 42.0;  // Максимальное входное напряжение (вольты)
const float minVoltage = 30.0;  // Минимальное входное напряжение (вольты)



//GSM Module RX pin to Arduino 10
//GSM Module TX pin to Arduino 11
#define rxPin 2
#define txPin 3
SoftwareSerial sim800(rxPin, txPin);

//GPS Module RX pin to Arduino 9
//GPS Module TX pin to Arduino 8
AltSoftSerial neogps;

TinyGPSPlus gps;

String sms_status, sender_number, received_date, msg;



boolean lastScooterState = false; 
int batteryPercentage = 0; 
float voltage = 0;



void setup() {
  delay(5000);
  
  Serial.begin(115200);
  Serial.println("Arduino serial initialize");
  
  sim800.begin(9600);
  neogps.begin(9600);
  Serial.println("neogps sim800l Software serial initialize");
  delay(1000);
  sim800.print("AT+CMGF=1\r"); //SMS text mode
  delay(1000);
  //delete all sms
  sim800.println("AT+CMGD=1,4");
  delay(1000);
  sim800.println("AT+CMGDA= \"DEL ALL\"");
  
   
  
  delay(1000);
  pinMode(RELAY_1, OUTPUT);
  digitalWrite(RELAY_1, LOW);
  pinMode(STATUS_PIN, INPUT_PULLUP);
  pinMode(STATUS_PIN, LOW);
  analogRead(analogInputPin);
  delay(1000);
  sms_status = "";
  sender_number = "";
  received_date = "";
  msg = "";
  delay(1000);
 sendSms("GPS Tracker is Online"); 
}

void loop() {
  
  while (sim800.available()) {
    parseData(sim800.readString());
  }
  while (Serial.available())  {
    sim800.println(Serial.readString());
  }
DIGITALSTAT();
}

void parseData(String buff) {
  

  
  Serial.println(buff);

  unsigned int len, index;
  index = buff.indexOf("\r");
  buff.remove(0, index + 2);
  buff.trim();
  if (buff != "OK") {
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();

    buff.remove(0, index + 2);
    Serial.println(buff);

    if (cmd == "+CMTI") {
      //get newly arrived memory location and store it in temp
      //temp = 4
      index = buff.indexOf(",");
      String temp = buff.substring(index + 1, buff.length());
      temp = "AT+CMGR=" + temp + "\r";
      //AT+CMGR=4 i.e. get message stored at memory location 4
      sim800.println(temp);
    }
    else if (cmd == "+CMGR") {
      extractSms(buff);
      Serial.println("extractSms Success.");
      if (sender_number == PHONE) {
        //Serial.println("doAction");
        doAction();
        deleteSms();
      }
    }

  }
  else {
  }
}
void extractSms(String buff) {
  unsigned int index;
  Serial.println(buff);

  index = buff.indexOf(",");
  sms_status = buff.substring(1, index - 1);
  buff.remove(0, index + 2);

  sender_number = buff.substring(0, 12);
  buff.remove(0, 17);   // 0 17
  
  received_date = buff.substring(0, 22);   //0 22
  buff.remove(0, buff.indexOf("\r"));
  buff.trim();

  index = buff.indexOf("\n\r");
  buff = buff.substring(0, index);
  buff.trim();
  msg = buff;
  buff = "";
  msg.toLowerCase();

  Serial.println("----------------------------------");
  Serial.println(sms_status);
  Serial.println(sender_number);
  Serial.println(received_date);
  Serial.println(msg);
  Serial.println("----------------------------------");
   
}
void doAction() {
  //case sensitive
  if (msg == "lock") {
    digitalWrite(RELAY_1, HIGH);
    Serial.println("Scooter blocked");
   
      sendSms("Scooter blocked");
    
  }
  else if (msg == "unlock") {
    digitalWrite(RELAY_1, LOW);
    Serial.println("Scooter Deblocked");
    sendSms("Scooter Deblocked");
    
  }  else if (msg == "status") {
     String text = (lastScooterState ) ? "ON" : "OFF";
     sendSms("Scooter Alarm Is "+ text);
     Serial.println("Scooter Status");
     }  
     else if (msg == "deletsms") {
        Serial.println("all mesages delletid");
        sim800.println ("AT+CMGDA=DEL ALL");
        sendSms("all mesages delletid");
    
     }  
       else if (msg == "location") {
     sendSmsGPS("Location");
     }   
      else if (msg == "batstat") {
 
    sendBatteryStatus(batteryPercentage,voltage);

}

       
       
       
       
       else if (msg == "bonjour trottinet") {
     Serial.println("bonjour trottinet");
     sendSms("Bonjour Edgar Cava?");
     
     }  else if (msg == "oui cava") {
     Serial.println("oui cava");
     sendSms("Quand tu seras libre, nous roulerons un peu.");
     }  else if (msg == "ok") {
     Serial.println("ok");
     sendSms("Alors je t'attends Edgar");
     }
  
 
  
  
  
  sms_status = "";
  sender_number = "";
  received_date = "";
  msg = "";
}
void deleteSms()
{
  
  sendATcommand("AT+CMGD=1,4", "OK", 2000);
  sendATcommand ("AT+CMGDA=DEL ALL","OK", 2000);
  Serial.println("All SMS are deleted.");
  
}

void sendSmsGPS(String text)
{
  // Can take up to 60 seconds
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 2000;)
  {
    while (neogps.available())
    {
      if (gps.encode(neogps.read()))
      {
        newData = true;
      }
    }
  }
  if (newData)      //If newData is true
  {
    float flat, flon;
    unsigned long age;
    Serial.print("Latitude= ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(" Longitude= ");
    Serial.println(gps.location.lng(), 6);
    newData = false;
    delay(300);
    sim800.print("AT+CMGF=1\r");
    delay(1000);
    sim800.print("AT+CMGS=\"" + PHONE + "\"\r");
    delay(1000);
    sim800.print("http://maps.google.com/maps?q=loc:");
    sim800.print(gps.location.lat(), 6);
    sim800.print(",");
    sim800.print(gps.location.lng(), 6);
    delay(100);
    sim800.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
    delay(1000);
    
  }
}
void sendSms(String text)
{
  sim800.print("AT+CMGF=1\r");
  delay(1000);
  sim800.print("AT+CMGS=\"" + PHONE + "\"\r");
  delay(1000);
  sim800.print(text);
  delay(100);
  sim800.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
  delay(1000);
  Serial.println("SMS Sent Successfully.");
}

int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout) {

  uint8_t x = 0,  answer = 0;
  char response[100];
  unsigned long previous;

  memset(response, '\0', 100);    // Initialice the string

  delay(100);

  while ( sim800.available() > 0) sim800.read();   // Clean the input buffer

  if (ATcommand[0] != '\0')
  {
    sim800.println(ATcommand);    // Send the AT command
  }


  x = 0;
  previous = millis();

  // this loop waits for the answer
  do {
    if (sim800.available() != 0) {  // if there are data in the UART input buffer, reads it and checks for the asnwer
      response[x] = sim800.read();
      //Serial.print(response[x]);
      x++;
      if (strstr(response, expected_answer) != NULL)    // check if the desired answer (OK) is in the response of the module
      {
        answer = 1;
      }
    }
  } while ((answer == 0) && ((millis() - previous) < timeout));   // Waits for the asnwer with time out

  return answer;
}

void DIGITALSTAT() {

  boolean currentScooterState = digitalRead(STATUS_PIN);

  if (currentScooterState != lastScooterState) {
    
    if (currentScooterState == LOW) {
      delay(1000);
      sendSms("Scooter is ON");
    } else {
     delay(1000);
     sendSms("Scooter is OFF");
    }
    lastScooterState = currentScooterState;
  }
 }

  void sendBatteryStatus(int batteryPercentage, float voltage) {
  
  int sensorValue = analogRead(analogInputPin);
  voltage = (sensorValue / 1023.0) * maxVoltage; 
  batteryPercentage = map(voltage,minVoltage, maxVoltage,0 ,100) ;
  
  String text = "Battery Percentage: " + String(batteryPercentage) + "%, Voltage: " + String(voltage) + "V";
  sim800.print("AT+CMGF=1\r");
  delay(1000);
  sim800.print("AT+CMGS=\"" + PHONE + "\"\r");
  delay(1000);
  sim800.print(text);
  delay(100);
  sim800.write(0x1A);
  delay(1000);
  Serial.println("SMS Sent Successfully");
  Serial.print("Battery Percentage: ");
  Serial.print(batteryPercentage);
  Serial.print("%, Voltage: ");
  Serial.print(voltage);
  Serial.println("V");

}

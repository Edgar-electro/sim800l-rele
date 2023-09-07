#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <TinyGPS++.h>

const String PHONE = "+33769888360";
const int RELAY_1 = 5;
const int STATUS_PIN  = 6;
const int analogInputPin = A0;

const float maxVoltage = 42.0;
const float minVoltage = 30.0;

SoftwareSerial sim800(2, 3); // RX и TX для SIM800L
AltSoftSerial neogps; // RX и TX для GPS
TinyGPSPlus gps;

String sms_status, sender_number, received_date, msg;

boolean lastScooterState = false;
int batteryPercentage = 0;
float voltage = 0;

void setup() {
  delay(5000);
  Serial.begin(115200);
   
  // Настройка GSM-модуля
  sim800.begin(9600);
  neogps.begin(9600);
  delay(1000);
  sim800.print("AT+CMGF=1\r"); //SMS text mode
  delay(1000);
  //delete all sms
  sim800.println("AT+CMGD=1,4");
  delay(1000);
  sim800.println("AT+CMGDA= \"DEL ALL\"");
  delay(1000);
  // Инициализация пинов и отправка SMS
  pinMode(RELAY_1, OUTPUT);
  digitalWrite(RELAY_1, LOW);
  pinMode(STATUS_PIN, INPUT_PULLUP);
  analogRead(analogInputPin);

  // Отправка стартового SMS
  sendSms("GPS Tracker is Online");
}


void loop() {
  while (sim800.available()) {
    parseData(sim800.readString());
  }
  while (Serial.available()) {
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
      index = buff.indexOf(",");
      String temp = buff.substring(index + 1, buff.length());
      temp = "AT+CMGR=" + temp + "\r";
      sim800.println(temp);
    } else if (cmd == "+CMGR") {
      extractSms(buff);
      if (sender_number == PHONE) {
        doAction();
        deleteSms();
      }
    }
  }
}

void extractSms(String buff) {
  unsigned int index;
  Serial.println(buff);

  index = buff.indexOf(",");
  sms_status = buff.substring(1, index - 1);
  buff.remove(0, index + 2);

  sender_number = buff.substring(0, 12);
  buff.remove(0, 17);
  received_date = buff.substring(0, 22);
  buff.remove(0, buff.indexOf("\r"));
  buff.trim();
  index = buff.indexOf("\n\r");
  buff = buff.substring(0, index);
  buff.trim();
  msg = buff;
  buff = "";
  msg.toLowerCase();
}

void doAction() {
  if (msg == "lock") {
    digitalWrite(RELAY_1, HIGH);
    sendSms("Scooter blocked");
  } else if (msg == "unlock") {
    digitalWrite(RELAY_1, LOW);
    sendSms("Scooter Deblocked");
  } else if (msg == "status") {
    String text = (lastScooterState) ? "ON" : "OFF";
    sendSms("Scooter Alarm Is " + text);
  } else if (msg == "deletsms") {
    sim800.println("AT+CMGDA=\"DEL ALL\"");
    sendSms("all messages deleted");
  } else if (msg == "location") {
    sendSmsGPS("Location");
  } else if (msg == "batstat") {
    sendBatteryStatus();
  } else if (msg == "gsmlivel") {
  checkSignalAndSendSMS();
}


  sms_status = "";
  sender_number = "";
  received_date = "";
  msg = "";
}

void deleteSms() {
  sendATcommand("AT+CMGD=1,4", "OK", 5000);
  sendATcommand("AT+CMGDA=\"DEL ALL\"", "OK", 5000);
  Serial.println("All SMS are deleted.");
}

void sendSmsGPS(String text){

  
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

void sendSms(String text) {
  sim800.print("AT+CMGF=1\r");
  delay(1000);
  sim800.print("AT+CMGS=\"" + PHONE + "\"\r");
  delay(1000);
  sim800.print(text);
  delay(100);
  sim800.write(0x1A); // ASCII-код для Ctrl+Z
  delay(1000);
  Serial.println("SMS Sent Successfully.");
}

int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout) {
  uint8_t x = 0, answer = 0;
  char response[100];
  unsigned long previous;

  memset(response, '\0', 100); // Инициализация строки

  delay(100);

  while (sim800.available() > 0) sim800.read(); // Очистка буфера ввода

  if (ATcommand[0] != '\0') {
    sim800.println(ATcommand); // Отправка AT-команды
  }

  x = 0;
  previous = millis();

  // Этот цикл ждет ответа
  do {
    if (sim800.available() != 0) {
      response[x] = sim800.read();
      x++;
      if (strstr(response, expected_answer) != NULL) {
        answer = 1;
      }
    }
  } while ((answer == 0) && ((millis() - previous) < timeout));

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

void sendBatteryStatus() {
  int sensorValue = analogRead(analogInputPin);
  float voltage = (sensorValue / 1023.0) * maxVoltage;
  int batteryPercentage = map(voltage, minVoltage, maxVoltage, 0, 100);
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

void checkSignalAndSendSMS() {
  // Отправить команду для запроса уровня сигнала GSM
  sim800.println("AT+CSQ");
  delay(1000); // Подождать секунду для получения ответа

  // Чтение ответа
  String signalResponse = "";
  while (sim800.available()) {
    signalResponse += sim800.readString();
  }

  // Отправить ответ в виде SMS на ваш номер
  sim800.print("AT+CMGF=1\r");
  delay(1000);
  sim800.print("AT+CMGS=\"" + PHONE + "\"\r");
  delay(1000);
  sim800.print("Signal Strength: " + signalResponse);
  delay(100);
  sim800.write(0x1A); // Отправить SMS
  delay(1000);

  Serial.println("Signal Strength sent via SMS: " + signalResponse);
}


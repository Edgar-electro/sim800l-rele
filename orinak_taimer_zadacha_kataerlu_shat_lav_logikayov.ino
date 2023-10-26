unsigned long previousMillis = 0; // Переменная для отслеживания времени
unsigned long interval = 1000; // Интервал в миллисекундах (1 секунда)
unsigned long countdown = 120; // Время в секундах (120 минут)

void setup() {
  Serial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (countdown > 0) {
      // Если счетчик больше нуля, продолжаем отсчет
      countdown--;
    } else {
      // Если счетчик достиг нуля, выполняем нужные действия
      performAction();
      countdown = 120; // Восстановить счетчик до 120 секунд (2 минуты)
    }
    
    // Проверка для выполнения других функций на разных значениях счетчика
    if (countdown == 100) {
      performAction100();
    }
    if (countdown == 80) {
      performAction80();
    }
    if (countdown == 60) {
      performAction60();
    }
    if (countdown == 40) {
      performAction40();
    }
  }

  // Здесь вы можете выполнить другие действия вне зависимости от времени
}

void performAction() {
  // Здесь вы можете выполнить действие, которое выполняется каждые 2 минуты
  Serial.println("Выполнение действия...");
  // Например, отправка данных, активация реле, и т. д.
}

void performAction100() {
  // Здесь вы можете выполнить действие, которое выполняется при countdown равному 100
  Serial.println("Выполнение действия при countdown = 100");
  // Дополнительные действия на этом этапе
}

void performAction80() {
  // Здесь вы можете выполнить действие, которое выполняется при countdown равному 80
  Serial.println("Выполнение действия при countdown = 80");
  // Дополнительные действия на этом этапе
}

void performAction60() {
  // Здесь вы можете выполнить действие, которое выполняется при countdown равному 60
  Serial.println("Выполнение действия при countdown = 60");
  // Дополнительные действия на этом этапе
}

void performAction40() {
  // Здесь вы можете выполнить действие, которое выполняется при countdown равному 40
  Serial.println("Выполнение действия при countdown = 40");
  // Дополнительные действия на этом этапе
}

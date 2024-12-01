// Khai báo chân điều khiển các bóng đèn
const int led1 = 12;
const int led2 = 14;
const int led3 = 27;
const int led4 = 26;

const int trigPin = 25;
const int echoPin = 33;

// Khai báo Task Handle cho mỗi core
TaskHandle_t Task1;
TaskHandle_t Task2;

QueueHandle_t coreMail;

void controlLEDs(int ledStatus) {
  digitalWrite(led1, (ledStatus >> 0) & 1);
  digitalWrite(led2, (ledStatus >> 1) & 1);
  digitalWrite(led3, (ledStatus >> 2) & 1);
  digitalWrite(led4, (ledStatus >> 3) & 1);
}

// (chạy trên Core 0)
void SomethingToDo(void *pvParameters) {
  int ledStatus = 0;
  while (true) {
    if (xQueueReceive(coreMail, &ledStatus, portMAX_DELAY) == pdPASS) {
      controlLEDs(ledStatus);
    }
  }
}

// (chạy trên Core 1)
void DistanceMeasurementTask(void *pvParameters) {
  long duration;
  float distance;

  while (true) {
    // Phát tín hiệu Trigger
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Đo thời gian Echo
    duration = pulseIn(echoPin, HIGH);

    // Tính khoảng cách theo cm
    distance = duration * 0.034 / 2;
    
    int ledStatus = 0;
    if (distance <= 10) {
      ledStatus = 0b0000;      
    }
    else if (distance <= 20) {
      ledStatus = 0b0001;      
    }
    else if (distance <= 30) {
      ledStatus = 0b0011;      
    }
    else if (distance <= 40) {
      ledStatus = 0b0111;      
    }
    else ledStatus = 0b1111;      

    controlLEDs(ledStatus);
    Serial.println("Khoảng cách là: " + String(distance) + " cm");
    // xQueueSend(coreMail, &ledStatus, portMAX_DELAY);
    
    // Dừng 500 ms trước khi đo tiếp
    delay(500);
  }
}

void setup() {
  // Thiết lập các chân LED là đầu ra
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(115200);

  coreMail = xQueueCreate(10, sizeof(int));

  // xTaskCreatePinnedToCore(SomethingToDo, "SomethingToDo", 2048, NULL, 1, &Task1, 0);
  xTaskCreatePinnedToCore(DistanceMeasurementTask, "Distance Measurement", 2048, NULL, 1, &Task2, 1);
}

void loop() {
  
}

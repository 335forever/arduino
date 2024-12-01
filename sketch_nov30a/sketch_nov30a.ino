#include <TFT_eSPI.h>    
#include <TFT_eWidget.h>
#include "DHT.h"
#include "custom_colors.h"

#define DHTPIN 5
#define DHTTYPE DHT11
#define BUTTON_PIN 18
#define POT_TEMP_PIN 26 
#define POT_HUM_PIN 25
#define BUZZER_PIN 19

TFT_eSPI tft  = TFT_eSPI();
DHT dht(DHTPIN, DHTTYPE);

MeterWidget   temps  = MeterWidget(&tft);
MeterWidget   humis  = MeterWidget(&tft);

#define LOOP_PERIOD 35 // Display updates every 35 ms
#define SWITCH_PERIOD 500

volatile bool switchPageFlag = false;

uint32_t updateTime = 0;
uint32_t switchPageTime = 0;
int currentPage = 1;
uint16_t tempWarningLevel = 40; // Initial temperature warning level
uint16_t humWarningLevel = 50; // Initial humidity warning level

unsigned long previousMillis = 0;  // Lưu thời gian trước đó
unsigned long interval = 1000;     // Thời gian chu kỳ (1 giây)
bool buzzerState = LOW;            // Trạng thái bật/tắt của buzzer

void initPage_1() {
  tft.fillScreen(TFT_WHITE);
  tft.fillRect(0, 0, 240, 30, CUSTOM_BLUE);
  tft.setTextColor(TFT_WHITE, CUSTOM_BLUE); 
  tft.setTextDatum(MC_DATUM);      
  tft.setTextSize(2);      
  tft.drawString("Dashboard", 120, 15);
  tft.setTextSize(1);

  temps.setZones(0, 100, tempWarningLevel - 20, tempWarningLevel + 20, 0, 0, tempWarningLevel - 10, tempWarningLevel + 10);
  temps.analogMeter(0, 30, 60, "Temp", "20", "30", "40", "50", "60");    

  humis.setZones(0, 100, humWarningLevel - 20, humWarningLevel + 20, 0, 0, humWarningLevel - 10, humWarningLevel + 10);
  humis.analogMeter(0, 158, 100, "Humi", "0", "25", "50", "75", "100");    
}

void initPage_2() {
  tft.fillScreen(TFT_WHITE);
  tft.fillRect(0, 0, 240, 30, CUSTOM_BLUE);
  tft.setTextColor(TFT_WHITE, CUSTOM_BLUE); 
  tft.setTextDatum(MC_DATUM);      
  tft.setTextSize(2);      
  tft.drawString("Setting", 120, 15);
  
  tft.setCursor(10, 40);
  tft.setTextColor(TFT_RED);
  tft.print("Temp Warning: ");

  tft.setCursor(10, 60);
  tft.setTextColor(TFT_BLUE);
  tft.print("Humi Warning: ");
  
  tft.setTextSize(1);      
}

bool isSafe(float value, uint16_t warningLevel, uint16_t delta = 20) {
  return abs(value - warningLevel) <= delta;
}

void runPage_1() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    h = 0;
    t = 0;
  }

  temps.updateNeedle(t, 0);
  humis.updateNeedle(h, 0);

  if (!isSafe(h, humWarningLevel) || !isSafe(t, tempWarningLevel)) {
    unsigned long currentMillis = millis();

    // Kiểm tra nếu đủ 1 giây trôi qua
    if (currentMillis - previousMillis >= interval) {
      // Lưu lại thời gian hiện tại
      previousMillis = currentMillis;

      // Chuyển trạng thái của buzzer (tắt nếu bật, bật nếu tắt)
      buzzerState = !buzzerState;

      // Cập nhật trạng thái buzzer
      digitalWrite(BUZZER_PIN, buzzerState);
    }
  } else {
    // Tắt buzzer khi nhiệt độ <= 30°C
    digitalWrite(BUZZER_PIN, LOW);
  }
}

uint16_t getNewValue(uint16_t oldValue, float newValue, float THRESHOLD_PERCENT = 1.0) {
  if (abs(newValue - oldValue) >= (oldValue * THRESHOLD_PERCENT / 100.0))
    return (uint16_t)round(newValue);
  else 
    return oldValue;
}


void runPage_2() {  
  // Đọc giá trị từ chiết áp và làm mượt
  tempWarningLevel = getNewValue(tempWarningLevel, analogRead(POT_TEMP_PIN) / 1023.0 * 100.0 / 4, 5);
  humWarningLevel = getNewValue(humWarningLevel, analogRead(POT_HUM_PIN) / 1023.0 * 100.0 / 4, 5);

  String tempStr = String(tempWarningLevel);
  String humiStr = String(humWarningLevel);

  while (tempStr.length() < 3) tempStr = "0" + tempStr;
  while (humiStr.length() < 3) humiStr = "0" + humiStr;

  tft.setTextSize(2);
  tft.setTextDatum(TL_DATUM);
  
  tft.setTextColor(TFT_RED, TFT_WHITE);
  tft.drawString(tempStr, 10 + 14 * 12, 40);

  tft.setTextColor(TFT_BLUE, TFT_WHITE);
  tft.drawString(humiStr, 10 + 14 * 12, 60);
  
  tft.setTextSize(1);      
}

void switchPageISR() {
  switchPageFlag = true;
}

void setup(void) 
{
  tft.init();
  tft.setRotation(0);
  
  initPage_1();
  
  Serial.begin(115200); // For debug

  dht.begin();
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(POT_TEMP_PIN, INPUT);
  pinMode(POT_HUM_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), switchPageISR, FALLING);
}


void loop() 
{
  if (switchPageFlag) {
    switchPageFlag = false;
    if (millis() - switchPageTime >= SWITCH_PERIOD) {
      switchPageTime = millis();
      currentPage = (currentPage == 1) ? 2 : 1;

      if (currentPage == 1) initPage_1();
      else initPage_2();
    }
  }
  
  if (millis() - updateTime >= LOOP_PERIOD) {
    updateTime = millis();

    if (currentPage == 1) runPage_1();
    else runPage_2();
  }
}



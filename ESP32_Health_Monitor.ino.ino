#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30100_PulseOximeter.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// --- OLED Settings ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- MAX30100 Settings ---
PulseOximeter pox;
unsigned long lastUpdate = 0;

// --- DS18B20 Settings ---
#define DS_PIN 4
OneWire oneWire(DS_PIN);
DallasTemperature sensors(&oneWire);
#define TEMP_OFFSET 0.0  // calibration offset

// Beat detection callback
void onBeatDetected() {
  Serial.println("Beat Detected!");
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // SDA, SCL for ESP32

  // --- OLED Initialization ---
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found!");
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Initializing...");
  display.display();

  // --- MAX30100 Initialization ---
  Serial.print("Initializing MAX30100... ");
  if (!pox.begin()) {
    Serial.println("FAILED");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("MAX30100 FAIL");
    display.display();
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_27_1MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);

  // --- DS18B20 Initialization ---
  sensors.begin();

  delay(1000);
}

void loop() {
  pox.update();  // process MAX30100 data

  if (millis() - lastUpdate > 2000) { // every 2 seconds
    float hr = pox.getHeartRate();
    float spo2 = pox.getSpO2();

    // --- Temperature ---
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0) + TEMP_OFFSET;
    float tempF = tempC * 9.0 / 5.0 + 32.0;

    // --- OLED Display ---
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);

    // HR
    display.setCursor(0, 0);
    display.print("HR: ");
    if (hr > 30 && hr < 200)
      display.print((int)hr);
    else
      display.print("--");

    // SpO2
    display.setCursor(0, 20);
    display.print("SpO2:");
    if (spo2 > 50 && spo2 <= 100)
      display.print((int)spo2);
    else
      display.print("--");
    display.print("%");

    // Temperature
    display.setCursor(0, 42);
    display.setTextSize(1);
    display.print("TEMP: ");
    if (tempC < -50) {
      display.print("--C / --F");
    } else {
      display.print(tempC, 1);
      display.print("C  ");
      display.print(tempF, 1);
      display.print("F");
    }

    display.display();

    // --- Serial Monitor Output ---
    Serial.print("HR: "); Serial.print(hr);
    Serial.print(" | SpO2: "); Serial.print(spo2);
    Serial.print(" | TEMP: "); Serial.print(tempC);
    Serial.print("C / "); Serial.print(tempF);
    Serial.println("F");

    lastUpdate = millis();
  }
}

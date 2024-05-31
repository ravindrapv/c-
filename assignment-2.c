
#include <WiFi.h>
#include <HTTPClient.h>
#include "DHTesp.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

// ThingSpeak API parameters
const char* server = "api.thingspeak.com";
const String apiKey = "YOUR_THINGSPEAK_API_KEY";

// DHT22 Sensor
const int DHT_PIN = 15;
DHTesp dht;

// RGB LED pins
const int RED_PIN = 13;
const int GREEN_PIN = 12;
const int BLUE_PIN = 14;

// Buzzer pin
const int BUZZER_PIN = 27;

// PIR Sensor pin
const int PIR_PIN = 33;

// I2C LCD parameters
const int LCD_COLS = 16;
const int LCD_ROWS = 2;
const int LCD_ADDR = 0x27;
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

void setup() {
  Serial.begin(115200);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  lcd.init();
  lcd.backlight();

  dht.setup(DHT_PIN, DHTesp::DHT22);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void loop() {
  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();
  Serial.printf("Temperature: %.2f C, Humidity: %.2f %%\n", temperature, humidity);

  // Send data to ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://" + String(server) + "/update?api_key=" + apiKey + "&field1=" + String(temperature) + "&field2=" + String(humidity);
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.println("Data sent to ThingSpeak");
    } else {
      Serial.printf("Error sending data to ThingSpeak: %d\n", httpResponseCode);
    }
    http.end();
  }

  // Display temperature and humidity on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print(" %");

  // RGB LED and Buzzer control logic
  if (temperature > 50 || humidity > 64) {
    alarm(); // Fire alarm
  } else if (temperature > 25) {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, LOW);
    noTone(BUZZER_PIN);
  } else if (temperature > 5) {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BLUE_PIN, HIGH);
    noTone(BUZZER_PIN);
  } else {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BLUE_PIN, LOW);
    noTone(BUZZER_PIN);
  }

  // Unauthorized entry detection
  if (digitalRead(PIR_PIN) == HIGH) {
    unauthorizedEntry();
  } else {
    noTone(BUZZER_PIN);
  }

  delay(2000); // Update every 2 seconds
}

void alarm() {
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
  digitalWrite(RED_PIN, HIGH);
  // Blink red LED and sound buzzer
  for (int i = 0; i < 5; i++) {
    digitalWrite(RED_PIN, HIGH);
    tone(BUZZER_PIN, 1000);
    delay(500);
    digitalWrite(RED_PIN, LOW);
    noTone(BUZZER_PIN);
    delay(500);
  }
}

void unauthorizedEntry() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Unauth. ENTRY");
  // Blink red LED and sound buzzer
  for (int i = 0; i < 5; i++) {
    digitalWrite(RED_PIN, HIGH);
    tone(BUZZER_PIN, 1000);
    delay(500);
    digitalWrite(RED_PIN, LOW);
    noTone(BUZZER_PIN);
    delay(500);
  }
}

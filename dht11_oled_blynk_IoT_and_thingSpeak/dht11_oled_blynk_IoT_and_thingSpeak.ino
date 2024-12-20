/*************************************************************
  This is a demo that combines reading sensor data, sending
  it to Blynk and ThingSpeak, and displaying it on an OLED screen.
 *************************************************************/

/* Blynk credentials */
#define BLYNK_TEMPLATE_ID           "TMPL2kXTQxOa"
#define BLYNK_TEMPLATE_NAME         "Tempidity Sensor"
#define BLYNK_AUTH_TOKEN            "K-RIbY4Xwiafj4WyQeLFu0_Fo-kawxa1"

/* WiFi credentials */
char ssid[] = "Fios-3LcLc";
char pass[] = "was55buy742paly";

#define BLYNK_PRINT Serial  // Optional: prints debug information

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ThingSpeak.h>
#include <U8g2lib.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_CCS811.h>


// Pin definitions for the DHT sensor and OLED
#define DHTPIN 4        // DHT11 sensor data pin
#define DHTTYPE DHT11   // DHT11 sensor type
#define OLED_DC 16      // OLED Data/Command pin (GPIO 16)
#define OLED_CS 5       // OLED Chip Select pin (GPIO 5)
#define OLED_RESET 17   // OLED reset pin (GPIO 17)
#define SCLK 18         // Clock pin for SPI
#define MOSI 23         // MOSI pin for SPI
// Pin definitions for power and reset
#define VCC_PIN 32
#define RST_PIN 33

// Initialize the OLED display using U8g2 (using SPI communication)
U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RESET);

DHT dht(DHTPIN, DHTTYPE);  // Initialize DHT11 sensor

// Blynk timer for sending data to Blynk Cloud
BlynkTimer timer;

// ThingSpeak settings
WiFiClient client;
unsigned long myChannelNumber = 2697252;  // Your ThingSpeak Channel Number
const char * myWriteAPIKey = "QDD8GRVVOR373RH0";  // Your ThingSpeak API Write Key

// Function to send data to ThingSpeak
void sendToThingSpeak(float humidity, float temperature, float tempF, String uptimeString) {
  ThingSpeak.setField(1, humidity);       // Field 1 for humidity
  ThingSpeak.setField(2, temperature);    // Field 2 for temperature in Celsius
  ThingSpeak.setField(3, tempF);          // Field 3 for temperature in Fahrenheit
  ThingSpeak.setField(4, uptimeString);   // Field 4 for uptime

  int response = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (response == 200) {
    Serial.println("Data sent to ThingSpeak successfully.");
  } else {
    Serial.println("Problem sending data to ThingSpeak. HTTP error code: " + String(response));
  }
}

// Function to update the OLED display and Blynk
void updateBlynkAndScreen() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();  // Celsius

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  float tempF = (temperature * 9.0 / 5.0) + 32;

  // Send data to Blynk
  Blynk.virtualWrite(V0, humidity);
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, tempF);

  // Uptime calculation
  unsigned long uptimeMillis = millis();
  unsigned long uptimeSecs = uptimeMillis / 1000;
  unsigned long hours = uptimeSecs / 3600;
  unsigned long minutes = (uptimeSecs % 3600) / 60;
  unsigned long seconds = uptimeSecs % 60;
  String uptimeString = String(hours) + ":" + String(minutes) + ":" + String(seconds);

  Blynk.virtualWrite(V3, uptimeString); // Send uptime to Blynk

  // Update OLED display
  u8g2.clearBuffer();                      // Clear the screen buffer
  u8g2.setFont(u8g2_font_ncenB08_tr);      // Choose a font
  
  // Display uptime
  u8g2.setCursor(0, 10);
  u8g2.print("Uptime: ");
  u8g2.print(uptimeString);

  // Display temperature in Celsius
  u8g2.setCursor(0, 25);
  u8g2.print("Temp C: ");
  u8g2.print(temperature);

  // Display temperature in Fahrenheit
  u8g2.setCursor(0, 40);
  u8g2.print("Temp F: ");
  u8g2.print(tempF);

  // Display humidity
  u8g2.setCursor(0, 55);
  u8g2.print("Humidity: ");
  u8g2.print(humidity);

  u8g2.sendBuffer();                       // Send the buffer to the OLED screen
}

// Function to handle ThingSpeak updates every 15 seconds
void updateThingSpeak() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();  // Celsius

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  float tempF = (temperature * 9.0 / 5.0) + 32;

  // Uptime calculation
  unsigned long uptimeMillis = millis();
  unsigned long uptimeSecs = uptimeMillis / 1000;
  unsigned long hours = uptimeSecs / 3600;
  unsigned long minutes = (uptimeSecs % 3600) / 60;
  unsigned long seconds = uptimeSecs % 60;
  String uptimeString = String(hours) + ":" + String(minutes) + ":" + String(seconds);

  // Send data to ThingSpeak
  sendToThingSpeak(humidity, temperature, tempF, uptimeString);
}

void setup() {
  Serial.begin(115200);

  // Initialize DHT sensor and OLED display
  dht.begin();
  u8g2.begin();


  // Initialize CCS811 sensor
  if (!ccs.begin()) {
    Serial.println("Failed to start CCS811 sensor! Please check wiring.");
    while (1);
  }

  // Ensure that the sensor is reset
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, HIGH);  // Set RST_PIN high to keep the sensor out of reset


  // Connect to WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  // Initialize ThingSpeak
  ThingSpeak.begin(client);

  // Connect to Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Setup a timer to update Blynk and screen every 1 second
  timer.setInterval(1000L, updateBlynkAndScreen);  // Update screen and Blynk every 1 second

  // Setup a timer to send data to ThingSpeak every 15 seconds
  timer.setInterval(15000L, updateThingSpeak);  // Send data to ThingSpeak every 15 seconds
}

void loop() {
  Blynk.run();
  timer.run();

  // Read CCS811 data
  if (ccs.available()) {
    if (!ccs.readData()) {
      float co2 = ccs.geteCO2();
      float tvoc = ccs.getTVOC();
      
      // Print sensor data to Serial
      Serial.print("CO2: ");
      Serial.print(co2);
      Serial.print(" ppm, TVOC: ");
      Serial.print(tvoc);
      Serial.println(" ppb");

      // Send data to Blynk
      Blynk.virtualWrite(V4, co2);   // CO2 data on virtual pin V4
      Blynk.virtualWrite(V5, tvoc);  // TVOC data on virtual pin V5

      // Send data to ThingSpeak
      ThingSpeak.setField(5, co2);   // Field 5 for CO2
      ThingSpeak.setField(6, tvoc);  // Field 6 for TVOC
      ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    } else {
      Serial.println("Failed to read CCS811 data.");
    }
  }
}

/*************************************************************
  Demo combining DHT11, CCS811, OLED, Blynk, and ThingSpeak.
 *************************************************************/

// Blynk credentials
#define BLYNK_TEMPLATE_ID           "TMPL2kXYQfzOa"
#define BLYNK_TEMPLATE_NAME         "Tempidity Sensor"
#define BLYNK_AUTH_TOKEN            "K-RIbY4Xwiafj4WyQeLFu0_Fo-kawxa1"

// WiFi credentials
char ssid[] = "Fios-3LcLc";
char pass[] = "was55buy742paly";

#define BLYNK_PRINT Serial  // Enable printing for debugging

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ThingSpeak.h>
#include <U8g2lib.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_CCS811.h>

// DHT11 sensor
#define DHTPIN 4        
#define DHTTYPE DHT11   
DHT dht(DHTPIN, DHTTYPE);  

// OLED Pin Definitions
#define OLED_DC 16
#define OLED_CS 5
#define OLED_RESET 17

// CCS811 Pin Definitions
#define RST_PIN 33  // CCS811 reset pin

// Initialize the OLED display (SPI mode)
U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RESET);

// CCS811 sensor
Adafruit_CCS811 ccs;

// ThingSpeak settings
WiFiClient client;
unsigned long myChannelNumber = 2697252;  // Your ThingSpeak Channel Number
const char *myWriteAPIKey = "QDD8GRVVOR373RH0";  // ThingSpeak API key

// Blynk timer
BlynkTimer timer;

// Function to send data to ThingSpeak
void sendToThingSpeak(float humidity, float temperature, float eco2, float tvoc) {
  ThingSpeak.setField(1, humidity);
  ThingSpeak.setField(2, temperature);
  ThingSpeak.setField(3, eco2);
  ThingSpeak.setField(4, tvoc);
  int response = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (response == 200) {
    Serial.println("Data sent to ThingSpeak successfully.");
  } else {
    Serial.println("Failed to send data to ThingSpeak. HTTP error: " + String(response));
  }
}

// Function to update Blynk and OLED display
void updateBlynkAndScreen() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  // Read CCS811 sensor data
  float eco2 = ccs.geteCO2();
  float tvoc = ccs.getTVOC();
  
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // Update Blynk
  Blynk.virtualWrite(V0, humidity);
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, eco2);
  Blynk.virtualWrite(V3, tvoc);

  // Update OLED display
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  // Display temperature, humidity, eCO2, and TVOC
  u8g2.setCursor(0, 20);
  u8g2.print("Temp: ");
  u8g2.print(temperature);
  u8g2.print(" C");

  u8g2.setCursor(0, 40);
  u8g2.print("Humidity: ");
  u8g2.print(humidity);
  u8g2.print(" %");

  u8g2.setCursor(0, 60);
  u8g2.print("eCO2: ");
  u8g2.print(eco2);
  u8g2.print(" ppm");

  u8g2.setCursor(0, 80);
  u8g2.print("TVOC: ");
  u8g2.print(tvoc);
  u8g2.print(" ppb");

  // Flip the screen
  u8g2.setFlipMode(true); // Change to false for normal
  u8g2.sendBuffer();

  // Send data to ThingSpeak
  sendToThingSpeak(humidity, temperature, eco2, tvoc);
}

// Setup function
void setup() {
  Serial.begin(115200);
  
  // Initialize DHT11 and OLED
  dht.begin();
  u8g2.begin();
  
  // Initialize CCS811 sensor
  if (!ccs.begin()) {
    Serial.println("Failed to start CCS811 sensor! Check wiring.");
    while (1);
  }

  // Reset CCS811
  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, HIGH);

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

  // Set intervals for updating Blynk, OLED, and ThingSpeak
  timer.setInterval(1000L, updateBlynkAndScreen);  // Update every second
}

// Main loop
void loop() {
  Blynk.run();
  timer.run();
}

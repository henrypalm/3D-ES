// Include necessary libraries
#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_CCS811.h> // Include Adafruit CCS811 library
#include <DHT.h>             // Include DHT sensor library

// DHT sensor setup
#define DHTPIN 4        // DHT data pin
#define DHTTYPE DHT11   // DHT 11 or DHT22
DHT dht(DHTPIN, DHTTYPE);  

// OLED Pin Definitions
#define OLED_DC 16
#define OLED_CS 5
#define OLED_RESET 17

// Initialize the OLED display (SPI mode)
U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R2, OLED_CS, OLED_DC, OLED_RESET); // Use rotation R2

// Initialize CCS811 sensor
Adafruit_CCS811 ccs811;

// Setup function
void setup() {
  Serial.begin(115200);

  // Initialize DHT sensor
  dht.begin();

  // Initialize OLED display
  u8g2.begin();  

  // Initialize CCS811 sensor
  if (!ccs811.begin()) {
    Serial.println("Failed to find CCS811 sensor.");
    while (1); // Stay here if sensor initialization fails
  }

  // Wait for the sensor to be ready
  delay(1000);
}

// Main loop
void loop() {
  // Read temperature and humidity
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); // Celsius
  
  // Check if the CCS811 sensor is ready
  if (ccs811.available()) {
    // Read data from the sensor
    if (!ccs811.readData()) {
      float eco2 = ccs811.geteCO2();
      float tvoc = ccs811.getTVOC();

      // Clear the buffer and prepare to write to OLED
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB08_tr);

      // Print temperature, humidity, eCO2, and TVOC values on OLED
      u8g2.setCursor(10, 10);
      u8g2.print("Temp: ");
      u8g2.print(temperature);
      u8g2.print(" C");

      u8g2.setCursor(10, 25); // Reduced gap to 15 pixels
      u8g2.print("Hum: ");
      u8g2.print(humidity);
      u8g2.print(" %");

      u8g2.setCursor(10, 40); // Reduced gap to 15 pixels
      u8g2.print("eCO2: ");
      u8g2.print(eco2);
      u8g2.print(" ppm");

      u8g2.setCursor(10, 55); // Reduced gap to 15 pixels
      u8g2.print("TVOC: ");
      u8g2.print(tvoc);
      u8g2.print(" ppb");

      // Send the buffer to the OLED
      u8g2.sendBuffer();  
    }
  }

  delay(2000);  // Wait for 2 seconds before the next reading
}

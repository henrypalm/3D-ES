// Blynk credentials
#define BLYNK_TEMPLATE_ID           "TMPL2kXYQfzOa"
#define BLYNK_TEMPLATE_NAME         "Tempidity Sensor"
#define BLYNK_AUTH_TOKEN            "K-RIbY4Xwiafj4WyQeLFu0_Fo-kawxa1"

// WiFi credentials
char ssid[] = "Fios-3LcLc";
char pass[] = "was55buy742paly";

// Include necessary libraries
#include <Wire.h>
#include <U8x8lib.h>          // Include U8x8 library for OLED
#include <Adafruit_CCS811.h>  // Include Adafruit CCS811 library
#include <DHT.h>              // Include DHT sensor library
#include <WiFi.h>             // Include WiFi library
#include <BlynkSimpleEsp32.h> // Include Blynk library
#include <ThingSpeak.h>       // Include ThingSpeak library

// ThingSpeak settings
WiFiClient client;
unsigned long myChannelNumber = 2704818;  // Your ThingSpeak Channel Number
const char *myWriteAPIKey = "QQZSBLQF1VTZ118N";  // ThingSpeak API key

// DHT sensor setup
#define DHTPIN 4        // DHT data pin
#define DHTTYPE DHT11   // DHT 11 or DHT22
DHT dht(DHTPIN, DHTTYPE);  

// OLED Pin Definitions (SPI Mode)
#define OLED_DC 16
#define OLED_CS 5
#define OLED_RESET 17
#define OLED_SCK 18
#define OLED_MOSI 23

// Initialize the OLED display using U8x8 (SPI mode)
U8X8_SH1106_128X64_NONAME_4W_HW_SPI u8x8(OLED_CS, OLED_DC, OLED_RESET); // Keep only CS, DC, and RST

// Initialize CCS811 sensor
Adafruit_CCS811 ccs811;

// Timing variables
unsigned long lastThingSpeakUpdate = 0;
const unsigned long thingSpeakInterval = 15000; // 15 seconds for ThingSpeak
unsigned long lastRead = 0;
const unsigned long readInterval = 2000; // 2 seconds for sensor readings

// Setup function
void setup() {
    Serial.begin(115200);

    // Connect to WiFi
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi!");

    // Initialize Blynk
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Initialize ThingSpeak
    ThingSpeak.begin(client);

    // Initialize DHT sensor
    dht.begin();

    // Initialize CCS811 sensor
    if (!ccs811.begin()) {
        Serial.println("Failed to find CCS811 sensor.");
        while (1); // Stay here if sensor initialization fails
    }

    // Wait for the sensor to be ready
    delay(1000);

    // Initialize OLED display
    u8x8.begin();
    u8x8.setFont(u8x8_font_chroma48medium8_r);  // Set font for display
    u8x8.setFlipMode(0);  // Flip the display if necessary
}

// Function to convert Celsius to Fahrenheit
float celsiusToFahrenheit(float celsius) {
    return (celsius * 9.0 / 5.0) + 32.0;
}

// Main loop
void loop() {
    unsigned long currentMillis = millis();

    // Read sensors regularly
    if (currentMillis - lastRead >= readInterval) {
        lastRead = currentMillis;

        // Read temperature and humidity
        float humidity = dht.readHumidity();
        float temperatureC = dht.readTemperature(); // Celsius
        float temperatureF = celsiusToFahrenheit(temperatureC); // Convert to Fahrenheit

        // Check if the CCS811 sensor is ready
        if (ccs811.available()) {
            // Read data from the sensor
            if (!ccs811.readData()) {
                float eco2 = ccs811.geteCO2();
                float tvoc = ccs811.getTVOC();

                // Clear the display and update it with sensor readings
                u8x8.clearDisplay();
                u8x8.setCursor(0, 0); 
                u8x8.print("Temp: ");
                u8x8.print(temperatureC);
                u8x8.print(" C");

                u8x8.setCursor(0, 1); 
                u8x8.print("Temp: ");
                u8x8.print(temperatureF);
                u8x8.print(" F");

                u8x8.setCursor(0, 2); 
                u8x8.print("Hum: ");
                u8x8.print(humidity);
                u8x8.print(" %");

                u8x8.setCursor(0, 3); 
                u8x8.print("eCO2: ");
                u8x8.print(eco2);
                u8x8.print(" ppm");

                u8x8.setCursor(0, 4); 
                u8x8.print("TVOC: ");
                u8x8.print(tvoc);
                u8x8.print(" ppb");

                // Send data to Blynk
                Blynk.virtualWrite(V0, temperatureC); // Virtual pin for temperature in Celsius
                Blynk.virtualWrite(V1, temperatureF); // Virtual pin for temperature in Fahrenheit
                Blynk.virtualWrite(V2, humidity);      // Virtual pin for humidity
                Blynk.virtualWrite(V3, eco2);          // Virtual pin for eCO2
                Blynk.virtualWrite(V4, tvoc);          // Virtual pin for TVOC

                // Send data to ThingSpeak
                ThingSpeak.setField(1, temperatureC); // Field for temperature in Celsius
                ThingSpeak.setField(2, temperatureF); // Field for temperature in Fahrenheit
                ThingSpeak.setField(3, humidity);      // Field for humidity
                ThingSpeak.setField(4, eco2);          // Field for eCO2
                ThingSpeak.setField(5, tvoc);          // Field for TVOC

                // Update ThingSpeak every 15 seconds
                if (currentMillis - lastThingSpeakUpdate >= thingSpeakInterval) {
                    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
                    lastThingSpeakUpdate = currentMillis; // Update the timer
                }
            }
        }
    }

    Blynk.run(); // Run Blynk
}

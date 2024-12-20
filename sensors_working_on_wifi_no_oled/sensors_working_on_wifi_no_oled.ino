
// Blynk credentials
#define BLYNK_TEMPLATE_ID           "TMPL2kXYQfzOa"
#define BLYNK_TEMPLATE_NAME         "Tempidity Sensor"
#define BLYNK_AUTH_TOKEN            "K-RIbY4Xwiafj4WyQeLFu0_Fo-kawxa1"

// WiFi credentials
char ssid[] = "Fios-3LcLc";
char pass[] = "was55buy742paly";

// Include necessary libraries
#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_CCS811.h> // Include Adafruit CCS811 library
#include <DHT.h>             // Include DHT sensor library
#include <WiFi.h>            // Include WiFi library
#include <BlynkSimpleEsp32.h> // Include Blynk library
#include <ThingSpeak.h>      // Include ThingSpeak library

// ThingSpeak settings
WiFiClient client;
unsigned long myChannelNumber = 2704818;  // Your ThingSpeak Channel Number
const char *myWriteAPIKey = "QQZSBLQF1VTZ118N";  // ThingSpeak API key

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

// Timing variables
unsigned long lastThingSpeakUpdate = 0;
const unsigned long thingSpeakInterval = 15000; // 15 seconds for ThingSpeak
unsigned long lastRead = 0;
const unsigned long readInterval = 2000; // 2 seconds for sensor readings

// Setup function
void setup() {
    Serial.begin(115200);

    // Connect to WiFi through Blynk
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Wait for connection with a timeout (20 seconds)
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
        delay(500);
        Serial.println("Connecting to WiFi...");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi!");
    } else {
        Serial.println("Failed to connect to WiFi.");
        while (1);  // Halt further execution if WiFi fails
    }

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

        // Validate sensor readings to prevent NaN values
        if (isnan(humidity) || isnan(temperatureC)) {
            Serial.println("Failed to read from DHT sensor!");
            return;
        }

        float temperatureF = celsiusToFahrenheit(temperatureC); // Convert to Fahrenheit

        // Check if the CCS811 sensor is ready
        if (ccs811.available()) {
            // Read data from the sensor
            if (!ccs811.readData()) {
                float eco2 = ccs811.geteCO2();
                float tvoc = ccs811.getTVOC();

                // Display data on the OLED
                u8g2.clearBuffer();
                u8g2.setFont(u8g2_font_ncenB08_tr);

                // Display temperature, humidity, eCO2, and TVOC
                u8g2.setCursor(10, 10);
                u8g2.print("Temp: "); u8g2.print(temperatureC); u8g2.print(" C");

                u8g2.setCursor(10, 25); 
                u8g2.print("Temp: "); u8g2.print(temperatureF); u8g2.print(" F");

                u8g2.setCursor(10, 40); 
                u8g2.print("Hum: "); u8g2.print(humidity); u8g2.print(" %");

                u8g2.setCursor(10, 55); 
                u8g2.print("eCO2: "); u8g2.print(eco2); u8g2.print(" ppm");

                u8g2.setCursor(10, 70); 
                u8g2.print("TVOC: "); u8g2.print(tvoc); u8g2.print(" ppb");

                u8g2.sendBuffer();  // Send the buffer to the OLED
                
                // Send data to Blynk
                Blynk.virtualWrite(V0, temperatureC); // Virtual pin for temperature in Celsius
                Blynk.virtualWrite(V1, temperatureF); // Virtual pin for temperature in Fahrenheit
                Blynk.virtualWrite(V2, humidity);      // Virtual pin for humidity
                Blynk.virtualWrite(V3, eco2);          // Virtual pin for eCO2
                Blynk.virtualWrite(V4, tvoc);          // Virtual pin for TVOC

                // Send data to ThingSpeak every 15 seconds
                if (currentMillis - lastThingSpeakUpdate >= thingSpeakInterval) {
                    ThingSpeak.setField(1, temperatureC); // Field for temperature in Celsius
                    ThingSpeak.setField(2, temperatureF); // Field for temperature in Fahrenheit
                    ThingSpeak.setField(3, humidity);      // Field for humidity
                    ThingSpeak.setField(4, eco2);          // Field for eCO2
                    ThingSpeak.setField(5, tvoc);          // Field for TVOC

                    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
                    lastThingSpeakUpdate = currentMillis; // Update the timer
                }
            }
        }
    }

    Blynk.run(); // Run Blynk
}

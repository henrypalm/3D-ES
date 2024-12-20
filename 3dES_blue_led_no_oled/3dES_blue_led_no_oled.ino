// Blynk credentials
#define BLYNK_TEMPLATE_ID           "TMPL2kXYQfzOa"
#define BLYNK_TEMPLATE_NAME         "Tempidity Sensor"
#define BLYNK_AUTH_TOKEN            "K-RIbY4Xwiafj4WyQeLFu0_Fo-kawxa1"

// WiFi credentials
char ssid[] = "Fios-3LcLc";
char pass[] = "was55buy742paly";

// Include necessary libraries
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

// Blue LED pin
#define LED_PIN 27  // GPIO pin for the blue LED

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

    // Set up the LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);  // Ensure LED is off initially

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

                // Turn on blue LED to indicate data reporting
                digitalWrite(LED_PIN, HIGH);

                // Update ThingSpeak every 15 seconds
                if (currentMillis - lastThingSpeakUpdate >= thingSpeakInterval) {
                    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
                    lastThingSpeakUpdate = currentMillis; // Update the timer
                }

                // Turn off the blue LED after reporting
                digitalWrite(LED_PIN, LOW);
            }
        }
    }

    Blynk.run(); // Run Blynk
}

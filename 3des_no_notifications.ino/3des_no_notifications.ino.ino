// works well, no notifications 11/14/24

// Blynk credentials
#define BLYNK_TEMPLATE_ID           "TMPL2kXYQfzOa"
#define BLYNK_TEMPLATE_NAME         "Tempidity Sensor"
#define BLYNK_AUTH_TOKEN            "K-RIbY4Xwiafj4WyQeLFu0_Fo-kawxa1"

// WiFi credentials
char ssid[] = "Fios-3LcLc";
char pass[] = "was55buy742paly";

// Include necessary libraries
#include <Adafruit_CCS811.h>
#include <DHT.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ThingSpeak.h>

// ThingSpeak settings
WiFiClient client;
unsigned long myChannelNumber = 2704818;
const char *myWriteAPIKey = "QQZSBLQF1VTZ118N";

// DHT sensor setup
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);  

// Pin definitions
#define BLUE_LED_PIN 27    // GPIO pin for the blue LED (high humidity)
#define GREEN_LED_PIN 25   // GPIO pin for the green LED (good air quality)
#define RED_LED_PIN 26     // GPIO pin for the red LED (poor air quality)
#define RST_PIN 18         // GPIO pin connected to CCS811 RST
#define INT_PIN 19         // GPIO pin connected to CCS811 INT

// Initialize CCS811 sensor
Adafruit_CCS811 ccs811;

// Timing variables
unsigned long lastThingSpeakUpdate = 0;
const unsigned long thingSpeakInterval = 15000;
unsigned long lastRead = 0;
const unsigned long readInterval = 2000;

// Setup function
void setup() {
    Serial.begin(115200);

    // Set up LED pins
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);

    // Set up RST and INT pins
    pinMode(RST_PIN, OUTPUT);
    digitalWrite(RST_PIN, HIGH);
    pinMode(INT_PIN, INPUT);

    // Reset the CCS811 sensor briefly
    digitalWrite(RST_PIN, LOW);
    delay(10);
    digitalWrite(RST_PIN, HIGH);

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
        while (1);
    }

    // Wait for the sensor to be ready
    delay(1000);

    // Set up an interrupt for the INT pin
    attachInterrupt(digitalPinToInterrupt(INT_PIN), readSensorData, FALLING);
}

// Function to convert Celsius to Fahrenheit
float celsiusToFahrenheit(float celsius) {
    return (celsius * 9.0 / 5.0) + 32.0;
}

// Read data from sensors and handle LED indicators
void readSensorData() {
    if (ccs811.available()) {
        if (!ccs811.readData()) {
            float eco2 = ccs811.geteCO2();
            float tvoc = ccs811.getTVOC();

            // LED indicators for air quality based on eCO2 and TVOC
            if (eco2 < 1500 && tvoc < 1000) { // Good air quality
                digitalWrite(GREEN_LED_PIN, HIGH);
                digitalWrite(RED_LED_PIN, LOW);
            } else { // Poor air quality
                digitalWrite(GREEN_LED_PIN, LOW);
                digitalWrite(RED_LED_PIN, HIGH);
            }

            // Read temperature and humidity
            float humidity = dht.readHumidity();
            float temperatureC = dht.readTemperature();
            float temperatureF = celsiusToFahrenheit(temperatureC);

            // LED indicator for high humidity
            if (humidity > 50) {
                digitalWrite(BLUE_LED_PIN, HIGH);
            } else {
                digitalWrite(BLUE_LED_PIN, LOW);
            }

            // Send data to Blynk
            Blynk.virtualWrite(V0, temperatureC);
            Blynk.virtualWrite(V1, temperatureF);
            Blynk.virtualWrite(V2, humidity);
            Blynk.virtualWrite(V3, eco2);
            Blynk.virtualWrite(V4, tvoc);

            // Send data to ThingSpeak
            ThingSpeak.setField(1, temperatureC);
            ThingSpeak.setField(2, temperatureF);
            ThingSpeak.setField(3, humidity);
            ThingSpeak.setField(4, eco2);
            ThingSpeak.setField(5, tvoc);

            unsigned long currentMillis = millis();
            if (currentMillis - lastThingSpeakUpdate >= thingSpeakInterval) {
                ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
                lastThingSpeakUpdate = currentMillis;
            }
        }
    }
}

// Main loop
void loop() {
    // Regularly check sensors if no interrupt
    unsigned long currentMillis = millis();
    if (currentMillis - lastRead >= readInterval) {
        lastRead = currentMillis;
        readSensorData();  // Manually read data in case the interrupt missed
    }

    // Run Blynk
    Blynk.run();
}

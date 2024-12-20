/*************************************************************
  This is a demo that combines reading sensor data and sending
  it to Blynk while displaying it on an OLED screen.
 *************************************************************/

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPL2kXTQxOa"
#define BLYNK_TEMPLATE_NAME         "Tempidity Sensor"
#define BLYNK_AUTH_TOKEN            "K-RIbY4Xwiafj4WyQeLFu0_Fo-kawxa1"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <U8g2lib.h>
#include <DHT.h>

// Pin definitions for the DHT sensor and OLED
#define DHTPIN 4        // DHT11 sensor data pin
#define DHTTYPE DHT11   // DHT11 sensor type
#define OLED_DC 16      // OLED Data/Command pin (GPIO 16)
#define OLED_CS 5       // OLED Chip Select pin (GPIO 5)
#define OLED_RESET 17   // OLED reset pin (GPIO 17)
#define SCLK 18         // Clock pin for SPI
#define MOSI 23         // MOSI pin for SPI

// Initialize the OLED display using U8g2 (using SPI communication)
U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ OLED_CS, /* dc=*/ OLED_DC, /* reset=*/ OLED_RESET);

DHT dht(DHTPIN, DHTTYPE);  // Initialize DHT11 sensor

// Blynk timer for sending data to Blynk Cloud
BlynkTimer timer;

char ssid[] = "Fios-3LcLc";    // Your WiFi credentials
char pass[] = "was55buy742paly";  // Your WiFi password

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  int value = param.asInt();  // Set incoming value from pin V0 to a variable
  Blynk.virtualWrite(V1, value);  // Send value to Virtual Pin 1
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

// Function to send sensor data to Blynk every second
void myTimerEvent()
{
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();  // Celsius

  // Check if the readings failed
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Convert Celsius to Fahrenheit
  float tempF = (temperature * 9.0 / 5.0) + 32;

  // Send data to Blynk
  Blynk.virtualWrite(V0, humidity);  // Send humidity to Virtual Pin 0
  Blynk.virtualWrite(V1, temperature);  // Send temperature to Virtual Pin 1
  Blynk.virtualWrite(V2, tempF);  // Send temperature in Fahrenheit to Virtual Pin 2
  
}

// Function to send uptime to Blynk
void sendUptime()
{
  unsigned long uptimeMillis = millis();  // Get uptime in milliseconds

  // Convert milliseconds to seconds
  unsigned long uptimeSecs = uptimeMillis / 1000;

  // Convert seconds to minutes and hours
  unsigned long hours = uptimeSecs / 3600;
  unsigned long minutes = (uptimeSecs % 3600) / 60;
  unsigned long seconds = uptimeSecs % 60;

  // Format the uptime as a string "hh:mm:ss"
  String uptimeString = String(hours) + ":" + String(minutes) + ":" + String(seconds);

  // Send the uptime to Virtual Pin V3 on Blynk
  Blynk.virtualWrite(V3, uptimeString);
}

void setup() {
  // Start the serial monitor
  Serial.begin(115200);

  // Initialize the DHT11 sensor
  dht.begin();

  // Initialize the OLED display
  u8g2.begin();  // Initialize the display

  // Connect to Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Setup a function to be called every second
  timer.setInterval(1000L, myTimerEvent);  // Sensor data
  timer.setInterval(1000L, sendUptime);    // Uptime every second
}

void loop() {
  // Run Blynk
  Blynk.run();

  // Run the timer
  timer.run();

  // Reading temperature and humidity from DHT11
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();  // Celsius

  // Check if the readings failed
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Convert Celsius to Fahrenheit
  float tempF = (temperature * 9.0 / 5.0) + 32;

  // Clear the display buffer
  u8g2.clearBuffer();

  // Display uptime
  u8g2.setFont(u8g2_font_ncenB08_tr);  // Choose a suitable font
  u8g2.drawStr(0, 15, "Uptime:");
  u8g2.setCursor(80, 15);
  unsigned long uptimeMillis = millis();  // Get uptime in milliseconds
  unsigned long uptimeSecs = uptimeMillis / 1000;  // Convert to seconds
  unsigned long hours = uptimeSecs / 3600;
  unsigned long minutes = (uptimeSecs % 3600) / 60;
  unsigned long seconds = uptimeSecs % 60;
  String uptimeString = String(hours) + ":" + String(minutes) + ":" + String(seconds);
  u8g2.print(uptimeString);

  // Display temperature in Celsius
  u8g2.drawStr(0, 30, "Temp (C):");
  u8g2.setCursor(80, 30);
  u8g2.print(temperature);
  u8g2.print(" C");

  // Display temperature in Fahrenheit
  u8g2.drawStr(0, 45, "Temp (F):");
  u8g2.setCursor(80, 45);
  u8g2.print(tempF);
  u8g2.print(" F");

  // Display humidity
  u8g2.drawStr(0, 60, "Humidity:");
  u8g2.setCursor(80, 60);
  u8g2.print(humidity);
  u8g2.print("%");

  // Send buffer to display
  u8g2.sendBuffer();

  // Print to the serial monitor (for debugging)
  Serial.print("Uptime: ");
  Serial.print(uptimeString);
  Serial.print("  Humidity: ");
  Serial.print(humidity);
  Serial.print("%  Temperature: ");
  Serial.print(temperature);
  Serial.print("C / ");
  Serial.print(tempF);
  Serial.println("F");

  // Wait for 2 seconds before reading again
  delay(2000);
}

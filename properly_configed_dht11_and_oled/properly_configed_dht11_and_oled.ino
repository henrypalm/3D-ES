#include <U8g2lib.h>
#include <DHT.h>

// Pin definitions for SPI
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

void setup() {
  // Start the serial monitor
  Serial.begin(115200);

  // Initialize the DHT11 sensor
  dht.begin();

  // Initialize the OLED display
  u8g2.begin();  // Initialize the display
}

void loop() {
  // Reading temperature and humidity from DHT11
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check if the readings failed
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Clear the display buffer
  u8g2.clearBuffer();

  // Print humidity
  u8g2.setFont(u8g2_font_ncenB08_tr);  // Choose a suitable font
  u8g2.drawStr(0, 15, "Humidity:");
  u8g2.setCursor(80, 15);
  u8g2.print(humidity);
  u8g2.print("%");

  // Print temperature
  u8g2.drawStr(0, 30, "Temp:");
  u8g2.setCursor(80, 30);
  u8g2.print(temperature);
  u8g2.print("C");

  // Send buffer to display
  u8g2.sendBuffer();

  // Print to the serial monitor (for debugging)
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%  Temperature: ");
  Serial.print(temperature);
  Serial.println("C");

  // Wait for 2 seconds before reading again
  delay(2000);
}

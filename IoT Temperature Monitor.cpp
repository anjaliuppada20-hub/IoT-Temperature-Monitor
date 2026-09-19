#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Wi-Fi credentials
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// DHT sensor
#define DHTPIN D4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

ESP8266WebServer server(80);

float temperature = 0.0;
float humidity = 0.0;

unsigned long lastRead = 0;
const unsigned long readInterval = 2000;

// Read sensor
void readSensor() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("DHT sensor error");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}

// OLED display
void updateOLED() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("IoT Temperature");

  display.setCursor(0, 20);
  display.print("Temp: ");
  display.print(temperature, 1);
  display.println(" C");

  display.setCursor(0, 35);
  display.print("Humidity: ");
  display.print(humidity, 1);
  display.println(" %");

  display.setCursor(0, 52);
  display.print(WiFi.localIP());

  display.display();
}

// Web dashboard
void handleRoot() {
  String html = "";

  html += "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<title>IoT Temperature Monitor</title>";

  html += "<style>";
  html += "body{font-family:Arial;text-align:center;background:#f2f2f2;}";
  html += ".card{max-width:400px;margin:50px auto;padding:30px;";
  html += "background:white;border-radius:15px;box-shadow:0 4px 12px #aaa;}";
  html += ".value{font-size:40px;font-weight:bold;}";
  html += "</style>";

  html += "</head>";

  html += "<body>";

  html += "<div class='card'>";

  html += "<h1>IoT Temperature Monitor</h1>";

  html += "<h2>Temperature</h2>";
  html += "<div class='value'>";
  html += String(temperature, 1);
  html += " &deg;C</div>";

  html += "<h2>Humidity</h2>";
  html += "<div class='value'>";
  html += String(humidity, 1);
  html += " %</div>";

  html += "<p>ESP8266 Web Dashboard</p>";

  html += "</div>";

  html += "</body>";
  html += "</html>";

  server.send(200, "text/html", html);
}

// JSON API
void handleData() {
  String json = "{";

  json += "\"temperature\":";
  json += String(temperature, 1);

  json += ",\"humidity\":";
  json += String(humidity, 1);

  json += "}";

  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);

  dht.begin();

  // OLED initialization
  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C
      )) {

    Serial.println("OLED initialization failed");
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Connecting WiFi...");

  display.display();

  // Wi-Fi connection
  WiFi.begin(ssid, password);

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();

  display.setCursor(0, 0);
  display.println("WiFi Connected");

  display.setCursor(0, 20);
  display.println(WiFi.localIP());

  display.display();

  // Web routes
  server.on("/", handleRoot);
  server.on("/data", handleData);

  server.begin();

  Serial.println("Web server started");

  delay(2000);
}

void loop() {

  server.handleClient();

  if (millis() - lastRead >= readInterval) {

    lastRead = millis();

    readSensor();
    updateOLED();
  }
}

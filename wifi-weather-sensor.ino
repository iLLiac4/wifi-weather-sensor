/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>
#include "DHT.h"

#define DHTPIN 0     // GPIO 0 pin of ESP8266
//#define DHTPIN 2     // GPIO 2 pin of ESP8266
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

const char* ssid     = "SSID";
const char* password = "PASSWORD";

const char* host = "data.sparkfun.com";
const char* publicKey = "....................";
const char* privateKey = "....................";

DHT dht(DHTPIN, DHTTYPE, 30); // 30 is for cpu clock of esp8266 80Mhz

void setup() {
  Serial.begin(115200);

  dht.begin();

  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("ESP8266 Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  delay(5000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index
  // Must send in temp in Fahrenheit!
  float   hi = 0.5 * (f + 61.0 + ((f - 68.0) * 1.2) + (h * 0.094));
  
  if (hi > 79) {
    hi = -42.379 +
             2.04901523 * f +
            10.14333127 * h +
            -0.22475541 * f*h +
            -0.00683783 * pow(f, 2) +
            -0.05481717 * pow(h, 2) +
             0.00122874 * pow(f, 2) * h +
             0.00085282 * f*pow(h, 2) +
            -0.00000199 * pow(f, 2) * pow(h, 2);

    if((h < 13) && (f >= 80.0) && (f <= 112.0))
      hi -= ((13.0 - h) * 0.25) * sqrt((17.0 - abs(f - 95.0)) * 0.05882);

    else if((h > 85.0) && (f >= 80.0) && (f <= 87.0))
      hi += ((h - 85.0) * 0.1) * ((87.0 - f) * 0.2);
  }
  float his = (hi - 32) * 0.5555555556;

// Print

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(his);
  Serial.println(" *C");

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/input/";
  url += publicKey;
  url += "?private_key=";
  url += privateKey;
  url += "&temp=";
  url += t;
  url += "&hum=";
  url += h;
  url += "&hidx=";
  url += his;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(10);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
  delay(20 * 1000); // Send data every 20 seconds
  ESP.deepSleep(35 * 60000000); // Send to sleep for 35 minutes
}

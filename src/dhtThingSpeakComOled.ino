#include <ESP8266WiFi.h>
#include "DHT.h"
#include <Adafruit_SSD1306.h>

/***************************
 * Begin Settings
 **************************/


const char* ssid     = "**********";
const char* password = "*********";

const char* host = "api.thingspeak.com";

const char* THINGSPEAK_API_KEY = "**********";

// Oled display Settings
#define OLED_RESET 0
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

// DHT Settings
#define DHTPIN 2
#define DHTTYPE DHT11

// Update every 600 seconds = 10 minutes. Min with Thingspeak is ~20 seconds
const int UPDATE_INTERVAL_SECONDS = 600;

/***************************
 * End Settings
 **************************/
 
// Initialize the temperature/ humidity sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(10);

  // set display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2000);
  display.clearDisplay();

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
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

void showTemp(float temp,float hud) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(3,1);
  display.println("Homersek.:");
  display.setCursor(3,10);
  display.print(temp);
  display.println("C");
  display.setCursor(3,20);
  display.println("Paratart.:");
  display.setCursor(3,30);
  display.print(hud);
  display.println("%");
  display.display();
  display.clearDisplay();
}

void loop() {      
    Serial.print("connecting to ");
    Serial.println(host);
    
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }

    // read values from the sensor
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature(false)-3;
    showTemp(temperature,humidity);   // show temp to oled display
    
    // We now create a URI for the request
    String url = "/update?api_key=";
    url += THINGSPEAK_API_KEY;
    url += "&field1=";
    url += String(temperature);
    url += "&field2=";
    url += String(humidity);
    
    Serial.print("Requesting URL: ");
    Serial.println(url);
    
    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");
    delay(10);
    while(!client.available()){
      delay(100);
      Serial.print(".");
    }
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    
    Serial.println();
    Serial.println("closing connection");


  // Go back to sleep. If your sensor is battery powered you might
  // want to use deep sleep here
  delay(1000 * UPDATE_INTERVAL_SECONDS);
}
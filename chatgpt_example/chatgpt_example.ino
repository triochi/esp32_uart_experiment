#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>


// LED Setup
#define LED_PIN 8
#define NUM_LEDS 1
Adafruit_NeoPixel pixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Access Point credentials
const char *apSSID = "ESP32C3_AP";
const char *apPassword = "12345678";

// Wi-Fi Client (Station) credentials
const char *staSSID = "AgenDuke";
const char *staPassword = "AgenDuke@1234!";
WebServer server(80);

// Function to determine if request comes from SoftAP or STA
bool getConnectionIsAPType(IPAddress clientIP) {
    IPAddress apIP = WiFi.softAPIP();  // SoftAP's IP (e.g., 192.168.4.1)
    uint32_t subnet = WiFi.softAPSubnetCIDR(); // Get SoftAP subnet mask

    if ((clientIP & subnet) == (apIP & subnet)) {
        return true;
    } else {
        return false;
    }
}

// Function to handle JSON POST requests
void handleRoot() {
   IPAddress clientIP = server.client().remoteIP();  // Get client's IP address
  bool isAPSequestType = getConnectionIsAPType(clientIP);
  if (server.method() == HTTP_GET) {
      pixel.setPixelColor(0, pixel.Color(128, 255, 0));
      pixel.show();   // Turn off all pixels initially
      server.send(200, "text/plain", "hello from esp32!");
      pixel.setPixelColor(0, pixel.Color(255, 0, 0));
      pixel.show();   // Turn off all pixels initially
  }
  else if (server.method() == HTTP_POST) {
    if (server.hasArg("plain")) {  // Check if there's raw data
        String jsonData = server.arg("plain");  // Get the JSON payload
        Serial.println("Received JSON:");
        Serial.println(jsonData);

        // Create a JSON object to parse
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, jsonData);

        if (error) {
            Serial.println("JSON Parsing Failed!");
            server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
            return;
        }

        // Extract values from JSON
        command = doc["command"].as<String>();
        identifier = doc["identifier"].as<int>();
        delayTimeInSec = doc["delayTimeInSec"].as<int>();
        workTimeInSec = doc["workTimeInSec"].as<int>();
        coolTimeInSec = doc["coolTimeInSec"].as<int>();

        // Print values to Serial Monitor
        Serial.println("Parsed JSON Data:");
        Serial.println("Command: " + command);
        Serial.println("Identifier: " + String(identifier));
        Serial.println("Delay Time: " + String(delayTimeInSec) + " sec");
        Serial.println("Work Time: " + String(workTimeInSec) + " sec");
        Serial.println("Cool Time: " + String(coolTimeInSec) + " sec");

        // Send a success response
        server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Command received\"}");
    } else {
        server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No JSON received\"}");
    }
  }
  else {
    server.send(405, "application/json", "{\"status\":\"error\",\"message\":\"Method Not Allowed\"}");
  }
}

void setup() {
    Serial.begin(115200);
    pixel.begin();  // Initialize LED
    pixel.setPixelColor(0, pixel.Color(255, 0, 0));
    pixel.show();   // Turn off all pixels initially
    // Start Access Point
    WiFi.softAP(apSSID, apPassword);
    Serial.println("Access Point started");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Connect to Wi-Fi as a client
    WiFi.begin(staSSID, staPassword);
    Serial.print("Connecting to WiFi...");
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected to WiFi!");
    Serial.print("Station IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.begin();
    Serial.println("HTTP server started");
//    
//    setColor(255, 0, 0);  // Red
//    delay(1000);
//    setColor(0, 255, 0);  // Green
//    delay(1000);
//    setColor(0, 0, 255);  // Blue
//    delay(1000);
//    setColor(0, 0, 0);    // Off
//    delay(1000);
    
}

void loop() {
    // Keep the AP and Client active
    server.handleClient();
    delay(5);  //allow the cpu to switch to other tasks
}

// Function to set color (RGB)
void setColor(int red, int green, int blue) {
    pixel.setPixelColor(0, pixel.Color(red, green, blue));
    pixel.show();
}

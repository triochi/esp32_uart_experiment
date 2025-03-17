#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>


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


void handleRoot() {
//  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp32!");
//  digitalWrite(led, 0);
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

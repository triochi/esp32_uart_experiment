#include <WiFi.h>
#include <WebServer.h>

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
}

void loop() {
    // Keep the AP and Client active
    server.handleClient();
    delay(5);  //allow the cpu to switch to other tasks
}

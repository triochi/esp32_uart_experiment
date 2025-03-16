#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* ssid = "........";
const char* password = "........";


// Access Point credentials
const char *apSSID = "ESP32C3_AP";
const char *apPassword = "12345678";

// Wi-Fi Client (Station) credentials
const char *staSSID = "AgenDuke";
const char *staPassword = "AgenDuke@1234!";

WebServer server(80);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp32!");
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
//
//  // Start Access Point
//  WiFi.softAP(apSSID, apPassword);
  Serial.println("Access Point started");
  Serial.print("AP IP Address: ");
//  Serial.println(WiFi.softAPIP());
  
//  // Connect to Wi-Fi as a client
//  WiFi.mode(WIFI_STA);
//  WiFi.begin(staSSID, staPassword);
//  Serial.println("");
//
//  // Wait for connection
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }
//  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(staSSID);
  Serial.print("IP address: ");
//  Serial.println(WiFi.localIP());

//  if (MDNS.begin("esp32")) {
//    Serial.println("MDNS responder started");
//  }
//
//  server.on("/", handleRoot);
//
//  server.on("/inline", []() {
//    server.send(200, "text/plain", "this works as well");
//  });
//
//  server.onNotFound(handleNotFound);

//  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(2);//allow the cpu to switch to other tasks
}

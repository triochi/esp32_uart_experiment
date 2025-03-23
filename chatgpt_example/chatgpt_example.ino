#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_mac.h"


// LED Setup
#define LED_PIN 8
#define NUM_LEDS 1

#define BOOT_BUTTON 9  // GPIO for BOOT button
#define RESET_HOLD_TIME 5000  // 5 seconds hold time

Adafruit_NeoPixel pixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Access Point credentials
const char *apSSID = "ESP32C3_AP";
const char *apPassword = "12345678";

// Wi-Fi Client (Station) credentials
const char *staSSID = "AgenDuke";
const char *staPassword = "AgenDuke@1234!";
String macAddress = "n.a";
WebServer server(80);
Preferences preferences;

void setColor(int red, int green, int blue);

// Function to determine if request comes from SoftAP or STA
bool getConnectionIsAPType(IPAddress clientIP) {
    IPAddress apIP = WiFi.softAPIP();  // SoftAP's IP (e.g., 192.168.4.1)
    uint32_t subnet = WiFi.softAPSubnetMask(); // Get SoftAP subnet mask
    if ((clientIP & subnet) == (apIP & subnet)) {
        return true;
    } else {
        return false;
    }
}

String getRealMacAddress() {
    uint8_t baseMac[6];
    esp_efuse_mac_get_default(baseMac); // Get the factory-set MAC

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);

    return String(macStr);
}

void handleRoot_AP() {
    String html = " <html><head> <title>SolarStudio WiFi Setup</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>body{font-family:Arial,sans-serif;display:flex;";
    html += "justify-content:center;align-items:center;height:100vh;";
    html += "background:linear-gradient(to bottom,#808080,#3c8c6c);";
    html += "margin: 0;}.container {background:rgba(255,255,255,0.9);";
    html += "padding: 20px;border-radius:10px;box-shadow: 0px 0px 10px rgba(0,0,0,0.2);";
    html += "text-align: center;width:90%;max-width:400px;}";
    html += "h2{color: #3c8c6c;}input{width:100%;padding:10px;margin: 10px 0;";
    html += "border:1px solid #ccc;border-radius:5px;font-size:16px;";
    html += "}input[type='submit']{background:#3c8c6c;color: white;";
    html += "border: none;cursor: pointer;font-weight: bold;}";
    html += "input[type='submit']:hover{background: #2c6a4c;}";
    html += ".mac {font-size: 14px;color: #555;margin-top: 15px;";
    html += "font-weight: bold;}</style></head>";
    html += "<body><div class='container'><h2>WiFi Configuration</h2>";
    html += "<form action='/save' method='post'><input type='text' name='ssid' ";
    html += "placeholder='WiFi SSID' required><br> <input type='password' ";
    html += "name='password' placeholder='WiFi Password' required><br>";
    html += "<input type='submit' value='Save & Connect'> </form>";
    html += "<p class='mac'>Device MAC: ";
    html += getRealMacAddress();
    html += "</p>";
    html += "<p class='mac'>Device STA IP address: ";
    html += WiFi.localIP().toString();
    html += "</p>";
    html += "</div></body> </html>";
    server.send(200, "text/html", html);
}


void handleRoot_STA() {
    String html = " <html><head> <title>SolarStudio 1.2</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>body{font-family:Arial,sans-serif;display:flex;";
    html += "justify-content:center;align-items:center;height:100vh;";
    html += "background:linear-gradient(to bottom,#808080,#3c8c6c);";
    html += "margin: 0;}.container {background:rgba(255,255,255,0.9);";
    html += "padding: 20px;border-radius:10px;box-shadow: 0px 0px 10px rgba(0,0,0,0.2);";
    html += "text-align: center;width:90%;max-width:400px;}";
    html += "h2{color: #3c8c6c;}input{width:100%;padding:10px;margin: 10px 0;";
    html += "border:1px solid #ccc;border-radius:5px;font-size:16px;";
    html += "}input[type='submit']{background:#3c8c6c;color: white;";
    html += "border: none;cursor: pointer;font-weight: bold;}";
    html += "input[type='submit']:hover{background: #2c6a4c;}";
    html += ".mac {font-size: 14px;color: #555;margin-top: 15px;";
    html += "font-weight: bold;}</style></head>";
    html += "<body><div class='container'><h2>SolarStudio 1.2</h2>";
    html += "<form action='/' method='post'><input type='text' name='plain' ";
    html += "placeholder='Test json string' required><br> <br>";
    html += "<input type='submit' value='Test'> </form>";
    html += "</div></body> </html>";
    server.send(200, "text/html", html);
}



// Handle form submission
void handleSave() {
    if (server.hasArg("ssid") && server.hasArg("password")) {
        String ssid = server.arg("ssid");
        String password = server.arg("password");

        // Store credentials in flash memory
        preferences.begin("wifi", false);
        preferences.putString("ssid", ssid);
        preferences.putString("password", password);
        preferences.end();

        // Send response
        server.send(200, "text/html", "<html><body><h2>WiFi Credentials Saved! Restarting...</h2></body></html>");

        delay(2000);
        ESP.restart();  // Restart to connect to Wi-Fi
    } else {
        server.send(400, "text/html", "Missing SSID or Password");
    }
}


// Function to handle JSON POST requests
void handleRoot() {
  IPAddress clientIP = server.client().remoteIP();  // Get client's IP address
  bool isAPRequestType = getConnectionIsAPType(clientIP);
  Serial.println("\nClient IP: " +server.client().remoteIP().toString());
  Serial.println(isAPRequestType?" AP":" STA");
  if (server.method() == HTTP_GET) {
    if(isAPRequestType) {
      handleRoot_AP();
    }
    else {
      handleRoot_STA();
    }
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
        String command = doc["command"].as<String>();
        int identifier = doc["identifier"].as<int>();
        int delayTimeInSec = doc["delayTimeInSec"].as<int>();
        int workTimeInSec = doc["workTimeInSec"].as<int>();
        int coolTimeInSec = doc["coolTimeInSec"].as<int>();

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


// Connect to WiFi using stored credentials
bool connectToWiFi() {
    preferences.begin("wifi", true);
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    preferences.end();

    if (ssid == "") {
        Serial.println("No Wi-Fi credentials stored.");
        return false;  // No saved credentials
    }

    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connecting to Wi-Fi: ");
    Serial.println(ssid);
    macAddress = WiFi.macAddress();  // Get MAC address

    int timeout = 20; // 10 seconds timeout
    while (WiFi.status() != WL_CONNECTED && timeout-- > 0) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
        return true;
    } else {
        Serial.println("\nWi-Fi Connection Failed.");
        return false;
    }
}

// Function to erase WiFi credentials and reset the device
void factoryReset() {
    Serial.println("Erasing WiFi credentials...");
    
    preferences.begin("wifi", false);
    preferences.clear();  // Clear stored WiFi credentials
    preferences.end();
    
    Serial.println("Restarting device...");
    delay(2000);
    ESP.restart();  // Restart ESP32 to apply changes
}

void setup() {
    Serial.begin(115200);
    pixel.begin();  // Initialize LED
    pixel.setPixelColor(0, pixel.Color(255, 0, 0));
    pixel.show();   // Turn off all pixels initially
    pinMode(BOOT_BUTTON, INPUT_PULLUP);  // Set BOOT button as input with pull-up
    if (!connectToWiFi()) {
        setColor(0, 0, 255);  // Blue
    }
    else {
        setColor(0, 255, 0);  // Green
    }

    Serial.println("Starting SoftAP mode...");
    WiFi.softAP(apSSID, apPassword);
    Serial.print("SoftAP IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Web server routes
    server.on("/", handleRoot);
    server.on("/save", HTTP_POST, handleSave);

    server.begin();
    Serial.println("Web Server started in SoftAP mode.");

    
    server.on("/", handleRoot);
    server.begin();
    server.on("/save", HTTP_POST, handleSave);
    Serial.println("HTTP server started. Mac address is:");
    Serial.println(macAddress);



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
    static unsigned long buttonPressTime = 0;
    static bool buttonHeld = false;

    if (digitalRead(BOOT_BUTTON) == LOW) {  // Button is pressed
        if (!buttonHeld) {  // First detection of press
            buttonPressTime = millis();  // Save time of press
            buttonHeld = true;
            Serial.println("BOOT button pressed...");
        }
        
        // Check if button is held for required time
        if (millis() - buttonPressTime >= RESET_HOLD_TIME) {
            Serial.println("Factory Reset Initiated!");
            setColor(128, 0, 128); // PURPLE
            factoryReset();
        }
    } else {
        buttonHeld = false;  // Reset state when button is released
    }
    // Keep the AP and Client active
    server.handleClient();
    delay(5);  //allow the cpu to switch to other tasks
}

// Function to set color (RGB)
void setColor(int red, int green, int blue) {
    pixel.setPixelColor(0, pixel.Color(red, green, blue));
    pixel.show();
}

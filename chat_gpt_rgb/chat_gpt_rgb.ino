#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

// Wi-Fi Credentials
const char* ssid = "AgenDuke";
const char* password = "AgenDuke@1234!";

// LED Setup
#define LED_PIN 8
#define NUM_LEDS 1
Adafruit_NeoPixel pixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Web Server on port 80
WebServer server(80);

// Variables to store RGB values
int red = 0, green = 0, blue = 0;

// HTML Web Page
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 RGB Controller</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; background-color: #222; color: white; }
        input { width: 80%%; }
        .slider-container { margin: 20px; }
    </style>
</head>
<body>
    <h2>ESP32 RGB LED Controller</h2>
    <div class="slider-container">
        <label>Red: <span id="redVal">0</span></label>
        <input type="range" min="0" max="255" value="0" id="red" oninput="updateColor()">
    </div>
    <div class="slider-container">
        <label>Green: <span id="greenVal">0</span></label>
        <input type="range" min="0" max="255" value="0" id="green" oninput="updateColor()">
    </div>
    <div class="slider-container">
        <label>Blue: <span id="blueVal">0</span></label>
        <input type="range" min="0" max="255" value="0" id="blue" oninput="updateColor()">
    </div>
    <script>
        function updateColor() {
            let r = document.getElementById("red").value;
            let g = document.getElementById("green").value;
            let b = document.getElementById("blue").value;
            document.getElementById("redVal").innerText = r;
            document.getElementById("greenVal").innerText = g;
            document.getElementById("blueVal").innerText = b;
            fetch(`/setColor?r=${r}&g=${g}&b=${b}`);
        }
    </script>
</body>
</html>
)rawliteral";

// Function to handle root page
void handleRoot() {
    server.send_P(200, "text/html", htmlPage);
}

// Function to update LED color
void handleSetColor() {
    if (server.hasArg("r")) red = server.arg("r").toInt();
    if (server.hasArg("g")) green = server.arg("g").toInt();
    if (server.hasArg("b")) blue = server.arg("b").toInt();

    pixel.setPixelColor(0, pixel.Color(red, green, blue));
    pixel.show();
    server.send(200, "text/plain", "OK");
}

void setup() {
    Serial.begin(115200);
    
    // Initialize LED
    pixel.begin();
    pixel.show(); // Turn off LED initially

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Start Web Server
    server.on("/", handleRoot);
    server.on("/setColor", handleSetColor);
    server.begin();
    Serial.println("Web Server started!");
}

void loop() {
    server.handleClient();
}

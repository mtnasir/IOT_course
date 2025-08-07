#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "*****";       // Replace with your WiFi SSID
const char* password = "******";  // Replace with your WiFi password

WebServer server(80);             // Create a web server object
const int ledPin = 2;             // GPIO2 for the LED

// --- Function Prototypes for Route Handlers ---
void handleRoot();
void handleLEDControl();
void handleNotFound();

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);        // Set LED pin as output

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());

  // --- Define Server Routes Using Function Handlers ---
  server.on("/", HTTP_GET, handleRoot);
  server.on("/LED", HTTP_GET, handleLEDControl);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  server.handleClient(); // Handle client requests
}

// --- Route Handler Implementations ---

/**
 * @brief Handles requests to the root URL ("/").
 */
void handleRoot() {
  String message = "Welcome!\nUse /LED?value=<0-255> to control brightness.";
  server.send(200, "text/plain", message);
}

/**
 * @brief Handles requests to control the LED brightness ("/LED").
 *        Parses the 'value' query parameter to set the PWM duty cycle.
 */
void handleLEDControl() {
  if (server.hasArg("value")) {
    int pwmValue = server.arg("value").toInt();
    if (pwmValue >= 0 && pwmValue <= 255) {
      analogWrite(ledPin, pwmValue); // Set PWM duty cycle
      String response = "Brightness set to " + String(pwmValue);
      server.send(200, "text/plain", response);
    } else {
      server.send(400, "text/plain", "Error: Value must be between 0 and 255.");
    }
  } else {
    server.send(400, "text/plain", "Error: Missing 'value' parameter.");
  }
}

/**
 * @brief Handles requests to any URL not explicitly defined.
 */
void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}
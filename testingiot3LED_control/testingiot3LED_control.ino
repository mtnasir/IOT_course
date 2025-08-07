#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "*****";       // Replace with your WiFi SSID
const char* password = "******";  // Replace with your WiFi password

// Create a web server object
WebServer server(80);

// LED pin
const int ledPin = 2; // GPIO2 (D2) for the LED

// Function prototypes
void handleRoot();
void handleLEDOn();
void handleLEDOff();
void handleNotFound();

void setup() {
  // Start Serial communication
  Serial.begin(115200);
  delay(10);

  // Configure LED pin as output
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Ensure LED is off initially

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Define server routes
  server.on("/", HTTP_GET, handleRoot);       // Root route
  server.on("/LED_ON", HTTP_GET, handleLEDOn); // Turn LED on
  server.on("/LED_OFF", HTTP_GET, handleLEDOff); // Turn LED off
  server.onNotFound(handleNotFound);          // Handle unknown routes

  // Start the server
  server.begin();
  Serial.println("HTTP server started.");
}

void handleRoot() {
  // Send a simple response
  String message = "Welcome to the LED control server!\n";
  message += "Use /LED_ON to turn the LED on.\n";
  message += "Use /LED_OFF to turn the LED off.";
  server.send(200, "text/plain", message);
}

void handleLEDOn() {
  digitalWrite(ledPin, HIGH); // Turn the LED on
  Serial.println("LED turned ON");
  server.send(200, "text/plain", "LED is now ON");
}

void handleLEDOff() {
  digitalWrite(ledPin, LOW); // Turn the LED off
  Serial.println("LED turned OFF");
  server.send(200, "text/plain", "LED is now OFF");
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

void loop() {
  server.handleClient(); // Handle incoming client requests
}
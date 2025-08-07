#include <WiFi.h>
#include <WebServer.h>


// Creating webserver object
WebServer server(80); // Create a webserver object that listens for HTTP requests on port 80

// Defining the WiFi credentials
const char* ssid = "*****";
const char* password = "******";


void handleRoot();              // Function prototypes for HTTP handlers
void handleNotFound();
void handleanalogue();          // Function to handle ADC readings

// ADC sensor pin
const int adcPin = 34; // Use GPIO34 (ADC1 channel) for analog input on ESP32

void setup(void) {
  Serial.begin(115200); // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  // Start WiFi connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); // This will display the IP address of the ESP32

  // Setting the webserver handlers
  server.on("/", HTTP_GET, handleRoot);        // Call the 'handleRoot' function when a client requests URI "/"
  server.onNotFound(handleNotFound);           // When a client requests an unknown URI, call function "handleNotFound"
  server.on("/ADC", HTTP_GET, handleanalogue); // Handle ADC readings
  server.begin();                              // Start the server
  Serial.println("HTTP server started");

  // ADC setup
  pinMode(adcPin, INPUT); // Configure the ADC pin as input
}

void handleanalogue() {
  // Read the ADC value
  int adcValue = analogRead(adcPin);
  float voltage = (adcValue / 4095.0) * 3.3; // Convert ADC value to voltage (assuming 3.3V reference)

  // Example: Convert voltage to a water level percentage (adjust as needed for your sensor)
  float ADC = (voltage / 3.3) * 100.0;

  // Create a response string
  String response = "The ADC reading is: " + String(adcValue) + "\n";
  response += "Voltage: " + String(voltage, 2) + " V\n";
 

  Serial.println(response);
  server.send(200, "text/plain", response); // Send the response to the client
}

void handleRoot() {
  Serial.println("handleRoot");
  server.send(200, "text/plain", "Hello IoT"); // Send a simple response
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 when there's no handler for the URI
}

void loop(void) {
  server.handleClient(); // Listen for HTTP requests from clients
  delay(20);
}
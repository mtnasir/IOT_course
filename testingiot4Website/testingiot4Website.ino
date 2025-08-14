#include <WiFi.h>
#include <WebServer.h>

const char* SSID = "********";
const char* PASS = "*******";

WebServer server(80);

const int ADC_PIN = 36;

void HandleADC();
void HandleRoot();

void setup(void) {
  Serial.begin(9600);
  WiFi.begin(SSID, PASS);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  // This will display the IP address of the ESP32

  pinMode(ADC_PIN, INPUT);

  server.on("/", HTTP_GET, HandleRoot);
  server.on("/ADC", HTTP_GET, HandleADC);
  server.begin();  // Start the server
  Serial.println("HTTP server started");
}

void HandleRoot() {
  const char html[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 ADC</title>

</head>
<body>
  <h2>ESP32 ADC</h2>
  <div id="reading">ADC value = --</div>
  <button id="btn">Read ADC</button>

  <script>
    async function readADC() {
      try {
        const res = await fetch('/ADC', { cache: 'no-store' });
        const text =  await res.text();
        document.getElementById('reading').textContent = text;
      } catch (e) {
        document.getElementById('reading').textContent = 'Error reading ADC';
      }
    }
    document.getElementById('btn').addEventListener('click', readADC);
  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", String(html));
}

void HandleADC() {
  int value = analogRead(ADC_PIN);
  String Message = "ADC value = " + String(value);
  server.send(200, "text/plain", Message);
}

void loop(void) {
  server.handleClient();
  delay(20);
}
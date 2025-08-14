#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char* SSID = "************";
const char* PASS = "*********";

WebServer server(80);

// PWM/LED settings
const int LED_PIN     = 2;     // GPIO2 (often onboard LED)


// Route handlers
void handleRoot();
void handleLEDControl();
void handleNotFound();

// Minimal HTML + JavaScript (served from flash)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 PWM</title>
</head>
<body>
  <h2>ESP32 PWM Control</h2>
  <label for="pwm">PWM (0-255):</label>
  <input id="pwm" type="number" min="0" max="255" step="1" value="0">
  <button id="send">Set</button>
  <div id="status"></div>

  <script>
    async function setPWM() {
      const input = document.getElementById('pwm');
      const status = document.getElementById('status');
      const val = parseInt(input.value, 10);

      if (Number.isNaN(val) || val < 0 || val > 255) {
        status.textContent = 'Please enter a number between 0 and 255.';
        return;
      }

      try {
        const res = await fetch('/LED?value=' + encodeURIComponent(val), { cache: 'no-store' });
        const text = await res.text();
        status.textContent = text;
      } catch (e) {
        status.textContent = 'Failed to send value.';
      }
    }

    document.getElementById('send').addEventListener('click', setPWM);
    document.getElementById('pwm').addEventListener('keydown', (e) => {
      if (e.key === 'Enter') setPWM();
    });
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  // WiFi (station mode)
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(SSID, PASS);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();
  Serial.print("WiFi connected. IP: ");
  Serial.println(WiFi.localIP());

  // LEDC PWM setup
 pinMode(LED_PIN,OUTPUT);
  analogWrite(LED_PIN, 0); // start at 0 (off)

  // Routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/LED", HTTP_GET, handleLEDControl);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  server.handleClient();
}

// Handlers

void handleRoot() {
  server.send_P(200, "text/html", index_html);
}

void handleLEDControl() {
  if (!server.hasArg("value")) {
    server.send(400, "text/plain", "Error: Missing 'value' parameter.");
    return;
  }

  int pwmValue = server.arg("value").toInt();
  if (pwmValue < 0 || pwmValue > 255) {
    server.send(400, "text/plain", "Error: Value must be between 0 and 255.");
    return;
  }

  // Apply PWM
  analogWrite(2, pwmValue);

  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/plain", "Brightness set to " + String(pwmValue));
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}
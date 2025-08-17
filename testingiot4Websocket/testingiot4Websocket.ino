#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

const char* SSID = "************";
const char* PASS = "***********";

WebServer server(80);         // For serving the HTML page
WebSocketsServer webSocket(81); // WebSocket server on port 81

const int ADC_PIN = 36; // GPIO36 (ADC1_CH0)

// Forward declarations
void handleRoot();
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

void setup() {
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
  Serial.println(WiFi.localIP());

  pinMode(ADC_PIN, INPUT);

  // Serve the HTML UI
  server.on("/", HTTP_GET, handleRoot);
  server.begin();
  Serial.println("HTTP server started on port 80");

  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  Serial.println("WebSocket server started on port 81");
}

void handleRoot() {
  // Simple page that connects to the WebSocket and requests ADC on button click
  const char html[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>ESP32 ADC (WebSocket)</title>
  <style>
    body { font-family: sans-serif; margin: 2rem; }
    #reading { margin: 1rem 0; font-size: 1.2rem; }
  </style>
</head>
<body>
  <h2>ESP32 ADC (WebSocket)</h2>
  <div id="status">WebSocket: connecting...</div>
  <div id="reading">ADC value = --</div>
  <button id="btn">Read ADC</button>

  <script>
    // Build ws://<host>:81 URL (use wss:// if you add TLS support)
    const proto = (location.protocol === 'https:') ? 'wss://' : 'ws://';
    const wsUrl = proto + location.hostname + ':81/';

    const statusEl = document.getElementById('status');
    const readingEl = document.getElementById('reading');
    const btn = document.getElementById('btn');

    let ws = new WebSocket(wsUrl);

    ws.onopen = () => {
      statusEl.textContent = 'WebSocket: connected';
    };

    ws.onclose = () => {
      statusEl.textContent = 'WebSocket: disconnected';
    };

    ws.onerror = (e) => {
      statusEl.textContent = 'WebSocket: error';
      console.error('WS error', e);
    };
    ws.onmessage = (e) => {
      readingEl.textContent = e.data;
      console.log('WS <=', e.data);
    };


    btn.addEventListener('click', () => {
      if (ws.readyState === WebSocket.OPEN) {
        ws.send('read'); // Ask the ESP32 to read the ADC
      } else {
        readingEl.textContent = 'WebSocket not connected';
      }
    });
  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", String(html));
}

// WebSocket event handler
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("Client [%u] connected from %s\n", num, ip.toString().c_str());
      webSocket.sendTXT(num, "Connected");
      break;
    }
    case WStype_DISCONNECTED:
      Serial.printf("Client [%u] disconnected\n", num);
      break;

    case WStype_TEXT: {
      // Reconstruct the text message
      String msg;
      msg.reserve(length);
      for (size_t i = 0; i < length; i++) msg += (char)payload[i];

      Serial.printf("Client [%u] says: %s\n", num, msg.c_str());

      if (msg == "read") {
        int value = analogRead(ADC_PIN); // 0..4095 by default
        String reply = "ADC value = " + String(value);
        webSocket.sendTXT(num, reply);
      } else {
        webSocket.sendTXT(num, "Unknown command");
      }
      break;
    }

    default:
      break;
  }
}

void loop() {
  server.handleClient(); // Serve HTTP
  webSocket.loop();      // Handle WebSocket frames
  delay(10);
}
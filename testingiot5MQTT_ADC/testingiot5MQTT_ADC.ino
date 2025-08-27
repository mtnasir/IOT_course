/*
  ESP32 + AWS IoT Core (MQTT over TLS) — Single Topic "ADC"
  - Subscribes to: "ADC"
  - Publishes to:  "ADC" (periodic ADC reading from pin 34)

  Notes:
  - PubSubClient publishes at QoS 0. You can subscribe at QoS 1, but AWS IoT does not support QoS 2.
  - Ensure your IoT policy allows iot:Publish/iot:Subscribe/iot:Receive on arn:...:topic/ADC and topicfilter/ADC
    and iot:Connect on arn:...:client/IOTcourse1 (to match THING_NAME in secret.h).
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include <time.h>        // time, gmtime, time_t
#include <sys/time.h>    // optional (not strictly needed here, kept for completeness)

#include "secrets.h"

// ========== Pins ==========
const int ADC_PIN = 34; // input-only pin

// ========== MQTT Topic ==========
static const char TOPIC_ADC[] = "ADC";

// ========== Globals ==========
WiFiClientSecure net;
PubSubClient mqtt(net);

// ========== Helpers ==========
void connectWiFi() {
  Serial.print("WiFi: connecting to "); Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start > 30000) {
      Serial.println("\nWiFi: connection timeout, restarting...");
      ESP.restart();
    }
  }
  Serial.print("\nWiFi: connected, IP=");
  Serial.println(WiFi.localIP());
}

// Option B: Robust SNTP time sync with configTime()
// Sets system time in UTC for TLS certificate validation.
void syncTime() {
  Serial.println("Fetching time from NTP via SNTP...");
  // Use multiple reliable servers
  configTime(0, 0, "pool.ntp.org", "time.google.com");

  struct tm tm_info;
  bool ok = false;

  // Wait up to ~30 seconds in 1s chunks for SNTP to set time
  for (int i = 0; i < 30; i++) {
    if (getLocalTime(&tm_info, 1000)) { // waits up to 1s each iteration
      ok = true;
      break;
    }
    Serial.print(".");
  }

  if (!ok) {
    Serial.println("\nNTP sync failed; restarting...");
    delay(1000);
    ESP.restart();
  }

  // At this point, system time is set (UTC)
  time_t now = time(nullptr);
  char ts[25];
  strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
  Serial.print("Current UTC time: ");
  Serial.println(ts);
}

void onMessage(char* topic, byte* payload, unsigned int length) {
  String msg;
  msg.reserve(length);
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  Serial.print("MQTT message on ["); Serial.print(topic); Serial.print("]: ");
  Serial.println(msg);

  // With a single "ADC" topic, we just print what we get.
  // If you publish here and are also subscribed to "ADC", you may see your own messages echoed back.
}

void ensureMqttConnected() {
  while (!mqtt.connected()) {
    Serial.print("MQTT: connecting as clientId=");
    Serial.println(THING_NAME);

    // Connect (no user/pass for AWS IoT)
    if (mqtt.connect(THING_NAME)) {
      Serial.println("MQTT: connected");
      // Subscribe to the single topic
      mqtt.subscribe(TOPIC_ADC, 1); // QoS 1 subscribe is fine; publishes remain QoS 0
    } else {
      Serial.print("MQTT connect failed, state=");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

// ========== Arduino ==========
void setup() {
  Serial.begin(115200);
  delay(100);

  connectWiFi();
  syncTime(); // Do SNTP sync (sets system UTC time) before TLS/MQTT

  // TLS credentials
  net.setCACert(AWS_ROOT_CA);
  net.setCertificate(DEVICE_CERT);
  net.setPrivateKey(DEVICE_PRIVATE_KEY);

  // MQTT setup
  mqtt.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORT);
  mqtt.setKeepAlive(60);
  mqtt.setBufferSize(512); // optional
  mqtt.setCallback(onMessage);

  ensureMqttConnected();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  if (!mqtt.connected()) {
    ensureMqttConnected();
  }
  mqtt.loop();

  // Publish ADC every 5 seconds
  static unsigned long lastPub = 0;
  if (millis() - lastPub > 5000) {
    lastPub = millis();

    int adcRaw = analogRead(ADC_PIN); // 0..4095 default on ESP32
    char payload[16];
    snprintf(payload, sizeof(payload), "%d", adcRaw);

    bool ok = mqtt.publish(TOPIC_ADC, payload); // PubSubClient publishes at QoS 0
    Serial.print("Publish "); Serial.print(TOPIC_ADC);
    Serial.print(" = "); Serial.print(payload);
    Serial.println(ok ? " [ok]" : " [fail]");
  }
}
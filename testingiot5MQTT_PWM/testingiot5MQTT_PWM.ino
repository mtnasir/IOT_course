/*
  ESP32 + AWS IoT Core (MQTT over TLS)
  - Subscribes to topic: "PWM"
  - Controls LED on pin 2 with duty 0..255 (or "ON"=255, "OFF"=0)

  Ensure your AWS IoT policy allows:
    - iot:Connect   on arn:...:client/IOTcourse1
    - iot:Subscribe on arn:...:topicfilter/PWM
    - iot:Receive   on arn:...:topic/PWM
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <time.h>
#include <sys/time.h>  // optional (not strictly needed, mirrors code2)

#include "secret.h"

// ========== Pins ==========
const int LED_PIN = 2;   // Built-in LED on many ESP32 boards

// ========== PWM (LEDC) ==========
const int PWM_CHANNEL  = 0;
const int PWM_FREQ_HZ  = 1000;
const int PWM_RES_BITS = 8; // duty: 0..255

// ========== MQTT Topic ==========
static const char TOPIC_PWM[] = "PWM";

// ========== Globals ==========
WiFiClientSecure net;
PubSubClient mqtt(net);

// ========== Wi-Fi ==========
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

// ========== Time Sync (updated to match code2) ==========
// Robust SNTP time sync with configTime(), prints ISO-8601 UTC timestamp
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

// ========== MQTT callback ==========
void onMessage(char* topic, byte* payload, unsigned int length) {
  String msg;
  msg.reserve(length);
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  Serial.print("MQTT message on ["); Serial.print(topic); Serial.print("]: ");
  Serial.println(msg);

  if (strcmp(topic, TOPIC_PWM) == 0) {
    int duty = -1;
    if (msg.equalsIgnoreCase("ON")) {
      duty = 255;
    } else if (msg.equalsIgnoreCase("OFF")) {
      duty = 0;
    } else {
      duty = msg.toInt(); // non-numeric -> 0, we'll clamp below
    }

    if (duty < 0) duty = 0;
    if (duty > 255) duty = 255;

    ledcWrite(PWM_CHANNEL, duty);
    Serial.printf("Set PWM duty to %d on LED pin %d\n", duty, LED_PIN);
  }
}

// ========== MQTT connect ==========
void ensureMqttConnected() {
  while (!mqtt.connected()) {
    Serial.print("MQTT: connecting as clientId=");
    Serial.println(THING_NAME);

    if (mqtt.connect(THING_NAME)) {
      Serial.println("MQTT: connected");
      // Subscribe with QoS 1 (AWS IoT supports QoS 0/1)
      mqtt.subscribe(TOPIC_PWM, 1);
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

  // PWM setup on LED pin
  ledcSetup(PWM_CHANNEL, PWM_FREQ_HZ, PWM_RES_BITS);
  ledcAttachPin(LED_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 0); // start off

  connectWiFi();
  syncTime(); // Do SNTP sync (sets system UTC time) before TLS/MQTT

  // TLS credentials
  net.setCACert(AWS_ROOT_CA);
  net.setCertificate(DEVICE_CERT);
  net.setPrivateKey(DEVICE_PRIVATE_KEY);

  // MQTT setup
  mqtt.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORT);
  mqtt.setKeepAlive(60);
  mqtt.setBufferSize(256); // adjust to 512/1024 if you expect larger payloads
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
}
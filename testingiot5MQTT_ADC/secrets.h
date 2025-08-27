// secret.h
#pragma once

// ========== Wi‑Fi ==========
static const char WIFI_SSID[] = "YOUR_WIFI_SSID";
static const char WIFI_PASS[] = "YOUR_WIFI_PASSWORD";

// ========== AWS IoT ==========
static const char AWS_IOT_ENDPOINT[] = "your-endpoint-ats.iot.us-east-1.amazonaws.com"; // e.g., a1b2c3d4e5f6-ats.iot.us-east-1.amazonaws.com
static const int  AWS_IOT_PORT      = 8883;

// MQTT Client ID (ThingName). Set to match your policy (e.g., IOTcourse1).
static const char THING_NAME[] = "IOTcourse1";

// ========== Certificates / Keys (PEM) ==========
// Paste full PEM contents including BEGIN/END lines between the markers.

// Amazon Root CA (Amazon Root CA 1 or the CA used by your endpoint)
static const char AWS_ROOT_CA[] PROGMEM = R"PEM(
-----BEGIN CERTIFICATE-----
...PASTE AMAZON ROOT CA 1 HERE...
-----END CERTIFICATE-----
)PEM";

// Your device certificate (public cert)
static const char DEVICE_CERT[] PROGMEM = R"PEM(
-----BEGIN CERTIFICATE-----
...PASTE YOUR DEVICE CERT HERE...
-----END CERTIFICATE-----
)PEM";

// Your device private key
static const char DEVICE_PRIVATE_KEY[] PROGMEM = R"PEM(
-----BEGIN PRIVATE KEY-----
...PASTE YOUR DEVICE PRIVATE KEY HERE...
-----END PRIVATE KEY-----
)PEM";

// Optional: Device public key (not used by MQTT/TLS on ESP32)
static const char DEVICE_PUBLIC_KEY[] PROGMEM = R"PEM(
-----BEGIN PUBLIC KEY-----
...OPTIONAL: PASTE YOUR DEVICE PUBLIC KEY HERE OR LEAVE EMPTY...
-----END PUBLIC KEY-----
)PEM";


// ***************** the aws policy is suggested to be:
// {
//   "Version": "2012-10-17",
//   "Statement": [
//     {
//       "Sid": "AllowConnectAsIOTcourse1",
//       "Effect": "Allow",
//       "Action": "iot:Connect",
//       "Resource": "arn:********************:client/IOTcourse1"
//     },
//     {
//       "Sid": "AllowPublishADC",
//       "Effect": "Allow",
//       "Action": "iot:Publish",
//       "Resource": "arn:*********************:topic/ADC"
//     },
//     {
//       "Sid": "AllowSubscribeADC",
//       "Effect": "Allow",
//       "Action": "iot:Subscribe",
//       "Resource": "arn:******************:topicfilter/ADC" //eg: arn:aws:iot:us-east-1:usernumber:topicfilter/ADC
//     },
//     {
//       "Sid": "AllowReceiveADC",
//       "Effect": "Allow",
//       "Action": "iot:Receive",
//       "Resource": "arn:***********************:topic/ADC"
//     }
//   ]
// }
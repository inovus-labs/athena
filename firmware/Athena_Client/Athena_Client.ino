#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// ==== WiFi credentials ====
const char* SSID = "Chackomash";
const char* PASS = "Ormailla";

// ==== Button config ====
// Option A: jumper-as-button. Connect GPIO5 to GND momentarily to trigger.
#define BUTTON_PIN 5
#define PRESS_ACTIVE_LOW true   // input goes LOW when pressed

// ==== Default target URL ====
const char* DEFAULT_URL = "https://api.gigwork.co.in";

// ====== WiFi connect ======
bool connectWiFi(uint32_t timeoutMs = 15000) {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  Serial.printf("Connecting to WiFi SSID=\"%s\"...\n", SSID);
  WiFi.begin(SSID, PASS);

  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < timeoutMs) {
    delay(250);
    Serial.print('.');
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWiFi connected. IP=%s RSSI=%d\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI());
    return true;
  }
  Serial.println("\nWiFi connect timeout.");
  return false;
}

// ====== HTTPS GET ======
void sendGETRequest(const String& url) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }

  Serial.println("Sending GET to: " + url);

  WiFiClientSecure client;
  client.setInsecure(); // For quick testing; replace with root CA for production

  HTTPClient http;
  if (!http.begin(client, url)) {
    Serial.println("HTTP begin() failed");
    return;
  }

  int code = http.GET();
  if (code > 0) {
    Serial.printf("HTTP %d\n", code);
  } else {
    Serial.printf("HTTP error: %s\n", http.errorToString(code).c_str());
  }
  http.end();
}

// ====== Serial command handler ======
void handleSerial() {
  if (!Serial.available()) return;

  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return;

  if (line.startsWith("GET")) {
    String url = String(DEFAULT_URL);
    int sp = line.indexOf(' ');
    if (sp > 0) {
      url = line.substring(sp + 1);
      url.trim();
      if (url.length() == 0) url = String(DEFAULT_URL);
    }
    sendGETRequest(url);
  } else if (line.equalsIgnoreCase("IP")) {
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  } else if (line.equalsIgnoreCase("RSSI")) {
    Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
  } else if (line.equalsIgnoreCase("HELP")) {
    Serial.println("Commands:");
    Serial.println("  GET                  -> request default URL");
    Serial.println("  GET <url>            -> request given URL");
    Serial.println("  IP / RSSI / HELP");
  } else {
    Serial.println("Unknown command. Type HELP.");
  }
}

// ====== Button helpers ======
bool isButtonPressed() {
  int v = digitalRead(BUTTON_PIN);
  return PRESS_ACTIVE_LOW ? (v == LOW) : (v == HIGH);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(BUTTON_PIN, PRESS_ACTIVE_LOW ? INPUT_PULLUP : INPUT);

  connectWiFi();

  Serial.println("\nReady.");
  Serial.println("Type commands: GET / GET <url> / IP / RSSI / HELP");
  Serial.println("Or touch GPIO5 jumper to GND to trigger default GET.");
}

void loop() {
  handleSerial();

  static bool wasPressed = false;
  static uint32_t lastPressAt = 0;
  bool pressed = isButtonPressed();
  if (pressed && !wasPressed) {
    uint32_t now = millis();
    if (now - lastPressAt > 300) { // debounce
      sendGETRequest(DEFAULT_URL);
      lastPressAt = now;
    }
  }
  wasPressed = pressed;

  delay(10);
}

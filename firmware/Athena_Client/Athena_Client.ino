#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// ====== CAMERA PIN DEFINITIONS for XIAO ESP32-C3 Sense ======
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    10
#define SIOD_GPIO_NUM    40
#define SIOC_GPIO_NUM    39

#define Y9_GPIO_NUM      48
#define Y8_GPIO_NUM      11
#define Y7_GPIO_NUM      12
#define Y6_GPIO_NUM      14
#define Y5_GPIO_NUM      16
#define Y4_GPIO_NUM      18
#define Y3_GPIO_NUM      17
#define Y2_GPIO_NUM      15
#define VSYNC_GPIO_NUM   38
#define HREF_GPIO_NUM    47
#define PCLK_GPIO_NUM    13

// ====== WIFI CONFIG ======
const char* ssid     = "Chackomash";
const char* password = "Ormailla";

// ====== BUTTON CONFIG ======
#define BUTTON_PIN 5   // connect button between GPIO5 and GND
#define PRESS_ACTIVE_LOW true

// ====== POST TARGET ======
const char* POST_URL = "https://gigwork.co.in";

// ====== WebServer ======
WebServer server(80);

// ====== CAPTURE HANDLER ======
void handleCapture() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }

  server.sendHeader("Content-Type", "image/jpeg");
  server.sendHeader("Content-Disposition", "inline; filename=capture.jpg");
  server.send_P(200, "image/jpeg", (const char *)fb->buf, fb->len);

  esp_camera_fb_return(fb);
}

// ====== POST REQUEST ======
void sendPOSTRequest(const String& url) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure(); // disable cert check for now

  HTTPClient http;
  if (!http.begin(client, url)) {
    Serial.println("HTTP begin() failed");
    return;
  }

  // Example payload
  String payload = "{\"device\":\"xiao-c3\",\"event\":\"button_press\"}";
  http.addHeader("Content-Type", "application/json");

  int code = http.POST(payload);
  if (code > 0) {
    Serial.printf("HTTP %d\n", code);
    String resp = http.getString();
    Serial.println("Response:");
    Serial.println(resp);
  } else {
    Serial.printf("HTTP error: %s\n", http.errorToString(code).c_str());
  }
  http.end();
}

// ====== WIFI CONNECT ======
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("ESP32-C3 IP Address: ");
  Serial.println(WiFi.localIP());
}

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  // ---- BUTTON ----
  pinMode(BUTTON_PIN, PRESS_ACTIVE_LOW ? INPUT_PULLUP : INPUT);

  // ---- CAMERA CONFIG ----
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_QVGA;  
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed!");
    return;
  }

  // ---- WIFI ----
  connectWiFi();

  // ---- ROUTES ----
  server.on("/", []() {
    server.send(200, "text/html", 
      "<html><body><h1>ESP32-C3 Camera</h1>"
      "<p><a href=\"/capture\">📸 Take Photo</a></p>"
      "</body></html>");
  });

  server.on("/capture", handleCapture);

  server.begin();
  Serial.println("Web server started!");
}

// ====== LOOP ======
void loop() {
  server.handleClient();

  static bool wasPressed = false;
  bool pressed = PRESS_ACTIVE_LOW ? (digitalRead(BUTTON_PIN) == LOW) : (digitalRead(BUTTON_PIN) == HIGH);

  if (pressed && !wasPressed) {
    Serial.println("Button pressed → sending POST request...");
    sendPOSTRequest(POST_URL);
  }
  wasPressed = pressed;
}

#include "esp_camera.h"
#include <WiFi.h>
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

// ====== HTTPS TARGET (single URL) ======
const char* POST_URL = "https://athena-ghc1.onrender.com/api/v1/getImageInfo";

// ====== WIFI CONNECT ======
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
}

// ====== SEND IMAGE AS MULTIPART (HTTPClient + WiFiClientSecure) ======
void sendImageMultipart() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return;
  }

  // Capture image
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed!");
    return;
  }

  // Multipart framing
  String boundary = "----ESP32Boundary7e8c9b";
  // If your server expects "image" instead of "images", change below.
  String prolog =
      "--" + boundary + "\r\n"
      "Content-Disposition: form-data; name=\"images\"; filename=\"photo.jpg\"\r\n"
      "Content-Type: image/jpeg\r\n\r\n";

  String epilog = "\r\n--" + boundary + "--\r\n";

  // Allocate one contiguous buffer for the whole body
  size_t totalLen = prolog.length() + fb->len + epilog.length();
  uint8_t* body = (uint8_t*)malloc(totalLen);
  if (!body) {
    Serial.println("malloc failed for multipart body");
    esp_camera_fb_return(fb);
    return;
  }

  // Fill the buffer: prolog + JPEG + epilog
  size_t off = 0;
  memcpy(body + off, prolog.c_str(), prolog.length());
  off += prolog.length();

  memcpy(body + off, fb->buf, fb->len);
  off += fb->len;

  memcpy(body + off, epilog.c_str(), epilog.length());
  off += epilog.length();

  if (off != totalLen) {
    Serial.println("length mismatch assembling multipart body");
    free(body);
    esp_camera_fb_return(fb);
    return;
  }

  // ---- HTTPS POST using HTTPClient over WiFiClientSecure ----
  WiFiClientSecure client;       // HTTPS
  client.setInsecure();          // NOTE: for production, pin the CA/cert instead

  HTTPClient http;
  Serial.println("Starting HTTPS POST...");
  if (!http.begin(client, POST_URL)) {
    Serial.println("http.begin() failed");
    free(body);
    esp_camera_fb_return(fb);
    return;
  }

  http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
  http.setUserAgent("esp32-cam/1.0");
  http.setTimeout(20000); // ms read timeout

  int code = http.sendRequest("POST", body, totalLen);

  Serial.print("HTTP status: ");
  Serial.println(code);

  if (code > 0) {
    String resp = http.getString();
    Serial.println("Response body:");
    Serial.println(resp);
  } else {
    Serial.print("HTTP error: ");
    Serial.println(http.errorToString(code));
  }

  http.end();
  free(body);
  esp_camera_fb_return(fb);
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

  // Image settings (adjust as needed)
  config.frame_size   = FRAMESIZE_QVGA;  // 320x240
  config.jpeg_quality = 12;              // lower = better quality, larger file
  config.fb_count     = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed!");
    return;
  }

  // ---- WIFI ----
  connectWiFi();
}

// ====== LOOP ======
void loop() {
  static bool wasPressed = false;
  bool pressed = PRESS_ACTIVE_LOW ? (digitalRead(BUTTON_PIN) == LOW)
                                  : (digitalRead(BUTTON_PIN) == HIGH);

  if (pressed && !wasPressed) {
    Serial.println("Button pressed → sending image via multipart/form-data...");
    sendImageMultipart();
  }
  wasPressed = pressed;
}

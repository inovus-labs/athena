#include <WiFi.h>
#include <HTTPClient.h>

// WiFi credentials
const char* ssid = "Alpha";
const char* password = "alphabet";

// Button pin
#define BUTTON_PIN 2  

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {  // Button pressed (active LOW)
    sendGETRequest();
    delay(1000); // debounce delay
  }
}

void sendGETRequest() {
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;

    // Target URL
    String url = "https://inovuslabs.org/";

    Serial.println("Sending GET request to: " + url);
    http.begin(url);

    int httpCode = http.GET();  // Send request
    if(httpCode > 0){
      Serial.printf("HTTP Response code: %d\n", httpCode);
    } else {
      Serial.printf("Error in request: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();  // Close connection
  } else {
    Serial.println("WiFi not connected!");
  }
}

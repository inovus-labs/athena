#include <Arduino.h>
#include <SPIFFS.h>
#include "AudioFileSourceSPIFFS.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

// Objects
AudioGeneratorMP3 *mp3;
AudioFileSourceSPIFFS *file;
AudioOutputI2S *out;

bool shouldPlay = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Type 'play' to start audio.");

  // Mount SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed!");
    while (1);
  }

  // Setup audio output using INTERNAL DAC
  out = new AudioOutputI2S(0, 1);   // 0=I2S num, 1=use internal DAC mode
  out->SetOutputModeMono(true);     // Mono output to DAC
  out->SetGain(0.5);                // Volume (0.0 - 1.0)
}

void loop() {
  // Check serial input
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.equalsIgnoreCase("play")) {
      Serial.println("Playing MP3...");
      file = new AudioFileSourceSPIFFS("/test.mp3");
      mp3 = new AudioGeneratorMP3();
      mp3->begin(file, out);
      shouldPlay = true;
    }
  }

  // Handle playback
  if (shouldPlay && mp3->isRunning()) {
    if (!mp3->loop()) {
      mp3->stop();
      Serial.println("Playback finished");
      shouldPlay = false;
    }
  }
}

# ATHENA

**ATHENA** is a DIY AI Glasses project built on **ESP32-C3**, **Gemini**, and **ElevenLabs**.  
It is designed as a **wearable assistant for the elderly and visually impaired** — giving them the power of **sight and sound** at the press of a button.  


## 🌟 What is ATHENA?

When the user **presses a side button on the glasses**:
1. **ESP32-C3** captures an image from the camera.  
2. The image is sent to a **custom backend**.  
3. **Gemini** analyzes the image and scene, and generates a **simple, conversational description**.  
4. The backend forwards this response to **ElevenLabs**, which generates **clear, natural audio**.  
5. The audio is sent back to the ESP32 and played through the built-in speaker.  

This means ATHENA can **describe the world** — helping the user understand their surroundings, identify objects, or get guidance in real-time.  


## 🛠 Tech Stack

### Hardware
- **ESP32-C3 Dev Board**  
- **Camera Module** (OV2640 or similar)  
- **I²S Speaker/DAC** for audio playback  
- **Physical Button** on the side of the glasses (GPIO input trigger)  
- **Battery Pack** for mobility  
- (Optional) **I²S Microphone** for future voice commands  

### Software
- **ESP32 Arduino firmware** (captures images, handles button input, plays audio)  
- **Custom Backend (Node.js/TS)**:
  - **Gemini** → image understanding + scene description  
  - **ElevenLabs** → converts description into natural audio  
- **Audio Streaming** back to ESP32 
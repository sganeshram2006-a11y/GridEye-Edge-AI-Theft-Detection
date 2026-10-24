#include <WiFi.h>
#include <HTTPClient.h>
#include <PZEM004Tv30.h>
#include "gridEye_retrained_random_forest_model.h"

// ============================================================
// GridEye - ESP32 Random Forest Edge Inference
// This sketch uses the retrained RandomForestClassifier
// generated from the four cleaned GridEye datasets.
//
// Features (must remain in this exact order):
// 0 V1
// 1 I1
// 2 P1
// 3 PF1
// 4 V2
// 5 I2
// 6 P2
// 7 PF2
// 8 Power_Diff  = P1 - P2
// 9 Current_Diff = I1 - I2
// 10 Voltage_Diff = abs(V1 - V2)
// ============================================================

// --- Wi-Fi Credentials ---
const char* ssid = "Your_Wifi_Name";
const char* password = "Your_Wifi_Password";

// --- ThingSpeak Settings ---
const char* serverName = "http://api.thingspeak.com/update";
String apiKey = "Your_API_KEY";

// --- PZEM Hardware Serial Setup ---
// PZEM 1: Supply / Utility side
PZEM004Tv30 pzem1(Serial2, 16, 17);

// PZEM 2: Consumer / metered side
PZEM004Tv30 pzem2(Serial1, 25, 26);

// --- ThingSpeak update interval ---
// ThingSpeak normally requires a minimum interval between updates.
// Local ML inference still runs every loop; only cloud upload is limited.
const unsigned long CLOUD_INTERVAL_MS = 15000;
unsigned long lastCloudUpload = 0;

void printPrediction(uint8_t classIndex, const double proba[4]) {
  Serial.println("==================================================");
  Serial.print(" ML PREDICTION: ");
  Serial.println(RF_CLASS_NAMES[classIndex]);

  Serial.print(" Direct Theft       : ");
  Serial.print(proba[0] * 100.0, 2);
  Serial.println("%");

  Serial.print(" Idle / Both OFF    : ");
  Serial.print(proba[1] * 100.0, 2);
  Serial.println("%");

  Serial.print(" Normal Operation   : ");
  Serial.print(proba[2] * 100.0, 2);
  Serial.println("%");

  Serial.print(" Partial Theft      : ");
  Serial.print(proba[3] * 100.0, 2);
  Serial.println("%");
  Serial.println("==================================================");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==================================================");
  Serial.println(" GridEye - RANDOM FOREST EDGE-AI INFERENCE");
  Serial.println(" ESP32 + Dual PZEM-004T");
  Serial.println("==================================================");

  WiFi.begin(ssid, password);
  Serial.print("[WiFi] Connecting");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("[WiFi] Connected!");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("[WiFi] Offline mode.");
    Serial.println("[ML] Edge inference remains active.");
  }
}

void loop() {

  // ==========================================================
  // 1. READ PZEM 1 - SUPPLY / UTILITY SIDE
  // ==========================================================
  float v1  = pzem1.voltage();
  float i1  = pzem1.current();
  float p1  = pzem1.power();
  float pf1 = pzem1.pf();

  delay(100);

  // ==========================================================
  // 2. READ PZEM 2 - CONSUMER / METERED SIDE
  // ==========================================================
  float v2  = pzem2.voltage();
  float i2  = pzem2.current();
  float p2  = pzem2.power();
  float pf2 = pzem2.pf();

  // ==========================================================
  // 3. VALIDATE SENSOR DATA
  // ==========================================================
  if (isnan(v1) || isnan(i1) || isnan(p1) || isnan(pf1) ||
      isnan(v2) || isnan(i2) || isnan(p2) || isnan(pf2) ||
      v1 > 300.0 || v2 > 300.0 ||
      i1 > 100.0 || i2 > 100.0) {

    Serial.println("[ERROR] Invalid PZEM response.");
    delay(1000);
    return;
  }

  // ==========================================================
  // 4. FEATURE ENGINEERING
  // EXACTLY THE SAME AS PYTHON TRAINING CODE
  // ==========================================================
  double powerDiff   = (double)p1 - (double)p2;
  double currentDiff = (double)i1 - (double)i2;
  double voltageDiff = fabs((double)v1 - (double)v2);

  double features[11] = {
    (double)v1,
    (double)i1,
    (double)p1,
    (double)pf1,
    (double)v2,
    (double)i2,
    (double)p2,
    (double)pf2,
    powerDiff,
    currentDiff,
    voltageDiff
  };

  // ==========================================================
  // 5. RANDOM FOREST EDGE INFERENCE
  // ==========================================================
  double probabilities[4];

  uint8_t prediction = rf_predict(features, probabilities);

  // ==========================================================
  // 6. PRINT MEASUREMENTS
  // ==========================================================
  Serial.println();
  Serial.println("--------------------------------------------------");

  Serial.printf("Supply   : %6.1f V | %6.3f A | %6.2f W | PF %.2f\n",
                v1, i1, p1, pf1);

  Serial.printf("Consumer : %6.1f V | %6.3f A | %6.2f W | PF %.2f\n",
                v2, i2, p2, pf2);

  Serial.printf("Power Diff   : %.3f W\n", powerDiff);
  Serial.printf("Current Diff : %.3f A\n", currentDiff);
  Serial.printf("Voltage Diff : %.3f V\n", voltageDiff);

  // ==========================================================
  // 7. PRINT ACTUAL ML CLASSIFICATION
  // ==========================================================
  printPrediction(prediction, probabilities);

  // ==========================================================
  // 8. SEND ORIGINAL 8 PZEM FEATURES TO THINGSPEAK
  // ==========================================================
  if (WiFi.status() == WL_CONNECTED &&
      millis() - lastCloudUpload >= CLOUD_INTERVAL_MS) {

    HTTPClient http;

    String url = String(serverName) +
                 "?api_key=" + apiKey +
                 "&field1=" + String(v1, 3) +
                 "&field2=" + String(i1, 4) +
                 "&field3=" + String(p1, 3) +
                 "&field4=" + String(pf1, 3) +
                 "&field5=" + String(v2, 3) +
                 "&field6=" + String(i2, 4) +
                 "&field7=" + String(p2, 3) +
                 "&field8=" + String(pf2, 3);

    http.begin(url);

    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.print("[CLOUD] ThingSpeak HTTP code: ");
      Serial.println(httpCode);
    } else {
      Serial.print("[CLOUD] Upload failed: ");
      Serial.println(http.errorToString(httpCode));
    }

    http.end();
    lastCloudUpload = millis();
  }

  // Local ML inference runs continuously.
  delay(3000);
}

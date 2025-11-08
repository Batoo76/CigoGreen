#include <Arduino.h>

constexpr uint8_t EMITTER_PIN = 4;           // Digital pin powering the IR emitter diode
constexpr uint8_t SENSOR_PIN = 8;            // Digital pin reading the photodiode receiver
constexpr uint32_t REPORT_INTERVAL_MS = 500; // Interval for serial reporting
constexpr uint16_t DEBOUNCE_MS = 10;         // Minimal time between state changes to filter noise

uint32_t beamBreakCount = 0;
bool lastSensorState = HIGH;
uint32_t lastTransitionTime = 0;
uint32_t lastReportTime = 0;

void setup() {
  Serial.begin(9600);

  pinMode(EMITTER_PIN, OUTPUT);
  digitalWrite(EMITTER_PIN, HIGH); // Keep the emitter on continuously

  pinMode(SENSOR_PIN, INPUT_PULLUP); // Sensor assumed to pull LOW when beam is interrupted
  lastSensorState = digitalRead(SENSOR_PIN);
  lastTransitionTime = millis();
  lastReportTime = lastTransitionTime;

  Serial.println("Photobarrier counter ready.");
}

void loop() {
  const uint32_t now = millis();
  const bool currentState = digitalRead(SENSOR_PIN);

  if (currentState != lastSensorState && now - lastTransitionTime >= DEBOUNCE_MS) {
    if (lastSensorState == LOW && currentState == HIGH) {
      ++beamBreakCount;
      delay(200);
    }

    lastSensorState = currentState;
    lastTransitionTime = now;
  }

  if (now - lastReportTime >= REPORT_INTERVAL_MS) {
    Serial.print("Beam breaks: ");
    Serial.println(beamBreakCount);
    lastReportTime = now;
  }
}

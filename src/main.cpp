#include <Arduino.h>

#ifndef SERIAL_BAUD_RATE
#define SERIAL_BAUD_RATE 9600
#endif

#ifndef PHOTO_EMITTER_PIN
#define PHOTO_EMITTER_PIN 4
#endif

#ifndef PHOTO_SENSOR_PIN
#define PHOTO_SENSOR_PIN 8
#endif

constexpr uint32_t SERIAL_BAUD = static_cast<uint32_t>(SERIAL_BAUD_RATE);
constexpr uint8_t EMITTER_PIN = static_cast<uint8_t>(PHOTO_EMITTER_PIN);   // Digital pin powering the IR emitter diode
constexpr uint8_t SENSOR_PIN = static_cast<uint8_t>(PHOTO_SENSOR_PIN);     // Digital pin reading the photodiode receiver
constexpr uint32_t REPORT_INTERVAL_MS = 500;    // Interval for periodic serial reporting
constexpr uint16_t DEBOUNCE_MS = 10;            // Minimal time between state changes to filter noise
constexpr uint16_t MANUAL_INCREMENT_DELAY_MS = 200; // Delay after manual increment to mimic previous behavior

uint32_t beamBreakCount = 0;
bool lastSensorState = HIGH;
uint32_t lastTransitionTime = 0;
uint32_t lastReportTime = 0;

void reportCount() {
  Serial.print(F("COUNT:"));
  Serial.println(beamBreakCount);
  lastReportTime = millis();
}

void handleSerialCommands() {
  while (Serial.available() > 0) {
    const char command = static_cast<char>(tolower(Serial.read()));
    switch (command) {
      case 'm': // manual increment
        ++beamBreakCount;
        reportCount();
        if (MANUAL_INCREMENT_DELAY_MS > 0) {
          delay(MANUAL_INCREMENT_DELAY_MS);
        }
        break;
      case 'c': // current count request
        reportCount();
        break;
      case 'z': // reset count
        beamBreakCount = 0;
        reportCount();
        break;
      default:
        // ignore unknown commands
        break;
    }
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD);

  pinMode(EMITTER_PIN, OUTPUT);
  digitalWrite(EMITTER_PIN, HIGH); // Keep the emitter on continuously

  pinMode(SENSOR_PIN, INPUT_PULLUP); // Sensor assumed to pull LOW when beam is interrupted
  lastSensorState = digitalRead(SENSOR_PIN);
  lastTransitionTime = millis();
  lastReportTime = lastTransitionTime;

  Serial.println(F("Photobarrier counter ready."));
  reportCount();
}

void loop() {
  const uint32_t now = millis();
  const bool currentState = digitalRead(SENSOR_PIN);

  handleSerialCommands();

  if (currentState != lastSensorState && now - lastTransitionTime >= DEBOUNCE_MS) {
    if (lastSensorState == LOW && currentState == HIGH) {
      ++beamBreakCount;
      reportCount();
    }

    lastSensorState = currentState;
    lastTransitionTime = now;
  }

  if (now - lastReportTime >= REPORT_INTERVAL_MS) {
    reportCount();
  }
}

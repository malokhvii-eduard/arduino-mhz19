#include <Arduino.h>
#include <Mhz19.h>

Mhz19 sensor;

void swapHardwareSerial() {
  Serial.flush();
  Serial.swap();
  delay(1000);
}

void setup() {
  Serial.begin(9600);

  swapHardwareSerial();  // GPIO15 (TX), GPIO13 (RX)

  sensor.begin(&Serial);
  sensor.setMeasuringRange(Mhz19MeasuringRange::Ppm_5000);
  sensor.enableAutoCalibration();

  swapHardwareSerial();  // GPIO1 (TX), GPIO3 (RX)

  Serial.println("Pre-heating...");
  delay(180000);  // Pre-heating, 3 minutes
  Serial.println("Ready...");
}

void loop() {
  swapHardwareSerial();  // GPIO15 (TX), GPIO13 (RX)

  auto carbonDioxide = String(sensor.getCarbonDioxide()) + " ppm";

  swapHardwareSerial();  // GPIO1 (TX), GPIO3 (RX)

  Serial.println(carbonDioxide);

  delay(30000);
}

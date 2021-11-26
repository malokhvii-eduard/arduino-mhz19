#include <Arduino.h>
#include <Mhz19.h>
#include <SoftwareSerial.h>

SoftwareSerial softwareSerial(D1, D2);
Mhz19 sensor;

void setup() {
  Serial.begin(9600);
  softwareSerial.begin(9600);

  sensor.begin(&softwareSerial);
  sensor.setMeasuringRange(Mhz19MeasuringRange::Ppm_5000);
  sensor.enableAutoCalibration();

  Serial.println("Pre-heating...");
  delay(180000);  // Pre-heating, 3 minutes
  Serial.println("Ready...");
}

void loop() {
  auto carbonDioxide = String(sensor.getCarbonDioxide()) + " ppm";
  Serial.println(carbonDioxide);

  delay(30000);
}

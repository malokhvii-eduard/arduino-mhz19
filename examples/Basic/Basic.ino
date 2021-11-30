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
  sensor.enableAutoBaseCalibration();

  Serial.println("Preheating...");  // Preheating, 3 minutes
  while (!sensor.isReady()) {
    delay(50);
  }

  Serial.println("Ready...");
}

void loop() {
  auto carbonDioxide = sensor.getCarbonDioxide();
  if (carbonDioxide >= 0) {
    Serial.println(String(carbonDioxide) + " ppm");
  }

  delay(20000);
}

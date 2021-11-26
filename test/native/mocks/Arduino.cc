#include <mocks/Arduino.h>

ArduinoMock* arduino = nullptr;

unsigned long millis() {
  assert(arduino != nullptr);
  return arduino->millis();
}

ArduinoMock* createArduinoMock() {
  if (!arduino) {
    arduino = new ArduinoMock();
  }

  return arduino;
}

void destroyArduinoMock() {
  if (arduino) {
    delete arduino;
    arduino = nullptr;
  }
}

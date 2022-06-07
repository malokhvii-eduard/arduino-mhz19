#ifndef ARDUINO_MHZ19_TEST_NATIVE_MOCKS_ARDUINO_H_
#define ARDUINO_MHZ19_TEST_NATIVE_MOCKS_ARDUINO_H_

#include <math.h>
#include <stdint.h>

/* Google Test */
#include <gmock/gmock.h>

/* Mocks */
#include <mocks/Stream.h>

unsigned long millis();

class ArduinoMock {
 public:
  MOCK_METHOD(unsigned long, millis, ());
};

ArduinoMock* createArduinoMock();
void destroyArduinoMock();

#endif  // ARDUINO_MHZ19_TEST_NATIVE_MOCKS_ARDUINO_H_

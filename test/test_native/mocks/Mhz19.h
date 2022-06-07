#ifndef ARDUINO_MHZ19_TEST_NATIVE_MOCKS_MHZ19_H_
#define ARDUINO_MHZ19_TEST_NATIVE_MOCKS_MHZ19_H_

/* Google Test */
#include <gmock/gmock.h>

/* Library */
#include <Mhz19.h>

class Mhz19Mock : public Mhz19 {
 public:
  MOCK_METHOD(bool, sendCommand, (const uint8_t*), (const, override));
};

#endif  // ARDUINO_MHZ19_TEST_NATIVE_MOCKS_MHZ19_H_

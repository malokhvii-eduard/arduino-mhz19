#ifndef ARDUINO_MHZ19_TEST_NATIVE_MOCKS_STREAM_H_
#define ARDUINO_MHZ19_TEST_NATIVE_MOCKS_STREAM_H_

#include <stdint.h>

/* Google Test */
#include <gmock/gmock.h>

class StreamMock {
 public:
  MOCK_METHOD(int, available, ());
  MOCK_METHOD(int, read, ());
  MOCK_METHOD(size_t, readBytes, (uint8_t *, size_t));
  MOCK_METHOD(size_t, write, (const uint8_t *, size_t));
  MOCK_METHOD(void, flush, ());
};

using Stream = StreamMock;

#endif  // ARDUINO_MHZ19_TEST_NATIVE_MOCKS_STREAM_H_

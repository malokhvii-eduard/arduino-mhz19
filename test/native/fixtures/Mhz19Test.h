#ifndef ARDUINO_MHZ19_TEST_NATIVE_FIXTURES_MHZ19_TEST_H_
#define ARDUINO_MHZ19_TEST_NATIVE_FIXTURES_MHZ19_TEST_H_

#include <array>

/* Google Test */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

/* Mocks */
#include <mocks/Arduino.h>
#include <mocks/Mhz19.h>

using MeasuringRangeSnapshot = std::array<uint16_t, 3>;
using SpanPointSnapshot = std::array<uint16_t, 3>;

class Mhz19Test : public ::testing::Test {
 protected:
  ::testing::StrictMock<StreamMock> serial;
  ::testing::StrictMock<Mhz19Mock> sensor;
};

class Mhz19TestWithSerial : public ::testing::Test {
 protected:
  void SetUp() override {
    arduino = createArduinoMock();
    sensor.serial_ = &serial;
  }

  void TearDown() override { destroyArduinoMock(); }

  ArduinoMock* arduino;

  ::testing::StrictMock<StreamMock> serial;
  Mhz19 sensor;
};

class Mhz19TestWithMeasuringRange
    : public Mhz19Test,
      public ::testing::WithParamInterface<MeasuringRangeSnapshot> {
 protected:
  void SetUp() override {
    auto measuringRangeSnapshot = GetParam();

    measuringRange =
        static_cast<Mhz19MeasuringRange>(measuringRangeSnapshot[0]);
    low = measuringRangeSnapshot[1];
    high = measuringRangeSnapshot[2];
  }

  Mhz19MeasuringRange measuringRange;
  uint16_t low;
  uint16_t high;
};

class Mhz19TestWithSpanPoint
    : public Mhz19Test,
      public ::testing::WithParamInterface<SpanPointSnapshot> {
 protected:
  void SetUp() override {
    auto spanPointSnapshot = GetParam();

    spanPoint = spanPointSnapshot[0];
    low = spanPointSnapshot[1];
    high = spanPointSnapshot[2];
  }

  uint16_t spanPoint;
  uint16_t low;
  uint16_t high;
};

class Mhz19TestWithWrongSpanPoint
    : public Mhz19Test,
      public ::testing::WithParamInterface<uint16_t> {
 protected:
  void SetUp() override { spanPoint = GetParam(); }

  uint16_t spanPoint;
};

#endif  // ARDUINO_MHZ19_TEST_NATIVE_FIXTURES_MHZ19_TEST_H_

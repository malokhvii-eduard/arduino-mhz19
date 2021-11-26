/* Google Test */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

/* Fixtures */
#include <fixtures/Mhz19Test.h>

using ::testing::_;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Range;
using ::testing::Return;
using ::testing::Values;

TEST_F(Mhz19Test, BeginsWithSerial) {
  sensor.begin(&serial);
  EXPECT_EQ(sensor.serial_, &serial);
}

TEST_F(Mhz19Test, GetsCarbonDioxide) {
  {
    InSequence s;
    EXPECT_CALL(sensor, writePacket(::internal::Mhz19CommandGetCarbonDioxide, 0,
                                    0, 0, 0, 0));
    EXPECT_CALL(sensor, readPacket(::internal::Mhz19CommandGetCarbonDioxide, _))
        .WillOnce(Invoke([&](const uint8_t command, uint8_t* packet) {
          packet[2] = 10;  // high
          packet[3] = 25;  // low
          return true;
        }));
  }

  EXPECT_EQ(sensor.getCarbonDioxide(), 2585);

  {
    InSequence s;
    EXPECT_CALL(sensor, writePacket(::internal::Mhz19CommandGetCarbonDioxide, 0,
                                    0, 0, 0, 0));
    EXPECT_CALL(sensor, readPacket(::internal::Mhz19CommandGetCarbonDioxide, _))
        .WillOnce(Invoke([&](const uint8_t command, uint8_t* packet) {
          packet[2] = 3;   // high
          packet[3] = 15;  // low
          return true;
        }));
  }

  EXPECT_EQ(sensor.getCarbonDioxide(), 783);
}

TEST_F(Mhz19Test, GetsCarbonDioxideWhenReadsPacketWithError) {
  {
    InSequence s;
    EXPECT_CALL(sensor, writePacket(::internal::Mhz19CommandGetCarbonDioxide, 0,
                                    0, 0, 0, 0));
    EXPECT_CALL(sensor, readPacket(::internal::Mhz19CommandGetCarbonDioxide, _))
        .WillOnce(Return(false));
  }

  EXPECT_EQ(sensor.getCarbonDioxide(), -1);
}

TEST_P(Mhz19TestWithMeasuringRange, SetsMeasuringRange) {
  EXPECT_CALL(sensor, writePacket(::internal::Mhz19CommandSetMeasuringRange, 0,
                                  0, 0, low, high));
  sensor.setMeasuringRange(measuringRange);
}

TEST_F(Mhz19Test, EnablesAutoCalibration) {
  EXPECT_CALL(sensor, writePacket(::internal::Mhz19CommandSetAutoCalibration,
                                  0xA0, 0, 0, 0, 0));
  sensor.enableAutoCalibration();
}

TEST_F(Mhz19Test, DisablesAutoCalibration) {
  EXPECT_CALL(sensor, writePacket(::internal::Mhz19CommandSetAutoCalibration, 0,
                                  0, 0, 0, 0));
  sensor.disableAutoCalibration();
}

TEST_F(Mhz19Test, CalibratesToZeroPoint) {
  EXPECT_CALL(sensor, writePacket(::internal::Mhz19CommandCalibrateToZeroPoint,
                                  0, 0, 0, 0, 0));
  sensor.calibrateToZeroPoint();
}

TEST_P(Mhz19TestWithSpanPoint, CalibratesToSpanPoint) {
  EXPECT_CALL(sensor, writePacket(::internal::Mhz19CommandCalibrateToSpanPoint,
                                  low, high, 0, 0, 0));
  EXPECT_TRUE(sensor.calibrateToSpanPoint(spanPoint));
}

TEST_P(Mhz19TestWithWrongSpanPoint, CalibratesToSpanPointWhenWrongSpanPoint) {
  EXPECT_CALL(sensor, writePacket(_, _, _, _, _, _)).Times(0);
  EXPECT_FALSE(sensor.calibrateToSpanPoint(spanPoint));
}

TEST_F(Mhz19Test, CalculatesPacketCheckSum) {
  uint8_t packet1[::internal::Mhz19PacketLength] = {0, 2, 3, 4, 5, 6, 7, 8, 0};
  uint8_t packet2[::internal::Mhz19PacketLength] = {0, 1,  3,  5, 7,
                                                    9, 11, 13, 0};

  EXPECT_EQ(Mhz19::calculatePacketCheckSum(packet1), 221);
  EXPECT_EQ(Mhz19::calculatePacketCheckSum(packet2), 207);
}

TEST_F(Mhz19TestWithSerial, ReadsPacket) {
  EXPECT_CALL(*arduino, millis())
      .WillOnce(Return(0))
      .WillOnce(Return(100))
      .WillOnce(Return(200));
  EXPECT_CALL(serial, available())
      .WillOnce(Return(0))
      .WillOnce(Return(0))
      .WillOnce(Return(::internal::Mhz19PacketLength));
  EXPECT_CALL(serial, readBytes(_, ::internal::Mhz19PacketLength))
      .WillOnce(Invoke([&](uint8_t* bytes, size_t length) {
        bytes[0] = 0xFF;
        bytes[1] = ::internal::Mhz19CommandGetCarbonDioxide;
        bytes[2] = 3;
        bytes[3] = 5;
        bytes[4] = 7;
        bytes[5] = 9;
        bytes[6] = 11;
        bytes[7] = 13;
        bytes[8] = 74;
        return ::internal::Mhz19PacketLength;
      }));

  uint8_t packet[::internal::Mhz19PacketLength];
  EXPECT_TRUE(
      sensor.readPacket(::internal::Mhz19CommandGetCarbonDioxide, packet));
  EXPECT_EQ(packet[0], 0xFF);
  EXPECT_EQ(packet[1], ::internal::Mhz19CommandGetCarbonDioxide);
  EXPECT_EQ(packet[2], 3);
  EXPECT_EQ(packet[3], 5);
  EXPECT_EQ(packet[4], 7);
  EXPECT_EQ(packet[5], 9);
  EXPECT_EQ(packet[6], 11);
  EXPECT_EQ(packet[7], 13);
  EXPECT_EQ(packet[8], 74);
}

TEST_F(Mhz19TestWithSerial, ReadsPacketWhenCorruptedStartingByte) {
  EXPECT_CALL(*arduino, millis()).WillOnce(Return(0));
  EXPECT_CALL(serial, available())
      .WillOnce(Return(::internal::Mhz19PacketLength));
  EXPECT_CALL(serial, readBytes(_, ::internal::Mhz19PacketLength))
      .WillOnce(Invoke([&](uint8_t* bytes, size_t length) {
        bytes[0] = 0;
        return ::internal::Mhz19PacketLength;
      }));

  uint8_t packet[::internal::Mhz19PacketLength];
  EXPECT_FALSE(
      sensor.readPacket(::internal::Mhz19CommandGetCarbonDioxide, packet));
}

TEST_F(Mhz19TestWithSerial, ReadsPacketWhenCorruptedCommandByte) {
  EXPECT_CALL(*arduino, millis()).WillOnce(Return(0));
  EXPECT_CALL(serial, available())
      .WillOnce(Return(::internal::Mhz19PacketLength));
  EXPECT_CALL(serial, readBytes(_, ::internal::Mhz19PacketLength))
      .WillOnce(Invoke([&](uint8_t* bytes, size_t length) {
        bytes[0] = 0xFF;
        bytes[1] = 0;
        return ::internal::Mhz19PacketLength;
      }));

  uint8_t packet[::internal::Mhz19PacketLength];
  EXPECT_FALSE(
      sensor.readPacket(::internal::Mhz19CommandSetAutoCalibration, packet));
}

TEST_F(Mhz19TestWithSerial, ReadsPacketWhenWrongChecksum) {
  EXPECT_CALL(*arduino, millis()).WillOnce(Return(0));
  EXPECT_CALL(serial, available())
      .WillOnce(Return(::internal::Mhz19PacketLength));
  EXPECT_CALL(serial, readBytes(_, ::internal::Mhz19PacketLength))
      .WillOnce(Invoke([&](uint8_t* bytes, size_t length) {
        bytes[0] = 0xFF;
        bytes[1] = ::internal::Mhz19CommandCalibrateToZeroPoint;
        bytes[8] = 0;
        return ::internal::Mhz19PacketLength;
      }));

  uint8_t packet[::internal::Mhz19PacketLength];
  EXPECT_FALSE(
      sensor.readPacket(::internal::Mhz19CommandCalibrateToZeroPoint, packet));
}

TEST_F(Mhz19TestWithSerial, ReadsPacketWhenReachesTimeout) {
  EXPECT_CALL(*arduino, millis()).WillOnce(Return(0)).WillOnce(Return(1000));
  EXPECT_CALL(serial, available()).WillOnce(Return(0));
  EXPECT_FALSE(sensor.readPacket(0, 0));
}

TEST_F(Mhz19TestWithSerial, WritesPacket) {
  EXPECT_CALL(serial, available())
      .WillOnce(Return(2))
      .WillOnce(Return(1))
      .WillOnce(Return(0));
  EXPECT_CALL(serial, read()).Times(2);
  EXPECT_CALL(serial, write(_, ::internal::Mhz19PacketLength));
  EXPECT_CALL(serial, flush());

  sensor.writePacket(::internal::Mhz19CommandGetCarbonDioxide, 2, 3, 4, 5, 6);
}

INSTANTIATE_TEST_SUITE_P(
    MeasuringRanges, Mhz19TestWithMeasuringRange,
    Values(
        MeasuringRangeSnapshot{
            static_cast<uint16_t>(Mhz19MeasuringRange::Ppm_1000), 3, 232},
        MeasuringRangeSnapshot{
            static_cast<uint16_t>(Mhz19MeasuringRange::Ppm_2000), 7, 208},
        MeasuringRangeSnapshot{
            static_cast<uint16_t>(Mhz19MeasuringRange::Ppm_3000), 11, 184},
        MeasuringRangeSnapshot{
            static_cast<uint16_t>(Mhz19MeasuringRange::Ppm_5000), 19, 136},
        MeasuringRangeSnapshot{
            static_cast<uint16_t>(Mhz19MeasuringRange::Ppm_10000), 39, 16}));

INSTANTIATE_TEST_SUITE_P(SpanPoints, Mhz19TestWithSpanPoint,
                         Values(SpanPointSnapshot{1500, 5, 220},
                                SpanPointSnapshot{2800, 10, 240},
                                SpanPointSnapshot{3700, 14, 116}));

INSTANTIATE_TEST_SUITE_P(WrongSpanPoints, Mhz19TestWithWrongSpanPoint,
                         Values(900, 11000, 500, 15000));

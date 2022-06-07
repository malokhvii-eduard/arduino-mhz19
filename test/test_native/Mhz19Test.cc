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
  sensor.serial_ = nullptr;
  EXPECT_FALSE(sensor.isPreheatingDone_);
  sensor.begin(&serial);
  EXPECT_EQ(sensor.serial_, &serial);
}

TEST_F(Mhz19Test, NotReadyWhenPreheatingInProgress) {
  EXPECT_CALL(*arduino, millis()).WillOnce(Return(100));
  EXPECT_FALSE(sensor.isReady());
}

TEST_F(Mhz19Test, ReadyWhenPreheatingDone) {
  EXPECT_CALL(*arduino, millis())
      .WillOnce(Return(100))
      .WillOnce(Return(MHZ19_PREHEATING_DURATION + 100));
  EXPECT_FALSE(sensor.isReady());
  EXPECT_TRUE(sensor.isReady());
  EXPECT_TRUE(sensor.isReady());
}

TEST_F(Mhz19Test, GetsCarbonDioxideWhenReady) {
  {
    InSequence s;
    EXPECT_CALL(*arduino, millis())
        .WillOnce(Return(MHZ19_PREHEATING_DURATION + 100));
    EXPECT_CALL(serial, write(Mhz19::CommandRead, Mhz19::PacketLength));
    EXPECT_CALL(serial, available()).WillOnce(Return(Mhz19::PacketLength));
    EXPECT_CALL(serial, readBytes(_, Mhz19::PacketLength))
        .WillOnce(Invoke([&](uint8_t* packet, size_t length) {
          packet[0] = 0xFF;
          packet[1] = Mhz19::CommandRead[2];
          packet[2] = 3;
          packet[3] = 5;
          packet[4] = 7;
          packet[5] = 9;
          packet[6] = 11;
          packet[7] = 13;
          packet[8] = 74;
          return Mhz19::PacketLength;
        }));
  }

  EXPECT_EQ(sensor.getCarbonDioxide(), 773);
}

TEST_F(Mhz19Test, GetsCarbonDioxideWhenNotReady) {
  EXPECT_CALL(*arduino, millis()).WillOnce(Return(100));
  EXPECT_EQ(sensor.getCarbonDioxide(), -1);
}

TEST_F(Mhz19Test, GetsCarbonDioxideWhenUnavailableBytes) {
  EXPECT_CALL(*arduino, millis())
      .WillOnce(Return(MHZ19_PREHEATING_DURATION + 100));
  EXPECT_CALL(serial, write(Mhz19::CommandRead, Mhz19::PacketLength));
  EXPECT_CALL(serial, available()).WillOnce(Return(0));

  EXPECT_EQ(sensor.getCarbonDioxide(), -1);
}

TEST_F(Mhz19Test, GetsCarbonDioxideWhenCorruptedStartingByte) {
  {
    InSequence s;
    EXPECT_CALL(*arduino, millis())
        .WillOnce(Return(MHZ19_PREHEATING_DURATION + 100));
    EXPECT_CALL(serial, write(Mhz19::CommandRead, Mhz19::PacketLength));
    EXPECT_CALL(serial, available()).WillOnce(Return(Mhz19::PacketLength));
    EXPECT_CALL(serial, readBytes(_, Mhz19::PacketLength))
        .WillOnce(Invoke([&](uint8_t* packet, size_t length) {
          packet[0] = 0;
          return Mhz19::PacketLength;
        }));
  }

  EXPECT_EQ(sensor.getCarbonDioxide(), -1);
}

TEST_F(Mhz19Test, GetsCarbonDioxideWhenCorruptedCommandByte) {
  {
    InSequence s;
    EXPECT_CALL(*arduino, millis())
        .WillOnce(Return(MHZ19_PREHEATING_DURATION + 100));
    EXPECT_CALL(serial, write(Mhz19::CommandRead, Mhz19::PacketLength));
    EXPECT_CALL(serial, available()).WillOnce(Return(Mhz19::PacketLength));
    EXPECT_CALL(serial, readBytes(_, Mhz19::PacketLength))
        .WillOnce(Invoke([&](uint8_t* packet, size_t length) {
          packet[0] = 0xFF;
          packet[1] = 0;
          return Mhz19::PacketLength;
        }));
  }

  EXPECT_EQ(sensor.getCarbonDioxide(), -1);
}

TEST_F(Mhz19Test, GetsCarbonDioxideWhenWrongChecksum) {
  {
    InSequence s;
    EXPECT_CALL(*arduino, millis())
        .WillOnce(Return(MHZ19_PREHEATING_DURATION + 100));
    EXPECT_CALL(serial, write(Mhz19::CommandRead, Mhz19::PacketLength));
    EXPECT_CALL(serial, available()).WillOnce(Return(Mhz19::PacketLength));
    EXPECT_CALL(serial, readBytes(_, Mhz19::PacketLength))
        .WillOnce(Invoke([&](uint8_t* packet, size_t length) {
          packet[0] = 0xFF;
          packet[1] = Mhz19::CommandCalibrateToZeroPoint[2];
          packet[8] = 0;
          return Mhz19::PacketLength;
        }));
  }

  EXPECT_EQ(sensor.getCarbonDioxide(), -1);
}

TEST_P(Mhz19TestWithMeasuringRange, SetsMeasuringRange) {
  EXPECT_CALL(fakeSensor, sendCommand(_))
      .WillOnce(Invoke([&](const uint8_t* command) {
        EXPECT_EQ(command[0], 0xFF);
        EXPECT_EQ(command[1], 0x01);
        EXPECT_EQ(command[2], 0x99);
        EXPECT_EQ(command[3], 0x00);
        EXPECT_EQ(command[4], 0x00);
        EXPECT_EQ(command[5], 0x00);
        EXPECT_EQ(command[6], high);
        EXPECT_EQ(command[7], low);
        EXPECT_EQ(command[8], Mhz19::calculatePacketCheckSum(command));
        return true;
      }));

  EXPECT_TRUE(fakeSensor.setMeasuringRange(measuringRange));
}

TEST_F(Mhz19Test, EnablesAutoBaseCalibration) {
  EXPECT_CALL(fakeSensor, sendCommand(Mhz19::CommandEnableAutoBaseCalibration))
      .WillOnce(Return(true));
  EXPECT_TRUE(fakeSensor.enableAutoBaseCalibration());
}

TEST_F(Mhz19Test, DisablesAutoBaseCalibration) {
  EXPECT_CALL(fakeSensor, sendCommand(Mhz19::CommandDisableAutoBaseCalibration))
      .WillOnce(Return(true));
  EXPECT_TRUE(fakeSensor.disableAutoBaseCalibration());
}

TEST_F(Mhz19Test, CalibratesToZeroPoint) {
  EXPECT_CALL(fakeSensor, sendCommand(Mhz19::CommandCalibrateToZeroPoint))
      .WillOnce(Return(true));
  EXPECT_TRUE(fakeSensor.calibrateToZeroPoint());
}

TEST_P(Mhz19TestWithSpanPoint, CalibratesToSpanPoint) {
  EXPECT_CALL(fakeSensor, sendCommand(_))
      .WillOnce(Invoke([&](const uint8_t* command) {
        EXPECT_EQ(command[0], 0xFF);
        EXPECT_EQ(command[1], 0x01);
        EXPECT_EQ(command[2], 0x88);
        EXPECT_EQ(command[3], high);
        EXPECT_EQ(command[4], low);
        EXPECT_EQ(command[5], 0x00);
        EXPECT_EQ(command[6], 0x00);
        EXPECT_EQ(command[7], 0x00);
        EXPECT_EQ(command[8], Mhz19::calculatePacketCheckSum(command));
        return true;
      }));

  EXPECT_TRUE(fakeSensor.calibrateToSpanPoint(spanPoint));
}

TEST_P(Mhz19TestWithWrongSpanPoint, CalibratesToSpanPointWhenWrongSpanPoint) {
  EXPECT_CALL(fakeSensor, sendCommand(_)).Times(0);
  EXPECT_FALSE(fakeSensor.calibrateToSpanPoint(spanPoint));
}

TEST_F(Mhz19Test, CalculatesPacketCheckSum) {
  uint8_t packet1[Mhz19::PacketLength] = {0, 2, 3, 4, 5, 6, 7, 8, 0};
  uint8_t packet2[Mhz19::PacketLength] = {0, 1, 3, 5, 7, 9, 11, 13, 0};

  EXPECT_EQ(Mhz19::calculatePacketCheckSum(packet1), 221);
  EXPECT_EQ(Mhz19::calculatePacketCheckSum(packet2), 207);
}

TEST_F(Mhz19Test, SendsCommand) {
  {
    InSequence s;
    EXPECT_CALL(serial, write(Mhz19::CommandRead, Mhz19::PacketLength));
    EXPECT_CALL(serial, readBytes(_, Mhz19::PacketLength))
        .WillOnce(Invoke([&](uint8_t* packet, size_t length) {
          packet[0] = 0xFF;
          packet[1] = Mhz19::CommandRead[2];
          packet[2] = 3;
          packet[3] = 5;
          packet[4] = 7;
          packet[5] = 9;
          packet[6] = 11;
          packet[7] = 13;
          packet[8] = 74;
          return Mhz19::PacketLength;
        }));
  }

  EXPECT_TRUE(sensor.sendCommand(Mhz19::CommandRead));
}

TEST_F(Mhz19Test, SendsCommandWhenCorruptedStartingByte) {
  {
    InSequence s;
    EXPECT_CALL(serial,
                write(Mhz19::CommandCalibrateToZeroPoint, Mhz19::PacketLength));
    EXPECT_CALL(serial, readBytes(_, Mhz19::PacketLength))
        .WillOnce(Invoke([&](uint8_t* packet, size_t length) {
          packet[0] = 0;
          return Mhz19::PacketLength;
        }));
  }

  EXPECT_FALSE(sensor.sendCommand(Mhz19::CommandCalibrateToZeroPoint));
}

TEST_F(Mhz19Test, SendsCommandWhenCorruptedCommandByte) {
  {
    InSequence s;
    EXPECT_CALL(serial, write(Mhz19::CommandEnableAutoBaseCalibration,
                              Mhz19::PacketLength));
    EXPECT_CALL(serial, readBytes(_, Mhz19::PacketLength))
        .WillOnce(Invoke([&](uint8_t* packet, size_t length) {
          packet[0] = 0xFF;
          packet[1] = 0;
          return Mhz19::PacketLength;
        }));
  }

  EXPECT_FALSE(sensor.sendCommand(Mhz19::CommandEnableAutoBaseCalibration));
}

TEST_F(Mhz19Test, ReadsPacketWhenWrongChecksum) {
  {
    InSequence s;
    EXPECT_CALL(serial, write(Mhz19::CommandDisableAutoBaseCalibration,
                              Mhz19::PacketLength));
    EXPECT_CALL(serial, readBytes(_, Mhz19::PacketLength))
        .WillOnce(Invoke([&](uint8_t* packet, size_t length) {
          packet[0] = 0xFF;
          packet[1] = Mhz19::CommandCalibrateToZeroPoint[2];
          packet[8] = 0;
          return Mhz19::PacketLength;
        }));
  }

  EXPECT_FALSE(sensor.sendCommand(Mhz19::CommandDisableAutoBaseCalibration));
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
            static_cast<uint16_t>(Mhz19MeasuringRange::Ppm_5000), 19, 136}));

INSTANTIATE_TEST_SUITE_P(SpanPoints, Mhz19TestWithSpanPoint,
                         Values(SpanPointSnapshot{1500, 5, 220},
                                SpanPointSnapshot{2800, 10, 240},
                                SpanPointSnapshot{3700, 14, 116}));

INSTANTIATE_TEST_SUITE_P(WrongSpanPoints, Mhz19TestWithWrongSpanPoint,
                         Values(900, 11000, 500, 15000));

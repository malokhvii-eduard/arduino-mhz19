#include <Mhz19.h>

#ifdef ARDUINO_MHZ19_UNIT_TEST

Mhz19::~Mhz19() {}

#endif

void Mhz19::begin(Stream *serial) { serial_ = serial; }

int16_t Mhz19::getCarbonDioxide() const {
  writePacket(::internal::Mhz19CommandGetCarbonDioxide, 0, 0, 0, 0, 0);

  uint8_t packet[::internal::Mhz19PacketLength];
  if (!readPacket(::internal::Mhz19CommandGetCarbonDioxide, packet)) {
    return -1;
  }

  return (256 * static_cast<int8_t>(packet[2])) +
         static_cast<int8_t>(packet[3]);
}

void Mhz19::setMeasuringRange(const Mhz19MeasuringRange measuringRange) {
  auto low = static_cast<uint8_t>(static_cast<uint16_t>(measuringRange) / 256);
  auto high = static_cast<uint8_t>(static_cast<uint16_t>(measuringRange) % 256);

  writePacket(::internal::Mhz19CommandSetMeasuringRange, 0, 0, 0, low, high);
}

void Mhz19::enableAutoCalibration() {
  writePacket(::internal::Mhz19CommandSetAutoCalibration, 0xA0, 0, 0, 0, 0);
}

void Mhz19::disableAutoCalibration() {
  writePacket(::internal::Mhz19CommandSetAutoCalibration, 0, 0, 0, 0, 0);
}

void Mhz19::calibrateToZeroPoint() {
  writePacket(::internal::Mhz19CommandCalibrateToZeroPoint, 0, 0, 0, 0, 0);
}

bool Mhz19::calibrateToSpanPoint(const uint16_t spanPoint) {
  if ((spanPoint < static_cast<uint16_t>(Mhz19MeasuringRange::Ppm_1000)) ||
      (spanPoint > static_cast<uint16_t>(Mhz19MeasuringRange::Ppm_10000))) {
    return false;
  }

  auto low = static_cast<uint8_t>(spanPoint / 256);
  auto high = static_cast<uint8_t>(spanPoint % 256);
  writePacket(::internal::Mhz19CommandCalibrateToSpanPoint, low, high, 0, 0, 0);

  return true;
}

uint8_t Mhz19::calculatePacketCheckSum(const uint8_t *packet) {
  uint8_t checkSum = 0;

  for (size_t i = 1; i < ::internal::Mhz19PacketLength - 1; i++) {
    checkSum += packet[i];
  }

  checkSum = 255 - checkSum;
  checkSum++;
  return checkSum;
}

bool Mhz19::readPacket(const uint8_t command, uint8_t *packet) const {
  auto duration = millis();
  while (serial_->available() <= 0) {
    if (millis() - duration >= ARDUINO_MHZ19_SERIAL_TIMEOUT) {
      return false;
    }
  }

  memset(packet, 0, ::internal::Mhz19PacketLength);
  serial_->readBytes(packet, ::internal::Mhz19PacketLength);

  auto checkSum = calculatePacketCheckSum(packet);
  if (packet[0] != 0xFF || packet[1] != command || packet[8] != checkSum) {
    return false;
  }
  return true;
}

void Mhz19::writePacket(const uint8_t command, const uint8_t byte3,
                        const uint8_t byte4, const uint8_t byte5,
                        const uint8_t byte6, const uint8_t byte7) const {
  uint8_t packet[::internal::Mhz19PacketLength] = {
      0xFF, 0x1, command, byte3, byte4, byte5, byte6, byte7, 0};
  packet[8] = calculatePacketCheckSum(packet);

  while (serial_->available() > 0) {
    serial_->read();
  }

  serial_->write(packet, ::internal::Mhz19PacketLength);
  serial_->flush();
}

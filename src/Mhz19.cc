#include <Mhz19.h>
#include <assert.h>

const uint8_t Mhz19::CommandRead[Mhz19::PacketLength] = {
    0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
const uint8_t Mhz19::CommandEnableAutoBaseCalibration[Mhz19::PacketLength] = {
    0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE6};
const uint8_t Mhz19::CommandDisableAutoBaseCalibration[Mhz19::PacketLength] = {
    0xFF, 0x01, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x86};
const uint8_t Mhz19::CommandCalibrateToZeroPoint[Mhz19::PacketLength] = {
    0xFF, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78};

Mhz19::Mhz19() : serial_(nullptr), isPreheatingDone_(false) {}

#ifdef ARDUINO_MHZ19_UNIT_TEST

Mhz19::~Mhz19() {}

#endif

void Mhz19::begin(Stream* serial) { serial_ = serial; }

bool Mhz19::isReady() const {
  if (isPreheatingDone_) {
    return true;
  }

  if (millis() > MHZ19_PREHEATING_DURATION) {
    isPreheatingDone_ = true;
    return true;
  }

  return false;
}

int Mhz19::getCarbonDioxide() const {
  assert(serial_ != nullptr);

  int carbonDioxide = -1;

  if (!isReady()) {
    return carbonDioxide;
  }

  uint8_t response[PacketLength];
  serial_->write(CommandRead, PacketLength);

  if (serial_->available()) {
    serial_->readBytes(response, PacketLength);

    auto checkSum = calculatePacketCheckSum(response);
    if (response[0] == 0xFF && response[1] == CommandRead[2] &&
        response[8] == checkSum) {
      auto high = static_cast<unsigned int>(response[2]);
      auto low = static_cast<unsigned int>(response[3]);
      carbonDioxide = static_cast<int>((256 * high) + low);
    }
  }

  return carbonDioxide;
}

bool Mhz19::setMeasuringRange(const Mhz19MeasuringRange measuringRange) {
  auto low = static_cast<uint8_t>(static_cast<uint16_t>(measuringRange) % 256);
  auto high = static_cast<uint8_t>(static_cast<uint16_t>(measuringRange) / 256);

  uint8_t command[PacketLength] = {0xFF, 0x01, 0x99, 0x00, 0x00,
                                   0x00, high, low,  0x00};
  command[8] = calculatePacketCheckSum(command);

  return sendCommand(command);
}

bool Mhz19::enableAutoBaseCalibration() {
  return sendCommand(CommandEnableAutoBaseCalibration);
}

bool Mhz19::disableAutoBaseCalibration() {
  return sendCommand(CommandDisableAutoBaseCalibration);
}

bool Mhz19::calibrateToZeroPoint() {
  return sendCommand(CommandCalibrateToZeroPoint);
}

bool Mhz19::calibrateToSpanPoint(const uint16_t spanPoint) {
  if ((spanPoint < static_cast<uint16_t>(Mhz19MeasuringRange::Ppm_1000)) ||
      (spanPoint > static_cast<uint16_t>(Mhz19MeasuringRange::Ppm_5000))) {
    return false;
  }

  auto low = static_cast<uint8_t>(spanPoint % 256);
  auto high = static_cast<uint8_t>(spanPoint / 256);

  uint8_t command[PacketLength] = {0xFF, 0x01, 0x88, high, low,
                                   0x00, 0x00, 0x00, 0x00};
  command[8] = calculatePacketCheckSum(command);

  return sendCommand(command);
}

uint8_t Mhz19::calculatePacketCheckSum(const uint8_t* packet) {
  uint8_t checkSum = 0;

  for (size_t i = 1; i < PacketLength - 1; i++) {
    checkSum += packet[i];
  }

  checkSum = 255 - checkSum;
  checkSum++;
  return checkSum;
}

bool Mhz19::sendCommand(const uint8_t* command) const {
  assert(serial_ != nullptr);

  uint8_t response[PacketLength];

  serial_->write(command, PacketLength);
  serial_->readBytes(response, PacketLength);

  auto checkSum = calculatePacketCheckSum(response);
  if (response[0] != 0xFF || response[1] != command[2] ||
      response[8] != checkSum) {
    return false;
  }

  return true;
}

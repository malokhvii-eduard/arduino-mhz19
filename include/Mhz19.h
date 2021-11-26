#ifndef ARDUINO_MHZ19_INCLUDE_MHZ19_H_
#define ARDUINO_MHZ19_INCLUDE_MHZ19_H_

#include <stdint.h>

#ifndef ARDUINO_MHZ19_SERIAL_TIMEOUT
#define ARDUINO_MHZ19_SERIAL_TIMEOUT 500
#endif

#ifndef ARDUINO_MHZ19_UNIT_TEST
/* Arduino */
#include <Arduino.h>

#define __NOT_VIRTUAL_METHOD
#else
/* Mocks */
#include <mocks/Arduino.h>

#define __NOT_VIRTUAL_METHOD virtual

#define protected public
#define private public
#define final
#endif

namespace internal {

enum Mhz19Command : uint8_t {
  Mhz19CommandGetCarbonDioxide = 0x86,
  Mhz19CommandSetMeasuringRange = 0x99,
  Mhz19CommandSetAutoCalibration = 0x79,
  Mhz19CommandCalibrateToZeroPoint = 0x87,
  Mhz19CommandCalibrateToSpanPoint = 0x88
};

enum : size_t { Mhz19PacketLength = 9 };

};  // namespace internal

enum class Mhz19MeasuringRange : uint16_t {
  Ppm_1000 = 1000,
  Ppm_2000 = 2000,
  Ppm_3000 = 3000,
  Ppm_5000 = 5000,
  Ppm_10000 = 10000
};

class Mhz19 {
 public:
#ifdef ARDUINO_MHZ19_UNIT_TEST
  virtual ~Mhz19();
#endif

  void begin(Stream *serial);

  int16_t getCarbonDioxide() const;

  void setMeasuringRange(const Mhz19MeasuringRange measuringRange);
  void enableAutoCalibration();
  void disableAutoCalibration();

  void calibrateToZeroPoint();
  bool calibrateToSpanPoint(const uint16_t spanPoint);

 private:
  static uint8_t calculatePacketCheckSum(const uint8_t *packet);

  __NOT_VIRTUAL_METHOD bool readPacket(const uint8_t command,
                                       uint8_t *packet) const;
  __NOT_VIRTUAL_METHOD void writePacket(
      const uint8_t command, const uint8_t byte3, const uint8_t byte4,
      const uint8_t byte5, const uint8_t byte6, const uint8_t byte7) const;

  Stream *serial_;
};

#undef __NOT_VIRTUAL_METHOD

#ifdef ARDUINO_MHZ19_UNIT_TEST
#undef protected
#undef private
#undef final
#endif

#endif  // ARDUINO_MHZ19_INCLUDE_MHZ19_H_

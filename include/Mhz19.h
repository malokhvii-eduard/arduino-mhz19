#ifndef ARDUINO_MHZ19_INCLUDE_MHZ19_H_
#define ARDUINO_MHZ19_INCLUDE_MHZ19_H_

#include <stdint.h>

#ifndef ARDUINO_MHZ19_UNIT_TEST
/* Arduino */
#include <Arduino.h>

#define __NOT_VIRTUAL
#else
/* Mocks */
#include <mocks/Arduino.h>

#define __NOT_VIRTUAL virtual

#define protected public
#define private public
#define final
#endif

#ifndef MHZ19_PREHEATING_DURATION
#define MHZ19_PREHEATING_DURATION 180000  // 3 minutes
#endif

enum class Mhz19MeasuringRange : uint16_t {
  Ppm_1000 = 1000,
  Ppm_2000 = 2000,
  Ppm_3000 = 3000,
  Ppm_5000 = 5000
};

class Mhz19 {
 public:
  Mhz19();

#ifdef ARDUINO_MHZ19_UNIT_TEST
  __NOT_VIRTUAL ~Mhz19();
#endif

  void begin(Stream* serial);

  bool isReady() const;
  int getCarbonDioxide() const;

  bool setMeasuringRange(const Mhz19MeasuringRange measuringRange);
  bool enableAutoBaseCalibration();
  bool disableAutoBaseCalibration();

  bool calibrateToZeroPoint();
  bool calibrateToSpanPoint(const uint16_t spanPoint);

 private:
  static const size_t PacketLength = 9;
  static const uint8_t CommandRead[];
  static const uint8_t CommandEnableAutoBaseCalibration[];
  static const uint8_t CommandDisableAutoBaseCalibration[];
  static const uint8_t CommandCalibrateToZeroPoint[];

  static uint8_t calculatePacketCheckSum(const uint8_t* packet);

  __NOT_VIRTUAL bool sendCommand(const uint8_t* command) const;

  Stream* serial_;
  mutable bool isPreheatingDone_;
};

#undef __NOT_VIRTUAL

#ifdef ARDUINO_MHZ19_UNIT_TEST
#undef protected
#undef private
#undef final
#endif

#endif  // ARDUINO_MHZ19_INCLUDE_MHZ19_H_

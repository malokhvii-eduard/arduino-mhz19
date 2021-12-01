# [2.0.0](https://github.com/malokhvii-eduard/arduino-mhz19/compare/v1.0.0...v2.0.0) (2021-12-01)


### Features

* **api:** fully re-write implementation, add preheating logic ([df12e9a](https://github.com/malokhvii-eduard/arduino-mhz19/commit/df12e9a9f267dab4b59b7c86ce86e06508477581))


### BREAKING CHANGES

* **api:** dropping `Mhz19MeasuringRange::Ppm_10000`.
`enableAutoCalibration()` renamed to `enableAutoBaseCalibration()`,
`disableAutoCalibration()` renamed to `disableAutoBaseCalibration()`

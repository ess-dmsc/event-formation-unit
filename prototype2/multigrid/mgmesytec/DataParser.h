/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Class to parse detector readout for multigrid via
/// sis3153 / Mesytec digitizer
///
//===----------------------------------------------------------------------===//

#pragma once
#include <common/FBSerializer.h>
#include <common/DataSave.h>
#include <common/ReadoutSerializer.h>
#include <logical_geometry/ESSGeometry.h>
#include <multigrid/mgmesytec/MG24Detector.h>
#include <multigrid/mgmesytec/MgSeqGeometry.h>

class MesytecData {
public:
  enum class error { OK = 0, ESIZE, EHEADER, EUNSUPP };

  /// @
  MesytecData(uint32_t module, std::string fileprefix = "") {
    MgMappings.select_module(module);
    dumptofile = !fileprefix.empty();
    if (dumptofile) {
      CsvFile = std::make_shared<DataSave>(fileprefix, 100000000);
      CsvFile->tofile("Trigger, HighTime, Time, Bus, Channel, ADC\n");
    }
  };

  ~MesytecData(){};

  uint32_t getPixel(); // @todo (too) simple implm. but agreed for now
  uint32_t getTime();  // @todo (too) simple implm. but agreed for now

  void setWireThreshold(int low, int high) {
     wireThresholdLo = low;
     wireThresholdHi = high;
   };

  void setGridThreshold(int low, int high) {
     gridThresholdLo = low;
     gridThresholdHi = high;
   };
  /** @brief parse a binary payload buffer, return number of data element
   * @todo Uses NMXHists  - refactor and move ?
   */
  error parse(const char *buffer, int size, NMXHists &hists, FBSerializer & fbserializer, ReadoutSerializer &serializer);

  /** @brief parse n 32 bit words from mesytec VMMR-8/16 card */
  void mesytec_parse_n_words(uint32_t *buffer, uint16_t nWords, NMXHists &hists, ReadoutSerializer &serializer);

  // Statistics updated by parse()
  struct {
    int readouts{0}; /**< number of channels read out */
    int discards{0}; /**< readouts discarded due to adc thresholds */
    int triggers{0}; /**< number of 0x58 blocks in packet */
    int events{0};   /**< number of events from this packets */
    int tx_bytes{0}; /**< number of bytes produced by librdkafka */
    int geometry_errors{0}; /**< number of invalid pixels from readout */
    int badtriggers{0}; /**< number of empty triggers or triggers without valid data */
  } stats;

  uint64_t FakePulseTime{0};

private:
  bool BusGood {false};
  bool WireGood {false};
  bool GridGood {false};
  bool TimeGood {false};
  uint8_t Bus;
  uint32_t Wire{0}; // initial alg.: wire with max adc
  uint32_t Grid{0}; // initial alg.: grid with max adc
  uint32_t Time;
  uint16_t HighTime;

  uint16_t wireThresholdLo{0};
  uint16_t wireThresholdHi{std::numeric_limits<uint16_t>::max()};
  uint16_t gridThresholdLo{0};
  uint16_t gridThresholdHi{std::numeric_limits<uint16_t>::max()};
  MgSeqGeometry MgMappings;
  ESSGeometry Geometry{36, 40, 20, 1};

  uint32_t PreviousTime{0};

  bool dumptofile{false};
  std::shared_ptr<DataSave> CsvFile;
};

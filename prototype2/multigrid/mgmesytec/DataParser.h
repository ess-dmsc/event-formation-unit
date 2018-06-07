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

class MesytecData {
public:
  enum class error { OK = 0, ESIZE, EHEADER, EUNSUPP };

  /// @
  MesytecData(uint32_t module, std::string fileprefix = "") {
    mgseq.select_module(module);
    dumptofile = !fileprefix.empty();
    if (dumptofile) {
      mgdata = std::make_shared<DataSave>(fileprefix, 100000000);
      mgdata->tofile("Trigger, HighTime, Time, Bus, Channel, ADC\n");
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

  // Statistics returned by parse()
  int readouts{0}; /**< number of channels read out */
  int discards{0};
  int triggers{0};
  int events{0};
  int tx_bytes{0};
  int geometry_errors{0};

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
  MG24Detector mgseq;
  ESSGeometry mg{4, 48, 20, 1};

  bool dumptofile{false};
  std::shared_ptr<DataSave>(mgdata);
};

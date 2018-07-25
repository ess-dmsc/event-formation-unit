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
#include <multigrid/mgmesytec/MgGeometry.h>

struct MgStats {
  size_t readouts{0}; /**< number of channels read out */
  size_t discards{0}; /**< readouts discarded due to adc thresholds */
  size_t triggers{0}; /**< number of 0x58 blocks in packet */
  size_t events{0};   /**< number of events from this packets */
  size_t tx_bytes{0}; /**< number of bytes produced by librdkafka */
  size_t geometry_errors{0}; /**< number of invalid pixels from readout */
  size_t badtriggers{0}; /**< number of empty triggers or triggers without valid data */
};

class VMMR16Parser {
public:
  /// \brief if it looks like a constructor...
  VMMR16Parser(std::shared_ptr<MgGeometry> mg_mappings);

  ~VMMR16Parser() = default;

  uint32_t getPixel(); // \todo (too) simple implm. but agreed for now
  uint32_t getTime();  // \todo (too) simple implm. but agreed for now

  void setSpoofHighTime(bool spoof);
  void setWireThreshold(uint16_t low, uint16_t high);
  void setGridThreshold(uint16_t low, uint16_t high);

  /** \brief parse n 32 bit words from mesytec VMMR-8/16 card */
  void mesytec_parse_n_words(uint32_t *buffer,
                             uint16_t nWords,
                             NMXHists &hists,
                             ReadoutSerializer &serializer,
                             MgStats& stats,
                             std::shared_ptr<DataSave> CsvFile);

  // Statistics updated by parse()
  uint8_t Bus {0};
  uint16_t Wire{0}; // initial alg.: wire with max adc
  uint16_t Grid{0}; // initial alg.: grid with max adc
  uint64_t TotalTime{0};
  bool ExternalTrigger{false};
  bool GoodEvent {false};

private:
  uint32_t LowTime{0};
  uint32_t PreviousLowTime{0};
  uint32_t HighTime{0};

  bool BusGood{false};
  bool WireGood{false};
  bool GridGood{false};
  bool TimeGood{false};

  uint16_t wireThresholdLo{0};
  uint16_t wireThresholdHi{std::numeric_limits<uint16_t>::max()};
  uint16_t gridThresholdLo{0};
  uint16_t gridThresholdHi{std::numeric_limits<uint16_t>::max()};

  std::shared_ptr<MgGeometry> MgMappings;

  bool spoof_high_time{false};
};


class MesytecData {
public:
  enum class error { OK = 0, ESIZE, EHEADER, EUNSUPP };

  /// \brief if it looks like a constructor...
  MesytecData(std::shared_ptr<MgGeometry> mg_mappings, std::string fileprefix = "");

  ~MesytecData() = default;

  uint32_t getPixel(); // \todo (too) simple implm. but agreed for now
  uint32_t getTime();  // \todo (too) simple implm. but agreed for now

  void setSpoofHighTime(bool spoof);
  void setWireThreshold(uint16_t low, uint16_t high);
  void setGridThreshold(uint16_t low, uint16_t high);

  /** \brief parse a binary payload buffer, return number of data element
   * \todo Uses NMXHists  - refactor and move ?
   */
  error parse(const char *buffer, int size, NMXHists &hists, FBSerializer &fbserializer, ReadoutSerializer &serializer);

  /** \brief parse n 32 bit words from mesytec VMMR-8/16 card */
  void mesytec_parse_n_words(uint32_t *buffer, uint16_t nWords, NMXHists &hists, ReadoutSerializer &serializer);

  // Statistics updated by parse()
  MgStats stats;

  uint64_t RecentPulseTime{0};

private:
  VMMR16Parser vmmr16Parser;

  std::shared_ptr<MgGeometry> MgMappings;
  // \todo deduce this from mappings
  ESSGeometry Geometry{36, 40, 20, 1};

  std::shared_ptr<DataSave> CsvFile {nullptr};
};

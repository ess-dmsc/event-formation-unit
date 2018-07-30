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
#include <common/ReadoutSerializer.h>
#include <multigrid/mgmesytec/MgEFU.h>
#include <multigrid/mgmesytec/HitFile.h>

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
  VMMR16Parser(MgEFU mg_efu, std::shared_ptr<ReadoutSerializer> s);

  ~VMMR16Parser() = default;

  void setSpoofHighTime(bool spoof);

  /** \brief parse n 32 bit words from mesytec VMMR-8/16 card */
  void parse(uint32_t *buffer, uint16_t nWords,
             MgStats& stats, bool dump_data);


  uint64_t time() const;
  bool externalTrigger() const;
  bool goodEvent() const;

  MgEFU mgEfu;
  std::vector<MGHit> converted_data;

private:

  std::shared_ptr<ReadoutSerializer> hit_serializer;

  MGHit hit;

  bool GoodEvent {false};

  bool spoof_high_time{false};
  uint32_t PreviousLowTime{0};
};

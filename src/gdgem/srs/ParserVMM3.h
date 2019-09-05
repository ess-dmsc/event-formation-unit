/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class to receive and generate Gd-GEM detector readout
/// from VMM3 ASICS via the SRS readout system
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <string.h>
#include <common/BitMath.h>

static const int maximumNumberVMM{32};
static const int maximumNumberFECs{16};

namespace Gem {

class VMM3SRSData {
public:
  // bytes
  static const int SRSHeaderSize{16};
  static const int HitAndMarkerSize{6};
  static const int Data1Size{4};

  ///< Do NOT rearrange fields, used for casting to data pointer
  struct SRSHdr {
    uint32_t frameCounter;   /// frame counter packet field
    uint32_t dataId;         /// data type identifier packet field + ID of the FEC card (0-255)
    uint32_t udpTimeStamp;   /// Transmission time for UDP packet
    uint32_t offsetOverflow; /// offset overflow in last frame (1 bit per VMM)
  };

  // \todo no need for this struct
  struct VMM3Marker {
    uint64_t fecTimeStamp;   /// 42 bit
  };

  /// Data common to all hits and markers, or other parser related data
  struct ParserData {
    uint8_t fecId{1};
    uint32_t nextFrameCounter{0};
  };

  /// Data related to a single Hit
  struct VMM3Data {
    uint64_t fecTimeStamp; /// 42 bits can change within a packet so must be here
    uint16_t bcid;         /// 12 bit - bcid after graydecode
    uint16_t adc;          /// 10 bit - adc value from vmm readout
    uint8_t tdc;           ///  8 bit - tdc value from vmm readout
    uint8_t chno;          ///  6 bit - channel number from readout
    uint8_t overThreshold; ///  1 bit - over threshold flag for channel from readout
    uint8_t vmmid;         ///  5 bit - asic identifier - unique id per fec 0 - 15
    uint8_t triggerOffset; ///  5 bit
    bool hasDataMarker;    ///
  };

  /// \brief create a data handler for VMM3 SRS data of fixed size Capacity
  /// \param maxelements The maximum number of readout elements
  VMM3SRSData(int maxelements) : maxHits(maxelements) {
    markers = new struct VMM3Marker[maximumNumberFECs * maximumNumberVMM];
    data = new struct VMM3Data[maxHits];
    memset(markers, 0, sizeof(struct VMM3Marker) * maximumNumberFECs * maximumNumberVMM);
  }

  /// Delete allocated data, set pointers to nullptr
  ~VMM3SRSData() {
    delete[] data;
    data = nullptr;
    delete[] markers;
    markers = nullptr;
  }

  // \todo use Buffer<char>
  /// \brief reveive readouts from a binary payload buffer, return number of
  /// data elements
  int receive(const char *buffer, int size);

  /// \brief parse the readouts into a data array
  /// \param data1 the raw (unbitreversed) data1 field of a SRS packet
  /// \param data2 the raw (unbitreversed) data2 field of a SRS packet
  /// \param vmd VMM2Data structure holding the parsed data (tdc, bcid, adc, ...)
  int parse(uint32_t data1, uint16_t data2, struct VMM3Data *vmd);

  /// Holds data common to all readouts in a packet
  struct SRSHdr srsHeader;

  /// holds all readout data in a packet (up to max_elems)
  struct VMM3Data *data{nullptr};

  /// See description above
  struct ParserData parserData;

  /// holds time bases for all vmms in a readout
  struct VMM3Marker *markers{nullptr};

  // Stat counters: Results of the data parsing
  struct {
    uint32_t readouts{0};        /// number of hits
    uint32_t markers{0};    ///  number of markers
    uint32_t errors{0};      /// bytes of invalid data
    uint32_t rxSeqErrors{0};  /// gaps in frame counter values
    uint32_t badFrames{0};   /// frames failing parsing
    uint32_t goodFrames{0};  /// frames passing parsing
  } stats;

  uint32_t maxHits{0};       /// Maximum capacity of data array
};

}

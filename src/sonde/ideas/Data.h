/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class to receive SoNDe data from IDEAS readout
///
/// Data formats based on IDEAS Readout and Control Packet Protocol Reference
/// DRAFT Reference : IDE-REP-Ref-V1.7 Date : 2017-05-23
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <common/DataSave.h>
#include <sonde/Geometry.h>
#include <memory>
#include <cassert>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Sonde {

const int MaxNumberOfEvents = 500;
const uint16_t NoAdcProvided = 0x1;

class IDEASData {
public:
  // Error codes, returned as negative numbers
  enum error { OK = 0, EBUFFER, EBADSIZE, EHEADER, EUNSUPP };

  struct SoNDeData {
    uint32_t Time;
    uint32_t PixelId;
    uint32_t Adc;
  };

  /// \note from IDEAS Readout and Control Packet Protocol Reference
  /// Ref: IDE-REP-Ref-V1.7 Date: 2017-05-23
  /// from: Table 18: Multi-Event Pulse-Height Data Packet fields.
  struct MEPHData {
    uint8_t TriggerType;
    uint8_t ASIC;
    uint8_t Channel;
    uint16_t ADCValue;
  } __attribute__((packed));
  static_assert(sizeof(MEPHData) == 5, "MEPHData struct is not packed");

  /// \note from IDEAS Readout and Control Packet Protocol Reference
  /// Ref: IDE-REP-Ref-V1.7 Date: 2017-05-23
  /// from: Table 17: Single-Event Pulse-Height Data Packet fields.
  struct SEPHHeader {
    uint8_t SourceId;
    uint8_t TriggerType;
    uint8_t ChannelId;
    uint16_t HoldDelay;
    uint16_t NumberOfSamples;
  } __attribute__((packed));
  static_assert(sizeof(SEPHHeader) == 7, "SEPHHeader struct is not packed");

  /// \note from IDEAS Readout and Control Packet Protocol Reference
  /// Ref: IDE-REP-Ref-V1.7 Date: 2017-05-23
  /// from: Table 3: Packet Header – System to PC packets.
  struct Header {
    uint16_t id;
    uint16_t pktseq;
    uint32_t timestamp;
    uint16_t length;
  } __attribute__((packed));
  static_assert(sizeof(Header) == 10, "Header struct is not packed");

  /// \todo document
  IDEASData(Geometry *geom, std::string fileprefix = "")
      : sondegeometry(geom) {
    dumptofile = !fileprefix.empty();
    if (dumptofile) {
      datafile = std::make_shared<DataSave>(fileprefix, 100000000);

      datafile->tofile(
          "#mpeh: hdr_count, evtime, trigger_type, asic, channel, sample\n");
      datafile->tofile("#seph: hdr_hdrtime, trigger_type, hold_delay, triggering_asic, "
                       "triggering_channel, sample\n");
      datafile->tofile("#tt: hdr_count, hdr_hdrtime, hdr_sysno, asic, channel\n");
    }
  }

  ~IDEASData() {}

  /// \brief parse a binary payload buffer, return number of data elements
  int parse_buffer(const char *buffer, int size);

  /// \brief Section 2.4.5 page 15
  int parse_trigger_time_data_packet(const char *buffer);

  /// \brief Section 2.4.3 page 12
  int parse_single_event_pulse_height_data_packet(const char *buffer);

  /// \brief Section 2.4.3 page 12
  int parse_multi_event_pulse_height_data_packet(const char *buffer);

  void addEvent(int time, int pixelid, int adc) {
    assert(events < MaxNumberOfEvents);
    XTRACE(PROCESS, INF, "event: %d, time: 0x%08x, pixel: %d, adc: %d", events, time, pixelid, adc);
    data[events].Time = time;
    data[events].PixelId = static_cast<uint32_t>(pixelid);
    data[events].Adc = adc;
    events++;
  };

  struct SoNDeData data[MaxNumberOfEvents];
  unsigned int events{0};  ///< number of valid events
  unsigned int errors{0};  ///< number of geometry errors in readout
  unsigned int samples{0}; ///< number of samples in readout

  uint64_t ctr_outof_sequence{0};
  uint64_t counterPacketTriggerTime{0};
  uint64_t counterPacketSingleEventPulseHeight{0};
  uint64_t counterPacketMultiEventPulseHeight{0};
  uint64_t counterPacketUnsupported{0};

private:
  Geometry *sondegeometry{nullptr};
  int next_seq_no{0}; /// Used to count lost packets, assumes we start at 0

  /// Protocol header fields
  int hdr_sysno{0};
  int hdr_type{0};
  int hdr_seqflag{0};
  int hdr_count{0};
  int hdr_hdrtime{0};
  int hdr_length{0};

  bool dumptofile{false};
  std::shared_ptr<DataSave>(datafile);
};

}

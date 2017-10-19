/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to receive SoNDe data from IDEAS readout
 *
 *  Data formats based on IDEAS Readout and Control Packet Protocol Reference DRAFT
 *  Reference : IDE-REP-Ref-V1.7 Date : 2017-05-23
 */

#pragma once

#include <cinttypes>
#include <dataformats/multigrid/inc/DataSave.h>
#include <sonde/Geometry.h>

class IDEASData {
public:
  // Error codes, returned as negative numbers
  enum error { OK = 0, EBUFFER, EBADSIZE, EHEADER, EUNSUPP};

  struct SoNDeData {
    uint32_t time;
    uint32_t pixel_id;
  };

  /** from IDEAS Readout and Control Packet Protocol Reference
   *  Ref: IDE-REP-Ref-V1.7 Date: 2017-05-23
   *  Direction: System -> PC
   */
  struct Header {
    uint16_t id;
    uint16_t pktseq;
    uint32_t timestamp;
    uint16_t length;
  } __attribute__((packed));

  /** Empty constructor */
  IDEASData(SoNDeGeometry * geom) : sondegeometry(geom) {
    #ifdef DUMPTOFILE
      mephdata.tofile("# hdr_count, evtime, trigger_type, asic, channel, sample\n");
      sephdata.tofile("# hdr_hdrtime, trigger_type, hold_delay, triggering_asic, triggering_channel, sample\n");
      eventdata.tofile("# hdr_count, hdr_hdrtime, hdr_sysno, asic, channel\n");
    #endif
  }

  ~IDEASData() { }

  /** @brief parse a binary payload buffer, return number of data elements
   */
  int parse_buffer(const char *buffer, int size);

 /** @brief Section 2.4.5 page 15 */
  int parse_trigger_time_data_packet(const char *buffer);

 /** @brief Section 2.4.3 page 12 */
  int parse_single_event_pulse_height_data_packet(const char *buffer);

 /** @brief Section 2.4.3 page 12 */
  int parse_multi_event_pulse_height_data_packet(const char *buffer);

  struct SoNDeData data[500];
  unsigned int events{0};  /**< number of valid events */
  unsigned int errors{0};  /**< number of geometry errors in readout */
  unsigned int samples{0}; /**< number of samples in readout */
private:
  SoNDeGeometry * sondegeometry{nullptr};

  // Protocol header fields
  int hdr_sysno{0};
  int hdr_type{0};
  int hdr_seqflag{0};
  int hdr_count{0};
  int hdr_hdrtime{0};
  int hdr_length{0};

#ifdef DUMPTOFILE
  DataSave mephdata{"single_ch_spec_", 1};
  DataSave sephdata{"all_ch_spec_", 1};
  DataSave eventdata{"TOF_", 1};
#endif
};

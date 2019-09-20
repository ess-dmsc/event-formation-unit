/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <arpa/inet.h>
#include <cassert>
#include <common/Trace.h>
#include <cstring>
#include <sonde/ideas/Data.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Sonde {

int IDEASData::parse_buffer(const char *buffer, int size) {
  samples = 0;
  events = 0;
  errors = 0;

  if (buffer == nullptr) {
    XTRACE(PROCESS, WAR, "Invalid buffer");
    return -IDEASData::EBUFFER;
  }

  if (size < 11) {
    XTRACE(PROCESS, WAR, "IDEAS readout header too short (%d bytes)", size);
    return -IDEASData::EBADSIZE;
  }

  struct Header *hdr = (struct Header *) buffer;

  int version = (ntohs(hdr->id) & 0xe000) >> 13;
  if (version != 0) {
    XTRACE(PROCESS, WAR, "Illegal version number (%d)", version);
    return -IDEASData::EHEADER;
  }
  hdr_sysno = (ntohs(hdr->id) & 0x1fff) >> 8;
  hdr_type = (ntohs(hdr->id) & 0x00ff);
  hdr_seqflag = (ntohs(hdr->pktseq) & 0xc000) >> 14;
  hdr_count = (ntohs(hdr->pktseq) & 0x3fff);

  hdr_hdrtime = ntohl(hdr->timestamp);
  hdr_length = ntohs(hdr->length);

  if (next_seq_no != hdr_count) {
    ctr_outof_sequence++;
  }

  next_seq_no = (hdr_count + 1) & 0x3fff;

  XTRACE(PROCESS, DEB, "version: %d, sysno: %d, type: 0x%02x", version,
         hdr_sysno, hdr_type);
  XTRACE(PROCESS, DEB, "sequence flag: %d, packet count: %d", hdr_seqflag,
         hdr_count);
  XTRACE(PROCESS, DEB, "time: 0x%08x, size: 0x%04x", hdr_hdrtime, hdr_length);

  if (hdr_length + (int) sizeof(struct Header) != size) {
    XTRACE(PROCESS, WAR, "Packet length mismatch: udp: %d, parsed: %d", size,
           hdr_length + (int) sizeof(struct Header));
    return -IDEASData::EHEADER;
  }

  auto pktdata = buffer + sizeof(struct Header);
  // auto pktdatasize = size - sizeof(struct Header);
  if (hdr_type == 0xD6) {
    counterPacketTriggerTime++;
    XTRACE(PROCESS, DEB, "Trigger Time Data Packet");
    return parse_trigger_time_data_packet(pktdata);
  } else if (hdr_type == 0xD5) {
    counterPacketSingleEventPulseHeight++;
    XTRACE(PROCESS, DEB, "Single Event Pulse Height Data Packet");
    return parse_single_event_pulse_height_data_packet(pktdata);
  } else if (hdr_type == 0xD4) {
    counterPacketMultiEventPulseHeight++;
    XTRACE(PROCESS, DEB, "Multi Event Pulse Height Data Packet");
    return parse_multi_event_pulse_height_data_packet(pktdata);
  } else {
    counterPacketUnsupported++;
    XTRACE(PROCESS, WAR, "Unsupported readout format: 0x%02x", hdr_type);
    return -IDEASData::EUNSUPP;
  }
}

/** Parse data according to IDEAS documentation */
int IDEASData::parse_trigger_time_data_packet(const char *buffer) {
  static const int BYTES_PER_ENTRY = 5;
  /**< \todo add check for minimum size */
  uint8_t *datap = (uint8_t *) (buffer);
  int nentries = *datap;
  XTRACE(PROCESS, DEB, "Number of readout events in packet: %d", nentries);

  if (nentries * BYTES_PER_ENTRY + 1 !=
      hdr_length) { /** magic packet numbers, check documentation */
    XTRACE(PROCESS, WAR, "Data length error: events %d (len %d), got: %d",
           nentries, nentries * BYTES_PER_ENTRY + 1, hdr_length);
    return -IDEASData::EHEADER;
  }

  for (int i = 0; i < nentries; i++) {
    auto timep = (uint32_t *) (datap + i * BYTES_PER_ENTRY + 1);
    auto aschp = (uint8_t *) (datap + i * BYTES_PER_ENTRY + 5);
    uint32_t time = ntohl(*timep);
    uint8_t asch = *aschp; // ASIC (2b) and CHANNEL (6b)

    int pixelid = sondegeometry->getdetectorpixelid(hdr_sysno, asch);
    if (pixelid >= 1) {
      addEvent(time, pixelid, NoAdcProvided);
      if (dumptofile) {
        datafile->tofile("%d, %u, %d, %d, %d\n", hdr_count, hdr_hdrtime,
                          hdr_sysno, asch >> 6, asch & 0x3f);
      }
    } else {
      XTRACE(PROCESS, WAR, "Geometry error in entry %d (asch %d)", i, asch);
      errors++;
    }
  }
  XTRACE(PROCESS, DEB, "Number of events in buffer: %u", events);
  return events;
}

/** Parse data according to IDEAS documentation
 */
int IDEASData::parse_single_event_pulse_height_data_packet(const char *buffer) {
  static const int BYTES_PER_ENTRY = 2;
  SEPHHeader * dp = (SEPHHeader*)(buffer);
  /// \todo check minimum header length

  /// use ntohs() ntohl() on multi-byte values
  int NumberOfSamples = ntohs(dp->NumberOfSamples);
  int HoldDelay = ntohs(dp->HoldDelay);
  XTRACE(PROCESS, DEB, "Number of readout events in packet: %d", NumberOfSamples);

  if (NumberOfSamples * BYTES_PER_ENTRY + sizeof(SEPHHeader) != hdr_length) {
    XTRACE(PROCESS, WAR, "Data length error: events %d (len %d), got: %d",
           NumberOfSamples, NumberOfSamples * BYTES_PER_ENTRY + sizeof(SEPHHeader), hdr_length);
    return -IDEASData::EHEADER;
  }

  XTRACE(PROCESS, DEB,
         "asic: %d, channel: %d, trigger type: %d, hold delay: %d", dp->SourceId,
         dp->ChannelId, dp->TriggerType, HoldDelay);

  int pixelid = sondegeometry->getdetectorpixelid(hdr_sysno, dp->SourceId, dp->ChannelId);
  if (pixelid >= 1) {
    addEvent(hdr_hdrtime, pixelid, Sonde::NoAdcProvided);
  }

  for (int i = 0; i < NumberOfSamples; i++) {
    samples++;
    uint16_t sample = ntohs(*(uint16_t *) (buffer + i * BYTES_PER_ENTRY + sizeof(SEPHHeader)));
    XTRACE(PROCESS, INF, "sample %3d: 0x%x (%d)", i, sample, sample);

    if (dumptofile) {
      datafile->tofile("%u, %d, %d, %d, %d, %d\n", hdr_hdrtime, dp->TriggerType,
                       dp->HoldDelay, dp->SourceId, dp->ChannelId, sample);
    }
  }

  return events;
}

/** Parse data according to IDEAS documentation */
int IDEASData::parse_multi_event_pulse_height_data_packet(const char *buffer) {
  static const int BYTES_PER_SAMPLE = sizeof(MEPHData);
  if (hdr_length < 12) {
    XTRACE(PROCESS, WAR, "data packet too short for mpeh data");
    return -IDEASData::EBADSIZE;
  }

  int N = *(uint8_t *) buffer;               /**< number of events            */
  int M = ntohs(*(uint16_t *) (buffer + 1)); /**< number of samples per event */

  XTRACE(PROCESS, DEB, "Readout events: %d, samples per event %d", N, M);

  int ExpectedLength = (4 + BYTES_PER_SAMPLE * M) * N + 3;
  if (ExpectedLength > hdr_length) {
    XTRACE(PROCESS, WAR, "Data length error: expected len %d, got: %d",
           ExpectedLength, hdr_length);
    return -IDEASData::EHEADER;
  }

  for (int n = 0; n < N; n++) {
    auto evoff = buffer + 3 + (4 + BYTES_PER_SAMPLE * M) * n; // Event offset
    uint32_t evtime = ntohl(*(uint32_t *) (evoff));
    for (int m = 0; m < M; m++) {
      samples++;
      int sampleoffset = BYTES_PER_SAMPLE * m;
      MEPHData * dp = (MEPHData*)(evoff + sampleoffset + 4);
      uint16_t ADCValue = ntohs(dp->ADCValue);
      int pixelid = sondegeometry->getdetectorpixelid(hdr_sysno, dp->ASIC, dp->Channel);
      if (pixelid >= 1) {
        addEvent(evtime, pixelid, ADCValue);
      }
      if (dumptofile) {
        datafile->tofile("%d, %u, %d, %d, %d, %d\n", hdr_count, evtime,
                         dp->TriggerType, dp->ASIC, dp->Channel, ADCValue);
      }
    }
  }

  return events;
}

}

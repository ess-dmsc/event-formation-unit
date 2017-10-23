/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <arpa/inet.h>
#include <cassert>
#include <common/Trace.h>
#include <cstring>
#include <sonde/ideas/Data.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

int IDEASData::parse_buffer(const char *buffer, int size) {
  samples = 0;
  events = 0;
  errors = 0;

  if (buffer == nullptr) {
    XTRACE(PROCESS, WAR, "Invalid buffer\n");
    return -IDEASData::EBUFFER;
  }

  if (size < 11) {
    XTRACE(PROCESS, WAR, "IDEAS readout header too short (%d bytes)\n", size);
    return -IDEASData::EBADSIZE;
  }

  struct Header * hdr = (struct Header *)buffer;

  int version = (ntohs(hdr->id) & 0xe000) >> 13;
  if (version != 0) {
    XTRACE(PROCESS, WAR, "Illegal version number (%d)\n", version);
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

  next_seq_no= (hdr_count + 1) & 0x3fff;

  XTRACE(PROCESS, DEB, "version: %d, sysno: %d, type: 0x%02x\n", version, hdr_sysno, hdr_type);
  XTRACE(PROCESS, DEB, "sequence flag: %d, packet count: %d\n", hdr_seqflag, hdr_count);
  XTRACE(PROCESS, DEB, "time: 0x%08x, size: 0x%04x\n", hdr_hdrtime, hdr_length);

  if (hdr_length + (int)sizeof(struct Header) != size) {
    XTRACE(PROCESS, WAR, "Packet length mismatch: udp: %d, parsed: %d\n",
         size, hdr_length + (int)sizeof(struct Header));
    return -IDEASData::EHEADER;
  }

  auto pktdata = buffer + sizeof(struct Header);
  //auto pktdatasize = size - sizeof(struct Header);
  if (hdr_type == 0xD6) {
    XTRACE(PROCESS, DEB, "Trigger Time Data Packet\n");
    return parse_trigger_time_data_packet(pktdata);
  } else if (hdr_type == 0xD5) {
    XTRACE(PROCESS, DEB, "Single Event Pulse Height Data Packet\n");
    return parse_single_event_pulse_height_data_packet(pktdata);
  } else if (hdr_type == 0xD4) {
    XTRACE(PROCESS, DEB, "Multi Event Pulse Height Data Packet\n");
    return parse_multi_event_pulse_height_data_packet(pktdata);
  } else {
    XTRACE(PROCESS, WAR, "Unsupported readout format: 0x%02x\n", hdr_type);
    return -IDEASData::EUNSUPP;
  }
}


/** Parse data according to IDEAS documentation */
int IDEASData::parse_trigger_time_data_packet(const char *buffer) {
  static const int BYTES_PER_ENTRY=5;
  /**< @todo add check for minimum size */
  uint8_t * datap = (uint8_t *)(buffer);
  int nentries = *datap;
  XTRACE(PROCESS, DEB, "Number of readout events in packet: %d\n", nentries);

  if (nentries * BYTES_PER_ENTRY + 1 != hdr_length) { /** magic packet numbers, check documentation */
    XTRACE(PROCESS, WAR, "Data length error: events %d (len %d), got: %d\n",
         nentries, nentries * BYTES_PER_ENTRY + 1, hdr_length);
    return -IDEASData::EHEADER;
  }

  for (int i = 0; i < nentries; i++) {
    auto timep = (uint32_t *)(datap + i * BYTES_PER_ENTRY + 1);
    auto aschp = (uint8_t  *)(datap + i * BYTES_PER_ENTRY + 5);
    uint32_t time = ntohl(*timep);
    uint8_t asch = *aschp; // ASIC (2b) and CHANNEL (6b)

    int pixelid = sondegeometry->getdetectorpixelid(hdr_sysno, asch);
    if (pixelid >= 1) {
      data[events].time = time;
      data[events].pixel_id = static_cast<uint32_t>(pixelid);
      #ifdef DUMPTOFILE
          eventdata.tofile("%d, %u, %d, %d, %d\n", hdr_count, hdr_hdrtime, hdr_sysno, asch>>6, asch & 0x3f);
      #endif
      XTRACE(PROCESS, INF, "event: %d, time: 0x%08x, pixel: %d\n", i, time, data[events].pixel_id);
      events++;
    } else {
      XTRACE(PROCESS, WAR, "Geometry error in entry %d (asch %d)\n", i, asch);
      errors++;
    }

  }
  XTRACE(PROCESS, DEB, "Number of events in buffer: %d\n", events);
  return events;
}

/** Parse data according to IDEAS documentation
 * does not generate events
 */
int IDEASData::parse_single_event_pulse_height_data_packet(const char *buffer){
    static const int BYTES_PER_ENTRY=2;
    /** @todo check minimum header length */
    int nentries = ntohs(*(uint16_t*)(buffer + 5));
    XTRACE(PROCESS, DEB, "Number of readout events in packet: %d\n", nentries);

    if (nentries * BYTES_PER_ENTRY + 7 != hdr_length) {
      XTRACE(PROCESS, WAR, "Data length error: events %d (len %d), got: %d\n",
           nentries, nentries * BYTES_PER_ENTRY + 5, hdr_length);
      return -IDEASData::EHEADER;
    }
    int asic = *(uint8_t*)(buffer);
    int trigger_type = *(uint8_t*)(buffer + 1);
    int channel = *(uint8_t*)(buffer + 2);
    int hold_delay = ntohs(*(uint16_t*)(buffer + 3));
    XTRACE(PROCESS, DEB, "asic: %d, channel: %d, trigger type: %d, hold delay: %d\n",
            asic, channel, trigger_type, hold_delay);

    int pixelid = sondegeometry->getdetectorpixelid(hdr_sysno, asic, channel);
    if (pixelid >= 1) {
      data[events].time = hdr_hdrtime;
      data[events].pixel_id = static_cast<uint32_t>(pixelid);
      events++;
    }

    for (int i = 0; i < nentries; i++) {
      samples++;
      uint16_t sample = ntohs(*(uint16_t *)(buffer + i*2 + 7));

      #ifdef DUMPTOFILE
      sephdata.tofile("%u, %d, %d, %d, %d, %d\n", hdr_hdrtime, trigger_type, hold_delay, asic, channel, sample);
      #endif

      XTRACE(PROCESS, INF, "sample %3d: 0x%x (%d)\n", i, sample, sample);
    }

  return events;
}

/** Parse data according to IDEAS documentation
 * does not generate events, always return 0 @todo
 */
int IDEASData::parse_multi_event_pulse_height_data_packet(const char *buffer){
    static const int BYTES_PER_SAMPLE=5;
    if (hdr_length < 12) {
      XTRACE(PROCESS, WAR, "data packet too short for mpeh data\n");
      return -IDEASData::EBADSIZE;
    }

    int N = *(uint8_t*)buffer;                /**< number of events            */
    int M = ntohs(*(uint16_t *)(buffer + 1)); /**< number of samples per event */
    XTRACE(PROCESS, DEB, "Readout events: %d, samples per event %d\n", N, M);

    int expect_len = (4 + BYTES_PER_SAMPLE * M) * N + 3;
    if ( expect_len > hdr_length) {
      XTRACE(PROCESS, WAR, "Data length error: expected len %d, got: %d\n",
           expect_len, hdr_length);
      return -IDEASData::EHEADER;
    }

  for (int n = 0; n < N; n++) {
    auto evoff = buffer + 3 + (4 + BYTES_PER_SAMPLE * M) * n; // Event offset
    uint32_t evtime = ntohl(*(uint32_t*)(evoff));
    for (int m = 0; m < M; m++) {
       samples++;
       //events++; //Careful, data array will be parsed
       int sampleoffset = BYTES_PER_SAMPLE * m;
       int trigger_type = *(uint8_t*)(evoff + sampleoffset + 4);
       int asic = *(uint8_t*)(evoff + sampleoffset + 5);
       int channel = *(uint8_t*)(evoff + sampleoffset + 6 );
       int sample = ntohs(*(uint16_t*)(evoff + sampleoffset + 7));

       int pixelid = sondegeometry->getdetectorpixelid(hdr_sysno, asic, channel);
       if (pixelid >= 1) {
         data[events].time = evtime;
         data[events].pixel_id = static_cast<uint32_t>(pixelid);
         events++;
       }
       XTRACE(PROCESS, INF, "time %x, tt %d, as %d, ch %d, sampl %x\n", evtime, trigger_type, asic, channel, sample);
       #ifdef DUMPTOFILE
       mephdata.tofile("%d, %u, %d, %d, %d, %d\n", hdr_count, evtime, trigger_type, asic, channel, sample);
       #endif
    }
  }

  return events;
}

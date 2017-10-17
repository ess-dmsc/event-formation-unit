/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <arpa/inet.h>
#include <cassert>
#include <common/Trace.h>
#include <cstring>
#include <sonde/ideas/Data.h>

using namespace std;

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

int IDEASData::receive(const char *buffer, int size) {

  if (buffer == nullptr) {
    XTRACE(PROCESS, WAR, "Invalid buffer\n");
    return 0;
  }

  if (size < 11) {
    XTRACE(PROCESS, WAR, "IDEAS readout header too short (%d bytes)\n", size);
    return 0;
  }

  struct Header * hdr = (struct Header *)buffer;

  int version = (ntohs(hdr->id) & 0xe000) >> 13;
  if (version != 0) {
    XTRACE(PROCESS, WAR, "Illegal version number (%d)\n", version);
    return 0;
  }
  hdr_sysno = (ntohs(hdr->id) & 0x1fff) >> 8;
  hdr_type = (ntohs(hdr->id) & 0x00ff);
  hdr_seqflag = (ntohs(hdr->pktseq) & 0xc000) >> 14;
  hdr_count = (ntohs(hdr->pktseq) & 0x3fff);
  hdr_hdrtime = ntohl(hdr->timestamp);
  hdr_length = ntohs(hdr->length);

  XTRACE(PROCESS, DEB, "version: %d, sysno: %d, type: 0x%02x\n", version, hdr_sysno, hdr_type);
  XTRACE(PROCESS, DEB, "sequence flag: %d, packet count: %d\n", hdr_seqflag, hdr_count);
  XTRACE(PROCESS, DEB, "time: 0x%08x, size: 0x%04x\n", hdr_hdrtime, hdr_length);

  if (hdr_length + (int)sizeof(struct Header) != size) {
    XTRACE(PROCESS, WAR, "Packet length mismatch: udp: %d, parsed: %d\n",
         size, hdr_length + (int)sizeof(struct Header));
    return 0;
  }

  if (hdr_type == 0xD6) {
    return parse_trigger_time_data_packet(buffer + sizeof(struct Header));
  } else if (hdr_type == 0xD5) {
    return parse_single_event_pulse_height_data_packet(buffer + sizeof(struct Header));
    return 0;
  } else if (hdr_type == 0xD4) {
    return parse_multi_event_pulse_height_data_packet(buffer + sizeof(struct Header));
    return 0;
  } else {
    XTRACE(PROCESS, WAR, "Unsupported readout format: 0x%02x\n", hdr_type);
    return 0;
  }
}


/** Parse data according to IDEAS documentation */
int IDEASData::parse_trigger_time_data_packet(const char *buffer) {
  uint8_t * datap = (uint8_t *)(buffer);
  int nentries = *datap;
  XTRACE(PROCESS, DEB, "Number of readout events in packet: %d\n", nentries);

  if (nentries * 5 + 1 != hdr_length) {
    XTRACE(PROCESS, WAR, "Data length error: events %d (len %d), got: %d\n",
         nentries, nentries * 5 + 1, hdr_length);
    return 0;
  }

  events = 0;
  errors = 0;
  for (int i = 0; i < nentries; i++) {
    auto timep = (uint32_t *)(datap + i*5 + 1);
    auto aschp  = (uint8_t *)(datap + i*5 + 5);
    uint32_t time = ntohl(*timep);
    uint8_t asch = *aschp; // ASIC (2b) and CHANNEL (6b)

    int pixelid = sondegeometry->getdetectorpixelid(0, asch);
    if (pixelid >= 1) {
      data[events].time = time;
      data[events].pixel_id = static_cast<uint32_t>(pixelid);
      #ifdef DUMPTOFILE
          dprintf(fd, "%d, %d, %d, %d, %d, %d\n", hdr_count, hdr_hdrtime, hdr_sysno, asch>>6, asch & 0x3f, pixelid);
      #endif
      XTRACE(PROCESS, DEB, "event: %d, time: 0x%08x, pixel: %d\n", i, time, data[events].pixel_id);
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
 * does not generate events, always return 0 @todo
 */
int IDEASData::parse_single_event_pulse_height_data_packet(const char *buffer){
    samples = 0;
    events = 0;
    errors = 0;
    int nentries = ntohs(*(uint16_t*)(buffer + 5));
    XTRACE(PROCESS, DEB, "Number of readout events in packet: %d\n", nentries);

    if (nentries * 2 + 7 != hdr_length) {
      XTRACE(PROCESS, WAR, "Data length error: events %d (len %d), got: %d\n",
           nentries, nentries * 2 + 5, hdr_length);
      return 0;
    }
    int asic = *(uint8_t*)(buffer);
    int trigger_type = *(uint8_t*)(buffer + 1);
    int channel = *(uint8_t*)(buffer + 2);
    int hold_delay = ntohs(*(uint16_t*)(buffer + 3));
    XTRACE(PROCESS, DEB, "asic: %d, channel: %d, trigger type: %d, hold delay: %d\n",
            asic, channel, trigger_type, hold_delay);

    for (int i = 0; i < nentries; i++) {
      samples++;
      uint16_t sample = ntohs(*(uint16_t *)(buffer + i*2 + 7));
      XTRACE(PROCESS, DEB, "sample %3d: 0x%x (%d)\n", i, sample, sample);
    }

  return 0;
}

/** Parse data according to IDEAS documentation
 * does not generate events, always return 0 @todo
 */
int IDEASData::parse_multi_event_pulse_height_data_packet(const char *buffer){
    samples = 0;
    events = 0;
    errors = 0;
    if (hdr_length < 12) {
      XTRACE(PROCESS, DEB, "data packet too short for mpeh data\n");
      return 0;
    }
    int nentries = *(uint8_t*)buffer;
    int msamples = ntohs(*(uint16_t *)(buffer + 1));
    XTRACE(PROCESS, DEB, "Readout events: %d, samples per event %d\n", nentries, msamples);

    int expect_len = (4 + 5 * msamples)*nentries + 3;
    if ( expect_len != hdr_length) {
      XTRACE(PROCESS, WAR, "Data length error: expected len %d, got: %d\n",
           expect_len, hdr_length);
      return 0;
    }

  for (int n = 0; n < nentries; n++) {
    auto evoff = buffer + 3 + (4+5*msamples)*n; // Event offset
    uint32_t evtime = ntohl(*(uint32_t*)(evoff));
    XTRACE(PROCESS, DEB, "event %d time %0x\n", n, evtime);
    for (int m = 0; m < msamples; m++) {
       samples++;
       int trigger_type = *(uint8_t*)(evoff + 4 + 5*m);
       int asic = *(uint8_t*)(evoff + 5 + 5*m);
       int channel = *(uint8_t*)(evoff + 6 + 5*m);
       int sample = ntohs(*(uint16_t*)(evoff + 7 + 5*m));
       XTRACE(PROCESS, DEB, "time %x, tt %d, as %d, ch %d, sampl %x\n", evtime, trigger_type, asic, channel, sample);
    }
  }

  return 0;
}

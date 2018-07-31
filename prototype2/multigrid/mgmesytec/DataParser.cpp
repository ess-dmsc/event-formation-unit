/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/DataParser.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

// clang-format off
// sis3153 and mesytec data types from
// Struck: mvme-src-0.9.2-281-g1c4c24c.tar
// Struck: Ethernet UDP Addendum revision 107
enum SisType : uint32_t {
  BeginReadout = 0xbb000000,
  EndReadout = 0xee000000
};
// clang-format on

// \todo put this somewhere else
std::string MesytecData::time_str() {
  char cStartTime[50];
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(cStartTime, 50, "%Y-%m-%d-%H-%M-%S", timeinfo);
  std::string startTime = cStartTime;
  return startTime;
}


MesytecData::MesytecData(MgEFU mg_efu, std::shared_ptr<ReadoutSerializer> s, std::string fileprefix)
    : vmmr16Parser(mg_efu, s) {

  if (!fileprefix.empty()) {
    dumpfile = std::make_shared<MGHitFile>();
    dumpfile->open_rw(fileprefix + "mesytec_" + time_str() + ".h5");
  }
}

// \todo can only create a single event per UDP buffer
uint32_t MesytecData::getPixel() {
  return Geometry.pixel3D(vmmr16Parser.mgEfu.x,
                          vmmr16Parser.mgEfu.y,
                          vmmr16Parser.mgEfu.z);
}

uint32_t MesytecData::getTime() {
  return static_cast<uint32_t>(vmmr16Parser.time() - RecentPulseTime);
}

MesytecData::error MesytecData::parse(const char *buffer,
                                      int size,
                                      EV42Serializer &EV42Serializer) {
  int bytesleft = size;
  memset(&stats, 0, sizeof(stats));

  if (buffer[0] != 0x60) {
    return error::EUNSUPP;
  }

  if (size < 19) {
    return error::ESIZE;
  }

  uint32_t *datap = (uint32_t *) (buffer + 3);
  bytesleft -= 3;

  if (dumpfile)
    vmmr16Parser.converted_data.clear();

  while (bytesleft > 16) {
    if ((*datap & 0x000000ff) != 0x58) {
      XTRACE(DATA, WAR, "expected data value 0x58\n");
      return error::EUNSUPP;
    }

    uint16_t len = ntohs((*datap & 0x00ffff00) >> 8);
    DTRACE(DEB, "sis3153 datawords %d\n", len);
    datap++;
    bytesleft -= 4;

    if ((*datap & 0xff000000) != SisType::BeginReadout) {
      XTRACE(DATA, WAR, "expected readout header value 0x%04x, got 0x%04x\n",
             SisType::BeginReadout, (*datap & 0xff000000));
      return error::EHEADER;
    }
    datap++;
    bytesleft -= 4;
    vmmr16Parser.parse(datap, len - 3, stats, static_cast<bool>(dumpfile));

    if (vmmr16Parser.externalTrigger()) {
      stats.tx_bytes += EV42Serializer.produce();
      EV42Serializer.set_pulse_time(RecentPulseTime);
      RecentPulseTime = vmmr16Parser.time();
    }

    if (vmmr16Parser.goodEvent()) {
      uint32_t pixel = getPixel();
      uint32_t time = getTime();

      DTRACE(DEB, "Event: pixel: %d, time: %d \n", pixel, time);
      if (pixel != 0) {
        stats.tx_bytes += EV42Serializer.addevent(time, pixel);
        stats.events++;
      } else {
        stats.geometry_errors++;
      }
    } else {
      stats.badtriggers++;
    }

    datap += (len - 3);
    bytesleft -= (len - 3) * 4;

    if (*datap != 0x87654321) {
      XTRACE(DATA, WAR, "Protocol mismatch, expected 0x87654321\n");
      return error::EHEADER;
    }
    datap++;
    bytesleft -= 4;

    if ((*datap & 0xff000000) != SisType::EndReadout) {
      return error::EHEADER;
    }
    datap++;
    bytesleft -= 4;
  }

  if (dumpfile) {
    dumpfile->data = std::move(vmmr16Parser.converted_data);
    dumpfile->write();
  }
    // printf("bytesleft %d\n", bytesleft);
  return error::OK;
}

/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/DataParser.h>
#include <netinet/in.h>

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

MesytecData::MesytecData(std::shared_ptr<MgEFU> mg_efu,
    bool spoof_ht, std::shared_ptr<MGHitFile> dump)

    : mgEfu(mg_efu)
    , dumpfile(dump) {
  vmmr16Parser.setSpoofHighTime(spoof_ht);
}

void MesytecData::set_geometry(ESSGeometry g)
{
  Geometry = g;
}

// \todo can only create a single event per UDP buffer
uint32_t MesytecData::getPixel() {
  if (!mgEfu)
    return 0;
  return Geometry.pixel3D(mgEfu->x(),
                          mgEfu->y(),
                          mgEfu->z());
}

uint32_t MesytecData::getTime() {
  return static_cast<uint32_t>(mgEfu->time() - RecentPulseTime);
}

MesytecData::error MesytecData::parse(const char *buffer,
                                      int size,
                                      EV42Serializer &serializer) {
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

  while (bytesleft > 16) {

    // Header1?
    if ((*datap & 0x000000ff) != 0x58) {
      XTRACE(DATA, WAR, "expected data value 0x58");
      return error::EUNSUPP;
    }
    uint16_t len = ntohs((*datap & 0x00ffff00) >> 8);
    DTRACE(DEB, "sis3153 datawords %d", len);
    datap++;
    bytesleft -= 4;

    // Header2?
    if ((*datap & 0xff000000) != SisType::BeginReadout) {
      XTRACE(DATA, WAR, "expected readout header value 0x%04x, got 0x%04x",
             SisType::BeginReadout, (*datap & 0xff000000));
      return error::EHEADER;
    }
    datap++;
    bytesleft -= 4;

    // Parse VMMR16
    vmmr16Parser.parse(datap, len - 3, stats);
    datap += (len - 3);
    bytesleft -= (len - 3) * 4;

    if (dumpfile) {
      dumpfile->data = std::move(vmmr16Parser.converted_data);
      dumpfile->write();
    }

    if (vmmr16Parser.externalTrigger()) {
      serializer.set_pulse_time(RecentPulseTime);
      stats.tx_bytes += serializer.produce();
      RecentPulseTime = vmmr16Parser.time();
    }

    if (mgEfu) {
      mgEfu->ingest(vmmr16Parser.converted_data);
      //stats.discards++;

      if (mgEfu->event_good()) {
        uint32_t pixel = getPixel();
        uint32_t time = getTime();

        DTRACE(DEB, "Event: pixel: %d, time: %d ", pixel, time);
        if (pixel != 0) {
          stats.tx_bytes += serializer.addevent(time, pixel);
          stats.events++;
        } else {
          stats.geometry_errors++;
        }
      } else {
        // \todo external triggers treated as "bad"?
        stats.badtriggers++;
      }
    }

    if (*datap != 0x87654321) {
      XTRACE(DATA, WAR, "Protocol mismatch, expected 0x87654321");
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

  return error::OK;
}

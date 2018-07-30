/** Copyright (C) 2017 European Spallation Source ERIC */

#include <arpa/inet.h>
#include <cassert>
#include <common/Trace.h>
#include <cstring>
#include <common/Hists.h>
#include <multigrid/mgmesytec/DataParser.h>
#include <common/ReadoutSerializer.h>
#include <string.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

// clang-format off
// sis3153 and mesytec data types from
// Struck: mvme-src-0.9.2-281-g1c4c24c.tar
// Struck: Ethernet UDP Addendum revision 107
enum SisType : uint32_t {
  BeginReadout = 0xbb000000,
  EndReadout = 0xee000000
};

// Mesytec Datasheet: VMMR-8/16 v00.01
enum MesytecType : uint32_t {
  Header = 0x40000000,
  ExtendedTimeStamp = 0x20000000,
  DataEvent1 = 0x30000000,
  DataEvent2 = 0x10000000,
  EndOfEvent = 0xc0000000,
  FillDummy = 0x00000000
};
// clang-format on

static constexpr uint32_t MesytecTypeMask{0xf0000000};
static constexpr uint32_t MesytecTimeMask{0x3fffffff};

static constexpr uint32_t MesytecDataWordsMask{0x000003ff};

static constexpr uint32_t MesytecModuleMask{0x00ff0000};
static constexpr uint8_t MesytecModuleBitShift{16};

static constexpr uint32_t MesytecHighTimeMask{0x0000ffff};

static constexpr uint32_t MesytecExternalTriggerMask{0x01000000};

static constexpr uint32_t MesytecBusMask{0x0f000000};
static constexpr uint8_t MesytecBusBitShift{24};
static constexpr uint32_t MesytecTimeDiffMask{0x0000ffff};

static constexpr uint32_t MesytecAddressMask{0x00fff000};
static constexpr uint8_t MesytecAddressBitShift{12};
static constexpr uint32_t MesytecAdcMask{0x00000fff};

MgEFU::MgEFU(std::shared_ptr<MgGeometry> mg_mappings) {
  if (!mg_mappings)
    throw std::runtime_error("No valid Multigrid geometry mappings provided.");

  MgMappings = mg_mappings;
}

void MgEFU::setWireThreshold(uint16_t low, uint16_t high) {
  wireThresholdLo = low;
  wireThresholdHi = high;
}

void MgEFU::setGridThreshold(uint16_t low, uint16_t high) {
  gridThresholdLo = low;
  gridThresholdHi = high;
}

void MgEFU::reset_maxima() {
  GridAdcMax = 0;
  WireAdcMax = 0;
  WireGood = false;
  GridGood = false;
}

bool MgEFU::ingest(uint8_t bus, uint16_t channel, uint16_t adc, NMXHists &hists) {
  if (MgMappings->isWire(channel) && adc >= wireThresholdLo && adc <= wireThresholdHi) {
    if (adc > WireAdcMax) {
      WireGood = true;
      WireAdcMax = adc;
      x = MgMappings->x(bus, channel);
      z = MgMappings->z(bus, channel);
      //DTRACE(INF, "     new wire adc max: ch %d\n", channel);
    }
    hists.binstrips(channel, adc, 0, 0);
    return true;
  } else if (MgMappings->isGrid(channel) && adc >= gridThresholdLo && adc <= gridThresholdHi) {
    if (adc > GridAdcMax) {
      GridGood = true;
      GridAdcMax = adc;
      y = MgMappings->y(bus, channel);
      //DTRACE(INF, "     new grid adc max: ch %d\n", channel);
    }
    hists.binstrips(0, 0, channel, adc);
    return true;
  }
  return false;
}

VMMR16Parser::VMMR16Parser(MgEFU mg_efu)
    : mgEfu(mg_efu) {
}

void VMMR16Parser::setSpoofHighTime(bool spoof) {
  spoof_high_time = spoof;
}

uint64_t VMMR16Parser::time() const
{
  return hit.total_time;
}

bool VMMR16Parser::externalTrigger() const
{
  return hit.external_trigger;
}

bool VMMR16Parser::goodEvent() const
{
  return GoodEvent;
}

void VMMR16Parser::parse(uint32_t *buffer,
                         uint16_t nWords,
                         NMXHists &hists,
                         ReadoutSerializer &serializer,
                         MgStats &stats,
                         bool dump_data) {

  stats.triggers++;
  hit.trigger_count++;

  mgEfu.reset_maxima();
  hit.bus = 0;
  hit.channel = 0;
  hit.adc = 0;
  hit.time_diff = 0;


  uint32_t *datap = buffer;

  uint16_t dataWords;

  uint16_t wordsleft = nWords;

  size_t chan_count{0};
  bool TimeGood{false};

  // Sneak peek on time although it is actually last in packet
  uint32_t *tptr = (buffer + nWords - 1);
  if ((*tptr & MesytecType::EndOfEvent) == MesytecType::EndOfEvent) {
    TimeGood = true;
    hit.low_time = *tptr & MesytecTimeMask;

    // Spoof high time if needed
    if (spoof_high_time) {
      if (hit.low_time < PreviousLowTime)
        hit.high_time++;
      PreviousLowTime = hit.low_time;
    }

    hit.total_time = (static_cast<uint64_t>(hit.high_time) << 30) + hit.low_time;
  }



  while (wordsleft > 0) {
    auto datatype = *datap & MesytecTypeMask;

    switch (datatype) {
    case MesytecType::Header:
      dataWords = static_cast<uint16_t>(*datap & MesytecDataWordsMask);
      assert(nWords > dataWords);
      hit.module = static_cast<uint8_t>((*datap & MesytecModuleMask) >> MesytecModuleBitShift);
      hit.external_trigger = (0 != (*datap & MesytecExternalTriggerMask));
//      DTRACE(INF, "   Header:  trigger=%zu,  data len=%d (words),  module=%d, external_trigger=%s\n",
//             stats.triggers, dataWords, hit.module, hit.external_trigger ? "true" : "false");
      break;

    case MesytecType::ExtendedTimeStamp:
      // This always comes before events on particular Bus
      hit.high_time = static_cast<uint16_t>(*datap & MesytecHighTimeMask);
      hit.total_time = (static_cast<uint64_t>(hit.high_time) << 30) + hit.low_time;
//      DTRACE(INF, "   ExtendedTimeStamp: high_time=%d\n", hit.high_time);
      break;

    case MesytecType::DataEvent1:
      // TODO: What if Bus number changes?

      hit.bus = static_cast<uint8_t>((*datap & MesytecBusMask) >> MesytecBusBitShift);
      hit.time_diff = static_cast<uint16_t>(*datap & MesytecTimeDiffMask);
//      DTRACE(INF, "   DataEvent1:  bus=%d,  time_diff=%d\n", hit.bus, hit.time_diff);
      break;

    case MesytecType::DataEvent2:
      // TODO: What if Bus number changes?

      // value in using something like getValue(Buffer, NBits, Offset) ?
      hit.bus = static_cast<uint8_t>((*datap & MesytecBusMask) >> MesytecBusBitShift);
      hit.channel = static_cast<uint16_t>((*datap & MesytecAddressMask) >> MesytecAddressBitShift);
      hit.adc = static_cast<uint16_t>(*datap & MesytecAdcMask);
      stats.readouts++;
      chan_count++;

      DTRACE(INF, "   DataEvent2:  %s\n", hit.debug().c_str());

      if (mgEfu.ingest(hit.bus, hit.channel, hit.adc, hists)) {
//        DTRACE(DEB, "   accepting %d,%d,%d,%d\n", time, Bus, channel, adc);
        serializer.addEntry(0, hit.channel, hit.total_time, hit.adc);

        if (dump_data) {
          converted_data.push_back(hit);
        }
      } else {
        //DTRACE(DEB, "   discarding %d,%d,%d,%d\n", time, bus, channel, adc);
        stats.discards++;
      }
      break;

    case MesytecType::FillDummy:
      break;

    default:

      if ((*datap & MesytecType::EndOfEvent) != MesytecType::EndOfEvent) {
        DTRACE(WAR, "   Unknown: 0x%08x\n", *datap);
      }

      break;
    }

    wordsleft--;
    datap++;
  }

  if (!chan_count) {
    DTRACE(INF, "   No hits:  %s\n", hit.debug().c_str());
    if (dump_data) {
      converted_data.push_back(hit);
    }
  }

  GoodEvent = TimeGood && mgEfu.GridGood && mgEfu.WireGood;
}

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


MesytecData::MesytecData(MgEFU mg_efu, std::string fileprefix)
    : vmmr16Parser(mg_efu) {

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
                                      NMXHists &hists,
                                      FBSerializer &fbserializer,
                                      ReadoutSerializer &serializer) {
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
      XTRACE(DATA, WAR, "expeced data value 0x58\n");
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
    vmmr16Parser.parse(datap, len - 3, hists, serializer,
        stats, static_cast<bool>(dumpfile));

    if (vmmr16Parser.externalTrigger()) {
//      if (fbserializer.get_pulse_time() == 0)
//        fbserializer.set_pulse_time(RecentPulseTime);
      stats.tx_bytes += fbserializer.produce();
//        XTRACE(PROCESS, WAR, "Updated fake pulse time = %zu to %zu by delta %zu\n",
//            FakePulseTime, fbserializer.get_pulse_time(), PreviousTime);
      fbserializer.set_pulse_time(RecentPulseTime);
      RecentPulseTime = vmmr16Parser.time();
    }

    if (vmmr16Parser.goodEvent()) {
      uint32_t pixel = getPixel();
      uint32_t time = getTime();

      DTRACE(DEB, "Event: pixel: %d, time: %d \n\n", pixel, time);
      if (pixel != 0) {
        stats.tx_bytes += fbserializer.addevent(time, pixel);
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

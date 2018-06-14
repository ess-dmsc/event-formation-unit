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
  EndReadout   = 0xee000000
};

// Mesytec Datasheet: VMMR-8/16 v00.01
enum MesytecType : uint32_t {
  Header            = 0x40000000,
  ExtendedTimeStamp = 0x20000000,
  DataEvent1        = 0x30000000,
  DataEvent2        = 0x10000000,
  EndOfEvent        = 0xc0000000,
  FillDummy         = 0x00000000
};
// clang-format on

static constexpr uint32_t MesytecTypeMask{0xf0000000};
static constexpr uint32_t MesytecTimeMask{0x3fffffff};

static constexpr uint32_t MesytecDataWordsMask{0x000003ff};

static constexpr uint32_t MesytecModuleMask{0x00ff0000};
static constexpr uint8_t MesytecModuleBitShift{16};

static constexpr uint32_t MesytecHighTimeMask{0x0000ffff};

static constexpr uint32_t MesytecBusMask{0x0f000000};
static constexpr uint8_t MesytecBusBitShift{24};
static constexpr uint32_t MesytecTimeDiffMask{0x0000ffff};

static constexpr uint32_t MesytecAddressMask{0x00fff000};
static constexpr uint8_t MesytecAddressBitShift{12};
static constexpr uint32_t MesytecAdcMask{0x00000fff};


// @todo can only create a single event per UDP buffer
uint32_t MesytecData::getPixel() {
  if (!GridGood || !WireGood) {
    return 0;
  }

  int x = mgseq.xcoord(0, Wire);
  int y = mgseq.ycoord(Grid);
  int z = mgseq.zcoord(Wire);
  return mg.pixel3D(x, y, z);
}

uint32_t MesytecData::getTime() {
  return Time;
}

void MesytecData::mesytec_parse_n_words(uint32_t *buffer,
                                        uint16_t nWords,
                                        NMXHists &hists,
                                        ReadoutSerializer &serializer) {
  uint32_t *datap = buffer;

  uint8_t module;

  uint16_t channel;
  uint16_t adc;
  uint16_t dataWords;
  uint16_t time_diff;

  bool accept {false};

  HighTime = 0;
  TimeGood = false;
  WireGood = false;
  GridGood = false;
  uint16_t GridAdcMax = 0;
  uint16_t WireAdcMax = 0;

  size_t chan_count {0};
  //printf("parse n words: %d\n", nWords);

  uint16_t wordsleft = nWords;
  while (wordsleft > 0) {
    auto datatype = *datap & MesytecTypeMask;

    switch (datatype) {
    case MesytecType::Header:
      dataWords = static_cast<uint16_t>(*datap & MesytecDataWordsMask);
      assert(nWords > dataWords);
      module = static_cast<uint8_t>((*datap & MesytecModuleMask) >> MesytecModuleBitShift);
      DTRACE(INF, "   Header:  trigger=%d,  data len=%d (words),  module=%d\n",
             stats.triggers, dataWords, module);
      break;

    case MesytecType::ExtendedTimeStamp:
      // This always comes before events on particular Bus
      // TODO: not meaningful for now
      HighTime = static_cast<uint16_t>(*datap & MesytecHighTimeMask);
      DTRACE(INF, "   ExtendedTimeStamp: high_time=%d\n", HighTime);
      break;

    case MesytecType::DataEvent1:
      BusGood = true;
      Bus = static_cast<uint8_t>((*datap & MesytecBusMask) >> MesytecBusBitShift);
      time_diff = static_cast<uint16_t>(*datap & MesytecTimeDiffMask);
      DTRACE(INF, "   DataEvent1:  bus=%d,  time_diff=%d\n", Bus, time_diff);
      break;

    case MesytecType::DataEvent2:
      // TODO: What if Bus number changes?

      // value in using something like getValue(Buffer, NBits, Offset) ?
      Bus = static_cast<uint8_t>((*datap & MesytecBusMask) >> MesytecBusBitShift);
      channel = static_cast<uint16_t>((*datap & MesytecAddressMask) >> MesytecAddressBitShift);
      adc = static_cast<uint16_t>(*datap & MesytecAdcMask);
      BusGood = true;
      stats.readouts++;
      chan_count++;

      //DTRACE(INF, "   DataEvent2:  bus=%d  channel=%d  adc=%d\n", Bus, channel, adc);

      accept = false;
      if (mgseq.isWire(channel) && adc >= wireThresholdLo && adc <= wireThresholdHi) {
        accept = true;
        if (adc > WireAdcMax) {
          WireGood = true;
          Wire = channel;
          WireAdcMax = adc;
          //DTRACE(INF, "     new wire adc max: ch %d\n", channel);
        }
        hists.binstrips(channel, adc, 0, 0);
      } else if (mgseq.isGrid(channel) && adc >= gridThresholdLo && adc <= gridThresholdHi) {
        accept = true;
        if (adc > GridAdcMax) {
          GridGood = true;
          Grid = channel;
          GridAdcMax = adc;
          //DTRACE(INF, "     new grid adc max: ch %d\n", channel);
        }
        hists.binstrips(0, 0, channel, adc);
      }

      if (accept) {
        //DTRACE(DEB, "   accepting %d,%d,%d,%d\n", time, bus, channel, adc);
        serializer.addEntry(0, channel, Time, adc);

        if (dumptofile) {
          mgdata->tofile("%d, %d, %d, %d, %d, %d\n",
              stats.triggers, HighTime, Time, Bus, channel, adc);
        }
      } else {
        //DTRACE(DEB, "   discarding %d,%d,%d,%d\n", time, bus, channel, adc);
        stats.discards++;
      }
      break;

    case MesytecType::FillDummy:
      DTRACE(INF, "   FillDummy\n");
      break;

    default:

      if ((*datap & MesytecType::EndOfEvent) == MesytecType::EndOfEvent) {
        TimeGood = true;
        Time = *datap & MesytecTimeMask;
        DTRACE(INF, "   EndOfEvent: timestamp=%d\n", Time);
        break;
      }

      DTRACE(WAR, "   Unknown: 0x%08x\n", *datap);
      break;
    }

    wordsleft--;
    datap++;
  }

  if (chan_count)
    DTRACE(INF, "   Bus=%d  Chan_count=%zu\n", Bus, chan_count);

  if (dumptofile && TimeGood && (!WireGood || !GridGood)) {
    mgdata->tofile("%d, %d, %d, -1, -1, -1\n",
                   stats.triggers, HighTime, Time);
  }

  if (nWords < 5)
    DTRACE(INF, "==========================================================\n"
                "================    SYNCHRO PULSE?   =====================\n"
                "==========================================================\n\n");

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
    stats.triggers++;
    mesytec_parse_n_words(datap, len - 3, hists, serializer);

    if (TimeGood && BusGood && GridGood && WireGood) {
      uint32_t pixel = getPixel();
      uint32_t time = getTime();

      if (time < PreviousTime) {
        stats.tx_bytes += fbserializer.produce();
//        XTRACE(PROCESS, WAR, "Updated fake pulse time = %zu to %zu by delta %zu\n",
//            FakePulseTime, fbserializer.get_pulse_time(), PreviousTime);
        FakePulseTime += PreviousTime;
        fbserializer.set_pulse_time(FakePulseTime);
      }

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

    PreviousTime = Time;

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
  // printf("bytesleft %d\n", bytesleft);
  return error::OK;
}

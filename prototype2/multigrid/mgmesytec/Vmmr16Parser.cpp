/** Copyright (C) 2017 European Spallation Source ERIC */

#include <arpa/inet.h>
#include <cassert>
#include <common/Trace.h>
#include <cstring>
#include <common/Hists.h>
#include <multigrid/mgmesytec/Vmmr16Parser.h>
#include <common/ReadoutSerializer.h>
#include <string.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

// clang-format off

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


VMMR16Parser::VMMR16Parser(MgEFU mg_efu, std::shared_ptr<ReadoutSerializer> s)
    : mgEfu(mg_efu), hit_serializer(s) {}

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

//      DTRACE(INF, "   DataEvent2:  %s\n", hit.debug().c_str());

      if (mgEfu.ingest(hit.bus, hit.channel, hit.adc)) {
//        DTRACE(DEB, "   accepting %d,%d,%d,%d\n", time, Bus, channel, adc);

        if (hit_serializer) {
          hit_serializer->addEntry(0, hit.channel, hit.total_time, hit.adc);
        }

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
//    DTRACE(INF, "   No hits:  %s\n", hit.debug().c_str());
    if (dump_data) {
      converted_data.push_back(hit);
    }
  }

  GoodEvent = TimeGood && mgEfu.event_good();
}

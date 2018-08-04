/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/Vmmr16Parser.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

// clang-format off
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

static constexpr uint32_t MesytecExternalTriggerMask{0x01000000};

static constexpr uint32_t MesytecBusMask{0x0f000000};
static constexpr uint8_t MesytecBusBitShift{24};
static constexpr uint32_t MesytecTimeDiffMask{0x0000ffff};

static constexpr uint32_t MesytecAddressMask{0x00fff000};
static constexpr uint8_t MesytecAddressBitShift{12};
static constexpr uint32_t MesytecAdcMask{0x00000fff};


void VMMR16Parser::setSpoofHighTime(bool spoof) {
  spoof_high_time = spoof;
}

uint64_t VMMR16Parser::time() const
{
  return hit.total_time; // \todo only the latest reported?
}

bool VMMR16Parser::externalTrigger() const
{
  return external_trigger_;
}

bool VMMR16Parser::timeGood() const
{
  return time_good_;
}

void VMMR16Parser::parse(uint32_t *buffer,
                         uint16_t nWords,
                         MgStats &stats) {

  converted_data.clear();
  time_good_ = false;
  external_trigger_ = false;

  trigger_count_++;
  stats.triggers = trigger_count_;

  uint32_t high_time {0};

  hit = MGHit();
  hit.trigger_count = trigger_count_;

  uint32_t *datap = buffer;

  uint16_t wordsleft = nWords;

  // Sneak peek on time although it is actually last in packet
  uint32_t *tptr = (buffer + nWords - 1);
  if ((*tptr & MesytecType::EndOfEvent) == MesytecType::EndOfEvent) {
    time_good_ = true;
    hit.low_time = *tptr & MesytecTimeMask;

    // Spoof high time if needed
    if (spoof_high_time) {
      if (hit.low_time < PreviousLowTime)
        hit.high_time++;
      PreviousLowTime = hit.low_time;
    }

    hit.total_time = (static_cast<uint64_t>(hit.high_time) << 30) + hit.low_time;
  }

  XTRACE(PROCESS, DEB, "VMMR16 Buffer:  size=%d, preparsed lowtime=%d, total_time=%zu",
      nWords, hit.low_time, hit.total_time);

  while (wordsleft > 0) {
    auto datatype = *datap & MesytecTypeMask;

    switch (datatype) {
    case MesytecType::Header:
      assert(nWords > static_cast<uint16_t>(*datap & MesytecDataWordsMask));
      hit.module = static_cast<uint8_t>((*datap & MesytecModuleMask) >> MesytecModuleBitShift);
      external_trigger_ = (0 != (*datap & MesytecExternalTriggerMask));
      if (external_trigger_)
      {
        converted_data.push_back(hit);
        converted_data.back().external_trigger = true;
      }
      XTRACE(PROCESS, DEB, "   Header:  trigger=%zu, module=%d, external_trigger=%s",
             stats.triggers, hit.module, hit.external_trigger ? "true" : "false");
      break;

    case MesytecType::ExtendedTimeStamp:
      // This always comes before events on particular Bus
      high_time = static_cast<uint16_t>(*datap & MesytecHighTimeMask);
      XTRACE(PROCESS, DEB, "   ExtendedTimeStamp: high_time=%d, total_time=%zu", hit.high_time, hit.total_time);
      break;

    case MesytecType::DataEvent1:
      // TODO: What if Bus number changes?

      hit.bus = static_cast<uint8_t>((*datap & MesytecBusMask) >> MesytecBusBitShift);
      hit.time_diff = static_cast<uint16_t>(*datap & MesytecTimeDiffMask);
      XTRACE(PROCESS, DEB, "   DataEvent1:  bus=%d,  time_diff=%d", hit.bus, hit.time_diff);
      break;

    case MesytecType::DataEvent2:
      // TODO: What if Bus number changes?

      // value in using something like getValue(Buffer, NBits, Offset) ?
      hit.bus = static_cast<uint8_t>((*datap & MesytecBusMask) >> MesytecBusBitShift);
      hit.channel = static_cast<uint16_t>((*datap & MesytecAddressMask) >> MesytecAddressBitShift);
      hit.adc = static_cast<uint16_t>(*datap & MesytecAdcMask);
      stats.readouts++;

      XTRACE(PROCESS, DEB, "   DataEvent2:  %s", hit.debug().c_str());

      converted_data.push_back(hit);

      break;

    case MesytecType::FillDummy:
      break;

    default:

      if ((*datap & MesytecType::EndOfEvent) != MesytecType::EndOfEvent) {
        DTRACE(WAR, "   Unknown: 0x%08x", *datap);
      }

      break;
    }

    wordsleft--;
    datap++;
  }

  for (auto& h : converted_data) {
    h.high_time = high_time;
    h.total_time = (static_cast<uint64_t>(h.high_time) << 30)
        + h.low_time
        + h.time_diff;
  }
}

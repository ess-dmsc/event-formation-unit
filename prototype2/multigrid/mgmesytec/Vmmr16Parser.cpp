/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/Vmmr16Parser.h>

#include <common/Trace.h>
//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB


static constexpr uint32_t TypeMask{0xf0000000};

static constexpr uint8_t LowTimeBits {30};
static constexpr uint32_t LowTimeMask{0x3fffffff};

static constexpr uint32_t DataWordsMask{0x000003ff};

static constexpr uint32_t ModuleMask{0x00ff0000};
static constexpr uint8_t ModuleBitShift{16};

static constexpr uint32_t HighTimeMask{0x0000ffff};

static constexpr uint32_t ExternalTriggerMask{0x01000000};

static constexpr uint32_t BusMask{0x0f000000};
static constexpr uint8_t BusBitShift{24};
static constexpr uint32_t TimeDiffMask{0x0000ffff};

static constexpr uint32_t ChannelMask{0x00fff000};
static constexpr uint8_t ChannelBitShift{12};

static constexpr uint32_t AdcMask{0x00000fff};


void VMMR16Parser::setSpoofHighTime(bool spoof) {
  spoof_high_time = spoof;
}

uint64_t VMMR16Parser::time() const
{
  return hit.total_time;
}

bool VMMR16Parser::externalTrigger() const
{
  return external_trigger_;
}

void VMMR16Parser::parse(uint32_t *buffer,
                         uint16_t nWords,
                         MgStats &stats) {

  converted_data.clear();
  external_trigger_ = false;
  hit = MGHit();

  trigger_count_++;
  stats.triggers = trigger_count_;
  hit.trigger_count = trigger_count_;

  uint32_t *datap = buffer;
  uint16_t wordsleft = nWords;

  /*
   * not using this implementation for now
  // Sneak peek on time although it is actually last in packet
  uint32_t *tptr = (buffer + nWords - 1);
  if ((*tptr & Type::EndOfEvent) == Type::EndOfEvent) {
    hit.low_time = *tptr & TimeMask;
  }
   */

  XTRACE(DATA, DEB, "VMMR16 Buffer:  size=%d", nWords);

  while (wordsleft > 0) {
    auto datatype = *datap & TypeMask;

    switch (datatype) {
    case Type::Header:
      // \todo something other than assert?
      assert(nWords > static_cast<uint16_t>(*datap & DataWordsMask));
      hit.module = static_cast<uint8_t>((*datap & ModuleMask) >> ModuleBitShift);
      external_trigger_ = (0 != (*datap & ExternalTriggerMask));
      if (external_trigger_)
      {
        converted_data.push_back(hit);
        converted_data.back().external_trigger = true;
      }
      XTRACE(DATA, DEB, "   Header:  trigger=%zu, module=%d, external_trigger=%s",
             stats.triggers, hit.module, external_trigger_ ? "true" : "false");
      break;

    case Type::ExtendedTimeStamp:
      // This always comes before events on particular Bus
      if (!spoof_high_time) {
        high_time_ = static_cast<uint16_t>(*datap & HighTimeMask);
        XTRACE(DATA, DEB, "   ExtendedTimeStamp: high_time=%d", high_time_);
      } else {
        XTRACE(DATA, DEB, "   ExtendedTimeStamp: ignored");
      }
      break;

    case Type::DataEvent1:
      hit.bus = static_cast<uint8_t>((*datap & BusMask) >> BusBitShift);
      hit.time_diff = static_cast<uint16_t>(*datap & TimeDiffMask);
      XTRACE(DATA, DEB, "   DataEvent1:  bus=%d, time_diff=%d", hit.bus, hit.time_diff);
      break;

    case Type::DataEvent2:
      // \todo use something like getValue(Buffer, NBits, Offset) ?
      hit.bus = static_cast<uint8_t>((*datap & BusMask) >> BusBitShift);
      hit.channel = static_cast<uint16_t>((*datap & ChannelMask) >> ChannelBitShift);
      hit.adc = static_cast<uint16_t>(*datap & AdcMask);
      converted_data.push_back(hit);
      stats.readouts++;

      XTRACE(DATA, DEB, "   DataEvent2:  bus=%d, channel=%d, adc=%d",
          hit.bus, hit.channel, hit.adc);


      break;

    case Type::FillDummy:
      break;

    default:

      if ((*datap & Type::EndOfEvent) == Type::EndOfEvent) {
        hit.low_time = *datap & LowTimeMask;
        XTRACE(DATA, DEB, "   EnfOfEvent:  low_time=%d", hit.low_time);
      } else {
        XTRACE(DATA, WAR, "   Unknown: 0x%08x", *datap);
      }

      break;
    }

    wordsleft--;
    datap++;
  }

  // Spoof high time if needed
  if (spoof_high_time)  {
    if (hit.low_time < previous_low_time_) {
      high_time_++;
    }
    previous_low_time_ = hit.low_time;
  }

  // Apply timestamp to all readouts
  hit.high_time = high_time_;
  hit.total_time = (static_cast<uint64_t>(high_time_) << LowTimeBits) + hit.low_time;
  for (auto& h : converted_data) {
    h.high_time = high_time_;
    h.low_time = hit.low_time;
    h.total_time = hit.total_time;
    XTRACE(DATA, DEB, "     %s", h.debug().c_str());
  }

}

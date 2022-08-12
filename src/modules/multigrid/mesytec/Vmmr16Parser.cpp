/** Copyright (C) 2017 European Spallation Source ERIC */

#include <multigrid/mesytec/Vmmr16Parser.h>

#include <common/debug/Trace.h>
// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multigrid {

// clang-format off
// Mesytec Datasheet: VMMR-8/16 v00.01
enum Type : uint32_t {
  Header            = 0x40000000,
  ExtendedTimeStamp = 0x20000000,
  DataEvent1        = 0x30000000,
  DataEvent2        = 0x10000000,
  EndOfEvent        = 0xc0000000,
  FillDummy         = 0x00000000
};

static constexpr uint32_t TypeMask            {0xf0000000};

static constexpr uint8_t  LowTimeBits         {30};
static constexpr uint32_t LowTimeMask         {0x3fffffff};

static constexpr uint32_t DataWordsMask       {0x000003ff};

static constexpr uint32_t ModuleMask          {0x00ff0000};
static constexpr uint8_t  ModuleBitShift      {16};

static constexpr uint32_t HighTimeMask        {0x0000ffff};

static constexpr uint32_t ExternalTriggerMask {0x01000000};

static constexpr uint32_t BusMask             {0x0f000000};
static constexpr uint8_t  BusBitShift         {24};
static constexpr uint32_t TimeDiffMask        {0x0000ffff};

static constexpr uint32_t ChannelMask         {0x00fff000};
static constexpr uint8_t  ChannelBitShift     {12};

static constexpr uint32_t AdcMask             {0x00000fff};

// clang-format on

void VMMR16Parser::spoof_high_time(bool spoof) { spoof_high_time_ = spoof; }

bool VMMR16Parser::spoof_high_time() const { return spoof_high_time_; }

uint64_t VMMR16Parser::time() const { return hit.total_time; }

bool VMMR16Parser::externalTrigger() const { return external_trigger_; }

size_t VMMR16Parser::trigger_count() const { return trigger_count_; }

size_t VMMR16Parser::parse(Buffer<uint32_t> buffer) {

  auto bytes = buffer.bytes();
  bool time_good{false};

  converted_data.clear();
  external_trigger_ = false;
  hit = Readout();

  trigger_count_++;
  hit.trigger_count = trigger_count_;

  uint16_t words{0};

  /*
   * not using this implementation for now
  // Sneak peek on time although it is actually last in packet
  uint32_t *tptr = (buffer + nWords - 1);
  if ((*tptr & Type::EndOfEvent) == Type::EndOfEvent) {
    hit.low_time = *tptr & TimeMask;
  }
   */

  XTRACE(DATA, DEB, "VMMR16 Buffer:  size=%d, trigger=%d", buffer.size,
         trigger_count_);

  for (; buffer; ++buffer) {
    switch (buffer[0] & TypeMask) {
    case Type::Header:
      words = static_cast<uint16_t>(buffer[0] & DataWordsMask);
      hit.module =
          static_cast<uint8_t>((buffer[0] & ModuleMask) >> ModuleBitShift);
      external_trigger_ = (0 != (buffer[0] & ExternalTriggerMask));
      if (external_trigger_) {
        converted_data.push_back(hit);
        converted_data.back().external_trigger = true;
      }
      XTRACE(DATA, DEB, "   Header:  module=%d, external_trigger=%s, words=%d",
             hit.module, external_trigger_ ? "true" : "false", words);
      if (words > buffer.size) {
        XTRACE(DATA, ERR, "   VMMR16 buffer size mismatch:  %d > %d", words,
               buffer.size);
        return buffer.bytes();
      }
      break;

    case Type::ExtendedTimeStamp:
      // This always comes before events on particular Bus
      if (!spoof_high_time_) {
        high_time_ = static_cast<uint16_t>(buffer[0] & HighTimeMask);
        XTRACE(DATA, DEB, "   ExtendedTimeStamp: high_time=%d", high_time_);
      } else {
        XTRACE(DATA, DEB, "   ExtendedTimeStamp: ignored");
      }
      break;

    case Type::DataEvent1:
      hit.bus = static_cast<uint8_t>((buffer[0] & BusMask) >> BusBitShift);
      hit.time_diff = static_cast<uint16_t>(buffer[0] & TimeDiffMask);
      XTRACE(DATA, DEB, "   DataEvent1:  bus=%d, time_diff=%d", hit.bus,
             hit.time_diff);
      break;

    case Type::DataEvent2:
      // \todo use something like getValue(Buffer, NBits, Offset) ?
      hit.bus = static_cast<uint8_t>((buffer[0] & BusMask) >> BusBitShift);
      hit.channel =
          static_cast<uint16_t>((buffer[0] & ChannelMask) >> ChannelBitShift);
      hit.adc = static_cast<uint16_t>(buffer[0] & AdcMask);
      converted_data.push_back(hit);

      XTRACE(DATA, DEB, "   DataEvent2:  bus=%d, channel=%d, adc=%d", hit.bus,
             hit.channel, hit.adc);

      break;

    case Type::FillDummy:
      XTRACE(DATA, DEB, "   FillDummy");
      break;

    default:

      if ((buffer[0] & Type::EndOfEvent) == Type::EndOfEvent) {
        hit.low_time = buffer[0] & LowTimeMask;
        time_good = true;
        XTRACE(DATA, DEB, "   EndOfEvent:  low_time=%d", hit.low_time);
      } else {
        XTRACE(DATA, WAR, "   EndOfEvent missing. Unknown field type: 0x%08x",
               buffer[0]);
        return buffer.bytes();
      }

      break;
    }
  }

  if (!time_good) {
    XTRACE(DATA, WAR, "   Missing timestamp. Rejecting data.");
    converted_data.clear();
    return bytes;
  }

  // Spoof high time if needed
  if (spoof_high_time_) {
    if (hit.low_time < previous_low_time_) {
      high_time_++;
    }
    previous_low_time_ = hit.low_time;
  }

  if (high_time_ < previous_high_time_) {
    XTRACE(DATA, WAR, "   High time overflow!");
  }
  previous_high_time_ = high_time_;

  // Apply timestamp to all readouts
  hit.high_time = high_time_;
  hit.total_time =
      (static_cast<uint64_t>(high_time_) << LowTimeBits) + hit.low_time;
  for (auto &h : converted_data) {
    h.high_time = high_time_;
    h.low_time = hit.low_time;
    h.total_time = hit.total_time;
    XTRACE(DATA, DEB, "     %s", h.debug().c_str());
  }

  return 0;
}

} // namespace Multigrid

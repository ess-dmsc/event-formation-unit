//
// Created by Jonas Nilsson on 2017-11-08.
//

#pragma once

#include <cstdint>
#include <memory>
#include "AdcParse.h"

namespace ADC {

struct PacketInfo {
  std::uint8_t *Pointer;
  std::size_t Size;
};

struct PacketHeaderSim : public PacketHeader {
  void increaseCounters() {
    GlobalCount = htons(ntohs(GlobalCount) + 1);
    ReadoutCount = htons(ntohs(ReadoutCount) + 1);
  }
};

struct DataHeaderSim : public DataHeader {
  void setTime(std::uint32_t TS_Sec, std::uint32_t TS_SecFrac) {
    TimeStamp.Seconds = htonl(TS_Sec);
    TimeStamp.SecondsFrac = htonl(TS_SecFrac);
  }
  void setChannel(std::uint16_t NewChannel) {
    Channel = htons(NewChannel);
  }
};

class PacketGenerator {
public:
  PacketGenerator(std::uint16_t OversamplingFactor);
  
  PacketInfo GeneratePacket(std::uint32_t TS_Sec, double SecFrac, std::uint16_t ChannelNr);
private:
  std::unique_ptr<std::uint8_t[]> TemplateData;
  PacketHeaderSim *PacketHeaderPtr;
  DataHeaderSim *DataHeaderPtr;
  std::size_t DataSize = {8970};
};
}

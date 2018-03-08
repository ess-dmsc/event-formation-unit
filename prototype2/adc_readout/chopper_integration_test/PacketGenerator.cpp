//
// Created by Jonas Nilsson on 2017-11-08.
//

#include "PacketGenerator.h"
#include <cmath>
#include <fstream>

namespace ADC {

PacketGenerator::PacketGenerator() {
  std::ifstream InStream("DataPacketTemplate.dat", std::ios::binary);
  if (not InStream.good()) {
    throw std::runtime_error("Could not read data packet template file.");
  }
  
  InStream.seekg(0, std::ios::end);
  std::size_t FileSize = InStream.tellg();
  if (DataSize != FileSize) {
    throw std::runtime_error("Incorrect length of the packet template file.");
  }
  InStream.seekg(0, std::ios::beg);
  TemplateData = std::unique_ptr<std::uint8_t[]>(new std::uint8_t[FileSize]);
  InStream.read(reinterpret_cast<char*>(TemplateData.get()), FileSize);
  PacketHeaderPtr = reinterpret_cast<PacketHeaderSim*>(TemplateData.get());
  DataHeaderPtr = reinterpret_cast<DataHeaderSim*>(TemplateData.get() + sizeof(PacketHeader));
  PacketHeaderPtr->GlobalCount = 0;
  PacketHeaderPtr->ReadoutCount = 0;
}

PacketInfo PacketGenerator::GeneratePacket(std::uint32_t TS_Sec, double SecFrac, std::uint16_t ChannelNr) {
  PacketInfo ReturnInfo;
  ReturnInfo.Pointer = TemplateData.get();
  ReturnInfo.Size = DataSize;
  PacketHeaderPtr->increaseCounters();
  DataHeaderPtr->setChannel(ChannelNr);
  static const double TimerCounterMax = 88052500.0/2;
  std::uint32_t SecondsFraction = std::uint32_t(TimerCounterMax * SecFrac + 0.5);
  DataHeaderPtr->setTime(TS_Sec, SecondsFraction);
  return ReturnInfo;
}

}

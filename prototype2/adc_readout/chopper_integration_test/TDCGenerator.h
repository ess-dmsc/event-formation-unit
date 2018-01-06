//
// Created by Jonas Nilsson on 2017-11-08.
//

#pragma once

#include <string>
#include "f142_logdata_generated.h"
namespace TDC {

using FBBuilder = flatbuffers::FlatBufferBuilder;

struct PacketInfo {
  std::uint8_t *Pointer;
  std::size_t Size;
};

class TDCGenerator {
public:
  TDCGenerator(std::string Name);
  PacketInfo GetTDCPacket(double TimeStamp);
private:
  std::string SourceName;
  std::unique_ptr<FBBuilder> Builder{new FBBuilder(128)};
};
}

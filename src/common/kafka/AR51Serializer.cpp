// Copyright (C) 2023 European Spallation Source, ERIC. see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of ar52 schema serialiser
///
/// See https://github.com/ess-dmsc/streaming-data-types
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <common/kafka/AR51Serializer.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB


static_assert(FLATBUFFERS_LITTLEENDIAN,
              "Flatbuffers only tested on little endian systems");


AR51Serializer::AR51Serializer(
    std::string SourceName, ProducerCallback Callback) :
    Source(SourceName), ProduceFunctor(Callback) {
}


nonstd::span<const uint8_t> & AR51Serializer::serialize(uint8_t * Data, int DataLength) {
  FBBuilder.Reset();
  auto DataBuffer = FBBuilder.CreateVector(Data, DataLength);

  auto HeaderOffset = CreateRawReadoutMessage(
    FBBuilder,
    FBBuilder.CreateString(Source),
    SeqNum++,
    DataBuffer);

  FinishRawReadoutMessageBuffer(FBBuilder, HeaderOffset);

  FBuffer = nonstd::span<const uint8_t>(FBBuilder.GetBufferPointer(),
                                        FBBuilder.GetSize());
  return FBuffer;
}

size_t AR51Serializer::produce() {
  if (FBuffer.size_bytes() != 0) {
    XTRACE(OUTPUT, DEB, "produce %zu bytes", FBuffer.size_bytes());

    if (ProduceFunctor) {
      ProduceFunctor(FBuffer, 0);
    }
    TxBytes += FBuffer.size_bytes();
    return FBuffer.size_bytes();
  }
  return 0;
}

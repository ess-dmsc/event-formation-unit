// Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Modularise sample runs.
///
//===----------------------------------------------------------------------===//
#include "DataModulariser.h"

DataModulariser::DataModulariser(std::size_t MaxSize)
    : Buffer{std::make_unique<std::uint8_t[]>(MaxSize)} {}

std::pair<void const *const, std::size_t>
DataModulariser::modularise(nonstd::span<std::uint16_t const> InSamples,
                            std::uint64_t Timestamp, std::uint16_t Channel) {
  DataHeader *HeaderPtr{reinterpret_cast<DataHeader *>(Buffer.get())};
  HeaderPtr->MagicValue = 0xABCD;
  auto TotalBytes =
      InSamples.size() * sizeof(std::uint16_t) + sizeof(DataHeader) + 4;
  HeaderPtr->Length = TotalBytes;
  HeaderPtr->Channel = Channel;
  HeaderPtr->Oversampling = 1;
  HeaderPtr->TimeStamp = RawTimeStamp{Timestamp};
  HeaderPtr->fixEndian();
  nonstd::span<std::int16_t> OutSpan(
      reinterpret_cast<std::int16_t *>(Buffer.get() + sizeof(DataHeader)),
      InSamples.size());
  std::transform(InSamples.begin(), InSamples.end(), OutSpan.begin(),
                 [](auto const &Sample) { return htons(Sample); });
  auto TrailerPtr = reinterpret_cast<std::uint32_t *>(
      Buffer.get() + InSamples.size() * sizeof(std::uint16_t) +
      sizeof(DataHeader));
  *TrailerPtr = htonl(0xBEEFCAFEu);

  return {reinterpret_cast<void const *const>(Buffer.get()), TotalBytes};
}

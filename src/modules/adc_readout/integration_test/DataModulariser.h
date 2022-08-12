// Copyright (C) 2019-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Modularise sample data (header file).
///
//===----------------------------------------------------------------------===//

#pragma once

#include <adc_readout/AdcParse.h>
#include <cstdint>
#include <memory>
#include <common/memory/span.hpp>
#include <utility>

class DataModulariser {
public:
  explicit DataModulariser(std::size_t MaxSize = 9000);
  std::pair<const void *, std::size_t>
  modularise(nonstd::span<std::uint16_t const> InSamples,
             std::uint64_t Timestamp, std::uint16_t Channel);

private:
  std::unique_ptr<std::uint8_t[]> Buffer;
};

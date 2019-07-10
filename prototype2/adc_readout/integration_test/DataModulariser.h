//
// Created by Jonas Nilsson on 2019-01-30.
//

#pragma once

#include "../AdcParse.h"
#include <cstdint>
#include <memory>
#include <span.hpp>
#include <utility>

class DataModulariser {
public:
  explicit DataModulariser(std::size_t MaxSize = 9000);
  std::pair<void const *const, std::size_t>
  modularise(nonstd::span<std::uint16_t const> InSamples,
             std::uint64_t Timestamp, std::uint16_t Channel);

private:
  std::unique_ptr<std::uint8_t[]> Buffer;
};

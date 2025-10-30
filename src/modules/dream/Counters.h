// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief DREAM application counters (for Grafana and low level debug)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <common/kafka/Producer.h>
#include <common/readout/ess/Parser.h>

struct Counters {
  // ESSReadout parser
  int64_t ErrorESSHeaders{0};

  // DREAM DataParser
  int64_t DataHeaders{0};
  int64_t Readouts{0};
  int64_t BufferErrors{0};
  int64_t DataLenErrors{0};
  int64_t FiberErrors{0};
  int64_t FENErrors{0};

  // DREAM Processing
  int64_t ProcessingIdle{0};
  int64_t Events{0};

  // Identification of the cause of produce calls
  int64_t ProduceCauseTimeout{0};
} __attribute__((aligned(64)));

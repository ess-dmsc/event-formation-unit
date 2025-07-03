// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TREX detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/readout/ess/Parser.h>
#include <memory>
#include <modules/trex/Counters.h>

namespace Trex {

class TrexBase : public Detector {
public:
  TrexBase(BaseSettings const &settings);
  ~TrexBase() = default;

  void processing_thread();

  struct Counters Counters {};
  
protected:
  std::unique_ptr<EV44Serializer> Serializer{nullptr};
};

} // namespace Trex

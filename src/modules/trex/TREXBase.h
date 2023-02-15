// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief TREX detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV44Serializer.h>
#include <modules/trex/Counters.h>

namespace Trex {

class TrexBase : public Detector {
public:
  TrexBase(BaseSettings const &settings);
  ~TrexBase() = default;

  void input_thread();
  void processing_thread();

protected:
  struct Counters Counters {};
  EV44Serializer *Serializer;
};

} // namespace Trex

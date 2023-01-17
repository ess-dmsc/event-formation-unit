// Copyright (C) 2021 - 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief VMM detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV44Serializer.h>
#include <vmm/Counters.h>

namespace VMM {

class VMMBase : public Detector {
public:
  VMMBase(BaseSettings const &settings);
  ~VMMBase() = default;

  void input_thread();
  void processing_thread();

protected:
  struct Counters Counters {};
  EV44Serializer *Serializer;
};

} // namespace VMM

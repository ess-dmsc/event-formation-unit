// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief NMX detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV42Serializer.h>
#include <nmx/Counters.h>

namespace Nmx {

class NMXBase : public Detector {
public:
  NMXBase(BaseSettings const &settings);
  ~NMXBase() = default;

  void input_thread();
  void processing_thread();

protected:
  struct Counters Counters {};
  EV42Serializer *Serializer;
};

} // namespace Nmx

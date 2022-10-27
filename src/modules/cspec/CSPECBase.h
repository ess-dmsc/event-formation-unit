// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CSPEC detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV42Serializer.h>
#include <cspec/Counters.h>

namespace Cspec {

class CspecBase : public Detector {
public:
  CspecBase(BaseSettings const &settings);
  ~CspecBase() = default;

  void input_thread();
  void processing_thread();

protected:
  struct Counters Counters {};
  EV42Serializer *Serializer;
};

} // namespace Cspec

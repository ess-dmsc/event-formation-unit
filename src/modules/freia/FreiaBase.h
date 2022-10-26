// Copyright (C) 2021 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Freia detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV42Serializer.h>
#include <freia/Counters.h>

namespace Freia {

class FreiaBase : public Detector {
public:
  FreiaBase(BaseSettings const &settings);
  ~FreiaBase() = default;

  void input_thread();
  void processing_thread();

protected:
  struct Counters Counters {};
  EV42Serializer *Serializer;
};

} // namespace Freia

// Copyright (C) 2021 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief DREAM detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV42Serializer.h>
#include <dream/Counters.h>

namespace Dream {

class DreamBase : public Detector {
public:
  explicit DreamBase(BaseSettings const &Settings);
  ~DreamBase() = default;

  void inputThread();
  void processingThread();

protected:
  struct Counters Counters;
  EV42Serializer *Serializer;
};

} // namespace Dream

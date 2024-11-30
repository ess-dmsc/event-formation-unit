// Copyright (C) 2016 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Estia detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <estia/Counters.h>

namespace Estia {

class EstiaBase : public Detector {
public:
  EstiaBase(BaseSettings const &settings);
  ~EstiaBase() = default;

  void processing_thread();

  struct Counters Counters {};
};

} // namespace Estia

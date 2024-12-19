// Copyright (C) 2016 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Freia detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <freia/Counters.h>

namespace Freia {

class FreiaBase : public Detector {
public:
  FreiaBase(BaseSettings const &settings);
  ~FreiaBase() = default;

  void processing_thread();

  struct Counters Counters {};

  std::string FlatBufferSource{"multiblade"};
};

} // namespace Freia

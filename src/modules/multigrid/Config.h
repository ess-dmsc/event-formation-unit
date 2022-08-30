// Copyright (C) 2017-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Configuration (json) of multi grid detector
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <multigrid/AbstractBuilder.h>
#include <multigrid/geometry/DetectorMappings.h>
#include <multigrid/reduction/Reduction.h>

namespace Multigrid {

struct Config {
  Config() = default;
  explicit Config(const std::string &jsonfile, std::string dump_path = "");

  DetectorMappings mappings;
  std::shared_ptr<AbstractBuilder> builder;
  Reduction reduction;

  std::string debug() const;
};

} // namespace Multigrid

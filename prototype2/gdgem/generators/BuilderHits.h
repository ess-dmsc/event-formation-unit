/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for creating NMX hits from h5 data
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/clustering/Hit.h>
#include <gdgem/nmx/AbstractBuilder.h>

namespace Gem {

class BuilderHits : public AbstractBuilder {
public:
  BuilderHits();

  /// \todo Martin document
  void process_buffer(char *buf, size_t size) override;

private:
  // preallocated
  std::vector<Hit> converted_data;
};

}

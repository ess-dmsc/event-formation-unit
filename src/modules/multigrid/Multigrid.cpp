// Copyright (C) 2018-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Multigrid detector
///
//===----------------------------------------------------------------------===//

#include <common/detector/Detector.h>
#include <multigrid/MultigridBase.h>

class MultiGrid : public MultigridBase {
public:
  explicit MultiGrid(BaseSettings Settings)
      : MultigridBase(std::move(Settings)) {}
};

DetectorFactory<MultiGrid> Factory;

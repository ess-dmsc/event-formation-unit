/** Copyright (C) 2016-2018 European Spallation Source */

#pragma once
#include <multigrid/AbstractBuilder.h>
#include <multigrid/mesytec/Readout.h>
#include <multigrid/geometry/SequoiaGeometry.h>

namespace Multigrid {

class BuilderReadouts : public AbstractBuilder {
public:
  BuilderReadouts();

  void parse(Buffer<uint8_t> buffer) override;

  std::shared_ptr<HitFile> dumpfile;

  SequoiaGeometry digital_geometry;

private:
  std::vector<Readout> converted_data;

  Hit hit;
};

}
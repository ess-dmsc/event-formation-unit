/** Copyright (C) 2016-2018 European Spallation Source */

#pragma once
#include <multigrid/AbstractBuilder.h>
#include <multigrid/mgmesytec/MesytecReadout.h>
#include <multigrid/geometry/SequoiaGeometry.h>

namespace Multigrid {

class ReadoutBuilder : public AbstractBuilder {
public:
  ReadoutBuilder();

  void parse(Buffer<uint8_t> buffer) override;

  std::shared_ptr<HitFile> dumpfile;

  SequoiaGeometry digital_geometry;

private:
  std::vector<MesytecReadout> converted_data;

  Hit hit;
};

}
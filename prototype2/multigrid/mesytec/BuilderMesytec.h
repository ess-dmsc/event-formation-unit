/** Copyright (C) 2016-2018 European Spallation Source */

#pragma once
#include <multigrid/AbstractBuilder.h>
#include <multigrid/mesytec/Sis3153Parser.h>
#include <multigrid/mesytec/Vmmr16Parser.h>
#include <multigrid/geometry/SequoiaGeometry.h>

namespace Multigrid {

class BuilderMesytec : public AbstractBuilder {
public:
  void parse(Buffer<uint8_t> buffer) override;

  Sis3153Parser sis3153parser;
  VMMR16Parser vmmr16Parser;

  std::shared_ptr<ReadoutFile> dumpfile;

  SequoiaGeometry digital_geometry;

private:
  Hit hit;
};

}
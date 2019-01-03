/** Copyright (C) 2016-2018 European Spallation Source */

#pragma once
#include <multigrid/AbstractBuilder.h>
#include <multigrid/mesytec/Sis3153Parser.h>
#include <multigrid/mesytec/Vmmr16Parser.h>
#include <multigrid/geometry/SequoiaGeometry.h>

namespace Multigrid {

class BuilderMesytec : public AbstractBuilder {
public:
  BuilderMesytec(const SequoiaGeometry& geometry, bool spoof_time,
      std::string dump_dir = "");

  void parse(Buffer<uint8_t> buffer) override;

  std::string debug() const override;


private:
  std::shared_ptr<ReadoutFile> dumpfile_;
  SequoiaGeometry digital_geometry_;

  Sis3153Parser sis3153parser_;
  VMMR16Parser vmmr16Parser_;

  // preallocated
  Hit hit_;
};

}
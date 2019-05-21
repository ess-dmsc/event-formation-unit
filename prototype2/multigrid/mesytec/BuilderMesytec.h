/** Copyright (C) 2016-2018 European Spallation Source */

#pragma once
#include <multigrid/mesytec/BuilderReadouts.h>
#include <multigrid/mesytec/Sis3153Parser.h>
#include <multigrid/mesytec/Vmmr16Parser.h>

namespace Multigrid {

class BuilderMesytec : public BuilderReadouts {
public:
  BuilderMesytec(const DigitalGeometry& geometry, bool spoof_time,
      std::string dump_dir = "");

  void parse(Buffer<uint8_t> buffer) override;

  std::string debug() const override;


private:
  std::shared_ptr<ReadoutFile> dumpfile_;
  Sis3153Parser sis3153parser_;
  VMMR16Parser vmmr16Parser_;
};

}
/** Copyright (C) 2016-2018 European Spallation Source */

#pragma once
#include <multigrid/AbstractBuilder.h>
#include <multigrid/mesytec/Readout.h>
#include <multigrid/geometry/SequoiaGeometry.h>

namespace Multigrid {

class BuilderReadouts : public AbstractBuilder {
public:
  BuilderReadouts(const SequoiaGeometry& geometry, std::string dump_dir = "");

  void parse(Buffer<uint8_t> buffer) override;

  std::string debug() const override;

protected:
  void build(const std::vector<Readout>& readouts);

  // preallocated
  Hit hit_;

private:
  std::shared_ptr<HitFile> dumpfile_;
  SequoiaGeometry digital_geometry_;

  std::vector<Readout> parsed_data_;
};

}
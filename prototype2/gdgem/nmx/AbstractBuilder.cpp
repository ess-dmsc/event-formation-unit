/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  \brief Bastract class for creating NMX hits
 */

#include "AbstractBuilder.h"

#include <common/Trace.h>

AbstractBuilder::AbstractBuilder(std::string __attribute__((unused)) dump_dir,
                                 bool dump_csv, bool dump_h5)
    : dump_csv_(dump_csv), dump_h5_(dump_h5) {
  if (dump_csv_) {
    vmmsave = std::make_shared<DataSave>(dump_dir + "VMM_", 100000000);
  }
}

AbstractBuilder::AbstractBuilder(std::shared_ptr<AbstractClusterer> x,
                                 std::shared_ptr<AbstractClusterer> y,
                                 std::string dump_dir, bool dump_csv, bool dump_h5) :
    AbstractBuilder(dump_dir, dump_csv, dump_h5) {
  clusterer_x = x;
  clusterer_y = y;
}

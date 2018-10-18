/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <gdgem/nmx/Readout.h>
#include <gdgem/clustering/test_data/OldReadoutFile.h>

int main(int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  if (argc < 2) {
    return EXIT_FAILURE;
  }

  std::string fname(argv[1]);

  std::cout << "Old file: " << fname << "\n";

  std::vector<OldReadout> old_data;
  OldReadoutFile::read(fname, old_data);

  std::cout << "Hit count: " << old_data.size() << "\n";

  std::vector<Readout> new_data;
  new_data.resize(old_data.size());
  for (size_t i = 0; i < old_data.size(); ++i) {
    auto &rnew = new_data[i];
    const auto &rold = old_data[i];
    rnew.fec = rold.fec;
    rnew.chip_id = rold.chip_id;
    rnew.bonus_timestamp = rold.bonus_timestamp;
    rnew.srs_timestamp = rold.srs_timestamp;
    rnew.channel = rold.channel;
    rnew.bcid = rold.bcid;
    rnew.tdc = rold.tdc;
    rnew.adc = rold.adc;
    rnew.over_threshold = rold.over_threshold;
  }

  auto newfile = ReadoutFile::create(fname + "_conv");
  newfile->push(new_data);

  return EXIT_SUCCESS;
}

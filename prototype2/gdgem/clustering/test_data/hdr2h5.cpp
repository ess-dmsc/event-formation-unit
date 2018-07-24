/// Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <gdgem/clustering/TestData.h>
#include <gdgem/clustering/TestDataLong.h>

int main(int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  SRSHitIO io;
  io.data = Run16_Long;
  io.write("run16long.h5");

  io.data.clear();
  io.read("run16long.h5");

  bool same = (io.data.size() == Run16_Long.size());
  if (same) {
    for (size_t i=0; i < io.data.size(); ++i)
      if (!(io.data[i] == Run16_Long[i]))
      {
        same = false;
        break;
      }
  }
  std::cout << "Are the same? " << (same) << "\n";

  return 0;
}

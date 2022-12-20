// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
//===----------------------------------------------------------------------===//

#include <common/testutils/TestBase.h>
#include <nmx/geometry/NMXGeometry.h>
#include <nmx/geometry/Config.h>
#include <common/readout/vmm3/Hybrid.h>


#include <common/debug/Trace.h>
#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

using namespace Nmx;

class NMXConfigGeometryTest : public TestBase {
protected:
  Config config{"NMX", NMX_MINI};
  NMXGeometry Geom;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(NMXConfigGeometryTest, Overlaps) {
  config = Config("NMX", NMX_MINI);
  config.loadAndApplyConfig();
  std::set<int> Plane0Coords;
  std::set<int> Plane1Coords;

  for (int RingId = 0; RingId <= config.MaxRing; RingId++){
    for (int FENId = 0; FENId <= config.MaxFEN; FENId++){
       for (int HybridId = 0; HybridId <= config.MaxHybrid; HybridId++){
         ESSReadout::Hybrid h = config.getHybrid(RingId, FENId, HybridId);
         if (h.Initialised){
           for(int Asic = 0; Asic < 2; Asic++){
            XTRACE(EVENT, DEB, "Ring %u, Fen %u, Hybrid %u", RingId, FENId, HybridId);
            for (int channel = 0; channel < 64; channel++){
              if (config.Plane[RingId][FENId][HybridId] == 0){
                int coord = Geom.coord(channel, Asic, config.Offset[RingId][FENId][HybridId], config.ReversedChannels[RingId][FENId][HybridId]);
                if (Plane0Coords.count(coord)) {
                  XTRACE(EVENT, ERR, "Channel %u, Coordinate %u already covered in Plane 0 by another hybrid", channel, coord);
                  return;
                } else {
                  Plane0Coords.insert(coord);
                }
              }
              else if (config.Plane[RingId][FENId][HybridId] == 1){
                int coord = Geom.coord(channel, Asic, config.Offset[RingId][FENId][HybridId], config.ReversedChannels[RingId][FENId][HybridId]);
                if (Plane1Coords.count(coord)) {
                  XTRACE(EVENT, ERR, "Coordinate %u already covered in Plane 0 by another hybrid", coord);
                  return;
                } else {
                  Plane1Coords.insert(coord);
                }
              }

            }
           }
         }
       }
    }
  }
}

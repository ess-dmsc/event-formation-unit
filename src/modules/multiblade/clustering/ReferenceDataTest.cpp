// Copyright (C) 2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Reference data test
///
/// Following PSI meeting and skype with FraPi 30 09 2020
//===----------------------------------------------------------------------===//

#include <test/TestBase.h>
#include <multiblade/clustering/EventBuilder.h>

using namespace Multiblade;

struct MBHits {
  float Time;
  float Channel;
  float AdcValue;
};

// Small reference dataset from Francesco Oct 2020
// 33 readouts, 10 events, 3 unmatched clusters
std::vector<struct MBHits> FPRefData {
  // Time (s)        Channel           ADC
  {9.1252640000e-03, 2.7000000000e+01, 2.1375000000e+04},
  {9.1254240000e-03, 2.8000000000e+01, 1.0315000000e+04},
  {9.1256320000e-03, 5.0000000000e+01, 5.8390000000e+03},
  {9.1258400000e-03, 4.9000000000e+01, 3.2740000000e+03},
  {9.1260960000e-03, 5.1000000000e+01, 1.9790000000e+03},
  {9.1588000000e-03, 1.0000000000e+01, 3.3407000000e+04},
  {9.1591200000e-03, 5.2000000000e+01, 8.6440000000e+03},
  {9.1593760000e-03, 5.1000000000e+01, 4.4420000000e+03},
  {9.1596320000e-03, 5.3000000000e+01, 2.4130000000e+03},
  {9.2895680000e-03, 1.8000000000e+01, 2.5456000000e+04},
  {9.2901120000e-03, 6.2000000000e+01, 4.1300000000e+03},
  {9.2902560000e-03, 6.1000000000e+01, 3.1590000000e+03},
  {9.4956480000e-03, 1.0000000000e+01, 2.4285000000e+04},
  {9.4960000000e-03, 6.0000000000e+01, 6.7930000000e+03},
  {9.4963840000e-03, 5.9000000000e+01, 2.9680000000e+03},
  {9.6056640000e-03, 1.0000000000e+00, 6.0840000000e+03},
  {9.7041760000e-03, 0.0000000000e+00, 1.1635000000e+04},
  {9.9865280000e-03, 4.2000000000e+01, 1.7840000000e+03},
  {1.0124720000e-02, 2.4000000000e+01, 1.9147000000e+04},
  {1.0125296000e-02, 4.6000000000e+01, 3.1150000000e+03},
  {1.0217376000e-02, 2.4000000000e+01, 2.1500000000e+04},
  {1.0218272000e-02, 6.3000000000e+01, 2.1770000000e+03},
  {1.0253808000e-02, 2.5000000000e+01, 1.4624000000e+04},
  {1.0254560000e-02, 6.1000000000e+01, 2.0910000000e+03},
  {1.0285024000e-02, 1.3000000000e+01, 2.7029000000e+04},
  {1.0285440000e-02, 4.2000000000e+01, 5.6720000000e+03},
  {1.0285872000e-02, 4.1000000000e+01, 2.4110000000e+03},
  {1.0286032000e-02, 4.3000000000e+01, 1.6460000000e+03},
  {1.0379984000e-02, 1.4000000000e+01, 3.2236000000e+04},
  {1.0380480000e-02, 3.3000000000e+01, 4.7520000000e+03},
  {1.0380640000e-02, 3.4000000000e+01, 3.1970000000e+03},
  {1.0454736000e-02, 7.0000000000e+00, 1.1311000000e+04},
  {1.0455744000e-02, 3.2000000000e+01, 1.5170000000e+03}
};

class ReferenceDataTest : public TestBase {
protected:
  EventBuilder builder{2000};
};


TEST_F(ReferenceDataTest, Constructor) {
  ASSERT_EQ(builder.matcher.matched_events.size(), 0);
  ASSERT_EQ(builder.p0.size(), 0);
  ASSERT_EQ(builder.p1.size(), 0);
}


TEST_F(ReferenceDataTest, LoadMBHits) {
  //EventBuilder builder2(2000U);
  uint32_t Readouts{0};
  uint32_t NoCoincidence{0};
  uint32_t GoodEvent{0};

  for (auto & MBHit : FPRefData) {
    uint64_t Time = (uint64_t)(MBHit.Time*1000000000ULL);
    uint16_t Channel = (uint16_t)MBHit.Channel;
    uint16_t AdcValue = (uint16_t)MBHit.AdcValue;

    if (MBHit.Channel < 32) {
      builder.insert({Time, Channel, AdcValue, StripPlane});
    } else {
      builder.insert({Time, (uint16_t)(Channel - 32), AdcValue, WirePlane});
    }
    Readouts++;
  }
  builder.flush();

  for (const auto &e : builder.Events) {
    //printf("%s\n", e.to_string({}, true).c_str());
    if (!e.both_planes()) {
      NoCoincidence++;
      continue;
    } else {
      auto x = static_cast<uint16_t>(std::round(e.ClusterA.coord_center()));
      auto y = static_cast<uint16_t>(std::round(e.ClusterB.coord_center()));
      printf("x: %d, y: %d\n", x, y);
      GoodEvent++;
    }
  }

  ASSERT_EQ(Readouts, 33);
  ASSERT_EQ(NoCoincidence, 3);
  ASSERT_EQ(GoodEvent, 10);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

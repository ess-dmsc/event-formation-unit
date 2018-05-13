/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/vmm2/BuilderVMM2.h>
#include <gdgem/vmm2/ParserVMM2TestData.h>
#include <gdgem/clustering/Clusterer1.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

class BuilderVMM2Test : public TestBase {
protected:
  BuilderVMM2 *builder;
  NMXHists hists;
  std::shared_ptr<Clusterer1> cx;
  std::shared_ptr<Clusterer1> cy;

  virtual void SetUp() {
    SRSTime time;
    SRSMappings geometry;
    cx = std::make_shared<Clusterer1>(200, 3, 3);
    cy = std::make_shared<Clusterer1>(200, 3, 3);

    geometry.define_plane(0, {{1, 0}, {1, 1}, {1, 6}, {1, 7}});
    geometry.define_plane(1, {{1, 10}, {1, 11}, {1, 14}, {1, 15}});
    builder = new BuilderVMM2(time, geometry, cx, cy, 0, 200, 0, 200, "", false, false);
  }
  virtual void TearDown() { delete builder; }
};

TEST_F(BuilderVMM2Test, DataTooShortForEventlets) {
  auto stats =
      builder->process_buffer((char *)srsdata_0_eventlets,
                              sizeof(srsdata_0_eventlets), hists);
  ASSERT_EQ(stats.valid_eventlets, 0);
  ASSERT_EQ(stats.geom_errors, 0);
}

TEST_F(BuilderVMM2Test, InvalidGeometry) {
  auto stats = builder->process_buffer((char *)srsdata_invalid_geometry,
                                       sizeof(srsdata_invalid_geometry),
                                       hists);
  ASSERT_EQ(stats.valid_eventlets, 1);
  ASSERT_EQ(stats.geom_errors, 1);
}

TEST_F(BuilderVMM2Test, Process22Eventlets) {
  auto stats =
      builder->process_buffer((char *)srsdata_22_eventlets,
                              sizeof(srsdata_22_eventlets), hists);
  ASSERT_EQ(stats.valid_eventlets, 22);
  ASSERT_EQ(stats.geom_errors, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

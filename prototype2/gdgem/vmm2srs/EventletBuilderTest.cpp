/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/vmm2srs/EventletBuilder.h>
#include <string>
#include <test/TestBase.h>
#include <unistd.h>

std::vector<uint8_t> data_3_ch0    { 0x00, 0x33, 0x71, 0x37, 0x56, 0x4d, 0x32, 0x00, 0x4c, 0x39, 0x2f, 0x60, // hdr
                                0xe0, 0x92, 0x24, 0x02, 0xfc, 0x00, 0x00, 0x00, // hit 1
                                0xe0, 0x92, 0x34, 0x01, 0xf2, 0x00, 0x00, 0x00, // hit 2
                                0xe0, 0x92, 0x20, 0x22, 0xea, 0x00, 0x00, 0x00, // hit 3
                              };

class EventletBuilderTest : public TestBase {
protected:
  EventletBuilder *builder;
  virtual void SetUp() {
    SRSTime time;
    SRSMappings geometry;
    geometry.define_plane(0, {{1, 0}, {1, 1}, {1, 6}, {1, 7}});
    geometry.define_plane(1, {{1, 10}, {1, 11}, {1, 14}, {1, 15}});
    builder = new EventletBuilder(time, geometry);
  }
  virtual void TearDown() { delete builder; }
};

TEST_F(EventletBuilderTest, Process) {
  Clusterer clusterer(30);
  NMXVMM2SRSData data(1125);

  //auto num1 = builder->process_readout(data, clusterer);
  //ASSERT_EQ(num1, 0);

  //data.elems = 4;
  //auto num2 = builder->process_readout(data, clusterer);
  //ASSERT_EQ(num2, 4);
  MESSAGE() << "Bad test, working on uninitialized data\n";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

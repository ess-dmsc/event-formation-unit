/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/generators/ReadoutBuilder.h>
#include <multigrid/generators/ReaderReadouts.h>
#include <multigrid/MgConfig.h>
#include <test/TestBase.h>

using namespace Multigrid;

class ReadoutBuilderTest : public TestBase {
protected:
  ReadoutBuilder builder;
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }

  void load_config(std::string jsonfile) {
    Multigrid::Config config(jsonfile);
    builder.digital_geometry = config.mappings;
  }
};


TEST_F(ReadoutBuilderTest, PCap1) {
  load_config(TEST_DATA_PATH "Sequoia_mappings.json");

  ReaderReadouts reader(TEST_DATA_PATH "readouts/154482_33meV_10148.7us_mgmesytec_2018-09-03-16-17-56");

  uint8_t buffer[9000];
  size_t readsz;
  size_t packets {0};

  for (;;) {
    readsz = reader.read((char*)&buffer);
    if (readsz > 0) {
      packets++;
      builder.parse(Buffer<uint8_t>(buffer, readsz));
    } else {
      break;
    }
  }
  
  EXPECT_EQ(packets, 4);
  EXPECT_EQ(builder.stats_discarded_bytes, 0);
  EXPECT_EQ(builder.stats_trigger_count, 0);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 0);
  EXPECT_EQ(builder.stats_digital_geom_errors, 0);

  EXPECT_EQ(builder.ConvertedData.size(), 1088);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

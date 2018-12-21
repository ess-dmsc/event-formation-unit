/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/MesytecBuilder.h>
#include <multigrid/mgmesytec/TestData.h>
#include <multigrid/MgConfig.h>
#include <tools/ReaderPcap.h>
#include <test/TestBase.h>

using namespace Multigrid;

class MesytecBuilderTest : public TestBase {
protected:
  MesytecBuilder builder;
  virtual void SetUp() {
  }
  virtual void TearDown() {
  }

  void load_config(std::string jsonfile) {
    Multigrid::Config config(jsonfile);
    builder.digital_geometry = config.mappings;
    builder.vmmr16Parser.spoof_high_time(config.spoof_high_time);
  }
};

// \todo use reference config and data

TEST_F(MesytecBuilderTest, ErrNoTimeStamp) {
  load_config(TEST_JSON_PATH "ILL_mappings.json");
  builder.parse(err_no_timestamp);

  EXPECT_EQ(builder.stats_discarded_bytes, 528);
  EXPECT_EQ(builder.stats_trigger_count, 0);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 0);
  EXPECT_EQ(builder.stats_digital_geom_errors, 0);

  EXPECT_EQ(builder.ConvertedData.size(), 0);
}

TEST_F(MesytecBuilderTest, ParseRecordedWSData) {
  load_config(TEST_JSON_PATH "ILL_mappings.json");
  builder.parse(ws1);

  EXPECT_EQ(builder.stats_discarded_bytes, 0);
  EXPECT_EQ(builder.stats_trigger_count, 1);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 0);
  EXPECT_EQ(builder.stats_digital_geom_errors, 8);

  EXPECT_EQ(builder.ConvertedData.size(), 120);
}

TEST_F(MesytecBuilderTest, ParseRecordedWSDataII) {
  load_config(TEST_JSON_PATH "ILL_mappings.json");
  builder.parse(ws2);

  EXPECT_EQ(builder.stats_discarded_bytes, 0);
  EXPECT_EQ(builder.stats_trigger_count, 2);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 0);
  EXPECT_EQ(builder.stats_digital_geom_errors, 16);

  EXPECT_EQ(builder.ConvertedData.size(), 240);
}

TEST_F(MesytecBuilderTest, ParseRecordedWSDataIII) {
  load_config(TEST_JSON_PATH "ILL_mappings.json");
  builder.parse(ws3);

  EXPECT_EQ(builder.stats_discarded_bytes, 0);
  EXPECT_EQ(builder.stats_trigger_count, 4);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 0);
  EXPECT_EQ(builder.stats_digital_geom_errors, 16);

  EXPECT_EQ(builder.ConvertedData.size(), 240);
}

TEST_F(MesytecBuilderTest, ParseRecordedWSDataMultipleTriggers) {
  load_config(TEST_JSON_PATH "ILL_mappings.json");
  builder.parse(ws4);

  EXPECT_EQ(builder.stats_discarded_bytes, 0);
  EXPECT_EQ(builder.stats_trigger_count, 34);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 0);
  EXPECT_EQ(builder.stats_digital_geom_errors, 0);

  EXPECT_EQ(builder.ConvertedData.size(), 54);
}

TEST_F(MesytecBuilderTest, PCap1) {
  load_config(TEST_DATA_PATH "Sequoia_mappings.json");

  ReaderPcap pcap(TEST_DATA_PATH "wireshark/mgmesytec_00001_20180830170213.pcapng");

  uint8_t buffer[10000];
  int rdsize;
  size_t packets {0};
  while ((rdsize = pcap.read((char *)&buffer, sizeof(buffer))) != -1) {
    if (rdsize == 0) {
      continue; // non udp data
    }
    builder.parse(Buffer<uint8_t>(buffer, rdsize));
    packets++;
  }

  EXPECT_EQ(packets, 25);
  EXPECT_EQ(builder.stats_discarded_bytes, 3784);
  EXPECT_EQ(builder.stats_trigger_count, 0);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 0);
  EXPECT_EQ(builder.stats_digital_geom_errors, 0);

  EXPECT_EQ(builder.ConvertedData.size(), 0);
}

TEST_F(MesytecBuilderTest, PCap2) {
  load_config(TEST_DATA_PATH "Sequoia_mappings.json");

  ReaderPcap pcap(TEST_DATA_PATH "wireshark/2018_03_30_Si_WHITE_1att_mvmeCrash.pcapng");

  uint8_t buffer[10000];
  int rdsize;
  size_t packets {0};
  while ((rdsize = pcap.read((char *)&buffer, sizeof(buffer))) != -1) {
    if (rdsize == 0) {
      continue; // non udp data
    }
    builder.parse(Buffer<uint8_t>(buffer, rdsize));
    packets++;
  }

  EXPECT_EQ(packets, 25);
  EXPECT_EQ(builder.stats_discarded_bytes, 3784);
  EXPECT_EQ(builder.stats_trigger_count, 0);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 0);
  EXPECT_EQ(builder.stats_digital_geom_errors, 0);

  EXPECT_EQ(builder.ConvertedData.size(), 0);
}

TEST_F(MesytecBuilderTest, PCap3) {
  load_config(TEST_DATA_PATH "Sequoia_mappings.json");

  ReaderPcap pcap(TEST_DATA_PATH "wireshark/2018_03_30_endOfHFSeries_mvmeCrash.pcapng");

  uint8_t buffer[10000];
  int rdsize;
  size_t packets {0};
  while ((rdsize = pcap.read((char *)&buffer, sizeof(buffer))) != -1) {
    if (rdsize == 0) {
      continue; // non udp data
    }
    builder.parse(Buffer<uint8_t>(buffer, rdsize));
    packets++;
  }

  EXPECT_EQ(packets, 102);
  EXPECT_EQ(builder.stats_discarded_bytes, 12047);
  EXPECT_EQ(builder.stats_trigger_count, 0);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 0);
  EXPECT_EQ(builder.stats_digital_geom_errors, 0);

  EXPECT_EQ(builder.ConvertedData.size(), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

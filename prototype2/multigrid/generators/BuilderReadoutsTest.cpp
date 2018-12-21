/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/generators/BuilderReadouts.h>
#include <multigrid/generators/ReaderReadouts.h>
#include <multigrid/MgConfig.h>
#include <test/TestBase.h>

using namespace Multigrid;

class BuilderReadoutsTest : public TestBase {
protected:
  BuilderReadouts builder;
  size_t packets {0};
  size_t readouts {0};
  size_t external_triggers {0};
  size_t time_errors {0};
  uint64_t ShortestPulsePeriod{std::numeric_limits<uint64_t>::max()};

  virtual void SetUp() {
  }
  virtual void TearDown() {
  }

  void load_config(const std::string& jsonfile) {
    Multigrid::Config config(jsonfile);
    builder.digital_geometry = config.mappings;
    MESSAGE() << "Digital geometry: " << builder.digital_geometry.debug() << "\n";
  }

  void feed_file(const std::string& filename) {
    ReaderReadouts reader(filename);

    readouts = reader.total();
    
    uint8_t buffer[9000];
    size_t readsz;

    for (;;) {
      readsz = reader.read((char*)&buffer);
      if (readsz > 0) {
        packets++;
        builder.parse(Buffer<uint8_t>(buffer, readsz));
      } else {
        break;
      }
    }
  }

  void inspect_converted_data()
  {
    uint64_t RecentPulseTime{0};

    uint64_t prev_time{0};
    for (const auto& h : builder.ConvertedData){

//      MESSAGE() << h.debug() << "\n";

      if (h.plane == 99) {
        external_triggers++;

        if (RecentPulseTime) {
          auto PulsePeriod = h.time - RecentPulseTime;
          ShortestPulsePeriod = std::min(ShortestPulsePeriod, PulsePeriod);
        }
        RecentPulseTime = h.time;

        continue;
      }
      if (prev_time > h.time) {
        time_errors++;
        MESSAGE() << "Timing error: " << prev_time << " > " << h.time << "\n";
      }
      prev_time = h.time;
    }

  }
};


TEST_F(BuilderReadoutsTest, t00004) {
  load_config(TEST_DATA_PATH "Sequoia_mappings.json");

  feed_file(TEST_DATA_PATH "readouts/154482");

  EXPECT_EQ(packets, 4);
  EXPECT_EQ(builder.stats_discarded_bytes, 0);
  EXPECT_EQ(builder.stats_trigger_count, 0);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 0);
  EXPECT_EQ(builder.stats_digital_geom_errors, 0);

  EXPECT_EQ(builder.ConvertedData.size(), 1088);
  EXPECT_EQ(builder.ConvertedData.size(), readouts);

  inspect_converted_data();
  EXPECT_EQ(external_triggers, 467);
  EXPECT_EQ(time_errors, 0);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
}

TEST_F(BuilderReadoutsTest, t00033) {
  load_config(TEST_DATA_PATH "Sequoia_mappings.json");

  feed_file(TEST_DATA_PATH "readouts/154493");

  EXPECT_EQ(packets, 33);
  EXPECT_EQ(builder.stats_discarded_bytes, 0);
  EXPECT_EQ(builder.stats_trigger_count, 0);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 0);
  EXPECT_EQ(builder.stats_digital_geom_errors, 0);

  EXPECT_EQ(builder.ConvertedData.size(), 8724);
  EXPECT_EQ(builder.ConvertedData.size(), readouts);

  inspect_converted_data();
  EXPECT_EQ(external_triggers, 2555);
  EXPECT_EQ(time_errors, 1);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
}

TEST_F(BuilderReadoutsTest, t00311) {
  load_config(TEST_DATA_PATH "Sequoia_mappings.json");

  feed_file(TEST_DATA_PATH "readouts/154492");

  EXPECT_EQ(packets, 311);
  EXPECT_EQ(builder.stats_discarded_bytes, 0);
  EXPECT_EQ(builder.stats_trigger_count, 0);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 0);
  EXPECT_EQ(builder.stats_digital_geom_errors, 8);

  EXPECT_EQ(builder.ConvertedData.size(), 84352);
  EXPECT_EQ(builder.ConvertedData.size() + builder.stats_digital_geom_errors,
      readouts);

  inspect_converted_data();
  EXPECT_EQ(external_triggers, 975);
  EXPECT_EQ(time_errors, 35);
  EXPECT_EQ(ShortestPulsePeriod, 0);
}

TEST_F(BuilderReadoutsTest, t03710) {
  load_config(TEST_DATA_PATH "Sequoia_mappings.json");

  feed_file(TEST_DATA_PATH "readouts/154478");

  EXPECT_EQ(packets, 3710);
  EXPECT_EQ(builder.stats_discarded_bytes, 0);
  EXPECT_EQ(builder.stats_trigger_count, 0);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 0);
  EXPECT_EQ(builder.stats_digital_geom_errors, 60280);

  EXPECT_EQ(builder.ConvertedData.size(), 948716);
  EXPECT_EQ(builder.ConvertedData.size() + builder.stats_digital_geom_errors,
            readouts);

  inspect_converted_data();
  EXPECT_EQ(external_triggers, 312);
  EXPECT_EQ(time_errors, 35);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
}

TEST_F(BuilderReadoutsTest, t10392) {
  load_config(TEST_DATA_PATH "Sequoia_mappings.json");

  feed_file(TEST_DATA_PATH "readouts/154484");

  EXPECT_EQ(packets, 10392);
  EXPECT_EQ(builder.stats_discarded_bytes, 0);
  EXPECT_EQ(builder.stats_trigger_count, 0);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 0);
  EXPECT_EQ(builder.stats_digital_geom_errors, 169752);

  EXPECT_EQ(builder.ConvertedData.size(), 2656636);
  EXPECT_EQ(builder.ConvertedData.size() + builder.stats_digital_geom_errors,
            readouts);

  inspect_converted_data();
  EXPECT_EQ(external_triggers, 300);
  EXPECT_EQ(time_errors, 1);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

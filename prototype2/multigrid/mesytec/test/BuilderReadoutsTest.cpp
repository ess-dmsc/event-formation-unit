/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

// OBSERVATIONS
//
// Timing errors appear to be happening in two cases:
//     A) Leftover readouts from previous run that have not been flushed
//          in either the Mesytec or the SIS electronics
//     B) Particularly large buffers / uncleared buffers in case of what
//          we have been referring to as the "bus glitch".
//
// Geometry errors are legitimate -- channels are outside the range of where
//    we expect to see valid data. These cases appear similar to the "bus glitch",
//    since we see all channels firing.
//
// CONSEQUENCES FOR PIPELINE
//    Timing errors: check for these and flush cluseters when this happens
//    Geometry errors: do nothing, such readouts are alrady discarded in builder

#include <test/TestBase.h>

#include <multigrid/generators/ReaderReadouts.h>
#include <multigrid/mesytec/BuilderReadouts.h>
#include <common/JsonFile.h>

using namespace Multigrid;

class BuilderReadoutsTest : public TestBase {
protected:
  Multigrid::BuilderReadouts builder {DetectorMappings()};

  size_t packets{0};
  size_t readouts{0};
  size_t external_triggers{0};
  size_t time_errors{0};
  uint64_t ShortestPulsePeriod{std::numeric_limits<uint64_t>::max()};

  size_t invalid_plane{0};
  std::vector<size_t> plane_counts;

  void SetUp() override {
    load_config(TEST_DATA_PATH "Sequoia_mappings2.json");
  }
  void TearDown() override {}

  void load_config(const std::string &jsonfile) {
    nlohmann::json root = from_json_file(jsonfile);
    DetectorMappings mappings = root["mappings"];
    builder = Multigrid::BuilderReadouts(mappings);
  }

  void feed_file(const std::string &filename) {
    ReaderReadouts reader(filename);

    readouts = reader.total();

    uint8_t buffer[9000];
    size_t readsz;

    while ((readsz = reader.read((char *) &buffer)) > 0) {
      packets++;
      builder.parse(Buffer<uint8_t>(buffer, readsz));
    }
  }

  void inspect_converted_data() {
    uint64_t RecentPulseTime{0};

    uint64_t prev_time{0};
    for (size_t i=0; i < builder.ConvertedData.size(); ++i) {

      const auto &h = builder.ConvertedData[i];

      if (prev_time > h.time) {
        time_errors++;
//        MESSAGE() << "Timing error [" << i << "]: " << prev_time << " > " << h.time << "\n";
      }
      prev_time = h.time;

      if (h.plane == Hit::PulsePlane) {
        external_triggers++;

        if (RecentPulseTime) {
          auto PulsePeriod = h.time - RecentPulseTime;
          ShortestPulsePeriod = std::min(ShortestPulsePeriod, PulsePeriod);
        }
        RecentPulseTime = h.time;
      } else if (h.plane == Hit::InvalidPlane) {
        invalid_plane++;
      } else {
        if (h.plane >= plane_counts.size()) {
          plane_counts.resize(h.plane+1, 0);
        }
        plane_counts[h.plane]++;
      }

//      if ((i > 80000) && (i < 85000)) {
//        MESSAGE() << h.to_string() << "\n";
//      }

    }

  }
};

TEST_F(BuilderReadoutsTest, t00004) {
  feed_file(TEST_DATA_PATH "readouts/154482");

  EXPECT_EQ(packets, 4);
  EXPECT_EQ(builder.stats_discarded_bytes, 0);
  EXPECT_EQ(builder.stats_trigger_count, 0);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 0);
  EXPECT_EQ(builder.stats_digital_geom_errors, 0);

  EXPECT_EQ(builder.ConvertedData.size(), 1088);
  EXPECT_EQ(builder.ConvertedData.size(), readouts);

  inspect_converted_data();
  EXPECT_EQ(time_errors, 0);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
  EXPECT_EQ(external_triggers, 467);
  EXPECT_EQ(invalid_plane, 0);
  EXPECT_EQ(plane_counts.size(), 18);
  EXPECT_EQ(plane_counts[0], 0);
  EXPECT_EQ(plane_counts[1], 0);
  EXPECT_EQ(plane_counts[2], 43);
  EXPECT_EQ(plane_counts[3], 234);
  EXPECT_EQ(plane_counts[4], 19);
  EXPECT_EQ(plane_counts[5], 0);
  EXPECT_EQ(plane_counts[6], 28);
  EXPECT_EQ(plane_counts[7], 53);
  EXPECT_EQ(plane_counts[8], 0);
  EXPECT_EQ(plane_counts[9], 0);
  EXPECT_EQ(plane_counts[10], 40);
  EXPECT_EQ(plane_counts[11], 71);
  EXPECT_EQ(plane_counts[12], 11);
  EXPECT_EQ(plane_counts[13], 17);
  EXPECT_EQ(plane_counts[14], 26);
  EXPECT_EQ(plane_counts[15], 41);
  EXPECT_EQ(plane_counts[16], 15);
  EXPECT_EQ(plane_counts[17], 23);
}

TEST_F(BuilderReadoutsTest, t00033) {
  feed_file(TEST_DATA_PATH "readouts/154493");

  EXPECT_EQ(packets, 33);
  EXPECT_EQ(builder.stats_discarded_bytes, 0);
  EXPECT_EQ(builder.stats_trigger_count, 0);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 0);
  EXPECT_EQ(builder.stats_digital_geom_errors, 0);

  EXPECT_EQ(builder.ConvertedData.size(), 8724);
  EXPECT_EQ(builder.ConvertedData.size(), readouts);

  inspect_converted_data();
  EXPECT_EQ(time_errors, 1);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
  EXPECT_EQ(external_triggers, 2555);
  EXPECT_EQ(invalid_plane, 0);
  EXPECT_EQ(plane_counts.size(), 18);
  EXPECT_EQ(plane_counts[0], 0);
  EXPECT_EQ(plane_counts[1], 0);
  EXPECT_EQ(plane_counts[2], 249);
  EXPECT_EQ(plane_counts[3], 1641);
  EXPECT_EQ(plane_counts[4], 184);
  EXPECT_EQ(plane_counts[5], 0);
  EXPECT_EQ(plane_counts[6], 148);
  EXPECT_EQ(plane_counts[7], 306);
  EXPECT_EQ(plane_counts[8], 0);
  EXPECT_EQ(plane_counts[9], 0);
  EXPECT_EQ(plane_counts[10], 388);
  EXPECT_EQ(plane_counts[11], 663);
  EXPECT_EQ(plane_counts[12], 342);
  EXPECT_EQ(plane_counts[13], 684);
  EXPECT_EQ(plane_counts[14], 362);
  EXPECT_EQ(plane_counts[15], 777);
  EXPECT_EQ(plane_counts[16], 164);
  EXPECT_EQ(plane_counts[17], 261);
}

TEST_F(BuilderReadoutsTest, t00311) {
  feed_file(TEST_DATA_PATH "readouts/154492");

  EXPECT_EQ(packets, 311);
  EXPECT_EQ(builder.stats_discarded_bytes, 0);
  EXPECT_EQ(builder.stats_trigger_count, 0);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 120);
  EXPECT_EQ(builder.stats_digital_geom_errors, 8);

  EXPECT_EQ(builder.ConvertedData.size(), 84232);
  EXPECT_EQ(readouts,
            builder.ConvertedData.size()
                + builder.stats_digital_geom_errors
                + builder.stats_readout_filter_rejects);

  inspect_converted_data();
  EXPECT_EQ(time_errors, 34);
  EXPECT_EQ(ShortestPulsePeriod, 0);
  EXPECT_EQ(external_triggers, 975);
  EXPECT_EQ(invalid_plane, 0);
  EXPECT_EQ(plane_counts.size(), 18);
  EXPECT_EQ(plane_counts[0], 0);
  EXPECT_EQ(plane_counts[1], 0);
  EXPECT_EQ(plane_counts[2], 89);
  EXPECT_EQ(plane_counts[3], 510);
  EXPECT_EQ(plane_counts[4], 52);
  EXPECT_EQ(plane_counts[5], 0);
  EXPECT_EQ(plane_counts[6], 71);
  EXPECT_EQ(plane_counts[7], 129);
  EXPECT_EQ(plane_counts[8], 0);
  EXPECT_EQ(plane_counts[9], 0);
  EXPECT_EQ(plane_counts[10], 91);
  EXPECT_EQ(plane_counts[11], 145);
  EXPECT_EQ(plane_counts[12], 5467);
  EXPECT_EQ(plane_counts[13], 11553);
  EXPECT_EQ(plane_counts[14], 17526);
  EXPECT_EQ(plane_counts[15], 45428);
  EXPECT_EQ(plane_counts[16], 686);
  EXPECT_EQ(plane_counts[17], 1510);
}

TEST_F(BuilderReadoutsTest, t03710) {
  feed_file(TEST_DATA_PATH "readouts/154478");

  EXPECT_EQ(packets, 3710);
  EXPECT_EQ(builder.stats_discarded_bytes, 0);
  EXPECT_EQ(builder.stats_trigger_count, 0);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 893050);
  EXPECT_EQ(builder.stats_digital_geom_errors, 60280);

  EXPECT_EQ(builder.ConvertedData.size(), 55666);
  EXPECT_EQ(readouts,
            builder.ConvertedData.size()
                + builder.stats_digital_geom_errors
                + builder.stats_readout_filter_rejects);

  inspect_converted_data();
  EXPECT_EQ(time_errors, 0);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
  EXPECT_EQ(external_triggers, 312);
  EXPECT_EQ(invalid_plane, 0);
  EXPECT_EQ(plane_counts.size(), 18);
  EXPECT_EQ(plane_counts[0], 1061);
  EXPECT_EQ(plane_counts[1], 0);
  EXPECT_EQ(plane_counts[2], 1073);
  EXPECT_EQ(plane_counts[3], 6817);
  EXPECT_EQ(plane_counts[4], 908);
  EXPECT_EQ(plane_counts[5], 0);
  EXPECT_EQ(plane_counts[6], 789);
  EXPECT_EQ(plane_counts[7], 1545);
  EXPECT_EQ(plane_counts[8], 0);
  EXPECT_EQ(plane_counts[9], 0);
  EXPECT_EQ(plane_counts[10], 1274);
  EXPECT_EQ(plane_counts[11], 2323);
  EXPECT_EQ(plane_counts[12], 4900);
  EXPECT_EQ(plane_counts[13], 10698);
  EXPECT_EQ(plane_counts[14], 5971);
  EXPECT_EQ(plane_counts[15], 14461);
  EXPECT_EQ(plane_counts[16], 1171);
  EXPECT_EQ(plane_counts[17], 2363);
}

TEST_F(BuilderReadoutsTest, t10392) {
  feed_file(TEST_DATA_PATH "readouts/154484");

  EXPECT_EQ(packets, 10392);
  EXPECT_EQ(builder.stats_discarded_bytes, 0);
  EXPECT_EQ(builder.stats_trigger_count, 0);
  EXPECT_EQ(builder.stats_readout_filter_rejects, 2477695);
  EXPECT_EQ(builder.stats_digital_geom_errors, 169752);

  EXPECT_EQ(builder.ConvertedData.size(), 178941);
  EXPECT_EQ(readouts,
            builder.ConvertedData.size()
                + builder.stats_digital_geom_errors
                + builder.stats_readout_filter_rejects);

  inspect_converted_data();
  EXPECT_EQ(time_errors, 1);
  EXPECT_EQ(ShortestPulsePeriod, 266662);
  EXPECT_EQ(external_triggers, 300);
  EXPECT_EQ(invalid_plane, 0);
  EXPECT_EQ(plane_counts.size(), 18);
  EXPECT_EQ(plane_counts[0], 0);
  EXPECT_EQ(plane_counts[1], 0);
  EXPECT_EQ(plane_counts[2], 378);
  EXPECT_EQ(plane_counts[3], 2408);
  EXPECT_EQ(plane_counts[4], 257);
  EXPECT_EQ(plane_counts[5], 0);
  EXPECT_EQ(plane_counts[6], 256);
  EXPECT_EQ(plane_counts[7], 532);
  EXPECT_EQ(plane_counts[8], 0);
  EXPECT_EQ(plane_counts[9], 0);
  EXPECT_EQ(plane_counts[10], 619);
  EXPECT_EQ(plane_counts[11], 1192);
  EXPECT_EQ(plane_counts[12], 27239);
  EXPECT_EQ(plane_counts[13], 61143);
  EXPECT_EQ(plane_counts[14], 22169);
  EXPECT_EQ(plane_counts[15], 53156);
  EXPECT_EQ(plane_counts[16], 2862);
  EXPECT_EQ(plane_counts[17], 6430);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

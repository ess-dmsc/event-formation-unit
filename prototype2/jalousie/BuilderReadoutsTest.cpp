/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

// OBSERVATIONS
//

#include <jalousie/CdtFile.h>
#include <jalousie/BuilderReadouts.h>
#include <test/TestBase.h>

using namespace Jalousie;

class BuilderReadoutsTest : public TestBase {
protected:
  BuilderReadouts builder;

  std::vector<Readout> all_data;

  size_t packets{0};
  size_t readouts{0};
  size_t chopper_events{0};
  size_t neutron_events{0};
  size_t time_errors{0};
  uint64_t ShortestPulsePeriod{std::numeric_limits<uint64_t>::max()};

  void SetUp() override {
  }
  void TearDown() override {
  }

  void feed_file(const std::string &filename) {
    CdtFile reader(filename);

    readouts = reader.total();

    uint8_t buffer[9000];
    size_t readsz;

    while ((readsz = reader.read((char *) &buffer)) > 0) {
      packets++;
      builder.parse(Buffer<uint8_t>(buffer, readsz));
      for (const auto&r : builder.parsed_data) {
        all_data.push_back(r);
      }
    }
  }

  void inspect_converted_data() {
    uint64_t RecentPulseTime{0};

    uint64_t prev_time{0};
    for (size_t i=0; i < all_data.size(); ++i) {

      const auto &h = all_data[i];

      if (prev_time > h.time) {
        time_errors++;
//        MESSAGE() << "Timing error [" << i << "]: " << prev_time << " > " << h.time << "\n";
      }
      prev_time = h.time;

      if (h.sub_id == Readout::chopper_sub_id) {
        chopper_events++;

        if (RecentPulseTime) {
          auto PulsePeriod = h.time - RecentPulseTime;
          ShortestPulsePeriod = std::min(ShortestPulsePeriod, PulsePeriod);
        }
        RecentPulseTime = h.time;
      }
      else {
        neutron_events++;
      }

//      if ((i > 80000) && (i < 85000)) {
//        MESSAGE() << h.debug() << "\n";
//      }

    }

  }
};

TEST_F(BuilderReadoutsTest, noise) {
  feed_file(TEST_DATA_PATH "noise.bin");

  EXPECT_EQ(packets, 417);

  EXPECT_EQ(all_data.size(), 249883);
  EXPECT_EQ(all_data.size(), readouts);

  inspect_converted_data();
  EXPECT_EQ(chopper_events, 113);
  EXPECT_EQ(neutron_events, 249770);
  EXPECT_EQ(time_errors, 4906);
  EXPECT_EQ(ShortestPulsePeriod, 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

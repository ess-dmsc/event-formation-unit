/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

// OBSERVATIONS
//

#include <jalousie/CdtFile.h>
#include <jalousie/BuilderReadouts.h>
#include <test/TestBase.h>
#include <common/DynamicHist.h>
#include <set>

using namespace Jalousie;


class BuilderReadoutsTest : public TestBase {
protected:
  // From bottom to top
  uint32_t board0 {1418045}; // SUMO3
  uint32_t board1 {1416964}; // SUMO4
  uint32_t board2 {1416799}; // SUMO5
  uint32_t board3 {1416697}; // SUMO6

  BuilderReadouts builder;

  std::vector<Readout> all_data;

  size_t packets{0};
  size_t readouts{0};
  size_t chopper_events{0};
  size_t neutron_events{0};
  size_t time_errors{0};
  uint64_t ShortestPulsePeriod{std::numeric_limits<uint64_t>::max()};
  uint64_t LongestPulsePeriod{0};

  std::set<uint32_t> boards;

  uint8_t max_anode{0};
  uint8_t max_cathode{0};

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

  void inspect_converted_data(uint32_t board_id_filter = 0) {
    uint64_t RecentPulseTime{0};

    uint64_t prev_time{0};
    for (size_t i=0; i < all_data.size(); ++i) {

      const auto &h = all_data[i];

      if (board_id_filter && (h.board != board_id_filter))
        continue;

      boards.insert(h.board);

      if (prev_time > h.time) {
        time_errors++;
//        MESSAGE() << "Timing error [" << i << "]: " << prev_time << " > " << h.time << "\n";
      }
      prev_time = h.time;

      if (h.sub_id == Readout::chopper_sub_id) {
        chopper_events++;

        if (RecentPulseTime) {
          if (h.time >= RecentPulseTime) {
            auto PulsePeriod = h.time - RecentPulseTime;
//            MESSAGE() << "Pulse period: " << PulsePeriod << "\n";
            if (h.time != RecentPulseTime) {
              ShortestPulsePeriod = std::min(ShortestPulsePeriod, PulsePeriod);
              LongestPulsePeriod = std::max(LongestPulsePeriod, PulsePeriod);
            }
          }
          else {
//            auto PulsePeriod = RecentPulseTime - h.time;
//            MESSAGE() << "Pulse period: -" << PulsePeriod << "\n";
          }
        }
        RecentPulseTime = h.time;
      }
      else {
        neutron_events++;
        max_anode = std::max(max_anode, h.anode);
        max_cathode = std::max(max_cathode, h.cathode);
      }

//      if ((i > 80000) && (i < 85000)) {
//        MESSAGE() << h.debug() << "\n";
//      }

    }
  }
};

TEST_F(BuilderReadoutsTest, examine_noise) {
  feed_file(TEST_DATA_PATH "noise.bin");

  EXPECT_EQ(packets, 417);

  EXPECT_EQ(all_data.size(), 249883);
  EXPECT_EQ(all_data.size(), readouts);

  inspect_converted_data();
  EXPECT_EQ(chopper_events, 113);
  EXPECT_EQ(neutron_events, 249770);
  EXPECT_EQ(time_errors, 4906);
  EXPECT_EQ(ShortestPulsePeriod, 18);
  EXPECT_EQ(LongestPulsePeriod, 10417099);
  EXPECT_EQ(max_anode, 63);
  EXPECT_EQ(max_cathode, 63);

  EXPECT_EQ(boards.size(), 4);
  EXPECT_TRUE(boards.count(board0));
  EXPECT_TRUE(boards.count(board1));
  EXPECT_TRUE(boards.count(board2));
  EXPECT_TRUE(boards.count(board3));
}

TEST_F(BuilderReadoutsTest, noise_board0) {
  feed_file(TEST_DATA_PATH "noise.bin");

  inspect_converted_data(board0);

  EXPECT_EQ(chopper_events, 28);
  EXPECT_EQ(neutron_events, 587);
  EXPECT_EQ(time_errors, 0);
  EXPECT_EQ(ShortestPulsePeriod, 10211387);
  EXPECT_EQ(LongestPulsePeriod, 10416684);
  EXPECT_EQ(max_anode, 62);
  EXPECT_EQ(max_cathode, 61);
}

TEST_F(BuilderReadoutsTest, noise_board1) {
  feed_file(TEST_DATA_PATH "noise.bin");

  inspect_converted_data(board1);

  EXPECT_EQ(chopper_events, 28);
  EXPECT_EQ(neutron_events, 84683);
  EXPECT_EQ(time_errors, 0);
  EXPECT_EQ(ShortestPulsePeriod, 10211056);
  EXPECT_EQ(LongestPulsePeriod, 10416684);
  EXPECT_EQ(max_anode, 63);
  EXPECT_EQ(max_cathode, 63);
}

TEST_F(BuilderReadoutsTest, noise_board2) {
  feed_file(TEST_DATA_PATH "noise.bin");

  inspect_converted_data(board2);

  EXPECT_EQ(chopper_events, 29);
  EXPECT_EQ(neutron_events, 117179);
  EXPECT_EQ(time_errors, 0);
  EXPECT_EQ(ShortestPulsePeriod, 10211548);
  EXPECT_EQ(LongestPulsePeriod, 10416683);
  EXPECT_EQ(max_anode, 63);
  EXPECT_EQ(max_cathode, 47);
}

TEST_F(BuilderReadoutsTest, noise_board3) {
  feed_file(TEST_DATA_PATH "noise.bin");

  inspect_converted_data(board3);

  EXPECT_EQ(chopper_events, 28);
  EXPECT_EQ(neutron_events, 47321);
  EXPECT_EQ(time_errors, 0);
  EXPECT_EQ(ShortestPulsePeriod, 10211219);
  EXPECT_EQ(LongestPulsePeriod, 10416684);
  EXPECT_EQ(max_anode, 62);
  EXPECT_EQ(max_cathode, 63);
}

TEST_F(BuilderReadoutsTest, examine_background) {
  feed_file(TEST_DATA_PATH "background_900V900V_20190709_.bin");

  EXPECT_EQ(packets, 8056);

  EXPECT_EQ(all_data.size(), 4833344);
  EXPECT_EQ(all_data.size(), readouts);

  inspect_converted_data();
  EXPECT_EQ(chopper_events, 43919);
  EXPECT_EQ(neutron_events, 4789425);
  EXPECT_EQ(time_errors, 100083);
  EXPECT_EQ(ShortestPulsePeriod, 10);
  EXPECT_EQ(LongestPulsePeriod, 744471);
  EXPECT_EQ(max_anode, 63);
  EXPECT_EQ(max_cathode, 127);

  EXPECT_EQ(boards.size(), 4);
  EXPECT_TRUE(boards.count(board0));
  EXPECT_TRUE(boards.count(board1));
  EXPECT_TRUE(boards.count(board2));
  EXPECT_TRUE(boards.count(board3));
}


TEST_F(BuilderReadoutsTest, background_board0) {
  feed_file(TEST_DATA_PATH "background_900V900V_20190709_.bin");

  inspect_converted_data(board0);

  EXPECT_EQ(chopper_events, 12327);
  EXPECT_EQ(neutron_events, 1121647);
  EXPECT_EQ(time_errors, 19);
  EXPECT_EQ(ShortestPulsePeriod, 41479);
  EXPECT_EQ(LongestPulsePeriod, 868059);
  EXPECT_EQ(max_anode, 63);
  EXPECT_EQ(max_cathode, 127);
}

TEST_F(BuilderReadoutsTest, background_board1) {
  feed_file(TEST_DATA_PATH "background_900V900V_20190709_.bin");

  inspect_converted_data(board1);

  EXPECT_EQ(chopper_events, 10373);
  EXPECT_EQ(neutron_events, 1021296);
  EXPECT_EQ(time_errors, 17);
  EXPECT_EQ(ShortestPulsePeriod, 41172);
  EXPECT_EQ(LongestPulsePeriod, 744050);
  EXPECT_EQ(max_anode, 63);
  EXPECT_EQ(max_cathode, 63);
}

TEST_F(BuilderReadoutsTest, background_board2) {
  feed_file(TEST_DATA_PATH "background_900V900V_20190709_.bin");

  inspect_converted_data(board2);

  EXPECT_EQ(chopper_events, 11095);
  EXPECT_EQ(neutron_events, 1100051);
  EXPECT_EQ(time_errors, 14);
  EXPECT_EQ(ShortestPulsePeriod, 41632);
  EXPECT_EQ(LongestPulsePeriod, 1364085);
  EXPECT_EQ(max_anode, 63);
  EXPECT_EQ(max_cathode, 63);
}

TEST_F(BuilderReadoutsTest, background_board3) {
  feed_file(TEST_DATA_PATH "background_900V900V_20190709_.bin");

  inspect_converted_data(board3);

  EXPECT_EQ(chopper_events, 10124);
  EXPECT_EQ(neutron_events, 1546431);
  EXPECT_EQ(time_errors, 21);
  EXPECT_EQ(ShortestPulsePeriod, 41323);
  EXPECT_EQ(LongestPulsePeriod, 868060);
  EXPECT_EQ(max_anode, 63);
  EXPECT_EQ(max_cathode, 63);
}
int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}

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
#include <algorithm>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

using namespace Multiblade;

// this will conditinally include the large datasets
// and create 'unit tests' for this.
// #define INCLUDE_DS1_SORTED
// #define INCLUDE_DS1_UNSORTED
// #define INCLUDE_DS2_SORTED
// #define INCLUDE_DS2_UNSORTED

#include <ReferenceDataTestData.h>

// Specific to MB16 and MB18 prototypes
const uint16_t NumberOfStrips{32};

class ReferenceDataTest : public TestBase {
public:
  struct {
    uint32_t Readouts{0}; // total number of readouts
    uint32_t NoCoincidence{0}; // clusters with only strips or wires
    uint32_t MatchedClusters{0}; // clusters with both strips and wires
    uint32_t MatchedEvents{0}; // EFU - FP common events
    uint32_t InvalidEvents{0}; // coord gaps or multiple strips (wires?)
    uint32_t TimerWraps{0}; // Count when CAEN timer resets
  } Stats;

  void clearStats() { std::fill_n((char*)&Stats, sizeof(Stats), 0); }

  // Create a list of unmatched but 'good' events
  std::vector<Event> EventsExtra;

  // Detect large jumps in readout time
  void FixJumpsAndSort(std::vector<struct MBHits> & vec, bool sort);

  // Do the clustering and event calulations
  void LoadAndProcessReadouts(std::vector<struct MBHits> & vec);

  // Compare previously calculated events with reference interpretation
  // print out some stats
  void CountMatches(std::vector<struct MBEvents> & evts);

protected:
  // builder uses a 2us time boxing
  EventBuilder builder{2010};
};

bool isStrip(uint16_t channel) {
  return channel < NumberOfStrips;
}

bool compareByTime(const struct MBHits & a, const struct MBHits & b) {
  return a.Time < b.Time;
}

// wires and strips share channel space, but we need each coordinate
// to start at 0
uint16_t GetWireCoord(uint32_t Channel) {
  assert(Channel >= NumberOfStrips);
  return Channel - NumberOfStrips;
}

bool isEqual(float t, uint16_t x, uint16_t y, struct MBEvents & res) {
  return (fabsf(t - res.time) < 0.0000009) // 900ns
          and ((uint16_t)(std::round(res.x)) == x)
          and ((uint16_t)(std::round(res.y)) == y);
}


void ReferenceDataTest::FixJumpsAndSort(std::vector<struct MBHits> & vec, bool sort) {
  int64_t Gap{43'000'000};
  int64_t PrevTime{0xffffffffff};
  std::vector<struct MBHits> temp;

  clearStats();
  for (auto & MBHit : vec) {
    int64_t Time = (uint64_t)(MBHit.Time*1000000000ULL);

    if ((PrevTime - Time ) < Gap) {
      temp.push_back(MBHit);
    } else {
      XTRACE(CLUSTER, DEB, "Wrap: %4d, Time: %lld, PrevTime: %lld, diff %lld\n",
             Stats.TimerWraps, Time, PrevTime, (PrevTime - Time));
      Stats.TimerWraps++;
      if (sort) {
        std::sort(temp.begin(), temp.end(), compareByTime);
      }
      LoadAndProcessReadouts(temp);

      temp.clear();
      temp.push_back(MBHit);
    }
    PrevTime = Time;
  }
  LoadAndProcessReadouts(temp);

  for (const auto &e : builder.Events) {
    if (!e.both_planes()) {
      Stats.NoCoincidence++;
      continue;
    } else {
      Stats.MatchedClusters++;
    }
  }
}

//
void ReferenceDataTest::LoadAndProcessReadouts(std::vector<struct MBHits> & vec) {
  for (auto & MBHit : vec) {
    //MBHit.print();
    uint64_t Time = (uint64_t)(MBHit.Time*1000000000ULL);
    uint16_t Channel = (uint16_t)MBHit.Channel;
    uint16_t AdcValue = (uint16_t)MBHit.AdcValue;

    if (isStrip(Channel)) {
      builder.insert({Time, Channel, AdcValue, StripPlane});
    } else {
      builder.insert({Time, GetWireCoord(Channel), AdcValue, WirePlane});
    }
    Stats.Readouts++;
  }
  builder.flush();
}


// Compare the calculated (t, x, y) with the reference data
// Very slow implementation, but this is only reference data
void ReferenceDataTest::CountMatches(std::vector<struct MBEvents> & evts) {
  for (const auto &e : builder.Events) {
    if (e.both_planes()) {
      float t = e.time_start()/1000000000.0;
      auto x = static_cast<uint16_t>(std::round(e.ClusterA.coord_center()));
      auto y = static_cast<uint16_t>(std::round(e.ClusterB.coord_center()));
      uint64_t i;
      for (i = 0; i < evts.size(); i++) {
        if (isEqual(t, x, y, evts[i])) {
          Stats.MatchedEvents++;
          evts[i].x = 0;
          evts[i].y = 0;
          evts[i].time = 0.0;
          break;
        }
      }
      if (i == evts.size()) {
        if (e.ClusterB.hits.size() < e.ClusterB.coord_span()) {
          Stats.InvalidEvents++;
          continue;
        }
        if (e.ClusterA.hits.size() < e.ClusterA.coord_span()) {
          Stats.InvalidEvents++;
          continue;
        }
        EventsExtra.push_back(e);
      }
    }
  }

  uint32_t EFUEvts = builder.Events.size() - Stats.NoCoincidence - Stats.InvalidEvents;
  uint32_t FPEvts = evts.size();
  printf("Readouts: %u, Clusters: %u, No coincidence: %u, Invalid: %u, EFU Events: %u\n",
          Stats.Readouts, (uint32_t)builder.Events.size(),
          Stats.NoCoincidence, Stats.InvalidEvents, EFUEvts);
  printf("Detected %u timer resets\n", Stats.TimerWraps);
  printf("Events (efu/fp) %u/%u (%5.2f%%) - matched %u (%5.2f%%)\n",
           EFUEvts, FPEvts, EFUEvts*100.0/FPEvts,
           Stats.MatchedEvents, Stats.MatchedEvents*100.0/FPEvts);

  if (0) {
    printf("\nFirst unmatched events (EFU missed):\n");
    int PrintMissed{2};
    for (auto & evt : evts) {
      if (evt.time != 0.0) {
        printf("%f %u %u\n", evt.time, (uint16_t)(std::round(evt.x)), (uint16_t)(std::round(evt.y)));
        PrintMissed--;
        if (PrintMissed == 0)
          break;
      }
    }

    if (EventsExtra.size() != 0) {
      printf("\nFirst extra event:\n");
      printf("%s\n", EventsExtra[0].to_string({}, true).c_str());
    }
  }
}

/// ---------------------------------------------------------------------

TEST_F(ReferenceDataTest, Constructor) {
  ASSERT_EQ(builder.matcher.matched_events.size(), 0);
  ASSERT_EQ(builder.p0.size(), 0);
  ASSERT_EQ(builder.p1.size(), 0);
}


TEST_F(ReferenceDataTest, LoadSmall1_Sorted_NotFiltered) {
  FixJumpsAndSort(FPRefData, true);
  ASSERT_EQ(Stats.Readouts, 33);
  ASSERT_EQ(Stats.NoCoincidence, 3);
  ASSERT_EQ(Stats.MatchedClusters, 10);
}

TEST_F(ReferenceDataTest, LoadSmall2_Sorted_NotFiltered) {
  ASSERT_EQ(builder.matcher.matched_events.size(), 0);
  FixJumpsAndSort(DS2S_ST_FF, true);
  CountMatches(DS2S_ST_FF_Res);

  ASSERT_EQ(Stats.Readouts, 29);
  ASSERT_EQ(Stats.NoCoincidence, 3);
  ASSERT_EQ(Stats.MatchedClusters, 10);
}

#ifdef HAS_REFDATA

// #ifdef INCLUDE_DS1_SORTED
// TEST_F(ReferenceDataTest, LoadLarge_Sorted_NotFiltered) {
//   ASSERT_EQ(builder.matcher.matched_events.size(), 0);
//   FixJumpsAndSort(DS1L_ST_FF, true);
//   CountMatches(DS1L_ST_FF_Res); // can't reuse, array is overwritten
//
//   ASSERT_EQ(Stats.Readouts, 334028);
//   printf("\n");
// }
// #endif // DS1_SORTED

#ifdef INCLUDE_DS1_UNSORTED
TEST_F(ReferenceDataTest, LoadLarge_NotSorted_NotFiltered) {
  ASSERT_EQ(builder.matcher.matched_events.size(), 0);
  FixJumpsAndSort(DS1L_SF_FF, true);
  CountMatches(DS1L_ST_FF_Res); // can't reuse, array is overwritten

  ASSERT_EQ(Stats.Readouts, 334028);
  printf("\n");
}
#endif // DS1_UNSORTED


// #ifdef INCLUDE_DS2_SORTED
// TEST_F(ReferenceDataTest, LoadLarge2_Sorted_NotFiltered) {
//   ASSERT_EQ(builder.matcher.matched_events.size(), 0);
//   FixJumpsAndSort(DS2L_ST_FF, true);
//   CountMatches(DS2L_ST_FF_Res); // can't reuse, array is overwritten
//
//   ASSERT_EQ(Stats.Readouts, 260716);
//   printf("\n");
// }
// #endif // DS2_SORTED

#ifdef INCLUDE_DS2_UNSORTED
TEST_F(ReferenceDataTest, LoadLarge2_NotSorted_NotFiltered) {
  ASSERT_EQ(builder.matcher.matched_events.size(), 0);
  FixJumpsAndSort(DS2L_SF_FF, true);
  CountMatches(DS2L_ST_FF_Res); // can't reuse, array is overwritten

  ASSERT_EQ(Stats.Readouts, 260716);
  printf("\n");
}
#endif // DS2_UNSORTED
#endif // HAS_REFDATA

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

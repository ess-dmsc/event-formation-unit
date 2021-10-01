// Copyright (C) 2020-2020 European Spallation Source, ERIC. See LICENSE file

#include <common/testutils/TestBase.h>

#include <common/reduction/HitVector.h>
#include <common/reduction/analysis/EventAnalyzer.h>
#include <common/reduction/clustering/GapClusterer.h>
#include <common/reduction/matching/CenterMatcher.h>
#include <gdgem/generators/BuilderHits.h>
#include <gdgem/tests/HitGenerator.h>

#include <common/debug/Trace.h>

#include <memory>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

using namespace Gem;

class NMXCombinedProcessingTest : public TestBase {
protected:
  typedef Gem::BuilderHits HitBuilder_t;
  std::shared_ptr<AbstractBuilder> builder_;
  std::shared_ptr<AbstractAnalyzer> analyzer_;
  std::shared_ptr<AbstractMatcher> matcher_;
  std::shared_ptr<AbstractClusterer> clusterer_x_;
  std::shared_ptr<AbstractClusterer> clusterer_y_;

  void SetUp() override {
    builder_ = std::make_shared<HitBuilder_t>();

    analyzer_ = std::make_shared<EventAnalyzer>("utpc");

    uint64_t maximum_latency = 5;
    uint8_t planeA = 0;
    uint8_t planeB = 1;
    auto matcher =
        std::make_shared<CenterMatcher>(maximum_latency, planeA, planeB);
    matcher->set_max_delta_time(30);
    matcher->set_time_algorithm("utpc");
    matcher_ = matcher;

    uint64_t max_time_gap = 5;
    uint16_t max_coord_gap = 100;
    clusterer_x_ = std::make_shared<GapClusterer>(max_time_gap, max_coord_gap);
    clusterer_y_ = std::make_shared<GapClusterer>(max_time_gap, max_coord_gap);
  }
  void TearDown() override {}
};

void _cluster_plane(HitVector &hits,
                    std::shared_ptr<AbstractClusterer> clusterer,
                    std::shared_ptr<AbstractMatcher> &matcher, bool flush) {
  sort_chronologically(hits);
  clusterer->cluster(hits);
  hits.clear();
  if (flush) {
    clusterer->flush();
  }

  if (!clusterer->clusters.empty()) {
    XTRACE(CLUSTER, DEB, "inserting %i cluster(s) in plane %i into matcher",
           clusterer->clusters.size(), clusterer->clusters.front().plane());
    matcher->insert(clusterer->clusters.front().plane(), clusterer->clusters);
  }
}

TEST_F(NMXCombinedProcessingTest, Dummy) {

  int numEvents = 2;
  uint64_t time = 0;
  uint16_t numHits{3};
  uint64_t timeGap = 40;
  uint32_t interHitTime = 1;
  int HitGap0{0};
  int DeadTime0Ns{0};
  bool NoShuffle{false};
  HitGenerator HitGen;

  // accumulate several hits from several events into a pseudo packet
  HitGen.setTimeParms(time, timeGap, interHitTime);
  auto &Events = HitGen.randomEvents(numEvents, 20, 1259); // avoid edge effects
  auto &Hits = HitGen.randomHits(numHits, HitGap0, DeadTime0Ns, NoShuffle);

  ASSERT_EQ(Events.size(), numEvents);
  HitGen.printEvents();
  HitGen.printHits();

  builder_->process_buffer(reinterpret_cast<char *>(&Hits[0]),
                           sizeof(Hit) * Hits.size());

  std::shared_ptr<HitBuilder_t> hitBuilderConcrete =
      std::dynamic_pointer_cast<HitBuilder_t>(builder_);
  ASSERT_EQ(hitBuilderConcrete->converted_data.size(), Hits.size());

  // XTRACE(CLUSTER, DEB, "x hits \n%s", visualize (builder_->hit_buffer_x,
  // "").c_str()); XTRACE(CLUSTER, DEB, "y hits \n%s", visualize
  // (builder_->hit_buffer_y, "").c_str());

  // from perform_clustering()
  {
    bool flush = true; // we're matching the last time for this clustering

    if (builder_->hit_buffer_x.size()) {
      _cluster_plane(builder_->hit_buffer_x, clusterer_x_, matcher_, flush);
    }

    if (builder_->hit_buffer_y.size()) {
      _cluster_plane(builder_->hit_buffer_y, clusterer_y_, matcher_, flush);
    }

    matcher_->match(flush);
    ASSERT_EQ(matcher_->matched_events.size(), numEvents);
  }

  // from process_events()
  {
    for (auto &event : matcher_->matched_events) {
      if (!event.both_planes()) {
        continue;
      }
      ReducedEvent reduced_event = analyzer_->analyze(event);
      ASSERT_TRUE(reduced_event.good);

      // XTRACE(CLUSTER, DEB, "matched event\n%s", event.visualize("").c_str());

      XTRACE(CLUSTER, DEB, "reduced event: %s",
             reduced_event.to_string_simple().c_str());

      // Look at various error metrics for doing the best match between the
      // ReducedEvent and HitGenerator events.
      double leastL1Error = 10000.0;
      int leastL1ErrorIndex = -1;

      double leastMaxError = 10000.0;
      __attribute__((unused)) int leastMaxErrorIndex = -1;

      double leastXError = 10000.0;
      double leastYError = 10000.0;
      double leastTimeError = 10000.0;

      for (int i = 0, size = Events.size(); i < size; ++i) {
        const NeutronEvent &e = Events[i];
        double xError =
            std::abs((double)e.XPos - (double)reduced_event.x.center);
        double yError =
            std::abs((double)e.YPos - (double)reduced_event.y.center);
        double timeError =
            std::abs((double)e.TimeNs - (double)reduced_event.time);

        double L1error = xError + yError + timeError;
        double maxError = std::max(std::max(xError, yError), timeError);

        if (L1error < leastL1Error) {
          leastL1Error = L1error;
          leastL1ErrorIndex = i;

          leastXError = xError;
          leastYError = yError;
          leastTimeError = timeError;
        }

        if (maxError < leastMaxError) {
          leastMaxError = maxError;
          leastMaxErrorIndex = i;
        }
      }
      XTRACE(CLUSTER, DEB,
             "best match, least errors: [xEr=%f, yEr=%f, tEr=%f], L1Er=%f, LmaxEr=%f",
             leastXError, leastYError, leastTimeError, leastL1Error,
             leastMaxError);

      // remove the matched Event from the original events list.
      ASSERT_TRUE(leastL1ErrorIndex != -1);
      std::swap(Events[leastL1ErrorIndex], Events.back());
      ASSERT_TRUE(!Events.empty());
      Events.pop_back();
    }
  }
  // checking should have emptied the precomuted Events.
  ASSERT_TRUE(Events.empty());

  builder_->hit_buffer_x.clear();
  builder_->hit_buffer_y.clear();
}

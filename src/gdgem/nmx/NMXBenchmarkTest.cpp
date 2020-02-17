/** Copyright (C) 2017 European Spallation Source ERIC */

#include <benchmark/benchmark.h>
#include <unistd.h>

#include <common/reduction/analysis/EventAnalyzer.h>
#include <common/reduction/clustering/GapClusterer.h>
#include <common/reduction/matching/CenterMatcher.h>
#include <gdgem/generators/BuilderHits.h>
#include <gdgem/tests/HitGenerator.h>

#include <fmt/format.h>

#include <common/BenchmarkUtil.h>
#include <common/Trace.h>

#include <memory>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

using namespace Gem;

class NmxBenchmarkTest : public benchmark::Fixture {
public:
  
  typedef Gem::BuilderHits HitBuilder_t;
  std::shared_ptr<AbstractBuilder> builder_;
  std::shared_ptr<AbstractAnalyzer> analyzer_;
  std::shared_ptr<AbstractMatcher> matcher_;
  std::shared_ptr<AbstractClusterer> clusterer_x_;
  std::shared_ptr<AbstractClusterer> clusterer_y_;

  void SetUp(__attribute__((unused))  const ::benchmark::State &state) override {

    // initialize NMX processor
    {
      builder_ = std::make_shared<HitBuilder_t>();

      analyzer_ = std::make_shared<EventAnalyzer>("center-of-mass");

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
  }

  void TearDown(__attribute__((unused)) const ::benchmark::State &state) override {
  }
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

BENCHMARK_DEFINE_F(NmxBenchmarkTest, Dummy)(benchmark::State &state) {
  uint32_t totalHitCount = 0;

  BenchmarkLoop(state, [&]{
    int numEvents = state.range(0) / (4 * 2);

    // generate Hits for builder
    BenchmarkLoopPaused(state, [&] {
      SetUp(state);

      uint64_t time = 0;
      uint16_t numHits = 4; // state.range(0);
      uint64_t timeGap = 40;
      uint32_t interHitTime = 1;

      HitGenerator HitGen;
      float Angle0{0.0};

      // accumulate several hits from several events into a pseudo packet
      for (int i = 0; i < numEvents; ++i) {
        HitGen.setTimes(time, timeGap, interHitTime);
        HitGen.makeHit(numHits, 0, 0, Angle0, false);
        time += numHits * 2 * interHitTime + timeGap;
      }

      // store hits in builder
      builder_->process_buffer(reinterpret_cast<char *>(&HitGen.Hits[0]),
                               sizeof(Hit) * HitGen.Hits.size());

      std::shared_ptr<HitBuilder_t> hitBuilderConcrete =
          std::dynamic_pointer_cast<HitBuilder_t>(builder_);

      assert(hitBuilderConcrete->converted_data.size() == HitGen.Hits.size());

      totalHitCount += hitBuilderConcrete->converted_data.size();
    });

    // perform_clustering()
    {
      bool flush = true; // we're matching the last time for this clustering

      if (builder_->hit_buffer_x.size())
        _cluster_plane(builder_->hit_buffer_x, clusterer_x_, matcher_, flush);

      if (builder_->hit_buffer_y.size())
        _cluster_plane(builder_->hit_buffer_y, clusterer_y_, matcher_, flush);

      matcher_->match(flush);
      assert(matcher_->matched_events.size() == (size_t)numEvents);
    }

    // process_events()
    {
      for (auto &event : matcher_->matched_events) {
        if (!event.both_planes())
          continue;

        ReducedEvent neutron_event_ = analyzer_->analyze(event);
        ::benchmark::DoNotOptimize(neutron_event_.good);
      }
    }

    builder_->hit_buffer_x.clear();
    builder_->hit_buffer_y.clear();
  });

  state.SetBytesProcessed(sizeof(Hit) * totalHitCount);
  state.SetItemsProcessed(totalHitCount);
  state.SetComplexityN(totalHitCount);
}
//BENCHMARK_REGISTER_F(NmxBenchmarkTest, Dummy)->RangeMultiplier(2)->Range(8, 8)->Complexity();
//BENCHMARK_REGISTER_F(NmxBenchmarkTest, Dummy)->RangeMultiplier(2)->Range(8, 8000)->Complexity();
BENCHMARK_REGISTER_F(NmxBenchmarkTest, Dummy)->RangeMultiplier(2)->Range(692, 692)->Complexity();
// BENCHMARK(NmxBenchmarkTest);

static void ClusterPlaneNoinline(benchmark::State &state) {
  Cluster c;
  uint8_t p;
  for (auto _ : state)
    ::benchmark::DoNotOptimize(p = c.plane());
}
BENCHMARK(ClusterPlaneNoinline);

static void ClusterPlaneInline(benchmark::State &state) {
  Cluster c;
  uint8_t p;
  for (auto _ : state)
    ::benchmark::DoNotOptimize(p = c.plane_header());
}
BENCHMARK(ClusterPlaneInline);

static void ClusterPlaneNoinlineFancyExtra(benchmark::State &state) {
  Cluster c;
  uint8_t p;
  BenchmarkLoop(state, [&] {
    ::benchmark::DoNotOptimize(p = c.plane());
  });
}
BENCHMARK(ClusterPlaneNoinlineFancyExtra);

static void ClusterPlaneInlineFancy(benchmark::State &state) {
  Cluster c;
  uint8_t p;
  BenchmarkLoop(state, [&]{
    ::benchmark::DoNotOptimize(p = c.plane_header());
  });
}
BENCHMARK(ClusterPlaneInlineFancy);

BENCHMARK_MAIN();
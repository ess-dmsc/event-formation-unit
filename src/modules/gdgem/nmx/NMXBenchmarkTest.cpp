// Copyright (C) 2020-2020 European Spallation Source, ERIC. See LICENSE file

#include <benchmark/benchmark.h>
#include <unistd.h>

#include <common/PoolAllocator.h>
#include <common/reduction/analysis/EventAnalyzer.h>
#include <common/reduction/clustering/GapClusterer.h>
#include <common/reduction/matching/CenterMatcher.h>

#include <gdgem/generators/BuilderHits.h>
#include <gdgem/tests/HitGenerator.h>

#include <fmt/format.h>

#include <common/BenchmarkUtil.h>
#include <common/debug/Trace.h>

#include <memory>

#include <thread>

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

  void SetUp(__attribute__((unused)) const ::benchmark::State &state) override {

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
      clusterer_x_ =
          std::make_shared<GapClusterer>(max_time_gap, max_coord_gap);
      clusterer_y_ =
          std::make_shared<GapClusterer>(max_time_gap, max_coord_gap);
    }
  }

  void TearDown(__attribute__((unused))
                const ::benchmark::State &state) override {}
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

  BenchmarkLoop(state, [&] {
    int numEvents = state.range(0) / (4 * 2);

    // generate Hits for builder
    BenchmarkLoopPaused(state, [&] {
      SetUp(state);

      uint64_t time = 0;
      uint16_t numHits = 4; // state.range(0);
      uint64_t timeGap = 40;
      uint32_t interHitTime = 1;
      int HitGap0{0};
      int DeadTime0Ns{0};
      bool NoShuffle{false};
      HitGenerator HitGen;

      // accumulate several hits from several events into a pseudo packet
      HitGen.setTimeParms(time, timeGap, interHitTime);
      HitGen.randomEvents(numEvents, 0, 1279);
      auto &Hits = HitGen.randomHits(numHits, HitGap0, DeadTime0Ns, NoShuffle);

      // store hits in builder
      builder_->process_buffer(reinterpret_cast<char *>(&Hits[0]),
                               sizeof(Hit) * Hits.size());

      std::shared_ptr<HitBuilder_t> hitBuilderConcrete =
          std::dynamic_pointer_cast<HitBuilder_t>(builder_);

      assert(hitBuilderConcrete->converted_data.size() == Hits.size());

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
// BENCHMARK_REGISTER_F(NmxBenchmarkTest, Dummy)->RangeMultiplier(2)->Range(8,
// 8)->Complexity(); BENCHMARK_REGISTER_F(NmxBenchmarkTest,
// Dummy)->RangeMultiplier(2)->Range(8, 8000)->Complexity();
BENCHMARK_REGISTER_F(NmxBenchmarkTest, Dummy)
    ->RangeMultiplier(2)
    ->Range(692, 692)
    ->Complexity();
// BENCHMARK(NmxBenchmarkTest);

static void ClusterPlaneNoinline(benchmark::State &state) {
  Cluster &c = *new Cluster();
  uint8_t p;
  for (auto _ : state)
    ::benchmark::DoNotOptimize(p = c.plane());
  delete &c;
}
BENCHMARK(ClusterPlaneNoinline);

static void ClusterPlaneInline(benchmark::State &state) {
  Cluster &c = *new Cluster();
  uint8_t p;
  for (auto _ : state)
    ::benchmark::DoNotOptimize(p = c.plane_header());
  delete &c;
}
BENCHMARK(ClusterPlaneInline);

static void ClusterPlaneNoinlineFancyExtra(benchmark::State &state) {
  Cluster &c = *new Cluster();
  uint8_t p;
  BenchmarkLoop(state, [&] { ::benchmark::DoNotOptimize(p = c.plane()); });
  delete &c;
}
BENCHMARK(ClusterPlaneNoinlineFancyExtra);

static void ClusterPlaneInlineFancy(benchmark::State &state) {
  Cluster &c = *new Cluster();
  uint8_t p;
  BenchmarkLoop(state,
                [&] { ::benchmark::DoNotOptimize(p = c.plane_header()); });
  delete &c;
}
BENCHMARK(ClusterPlaneInlineFancy);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static void Vec_PushBackTest_StdVector(benchmark::State &state) {
  BenchmarkLoop(state, [&] {
    std::vector<Hit> v;
    for (int i = 0; i < 16; ++i) {
      v.push_back(Hit{});
    }
    ::benchmark::DoNotOptimize(v.data());
    ::benchmark::ClobberMemory();
  });
}
BENCHMARK(Vec_PushBackTest_StdVector);

static void Vec_PushBackTest_StdVector_NoDealloc(benchmark::State &state) {
  std::vector<Hit> v;
  v.reserve(16);
  BenchmarkLoop(state, [&] {
    v.clear();
    for (int i = 0; i < 16; ++i) {
      v.push_back(Hit{});
    }
    ::benchmark::DoNotOptimize(v.data());
    ::benchmark::ClobberMemory();
  });
}
BENCHMARK(Vec_PushBackTest_StdVector_NoDealloc);

static void Vec_PushBackTest_MyVector(benchmark::State &state) {
  BenchmarkLoop(state, [&] {
    MyVector<Hit> v;
    for (int i = 0; i < 16; ++i) {
      v.push_back(Hit{});
    }
    ::benchmark::DoNotOptimize(v.data());
    ::benchmark::ClobberMemory();
  });
}
BENCHMARK(Vec_PushBackTest_MyVector);

static void Vec_PushBackTest_MyVector_NoDealloc(benchmark::State &state) {
  MyVector<Hit> v;
  BenchmarkLoop(state, [&] {
    v.clear();
    for (int i = 0; i < 16; ++i) {
      v.push_back(Hit{});
    }
    ::benchmark::DoNotOptimize(v.data());
    ::benchmark::ClobberMemory();
  });
}
BENCHMARK(Vec_PushBackTest_MyVector_NoDealloc);

static void Vec_PushBackTest_MyVector_Reserve(benchmark::State &state) {
  BenchmarkLoop(state, [&] {
    MyVector<Hit> v;
    v.reserve(16);
    for (int i = 0; i < 16; ++i) {
      v.push_back(Hit{});
    }
    ::benchmark::DoNotOptimize(v.data());
    ::benchmark::ClobberMemory();
  });
}
BENCHMARK(Vec_PushBackTest_MyVector_Reserve);

static void Vec_PushBackTest_MyVector_PoolAllocator(benchmark::State &state) {
  BenchmarkLoop(state, [&] {
    using FixedPoolCfg = PoolAllocatorConfig<Hit, sizeof(Hit) * 16, 16>;
    FixedPoolCfg::PoolType pool;
    PoolAllocator<FixedPoolCfg> alloc(pool);

    MyVector<Hit, decltype(alloc)> v(alloc);
    for (int i = 0; i < 16; ++i) {
      v.push_back(Hit{});
    }
    ::benchmark::DoNotOptimize(v.data());
    ::benchmark::ClobberMemory();
  });
}
BENCHMARK(Vec_PushBackTest_MyVector_PoolAllocator);

static void
Vec_PushBackTest_MyVector_PoolAllocator_NoDealloc(benchmark::State &state) {
  using FixedPoolCfg = PoolAllocatorConfig<Hit, sizeof(Hit) * 16, 16>;
  FixedPoolCfg::PoolType pool;
  PoolAllocator<FixedPoolCfg> alloc(pool);

  MyVector<Hit, decltype(alloc)> v(alloc);
  BenchmarkLoop(state, [&] {
    v.clear();
    for (int i = 0; i < 16; ++i) {
      v.push_back(Hit{});
    }
    ::benchmark::DoNotOptimize(v.data());
    ::benchmark::ClobberMemory();
  });
}
BENCHMARK(Vec_PushBackTest_MyVector_PoolAllocator_NoDealloc);

static void Vec_PushBackTest_RawArray(benchmark::State &state) {
  Hit v[16];
  size_t count = 0;
  BenchmarkLoop(state, [&] {
    // run dtor, if any
    for (size_t i = 0; i < count; ++i) {
      v[i].~Hit();
    }

    count = 0;
    for (int i = 0; i < 16; ++i) {
      ::benchmark::DoNotOptimize(v[count++] = Hit{});
    }
    ::benchmark::DoNotOptimize(v);
    ::benchmark::ClobberMemory();
  });
}
BENCHMARK(Vec_PushBackTest_RawArray);

static void Vec_PushBackTest_MyVector_LinearAllocator(benchmark::State &state) {
  BenchmarkLoop(state, [&] {
    MyVector<Hit, GreedyHitAllocator<Hit>> v;
    v.reserve(16);
    for (int i = 0; i < 16; ++i) {
      v.push_back(Hit{});
    }
    ::benchmark::DoNotOptimize(v.data());
    ::benchmark::ClobberMemory();
  });
}
BENCHMARK(Vec_PushBackTest_MyVector_LinearAllocator);

static void
Vec_PushBackTest_MyVector_LinearAllocator_NoDealloc(benchmark::State &state) {
  MyVector<Hit, GreedyHitAllocator<Hit>> v;
  v.reserve(16);
  BenchmarkLoop(state, [&] {
    v.clear();
    for (int i = 0; i < 16; ++i) {
      v.push_back(Hit{});
    }
    ::benchmark::DoNotOptimize(v.data());
    ::benchmark::ClobberMemory();
  });
}
BENCHMARK(Vec_PushBackTest_MyVector_LinearAllocator_NoDealloc);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template <typename Lambda>
void BenchmarkLoop_Copy(benchmark::State &state, Lambda loopBody) {
  BenchmarkOuterStartTiming();
  for (auto _ : state) {
    loopBody();
  }
  BenchmarkOuterStopTiming();
}

template <typename Lambda>
void BenchmarkLoop_Ref(benchmark::State &state, const Lambda &loopBody) {
  BenchmarkOuterStartTiming();
  for (auto _ : state) {
    loopBody();
  }
  BenchmarkOuterStopTiming();
}

void BenchmarkLoop_StdFun(benchmark::State &state,
                          std::function<void()> loopBody) {
  BenchmarkOuterStartTiming();
  for (auto _ : state) {
    loopBody();
  }
  BenchmarkOuterStopTiming();
}

inline __attribute__((always_inline)) void
BenchmarkLoop_StdFunInline(benchmark::State &state,
                           std::function<void()> loopBody) {
  BenchmarkOuterStartTiming();
  for (auto _ : state) {
    loopBody();
  }
  BenchmarkOuterStopTiming();
}

size_t _x, _y, _z, _w, _b, _c, _e, _t, _u, _i, _o, _p, _v, _n, _m;

//////////////////////////////////////////////////////////////////////////
// NoOp Small

static void CallTest_Small_Copy(benchmark::State &state) {
  size_t x = _x, y = _y;
  BenchmarkLoop_Copy(state, [&] {
    x += y;
    ::benchmark::DoNotOptimize(x);
    ::benchmark::ClobberMemory();
  });
  ::benchmark::DoNotOptimize(x);
}
BENCHMARK(CallTest_Small_Copy);

static void CallTest_Small_Ref(benchmark::State &state) {
  size_t x = _x, y = _y;
  BenchmarkLoop_Ref(state, [&] {
    x += y;
    ::benchmark::DoNotOptimize(x);
    ::benchmark::ClobberMemory();
  });
  ::benchmark::DoNotOptimize(x);
}
BENCHMARK(CallTest_Small_Ref);

static void CallTest_Small_RefCap(benchmark::State &state) {
  size_t x = _x, y = _y;
  BenchmarkLoop_Ref(state, [&x, y] {
    x += y;
    ::benchmark::DoNotOptimize(x);
    ::benchmark::ClobberMemory();
  });
  ::benchmark::DoNotOptimize(x);
}
BENCHMARK(CallTest_Small_RefCap);

static void CallTest_Small_StdFun(benchmark::State &state) {
  size_t x = _x, y = _y;
  auto fun = [&x, &y] {
    x += y;
    ::benchmark::DoNotOptimize(x);
    ::benchmark::ClobberMemory();
  };
  BenchmarkLoop_StdFun(state, std::move(fun));
  ::benchmark::DoNotOptimize(x);
}
BENCHMARK(CallTest_Small_StdFun);

static void CallTest_Small_StdFunInline(benchmark::State &state) {
  size_t x = _x, y = _y;
  auto fun = [&x, &y] {
    x += y;
    ::benchmark::DoNotOptimize(x);
    ::benchmark::ClobberMemory();
  };
  BenchmarkLoop_StdFunInline(state, std::move(fun));
  ::benchmark::DoNotOptimize(x);
}
BENCHMARK(CallTest_Small_StdFunInline);

static void CallTest_Small_StdFunRef(benchmark::State &state) {
  size_t x = _x, y = _y;
  auto fun = [&] {
    x += y;
    ::benchmark::DoNotOptimize(x);
    ::benchmark::ClobberMemory();
  };
  BenchmarkLoop_StdFun(state, std::move(fun));
  ::benchmark::DoNotOptimize(x);
}
BENCHMARK(CallTest_Small_StdFunRef);

static void CallTest_Small_StdFunRefGlob(benchmark::State &state) {
  auto fun = [&] {
    _x += _y;
    ::benchmark::DoNotOptimize(_x);
    ::benchmark::ClobberMemory();
  };
  BenchmarkLoop_StdFun(state, std::move(fun));
  ::benchmark::DoNotOptimize(_x);
}
BENCHMARK(CallTest_Small_StdFunRefGlob);

//////////////////////////////////////////////////////////////////////////
// NoOp Big

static void CallTest_Big_Copy(benchmark::State &state) {
  BenchmarkLoop_Copy(state, [&] {
    _z += _w + _b + _c + _e + _t + _u + _i + _o + _p + _c + _v + _b + _n + _m;
    ::benchmark::DoNotOptimize(_z);
    ::benchmark::ClobberMemory();
  });
  ::benchmark::DoNotOptimize(_z);
}
BENCHMARK(CallTest_Big_Copy);

static void CallTest_Big_Ref(benchmark::State &state) {
  BenchmarkLoop_Ref(state, [&] {
    _z += _w + _b + _c + _e + _t + _u + _i + _o + _p + _c + _v + _b + _n + _m;
    ::benchmark::DoNotOptimize(_z);
    ::benchmark::ClobberMemory();
  });
  ::benchmark::DoNotOptimize(_z);
}
BENCHMARK(CallTest_Big_Ref);

static void CallTest_Big_RefCap(benchmark::State &state) {
  size_t z = _z, w = _w, b = _b, c = _c, e = _e, t = _t, u = _u, i = _i, o = _o,
         p = _p, v = _v, n = _n, m = _m;

  BenchmarkLoop_Ref(state, [&z, w, b, c, e, t, u, i, o, p, v, n, m] {
    z += w + b + c + e + t + u + i + o + p + c + v + b + n + m;
    ::benchmark::DoNotOptimize(z);
    ::benchmark::ClobberMemory();
  });
  ::benchmark::DoNotOptimize(z);
}
BENCHMARK(CallTest_Big_RefCap);

static void CallTest_Big_StdFun(benchmark::State &state) {
  size_t z = _z, w = _w, b = _b, c = _c, e = _e, t = _t, u = _u, i = _i, o = _o,
         p = _p, v = _v, n = _n, m = _m;

  auto fun = [&z, w, b, c, e, t, u, i, o, p, v, n, m] {
    z += w + b + c + e + t + u + i + o + p + c + v + b + n + m;
    ::benchmark::DoNotOptimize(z);
    ::benchmark::ClobberMemory();
  };
  BenchmarkLoop_StdFun(state, std::move(fun));
  ::benchmark::DoNotOptimize(_z);
}
BENCHMARK(CallTest_Big_StdFun);

static void CallTest_Big_StdFunInline(benchmark::State &state) {
  size_t z = _z, w = _w, b = _b, c = _c, e = _e, t = _t, u = _u, i = _i, o = _o,
         p = _p, v = _v, n = _n, m = _m;

  auto fun = [&z, w, b, c, e, t, u, i, o, p, v, n, m] {
    z += w + b + c + e + t + u + i + o + p + c + v + b + n + m;
    ::benchmark::DoNotOptimize(z);
    ::benchmark::ClobberMemory();
  };
  BenchmarkLoop_StdFunInline(state, std::move(fun));
  ::benchmark::DoNotOptimize(_z);
}
BENCHMARK(CallTest_Big_StdFunInline);

static void CallTest_Big_StdFunRef(benchmark::State &state) {
  size_t z = _z, w = _w, b = _b, c = _c, e = _e, t = _t, u = _u, i = _i, o = _o,
         p = _p, v = _v, n = _n, m = _m;

  auto fun = [&] {
    z += w + b + c + e + t + u + i + o + p + c + v + b + n + m;
    ::benchmark::DoNotOptimize(z);
    ::benchmark::ClobberMemory();
  };
  BenchmarkLoop_StdFun(state, std::move(fun));
  ::benchmark::DoNotOptimize(_z);
}
BENCHMARK(CallTest_Big_StdFunRef);

static void CallTest_Big_StdFunRefGlob(benchmark::State &state) {
  auto fun = [&] {
    _z += _w + _b + _c + _e + _t + _u + _i + _o + _p + _c + _v + _b + _n + _m;
    ::benchmark::DoNotOptimize(_z);
    ::benchmark::ClobberMemory();
  };
  BenchmarkLoop_StdFun(state, std::move(fun));
  ::benchmark::DoNotOptimize(_z);
}
BENCHMARK(CallTest_Big_StdFunRefGlob);

//////////////////////////////////////////////////////////////////////////
// Op Small

static void CallTest_Op_Small_Copy(benchmark::State &state) {
  size_t x = _x, y = _y;
  BenchmarkLoop_Copy(state, [&] { x += y; });
  ::benchmark::DoNotOptimize(x);
  ::benchmark::ClobberMemory();
}
BENCHMARK(CallTest_Op_Small_Copy);

static void CallTest_Op_Small_Ref(benchmark::State &state) {
  size_t x = _x, y = _y;
  BenchmarkLoop_Ref(state, [&] { x += y; });
  ::benchmark::DoNotOptimize(x);
  ::benchmark::ClobberMemory();
}
BENCHMARK(CallTest_Op_Small_Ref);

static void CallTest_Op_Small_RefCap(benchmark::State &state) {
  size_t x = _x, y = _y;
  BenchmarkLoop_Ref(state, [&x, y] { x += y; });
  ::benchmark::DoNotOptimize(x);
  ::benchmark::ClobberMemory();
}
BENCHMARK(CallTest_Op_Small_RefCap);

static void CallTest_Op_Small_StdFun(benchmark::State &state) {
  size_t x = _x, y = _y;
  auto fun = [&x, &y] { x += y; };
  BenchmarkLoop_StdFun(state, std::move(fun));
  ::benchmark::DoNotOptimize(x);
  ::benchmark::ClobberMemory();
}
BENCHMARK(CallTest_Op_Small_StdFun);

static void CallTest_Op_Small_StdFunInline(benchmark::State &state) {
  size_t x = _x, y = _y;
  auto fun = [&x, &y] { x += y; };
  BenchmarkLoop_StdFunInline(state, std::move(fun));
  ::benchmark::DoNotOptimize(x);
  ::benchmark::ClobberMemory();
}
BENCHMARK(CallTest_Op_Small_StdFunInline);

static void CallTest_Op_Small_StdFunRef(benchmark::State &state) {
  size_t x = _x, y = _y;
  auto fun = [&] { x += y; };
  BenchmarkLoop_StdFun(state, std::move(fun));
  ::benchmark::DoNotOptimize(x);
  ::benchmark::ClobberMemory();
}
BENCHMARK(CallTest_Op_Small_StdFunRef);

static void CallTest_Op_Small_StdFunRefGlob(benchmark::State &state) {
  auto fun = [&] { _x += _y; };
  BenchmarkLoop_StdFun(state, std::move(fun));
  ::benchmark::DoNotOptimize(_x);
}
BENCHMARK(CallTest_Op_Small_StdFunRefGlob);

//////////////////////////////////////////////////////////////////////////
// Op Big

static void CallTest_Op_Big_Copy(benchmark::State &state) {
  BenchmarkLoop_Copy(state, [&] {
    _z += _w + _b + _c + _e + _t + _u + _i + _o + _p + _c + _v + _b + _n + _m;
  });
  ::benchmark::DoNotOptimize(_z);
  ::benchmark::ClobberMemory();
}
BENCHMARK(CallTest_Op_Big_Copy);

static void CallTest_Op_Big_Ref(benchmark::State &state) {
  BenchmarkLoop_Ref(state, [&] {
    _z += _w + _b + _c + _e + _t + _u + _i + _o + _p + _c + _v + _b + _n + _m;
  });
  ::benchmark::DoNotOptimize(_z);
  ::benchmark::ClobberMemory();
}
BENCHMARK(CallTest_Op_Big_Ref);

static void CallTest_Op_Big_RefCap(benchmark::State &state) {
  size_t z = _z, w = _w, b = _b, c = _c, e = _e, t = _t, u = _u, i = _i, o = _o,
         p = _p, v = _v, n = _n, m = _m;

  BenchmarkLoop_Ref(state, [&z, w, b, c, e, t, u, i, o, p, v, n, m] {
    z += w + b + c + e + t + u + i + o + p + c + v + b + n + m;
  });
  ::benchmark::DoNotOptimize(z);
  ::benchmark::ClobberMemory();
}
BENCHMARK(CallTest_Op_Big_RefCap);

static void CallTest_Op_Big_StdFun(benchmark::State &state) {
  size_t z = _z, w = _w, b = _b, c = _c, e = _e, t = _t, u = _u, i = _i, o = _o,
         p = _p, v = _v, n = _n, m = _m;

  auto fun = [&z, w, b, c, e, t, u, i, o, p, v, n, m] {
    z += w + b + c + e + t + u + i + o + p + c + v + b + n + m;
  };
  BenchmarkLoop_StdFun(state, std::move(fun));
  ::benchmark::DoNotOptimize(z);
  ::benchmark::ClobberMemory();
}
BENCHMARK(CallTest_Op_Big_StdFun);

static void CallTest_Op_Big_StdFunInline(benchmark::State &state) {
  size_t z = _z, w = _w, b = _b, c = _c, e = _e, t = _t, u = _u, i = _i, o = _o,
         p = _p, v = _v, n = _n, m = _m;

  auto fun = [&z, w, b, c, e, t, u, i, o, p, v, n, m] {
    z += w + b + c + e + t + u + i + o + p + c + v + b + n + m;
  };
  BenchmarkLoop_StdFunInline(state, std::move(fun));
  ::benchmark::DoNotOptimize(z);
  ::benchmark::ClobberMemory();
}
BENCHMARK(CallTest_Op_Big_StdFunInline);

static void CallTest_Op_Big_StdFunRef(benchmark::State &state) {
  size_t z = _z, w = _w, b = _b, c = _c, e = _e, t = _t, u = _u, i = _i, o = _o,
         p = _p, v = _v, n = _n, m = _m;

  auto fun = [&] {
    z += w + b + c + e + t + u + i + o + p + c + v + b + n + m;
  };
  BenchmarkLoop_StdFun(state, std::move(fun));
  ::benchmark::DoNotOptimize(z);
  ::benchmark::ClobberMemory();
}
BENCHMARK(CallTest_Op_Big_StdFunRef);

static void CallTest_Op_Big_StdFunRefGlob(benchmark::State &state) {
  auto fun = [&] {
    _z += _w + _b + _c + _e + _t + _u + _i + _o + _p + _c + _v + _b + _n + _m;
  };
  BenchmarkLoop_StdFun(state, std::move(fun));
  ::benchmark::DoNotOptimize(_z);
  ::benchmark::ClobberMemory();
}
BENCHMARK(CallTest_Op_Big_StdFunRefGlob);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void someFunc(){}

static void SharedPtr_SharedPtrVec(benchmark::State &state) {
  //std::thread someThread (someFunc);
  auto ptrVec = std::make_shared<std::vector<int>>();
  size_t x = 0;
  BenchmarkLoop(state, [&] {
    x += ptrVec->size();
    ::benchmark::DoNotOptimize(x);
    ::benchmark::ClobberMemory();
  });
  //someThread.join();
  ::benchmark::DoNotOptimize(x);
  ::benchmark::ClobberMemory();
}
BENCHMARK(SharedPtr_SharedPtrVec);

static void SharedPtr_RawVec(benchmark::State &state) {
  auto ptrVec = new std::vector<int>();
  size_t x = 0;
  BenchmarkLoop(state, [&] {
    x += ptrVec->size();
    ::benchmark::DoNotOptimize(x);
    ::benchmark::ClobberMemory();
  });
  ::benchmark::DoNotOptimize(x);
  ::benchmark::ClobberMemory();
  delete ptrVec;
}
BENCHMARK(SharedPtr_RawVec);

BENCHMARK_MAIN();

// Copyright (C) 2016 - 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief unit tests for ModulePipeline class
/// \todo this is currently poorly understood and probably needs better tests
//===----------------------------------------------------------------------===//

#include <multigrid/reduction/ModulePipeline.h>
#include <multigrid/geometry/PlaneMappings.h>
#include <common/testutils/TestBase.h>

using namespace Multigrid;

class ModulePipelineTest : public TestBase {
protected:
  ModulePipeline pipeline;

  void SetUp() override {}
  void TearDown() override {}
};


TEST_F(ModulePipelineTest, Constructor) {
  ASSERT_EQ(pipeline.out_queue.size(), 0);
  ASSERT_EQ(pipeline.stats.time_seq_errors, 0);
  ASSERT_EQ(pipeline.stats.invalid_planes, 0); ///< \todo can never happen?
  ASSERT_EQ(pipeline.grid_clusterer.stats_cluster_count, 0);
  ASSERT_EQ(pipeline.wire_clusterer.stats_cluster_count, 0);
}


TEST_F(ModulePipelineTest, Ingest) {
  pipeline.ingest({0, 0, 100, wire_plane}); // time, coord, weight, plane
  pipeline.ingest({0, 0, 100, grid_plane});
  pipeline.ingest({2000, 1, 100, wire_plane});
  pipeline.ingest({2000, 1, 100, grid_plane});
  pipeline.process_events(false);
  ASSERT_EQ(pipeline.stats.wire_clusters, 1);
  ASSERT_EQ(pipeline.stats.grid_clusters, 1);

  ASSERT_EQ(pipeline.stats.events_total, 0);

  ASSERT_EQ(pipeline.stats.time_seq_errors, 0);
  ASSERT_EQ(pipeline.stats.invalid_planes, 0);
 }


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

/** Copyright (C) 2018 European Spallation Source ERIC */

#include <logical_geometry/ESSGeometry.h>
#include <gdgem/srs/SRSMappings.h>
#include <gdgem/srs/SRSTime.h>

#include <gdgem/clustering/Clusterer1.h>
#include <gdgem/clustering/ClusterMatcher.h>

#include <gdgem/vmm2/EventFormationTestData.h>
#include <gdgem/vmm2/BuilderVMM2.h>
#include <memory>
#include <test/TestBase.h>

class EventFormationTest : public TestBase {
protected:
  Event event;
  SRSTime time_intepreter;
  SRSMappings geometry_interpreter;

  std::shared_ptr<Clusterer1> cx;
  std::shared_ptr<Clusterer1> cy;

  virtual void SetUp() {
    time_intepreter.set_bc_clock(20.0);
    time_intepreter.set_tac_slope(60.0);
    time_intepreter.set_trigger_resolution(3.125);
    time_intepreter.set_target_resolution(0.5);
    // acquisition window parameter?

    geometry_interpreter.define_plane(0, {{1, 0}, {1, 1}, {1, 6}, {1, 7}});
    geometry_interpreter.define_plane(1, {{1, 10}, {1, 11}, {1, 14}, {1, 15}});

    cx = std::make_shared<Clusterer1>(200, 3, 3);
    cy = std::make_shared<Clusterer1>(200, 3, 3);
  }
  virtual void TearDown() {}
};

TEST_F(EventFormationTest, Initial) {
  ClusterMatcher matcher(10);

  auto builder = std::make_shared<BuilderVMM2>(time_intepreter, geometry_interpreter,
      cx, cy, 0, 200, 0, 200, "", 0, 0);

  uint64_t readouts = 0;
  uint64_t readouts_error_bytes = 0;
  uint64_t readouts_discarded = 0;
  uint64_t clusters_xy = 0;
  uint64_t clusters_x = 0;
  uint64_t clusters_y = 0;
  uint64_t clusters_discarded = 0;

  for (auto pkt : Run16_1_to_16) {
    auto stats = builder->process_buffer((char *)&pkt[0], pkt.size());
    readouts += stats.valid_eventlets;
    readouts_error_bytes += stats.error_bytes; // From srs data parser

    matcher.merge(builder->clusterer_x->clusters);
    matcher.merge(builder->clusterer_y->clusters);
    matcher.match_end(false);


    while (!matcher.matched_clusters.empty()) {
      //XTRACE(PROCESS, DEB, "event_ready()\n");
      event = matcher.matched_clusters.front();
      matcher.matched_clusters.pop_front();

      event.analyze(true /*analyze_weighted*/,
                    3 /*analyze_max_timebins */,
                    7 /*analyze_max_timedif*/);
      if (event.valid()) {
        printf("\nHave a cluster:\n");
        event.debug2();
        clusters_xy++;
      } else {
        printf("No valid cluster:\n");
        event.debug2();
        if (event.x.entries.size() != 0) {
          clusters_x++;
        } else {
          clusters_y++;
        }
        readouts_discarded += event.x.entries.size() + event.y.entries.size();
        clusters_discarded++;
      }
    }
  }

  // @todo assertions need to be validated by Doro
  ASSERT_EQ(readouts, 204); // Run16 packets 1 to 16
//  ASSERT_EQ(clusters_xy, 1);
  // ASSERT_EQ(clusters_x, 7);
  // ASSERT_EQ(clusters_y, 19);

}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

/** Copyright (C) 2018 European Spallation Source ERIC */


#include <gdgem/nmx/Clusterer.h>
#include <gdgem/nmx/Hists.h>
#include <gdgem/vmm2srs/EventFormationTestData.h>
#include <gdgem/vmm2srs/EventletBuilderSRS.h>
#include <gdgem/vmm2srs/SRSMappings.h>
#include <gdgem/vmm2srs/SRSTime.h>
//#include <logical_geometry/Geometry.h>
#include <memory>
#include <test/TestBase.h>

class EventFormationTest : public TestBase {
protected:
  EventNMX event;
  NMXHists hists;
  SRSTime time_intepreter;
  SRSMappings geometry_interpreter;

  virtual void SetUp() {
    time_intepreter.set_bc_clock(20.0);
    time_intepreter.set_tac_slope(60.0);
    time_intepreter.set_trigger_resolution(3.125);
    time_intepreter.set_target_resolution(0.5);
    // acquisition window parameter?

    geometry_interpreter.define_plane(0, {{1, 0}, {1, 1}, {1, 6}, {1, 7}});
    geometry_interpreter.define_plane(1, {{1, 10}, {1, 11}, {1, 14}, {1, 15}});
  }
  virtual void TearDown() {}
};

TEST_F(EventFormationTest, Initial) {
  Clusterer clusterer(20000); // cluster_min_timespan

  auto builder = std::make_shared<BuilderSRS>(time_intepreter, geometry_interpreter, "", 0, 0);

  uint64_t readouts = 0;
  uint64_t readouts_error_bytes = 0;
  uint64_t readouts_discarded = 0;
  uint64_t clusters_xy = 0;
  uint64_t clusters_x = 0;
  uint64_t clusters_y = 0;
  uint64_t clusters_discarded = 0;

  for (auto pkt : Run16_1_to_16) {
    auto stats = builder->process_buffer((char *)&pkt[0], pkt.size(), clusterer, hists);
    readouts += stats.valid_eventlets;
    readouts_error_bytes += stats.error_bytes; // From srs data parser

    while (clusterer.event_ready()) {
      //XTRACE(PROCESS, DEB, "event_ready()\n");
      event = clusterer.get_event();
      hists.bin(event);
      event.analyze(true /*analyze_weighted*/, 3 /*analyze_max_timebins */, 7 /*analyze_max_timedif*/);
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

  // \todo assertions need to be validated by Doro
  EXPECT_EQ(readouts, 204); // Run16 packets 1 to 16
  EXPECT_EQ(clusters_xy, 1);
  // EXPECT_EQ(clusters_x, 7);
  // EXPECT_EQ(clusters_y, 19);

}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


//             if ( (!nmx_opts.enforce_lower_uncertainty_limit ||
//                    event.meets_lower_cirterion(nmx_opts.lower_uncertainty_limit)) &&
//                  (!nmx_opts.enforce_minimum_eventlets ||
//                  (event.x.entries.size() >= nmx_opts.minimum_eventlets &&
//                   event.y.entries.size() >= nmx_opts.minimum_eventlets))) {
//
//               // printf("\nHave a cluster:\n");
//               // event.debug2();
//
//               coords[0] = event.x.center_rounded();
//               coords[1] = event.y.center_rounded();
//               pixelid = geometry.to_pixid(coords);
//               if (pixelid == 0) {
//                 mystats.geom_errors++;
//               } else {
//                 time = static_cast<uint32_t>(event.time_start());
//
//                 XTRACE(PROCESS, DEB, "time: %d, pixelid %d\n", time, pixelid);
//
//                 mystats.tx_bytes += flatbuffer.addevent(time, pixelid);
//                 mystats.clusters_events++;
//               }
//             } else { // Does not meet criteria
//               /** \todo increments counters when failing this */
//               // printf("\nInvalid cluster:\n");
//               // event.debug2();
//             }
//           }
//         }
//       }
//     }
//   }
// }

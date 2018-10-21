/** Copyright (C) 2017-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
///  Unit tests for Multiblade::EventBuilder using Google Test.
/// Here the various counters are tested.
///
/// Author: Carsten Søgaard, Niels Bohr Institute, University of Copenhagen
///
//===----------------------------------------------------------------------===//

#pragma GCC diagnostic push
#ifdef SYSTEM_NAME_DARWIN
#pragma GCC diagnostic ignored "-Wkeyword-macro"
#pragma GCC diagnostic ignored "-Wmacro-redefined"
#endif
#define private public
#include "common/EventBuilder.h"
#ifdef private
#undef private
#define private private
#endif
#pragma GCC diagnostic pop

#include "TestData.h"
#include "test/TestBase.h"

class MBEventBuilderTest : public TestBase {
protected:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

/// \todo add further checks
TEST_F(MBEventBuilderTest, Constructor) {
  Multiblade::EventBuilder evbuilder;
  ASSERT_EQ(evbuilder.getNumberOfEvents(), 0);
  auto coords = evbuilder.getPosition();
  ASSERT_TRUE(coords[0] == 0.00);
  ASSERT_TRUE(coords[1] == 0.00);
  ASSERT_TRUE(coords[2] == 0.00);
  ASSERT_EQ(evbuilder.getWirePosition(), 0.0);
  ASSERT_EQ(evbuilder.getStripPosition(), 0.0);
}

TEST_F(MBEventBuilderTest, EventBuilderDump) {
  Multiblade::EventBuilder evbuilder;
  MESSAGE() << "This is NOT a test, just calling debug print functions\n";
  MESSAGE() << evbuilder.print() << "\n";
}

TEST_F(MBEventBuilderTest, CheckAdjacencyEmpty) {
  Multiblade::EventBuilder evbuilder;
  std::vector<Multiblade::point> nopoints;
  ASSERT_FALSE(evbuilder.checkAdjacency(nopoints));
  std::vector<Multiblade::point> onepoint = {{31, 1000}}; // ch, adc
  ASSERT_TRUE(evbuilder.checkAdjacency(onepoint));
}

TEST_F(MBEventBuilderTest, CalculatePosition) {
  Multiblade::EventBuilder evbuilder;
  std::vector<Multiblade::point> threepoints = {{10, 1000}, {20, 1000}, {30, 1000}}; // ch, adc
  ASSERT_EQ(evbuilder.calculatePosition(threepoints), 20.0);

  std::vector<Multiblade::point> adcon10only = {{10, 1000}, {20, 0}, {30, 0}}; // ch, adc
  ASSERT_EQ(evbuilder.calculatePosition(adcon10only), 10.0);

  evbuilder.setUseWeightedAverage(false);
  std::vector<Multiblade::point> adcmax20 = {{10, 998}, {20, 1000}, {30, 999}}; // ch, adc
  ASSERT_EQ(evbuilder.calculatePosition(adcmax20), 20.0);
}

TEST_F(MBEventBuilderTest, ProcessClusters) {
  Multiblade::EventBuilder evbuilder;
  ASSERT_EQ(evbuilder.m_rejected_position, 0);
  evbuilder.m_wire_cluster = {{0, 0}}; // ch's < 32
  evbuilder.m_strip_cluster = {{32, 0}}; // ch's >= 32
  ASSERT_FALSE(evbuilder.processClusters());
  ASSERT_EQ(evbuilder.m_rejected_position, 1);
}

TEST_F(MBEventBuilderTest, CheckIncrementOfCounters) {
  Multiblade::EventBuilder evbuilder;
  for (unsigned int i = 0; i < evbuilder.m_2D_wires.size(); i++) {
    ASSERT_EQ(evbuilder.m_2D_wires.at(i), 0);
    ASSERT_EQ(evbuilder.m_2D_strips.at(i), 0);
    ASSERT_EQ(evbuilder.m_1D_wires.at(i), 0);
    ASSERT_EQ(evbuilder.m_1D_strips.at(i), 0);
  }

  std::vector<Multiblade::point> empty;
  std::vector<Multiblade::point> twopoints = {{1, 1000}, {2, 2000}};
  std::vector<Multiblade::point> sixpoints = {{1, 1000}, {2, 2000}, {3, 3000}, {4, 4000}, {5, 5000}, {6, 6000}};

  evbuilder.incrementCounters(sixpoints, sixpoints);
  ASSERT_EQ(evbuilder.m_2D_wires.at(5), 1);
  ASSERT_EQ(evbuilder.m_2D_strips.at(5), 1);

  evbuilder.incrementCounters(twopoints, empty);
  ASSERT_EQ(evbuilder.m_1D_wires.at(1), 1);
  ASSERT_EQ(evbuilder.m_1D_strips.at(1), 0);

  evbuilder.incrementCounters(sixpoints, empty);
  ASSERT_EQ(evbuilder.m_1D_wires.at(5), 1);
  ASSERT_EQ(evbuilder.m_1D_strips.at(5), 0);

  evbuilder.incrementCounters(empty, twopoints);
  ASSERT_EQ(evbuilder.m_1D_wires.at(1), 1);
  ASSERT_EQ(evbuilder.m_1D_strips.at(1), 1);

  evbuilder.incrementCounters(empty, sixpoints);
  ASSERT_EQ(evbuilder.m_1D_wires.at(5), 1);
  ASSERT_EQ(evbuilder.m_1D_strips.at(5), 1);

}



/// not easy to test
TEST_F(MBEventBuilderTest, AddDataPoint) {
  Multiblade::EventBuilder evbuilder;
  bool retval = evbuilder.addDataPoint(0, 1000, 0); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(32, 1000, 10); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(32, 1000, 10000); // ch, adc, time, outside window
  ASSERT_EQ(retval, true);
  ASSERT_EQ(evbuilder.getNumberOfEvents(), 1);
}

/// not easy to test
TEST_F(MBEventBuilderTest, AddDataPointBelowADCThreshold) {
  Multiblade::EventBuilder evbuilder;
  evbuilder.setThreshold(1000);
  bool retval = evbuilder.addDataPoint(0, 999, 0); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(32, 999, 10); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(32, 999, 10000); // ch, adc, time, outside window
  ASSERT_EQ(retval, false);
  ASSERT_EQ(evbuilder.getNumberOfEvents(), 0);
}

TEST_F(MBEventBuilderTest, AddDataPointAboveADCThreshold) {
  Multiblade::EventBuilder evbuilder;
  evbuilder.setThreshold(1000);
  bool retval = evbuilder.addDataPoint(0, 1000, 0); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(32, 1000, 10); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(32, 1000, 10000); // ch, adc, time, outside window
  ASSERT_EQ(retval, true);
  ASSERT_EQ(evbuilder.getNumberOfEvents(), 1);
}

/// not easy to test
TEST_F(MBEventBuilderTest, AddDataPointInvalidChannels) {
  Multiblade::EventBuilder evbuilder;
  bool retval = evbuilder.addDataPoint(64, 1000, 0); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(64, 1000, 10); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(64, 1000, 10000); // ch, adc, time, outside window
  ASSERT_EQ(retval, false);
  ASSERT_EQ(evbuilder.getNumberOfEvents(), 0);
}

/// not easy to test
TEST_F(MBEventBuilderTest, AddDataPointValidChannels) {
  Multiblade::EventBuilder evbuilder;
  bool retval = evbuilder.addDataPoint(31, 1000, 0); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(63, 1000, 10); // ch, adc, time
  ASSERT_EQ(retval, false);
  retval = evbuilder.addDataPoint(63, 1000, 10000); // ch, adc, time, outside window
  ASSERT_EQ(retval, true);
  ASSERT_EQ(evbuilder.getNumberOfEvents(), 1);
}

TEST_F(MBEventBuilderTest, EventCounter) {

  // Test that events are counted correctly

  // Instanciate the event-builder
  Multiblade::EventBuilder p;

  // Initialize the expected number of events counter
  uint nevents = 1;

  for (uint i = 0; i <= 15; i++) {

    // Read a subset of the test data
    uint begin = i * 5;
    uint end = begin + 5;
    std::vector<uint> datapoint(&data[begin], &data[end]);
    // Wire data-points
    EXPECT_EQ(datapoint[4], p.addDataPoint(datapoint[0], datapoint[2], datapoint[3]));
    // Strip data-points
    EXPECT_FALSE(p.addDataPoint(datapoint[1], datapoint[2], datapoint[3]));

    // Validation of event counter
    if (datapoint[4]) {
      EXPECT_EQ(nevents, p.getNumberOfEvents());
      // Increment validation iterator
      nevents++;
    }
  }

  // Provide a non-adjacent event, which is discarded
  for (uint i = 10; i < 15; i++) {

    // Remove a point (making it non adjacent)
    if (i != 12) {
      // Read a subset of the test data
      uint begin = i * 5;
      uint end = begin + 5;
      std::vector<uint> datapoint(&data[begin], &data[end]);
      // Wire data-points
      EXPECT_EQ(datapoint[4], p.addDataPoint(datapoint[0], datapoint[2], datapoint[3]));
      // Strip data-points
      EXPECT_FALSE(p.addDataPoint(datapoint[1], datapoint[2], datapoint[3]));
    }
  }

  // Validation of event counter at "end of run".
  // The counter was not incremented before, due to the non-adjacent point.
  p.lastPoint();
  EXPECT_EQ(nevents, p.getNumberOfEvents());
}

#if 0
TEST(MBEventBuilder__Test, ClusterCounters) {

  // Test the counters of number of points per event

  // Instaciate the event-counter and configure
  Multiblade::EventBuilder p;
  p.setTimeWindow(config[0]);
  p.setNumberOfWireChannels(config[1]);
  p.setNumberOfStripChannels(config[2]);

  // Validation counters (similar to those in the event-builder).
  // Index 0 = 1 datapoint, index 1 = 2 datapoints, etc.
  std::array<uint64_t, 6> wireclusterpoints = {{0, 0, 0, 0, 0, 0}};
  std::array<uint64_t, 6> stripclusterpoints = {{0, 0, 0, 0, 0, 0}};

  // Counters for number of points per event
  uint wirepoints = 0;
  uint strippoints = 0;

  // Iterator for validation data
  // std::vector<double>::iterator valw = validation_weighted.begin();

  // Test the case when there are both wire and strip points
  for (uint i = 0; i <= 21; i++) {
    uint begin = i * 5;
    uint end = begin + 5;
    std::vector<uint> datapoint(&data[begin], &data[end]);
    // Wire data-points
    bool retval = p.addDataPoint(datapoint[0], datapoint[2], datapoint[3]);
    ASSERT_EQ(retval, datapoint[4]);
    // Strip data-points
    retval = p.addDataPoint(datapoint[1], datapoint[2], datapoint[3]);
    ASSERT_EQ(retval, datapoint[4]);

    if (!datapoint[4]) {

      wirepoints++;
      strippoints++;

    } else {

      // Increment the correct positions in the counters
      wireclusterpoints[wirepoints - 1]++;
      stripclusterpoints[strippoints - 1]++;

      // Check all indexes of the counters for each event.
      EXPECT_EQ(wireclusterpoints[0], p.get2DWireClusterCounter()[0]);
      EXPECT_EQ(wireclusterpoints[1], p.get2DWireClusterCounter()[1]);
      EXPECT_EQ(wireclusterpoints[2], p.get2DWireClusterCounter()[2]);
      EXPECT_EQ(wireclusterpoints[3], p.get2DWireClusterCounter()[3]);
      EXPECT_EQ(wireclusterpoints[4], p.get2DWireClusterCounter()[4]);
      EXPECT_EQ(wireclusterpoints[5], p.get2DWireClusterCounter()[5]);

      EXPECT_EQ(stripclusterpoints[0], p.get2DStripClusterCounter()[0]);
      EXPECT_EQ(stripclusterpoints[1], p.get2DStripClusterCounter()[1]);
      EXPECT_EQ(stripclusterpoints[2], p.get2DStripClusterCounter()[2]);
      EXPECT_EQ(stripclusterpoints[3], p.get2DStripClusterCounter()[3]);
      EXPECT_EQ(stripclusterpoints[4], p.get2DStripClusterCounter()[4]);
      EXPECT_EQ(stripclusterpoints[5], p.get2DStripClusterCounter()[5]);

      // Reset the points counters (1, since a point is provided when finishing
      // the cluster
      wirepoints = 1;
      strippoints = 1;
    }
  }

  // End the "run" and reset all counters
  p.lastPoint();
  p.resetCounters();

  // Reset counters
  wireclusterpoints = {{0, 0, 0, 0, 0, 0}};
  stripclusterpoints = {{0, 0, 0, 0, 0, 0}};

  // Reset the counters to "start of run"
  wirepoints = 0;
  strippoints = 0;

  // Test the case of only wire points
  for (uint i = 0; i <= 21; i++) {
    uint begin = i * 5;
    uint end = begin + 5;
    std::vector<uint> datapoint(&data[begin], &data[end]);
    // Wire data-points
    p.addDataPoint(datapoint[0], datapoint[2], datapoint[3]);

    if (!datapoint[4]) {

      wirepoints++;

    } else {

      // Incement at the correct index.
      wireclusterpoints[wirepoints - 1]++;

      // Test counter at all indexes
      EXPECT_EQ(wireclusterpoints[0], p.get1DWireClusterCounter()[0]);
      EXPECT_EQ(wireclusterpoints[1], p.get1DWireClusterCounter()[1]);
      EXPECT_EQ(wireclusterpoints[2], p.get1DWireClusterCounter()[2]);
      EXPECT_EQ(wireclusterpoints[3], p.get1DWireClusterCounter()[3]);
      EXPECT_EQ(wireclusterpoints[4], p.get1DWireClusterCounter()[4]);
      EXPECT_EQ(wireclusterpoints[5], p.get1DWireClusterCounter()[5]);

      EXPECT_EQ(0, p.get1DStripClusterCounter()[0]);
      EXPECT_EQ(0, p.get1DStripClusterCounter()[1]);
      EXPECT_EQ(0, p.get1DStripClusterCounter()[2]);
      EXPECT_EQ(0, p.get1DStripClusterCounter()[3]);
      EXPECT_EQ(0, p.get1DStripClusterCounter()[4]);
      EXPECT_EQ(0, p.get1DStripClusterCounter()[5]);

      wirepoints = 1;
    }
  }

  // "End of run" and reset all counters
  p.lastPoint();
  p.resetCounters();

  // Reset validation counters
  wireclusterpoints = {{0, 0, 0, 0, 0, 0}};
  stripclusterpoints = {{0, 0, 0, 0, 0, 0}};

  // Reset event counters
  wirepoints = 0;
  strippoints = 0;

  // Test the case of only strip points
  for (uint i = 0; i <= 21; i++) {
    uint begin = i * 5;
    uint end = begin + 5;
    std::vector<uint> datapoint(&data[begin], &data[end]);
    // Strip data-points
    p.addDataPoint(datapoint[1], datapoint[2], datapoint[3]);

    if (!datapoint[4]) {

      strippoints++;

      EXPECT_EQ(strippoints, p.getStripClusterSize());

    } else {

      // Incement at the correct index.
      stripclusterpoints[strippoints - 1]++;

      // Test counter at all indexes
      EXPECT_EQ(0, p.get1DWireClusterCounter()[0]);
      EXPECT_EQ(0, p.get1DWireClusterCounter()[1]);
      EXPECT_EQ(0, p.get1DWireClusterCounter()[2]);
      EXPECT_EQ(0, p.get1DWireClusterCounter()[3]);
      EXPECT_EQ(0, p.get1DWireClusterCounter()[4]);
      EXPECT_EQ(0, p.get1DWireClusterCounter()[5]);

      EXPECT_EQ(stripclusterpoints[0], p.get1DStripClusterCounter()[0]);
      EXPECT_EQ(stripclusterpoints[1], p.get1DStripClusterCounter()[1]);
      EXPECT_EQ(stripclusterpoints[2], p.get1DStripClusterCounter()[2]);
      EXPECT_EQ(stripclusterpoints[3], p.get1DStripClusterCounter()[3]);
      EXPECT_EQ(stripclusterpoints[4], p.get1DStripClusterCounter()[4]);
      EXPECT_EQ(stripclusterpoints[5], p.get1DStripClusterCounter()[5]);

      strippoints = 1;
    }
  }
}


TEST(MBEventBuilder__Test, NoDataRecieved) {

  Multiblade::EventBuilder p;

  EXPECT_EQ(0, p.getNumberOfPositionRejected());

  p.lastPoint();

  EXPECT_DOUBLE_EQ(-1, p.getWirePosition());
  EXPECT_DOUBLE_EQ(-1, p.getStripPosition());

  EXPECT_EQ(1, p.getNumberOfPositionRejected());
}
#endif



int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

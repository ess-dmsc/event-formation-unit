/** Copyright (C) 2017-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
///  Unit tests for MultiBladeEventBuilder using Google Test.
/// Here the clustering algoritm is tested
///
/// Author: Carsten Søgaard, Niels Bohr Institute, University of Copenhagen
///
//===----------------------------------------------------------------------===//

/// \todo - no longer valid - new data format

#include "mbcommon/MultiBladeEventBuilder.h"
#include "MultiBladeTestData.h"
#include "test/TestBase.h"

TEST(MBEventBuilder__Test, Threshold) {

  MultiBladeEventBuilder p;
  p.setThreshold(10);
  p.setTimeWindow(1);

  // ADC below threshold
  EXPECT_FALSE(p.addDataPoint(0, 5, 100));
  EXPECT_EQ(0U, p.getClusterClock());

  EXPECT_FALSE(p.addDataPoint(0, 10, 100));
  EXPECT_EQ(100, p.getClusterClock());
}

TEST(MBEventBuilder__Test, TimeWindow) {

  // Perform 4 tests with different windows and different first clock number
  // while still ensuring the clock-cycle number does not overflow.
  // Overflow will be tested elsewhere.
  for (int itest = 0; itest < 4; itest++) {

    // Instanciate the event builder
    MultiBladeEventBuilder p;

    // Set the number of clock-cycles for the time-window
    uint32_t window = static_cast<uint32_t>(itest) * 2 + 2;
    p.setTimeWindow(window);

    // Set the clock-cycle number for the fist point. 64 bit for the for loop
    // (see below)
    uint64_t first_clock = static_cast<uint64_t>(itest) * 1000;

    // Expect false while clock-cycle number is within window
    for (uint64_t clock = first_clock; clock < first_clock + window; clock++)
      EXPECT_FALSE(p.addDataPoint(0, 300, clock));

    // Expect true when clock-cycle number is outside window
    EXPECT_TRUE(p.addDataPoint(0, 300, first_clock + window));
  }
}

TEST(MBEventBuilder__Test, TimeWindow_OverFlow) {

  // The clock-cycle is a 32-bit counter. At ESS it is planned to be reset
  // before the maximum is reached.
  // This test is to ensure proper handling if for some reason the clock will
  // not be reset.

  // Instanciate the event-builder
  MultiBladeEventBuilder p;

  // Set the number of clock-cycles for the time-window
  uint64_t window = 10;
  p.setTimeWindow(window);

  // Set the first clock-cycle number 5 below max of uint32_t
  // The variable is 64 bit for the for loop below.
  uint64_t start = std::numeric_limits<uint32_t>::max() - 5;

  // For loop with 64 bit counter to ensure proper running.
  // Expect false while clock-cycle number is within window - event though the
  // 32 bit clock counter has overflown.
  for (uint64_t clock = start; clock < start + window; clock++)
    EXPECT_FALSE(p.addDataPoint(0, 300, static_cast<uint32_t>(clock)));

  // Expect true when clock-cycle number is outside window
  EXPECT_TRUE(p.addDataPoint(0, 300, start + window));
}

TEST(MBEventBuilder__Test, ClusterTime) {

  // Test that the clock-cycle number of the cluster currently being processed
  // is set properly.
  // Also test that the time-stamp is calculated correctly.

  // Instanciate the event-builder
  MultiBladeEventBuilder p;

  // Set the number of clock-cycles for the time-window
  uint64_t window = 10;
  p.setTimeWindow(window);

  // Before any data-points are provided to the event-builder the clock-cycle
  // number must be 0.
  EXPECT_EQ(0, p.getClusterClock());

  // Since this is the first point the cluster clock should be set
  p.addDataPoint(0, 300, 1000);
  EXPECT_EQ(1000, p.getClusterClock());

  // Since this is within the time-window, the cluster clock should not change
  p.addDataPoint(1, 300, 1001);
  EXPECT_EQ(1000, p.getClusterClock());

  // This data point is outside the time-window, therefore the cluster currently
  // stored is processed and
  // the timestamp of the cluster calculated. The provided clock-cycle number
  // will now be stored for the
  // new cluster
  p.addDataPoint(2, 300, 1010);
  EXPECT_DOUBLE_EQ(1000, p.getTimeStamp());
  EXPECT_EQ(1010, p.getClusterClock());
}

TEST(MBEventBuilder__Test, Adjacency) {

  // This test ensures that the event-builder checks the data-points for
  // adjacency correctly.
  // Signals are expected to originate from adjacent wires or strips.
  // Non-adjacent points may stem from pile-up,
  // and are therfore discarded.

  // Single points first. This tests that the adjacency test does not fail when
  // the event consists of only
  // one data-point (wire or strip).
  // Cases when no wire or strips signals are present are implicitly tested here
  // as well.

  // i = 0 -> one wire point
  // i = 1 -> one strip point
  for (int i = 0; i < 2; i++) {

    // Instanciate the event-builder.
    MultiBladeEventBuilder p;
    // For simplicity, set the time window to one clock-cycle and the clock
    // duration to one second.
    p.setTimeWindow(1);
    // For simplicity, set the location algorithm to max ADC.
    p.setUseWeightedAverage(false);

    // First data-point must always return false
    EXPECT_FALSE(p.addDataPoint(0 + i * 32, 300, 1));
    // This point is outside the time-window so the cluster is complete.
    // The event-builder should then return true.
    EXPECT_TRUE(p.addDataPoint(1 + i * 32, 300, 1000));

    switch (i) {
    // Test if the location is correctly determined for one wire point
    case 0:
      EXPECT_DOUBLE_EQ(0 + i, p.getWirePosition());
      EXPECT_DOUBLE_EQ(-1., p.getStripPosition());
      break;

    // Test if the location is correctly determined for one wire point
    case 1:
      EXPECT_DOUBLE_EQ(-1., p.getWirePosition());
      EXPECT_DOUBLE_EQ(0 + i * 32, p.getStripPosition());
      break;
    }
  }

  // Test that the event-builder correctly identifies, adjacent data-points.
  // We only test wire points, since the same algorithm is used for both wire
  // and strip.

  // Adjacent points
  uint8_t adjacent[3][3] = {{3, 4, 5}, {10, 11, 12}, {29, 30, 31}};

  for (int i = 0; i < 3; i++) {

    // Instanciate the event-builder
    MultiBladeEventBuilder p;
    p.setTimeWindow(3);
    // For simplicity we use the maximum ADC location algorithm
    p.setUseWeightedAverage(false);

    for (int ii = 0; ii < 3; ii++)
      // Last point will have the maximum ADC-value
      EXPECT_FALSE(p.addDataPoint(adjacent[i][ii], 300 * ii, ii));

    // A point outsinde the timewindow to trigger processing of the cluster
    EXPECT_TRUE(p.addDataPoint(0, 300, 1000));
    // We test that the correct location is found
    EXPECT_DOUBLE_EQ(adjacent[i][2], p.getWirePosition());
    // Since no strip points were added, the location should be -1
    EXPECT_DOUBLE_EQ(-1., p.getStripPosition());
  }

  // Test that non-adjacent points are correctly identified.

  // Not-adjacent points
  uint8_t notadjacent[3][3] = {{3, 4, 6}, {9, 11, 12}, {27, 29, 31}};

  for (int i = 0; i < 3; i++) {

    MultiBladeEventBuilder p;
    p.setTimeWindow(10);

    // Check that the adjacency rejected counter is 0
    EXPECT_EQ(0, p.getNumberOfAdjacencyRejected());

    for (int ii = 0; ii < 3; ii++)
      EXPECT_FALSE(p.addDataPoint(notadjacent[i][ii], 300, ii));

    // A point outsinde the timewindow to trigger processing of the cluster
    EXPECT_FALSE(p.addDataPoint(0, 300, 1000));

    // Test that the counter for adjacency rejected points is incremented
    // properly.
    EXPECT_EQ(1, p.getNumberOfAdjacencyRejected());
  }

  // Test that the adjacency check works for cases with both wire and strip
  // points.

  for (uint i = 0; i < 3; i++) {

    MultiBladeEventBuilder p;
    p.setTimeWindow(3);

    // Check that the adjacency rejected counter is 0
    EXPECT_EQ(0, p.getNumberOfAdjacencyRejected());

    for (uint ii = 0; ii < 3; ii++) {

      // This will give two events where the wire points are non-adjacent and
      // the strip points adjacent
      // One event where the oppostite is the case.
      if (i / 2 == 0) {
        p.addDataPoint(notadjacent[i][ii], 300, ii);
        p.addDataPoint(adjacent[i][ii] + 32, 300, ii);
      } else {
        p.addDataPoint(adjacent[i][ii], 300, ii);
        p.addDataPoint(notadjacent[i][ii] + 32, 300, ii);
      }
    }

    // A point outsinde the timewindow to trigger processing of the cluster
    EXPECT_FALSE(p.addDataPoint(0, 300, 1000));

    // Test that the counter for adjacency rejected points is incremented
    // properly.
    EXPECT_EQ(1, p.getNumberOfAdjacencyRejected());
  }
}

TEST(MBEventBuilder__Test, WireStripPoints) {

  // Test whether data-points are registered properly as wire or strip points.

  // Array of number of wire and strip channels
  uint npoints[5] = {2, 4, 8, 16, 32};

  // Iterate over the 5 cases.
  for (uint i = 0; i < 5; i++) {

    MultiBladeEventBuilder p;
    p.setNumberOfWireChannels(npoints[i]);
    p.setNumberOfStripChannels(npoints[i]);
    p.setTimeWindow(1000);

    EXPECT_EQ(0U, p.getWireClusterSize());
    EXPECT_EQ(0U, p.getWireClusterSize());

    for (uint8_t ii = 0; ii < npoints[i]; ii++) {

      // Add a wire point
      p.addDataPoint(ii, 300, ii);
      // Test that the wire cluster is incremented
      EXPECT_EQ(ii + 1, p.getWireClusterSize());
      // Test that the strip cluster is not incremented
      EXPECT_EQ(ii, p.getStripClusterSize());

      // Add a strip point
      p.addDataPoint(ii + npoints[i], 300, ii + npoints[i]);
      // Test that the wire cluster is not incremented
      EXPECT_EQ(ii + 1, p.getWireClusterSize());
      // Test that the strip cluster is incremented
      EXPECT_EQ(ii + 1, p.getStripClusterSize());
    }

    // Test that the event-builder returns false
    // when the channel number exceeds the sum of wire and strip channels
    EXPECT_FALSE(p.addDataPoint(npoints[i] * 2 + 1, 300, 100));
    // Test that the clusters are not incremented
    EXPECT_EQ(npoints[i], p.getWireClusterSize());
    EXPECT_EQ(npoints[i], p.getStripClusterSize());
  }
}

TEST(MBEventBuilder__Test, Clustering) {

  // Test the full clustering algorithm with test data from multiBLadeTestData.h
  // We test the configuration functions also

  // Instanciate the event-builder.
  MultiBladeEventBuilder p;
  // Configure the event-builder using the config info in multiBladeTestData.h
  p.setTimeWindow(config[0]);
  p.setNumberOfWireChannels(config[1]);
  p.setNumberOfStripChannels(config[2]);

  // We initialize the first expected time-stamp value from the test data.
  uint32_t timestamp = data[3];

  // First we test the weighted average method

  // Set up an iterator for the validation data.
  std::vector<double>::iterator valw = validation_weighted.begin();

  for (uint i = 0; i <= 15; i++) {
    // Calculate the start and end points of one data-point
    uint begin = i * 5;
    uint end = begin + 5;
    // Create a sub-vector of the test data
    std::vector<uint> datapoint(&data[begin], &data[end]);
    // Wire data-points
    EXPECT_EQ(datapoint[4], p.addDataPoint(datapoint[0], datapoint[2], datapoint[3]));
    // Strip data-points
    EXPECT_FALSE(p.addDataPoint(datapoint[1], datapoint[2], datapoint[3]));

    // Validation of calculated wire/strip locations
    if (datapoint[4]) {
      EXPECT_DOUBLE_EQ(*valw, p.getWirePosition());
      EXPECT_DOUBLE_EQ(*valw + 32., p.getStripPosition());
      EXPECT_EQ(timestamp, p.getTimeStamp());

      // Read next cluster timestamp
      timestamp = datapoint[3];
      // Increment validation iterator
      valw++;
    }
  }

  // End the "run"
  p.lastPoint();
  // Check that the last event is processed correctly
  EXPECT_DOUBLE_EQ(*valw, p.getWirePosition());
  EXPECT_DOUBLE_EQ(*valw, p.getPosition()[0]);
  EXPECT_DOUBLE_EQ(*valw + 32, p.getStripPosition());
  EXPECT_DOUBLE_EQ(*valw + 32, p.getPosition()[1]);
  EXPECT_EQ(timestamp, p.getTimeStamp());
  EXPECT_EQ(timestamp, p.getPosition()[2]);

  // Maximim ADC-method

  // Switch to maximum ADC method
  p.setUseWeightedAverage(false);

  // Reset the expected time-stamp
  timestamp = data[3];

  // Set up an iterator for the validation data.
  std::vector<double>::iterator valm = validation_max.begin();

  for (uint i = 0; i <= 15; i++) {
    uint begin = i * 5;
    uint end = begin + 5;
    std::vector<uint> datapoint(&data[begin], &data[end]);
    // Wire data-points
    EXPECT_EQ(datapoint[4], p.addDataPoint(datapoint[0], datapoint[2], datapoint[3]));
    // Strip data-points
    EXPECT_FALSE(p.addDataPoint(datapoint[1], datapoint[2], datapoint[3]));

    // Validation of calculated wire/strip locations
    if (datapoint[4]) {
      EXPECT_DOUBLE_EQ(*valm, p.getWirePosition());
      EXPECT_DOUBLE_EQ((*valm) + 32., p.getStripPosition());
      EXPECT_EQ(timestamp, p.getTimeStamp());

      // Read next cluster timestamp
      timestamp = datapoint[3];
      // Increment validation iterator
      valm++;
    }
  }

  // End the "run"
  p.lastPoint();
  EXPECT_DOUBLE_EQ(*valm, p.getWirePosition());
  EXPECT_DOUBLE_EQ(*valm + 32, p.getStripPosition());
  EXPECT_EQ(timestamp, p.getTimeStamp());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

//
// Created by soegaard on 6/2/17.
//

#include "multiBladeEventBuilder.h"
#include <algorithm>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

multiBladeEventBuilder::multiBladeEventBuilder()
    : m_ADC_theshold(0), m_time_window(185), m_nwire_channels(32),
      m_nstrip_channels(32), m_use_weighted_average(true), m_wire_cluster(0),
      m_strip_cluster(0), m_time_stamp(0), m_cluster_clock(0),
      m_first_signal(true), m_nevents(0) {
  resetCounters();
}

multiBladeEventBuilder::~multiBladeEventBuilder() = default;

bool multiBladeEventBuilder::addDataPoint(const uint8_t &channel,
                                          const uint16_t &ADC,
                                          const uint32_t &clock) {

  XTRACE(PROCESS, DEB, "Data-point received (%d, %d, %d)\n",
         static_cast<uint>(channel), static_cast<uint>(ADC),
         static_cast<uint>(clock));

  // Increment the counter for number of data-points received.
  m_datapoints_received++;

  if (ADC < m_ADC_theshold) {
    XTRACE(PROCESS, DEB, "ADC-value below threshold. %d < %d\n",
           static_cast<uint>(ADC), static_cast<uint>(m_ADC_theshold));
    return false;
  }

  // Sanity check. Recieved channel number must not exceed the sum of wire and
  // strip channels
  if (channel >= m_nwire_channels + m_nstrip_channels) {
    XTRACE(PROCESS, WAR,
           "Recieved channel number : %d - max channel-number : %d\n",
           static_cast<uint>(channel),
           static_cast<uint>(m_nwire_channels + m_nstrip_channels));
    return false;
  }

  if (m_first_signal) {

    m_cluster_clock = clock;
    m_first_signal = false;

    XTRACE(PROCESS, DEB, "First signal. Setting start clock to : %d\n", clock);
  }

  // Calculate the number of clock-cycles from the timestamp
  uint32_t clock_diff = clock - m_cluster_clock;

  // Check if we are within the time-window
  if (clock_diff < m_time_window) {

    // point is within time-window
    XTRACE(PROCESS, DEB, "Within time-window [%d < %d]\n", clock_diff,
           m_time_window);

    // Add point to cluster
    addPointToCluster(channel, ADC);

    return false;
  }

  // point is outside time-window - the cluster is complete.
  XTRACE(PROCESS, DEB, "Outside time-window [%d > %d]\n", clock_diff,
         m_time_window);

  // multiBladeEventBuilder the stored clusters. True is returned if all checks
  // pass.
  bool position_determined = processClusters();

  // Position calculated and monitoring info stored. Clear vectors for next
  // cluster.
  m_wire_cluster.clear();
  m_strip_cluster.clear();

  // Current data corresponds to next cluster, so store it in the vectors and
  // set the new time-stamp.
  addPointToCluster(channel, ADC);
  m_cluster_clock = clock;

  if (position_determined)
    m_nevents++;

  return position_determined;
}

bool multiBladeEventBuilder::processClusters() {

  // Currently removes clusters containing non-adjacent points.
  // Possible future modification could include formation of a new cluster.
  if (!pointsAdjacent()) {
    m_wire_cluster.clear();
    m_strip_cluster.clear();

    m_rejected_adjacency++;

    return false;
  }

  // Calculate the wire and strip positions.
  m_wire_pos = calculatePosition(m_wire_cluster);
  m_strip_pos = calculatePosition(m_strip_cluster);

  // Calculate the time-stamp
  m_time_stamp = m_cluster_clock;

  XTRACE(PROCESS, DEB, "Calculated position : Pos(%1.4f, %1.4f)\n", m_wire_pos,
         m_strip_pos);

  if ((m_wire_pos < 0) && (m_strip_pos < 0)) {

    m_rejected_position++;

    XTRACE(PROCESS, WAR, "Both positions less than 0 - not an event!\n");

    return false;
  } else {

    // For monitoring and debugging
    incrementCounters(m_wire_cluster, m_strip_cluster);
  }

  return true;
}

bool multiBladeEventBuilder::pointsAdjacent() {

  // Check if wire points are adjacent
  bool wires_adjacent = checkAdjacency(m_wire_cluster);
  // Check if strip points are adjacent
  bool strips_adjacent = checkAdjacency(m_strip_cluster);

  // Both sets have to be adjacent for a valid cluster
  bool adjacent = wires_adjacent && strips_adjacent;

  XTRACE(PROCESS, DEB, "Wires are%s adjacent, strips are%s adjacent!\n",
         (wires_adjacent ? "" : "not"), (strips_adjacent ? "" : "not"));

  return adjacent;
}

bool multiBladeEventBuilder::checkAdjacency(std::vector<point> &cluster) {

  if (cluster.size() <= 1)
    return true;

  // Sort the signals by channel number.
  std::sort(cluster.begin(), cluster.end());

// Cluster iterator

#if 1
  if ((cluster.back().channel - cluster.front().channel) >
      (cluster.size() - 1)) {
    cluster.clear();
    return false;
  }
#else
  auto it1 = cluster.begin();
  // Loop until the second last data-point
  while (it1 != --cluster.end()) {

    // Get the next element relative to the main iterator
    auto it2 = std::next(it1);

    // Get the channel numbers
    auto channel1 = it1->channel;
    auto channel2 = it2->channel;

    // Calculate the difference in channel number
    auto diff = channel2 - channel1;

    // Check if the difference is larger than 1. If so, they are not adjacent
    // ...
    if (diff > 1) {
      // Clear the cluster -- ie. remove it from the analysis.
      cluster.clear();
      // Break the while-loop
      return false;

    } else {
      // Move iterator to next element
      it1++;
    }
  }
#endif

  return true;
}

double multiBladeEventBuilder::calculatePosition(std::vector<point> &cluster) {

  if (m_use_weighted_average) {
    uint64_t sum_numerator = 0;
    uint64_t sum_denominator = 0;
    for (auto &it : cluster) {
      // printf("channel %d, adc: %d\n", it.channel, it.ADC);
      sum_numerator += it.channel * it.ADC;
      sum_denominator += it.ADC;
    }
    return (sum_denominator == 0 ? -1
                                 : static_cast<double>(sum_numerator) /
                                       static_cast<double>(sum_denominator));
  } else {
    uint8_t max_channel = 0;
    uint64_t max_ADC = 0;
    for (auto &it : cluster) {
      // printf("channel %d, adc: %d\n", it.channel, it.ADC);
      if (it.ADC > max_ADC) {
        max_ADC = it.ADC;
        max_channel = it.channel;
      }
    }

    return (max_ADC == 0 ? -1. : static_cast<double>(max_channel));
  }
}

void multiBladeEventBuilder::lastPoint() {

  if (processClusters())
    m_nevents++;

  // Position calculated and monitoring info stored. Clear vectors for next
  // cluster.
  m_wire_cluster.clear();
  m_strip_cluster.clear();

  // Reset the current cluster-clock and reset for first data-point
  m_cluster_clock = 0;
  m_first_signal = true;
}

std::vector<double> multiBladeEventBuilder::getPosition() {
  std::vector<double> coordinates(3);
  coordinates[0] = m_wire_pos;
  coordinates[1] = m_strip_pos;
  coordinates[2] = getTimeStamp();

  return coordinates;
}

void multiBladeEventBuilder::addPointToCluster(uint32_t channel, uint32_t ADC) {

  point point = {channel, ADC};
  if (channel < m_nwire_channels)
    m_wire_cluster.push_back(point);
  else
    m_strip_cluster.push_back(point);
}

void multiBladeEventBuilder::resetCounters() {

  m_datapoints_received = 0;
  m_nevents = 0;
  m_rejected_adjacency = 0;
  m_rejected_position = 0;
  m_2D_wires = {{0, 0, 0, 0, 0, 0}};
  m_2D_strips = {{0, 0, 0, 0, 0, 0}};
  m_1D_wires = {{0, 0, 0, 0, 0, 0}};
  m_1D_strips = {{0, 0, 0, 0, 0, 0}};
}

void multiBladeEventBuilder::incrementCounters(
    const std::vector<point> &m_wire_cluster,
    const std::vector<point> &m_strip_cluster) {

  // Increment counters for wire and strip cluster sizes, for clusters with both
  // wire and strip signals
  if (!m_wire_cluster.empty() && !m_strip_cluster.empty()) {
    if (m_wire_cluster.size() <= 5) {
      m_2D_wires.at(m_wire_cluster.size() - 1)++;
    } else {
      m_2D_wires.at(5)++;

      XTRACE(PROCESS, DEB,
             "More points than expected! Number of wire data points = %d\n",
             static_cast<int>(m_wire_cluster.size()));
    }
    if (m_strip_cluster.size() <= 5) {
      m_2D_strips.at(m_strip_cluster.size() - 1)++;
    } else {
      m_2D_strips.at(5)++;

      XTRACE(PROCESS, DEB,
             "More points than expected! Number of strip data points = %d\n",
             static_cast<int>(m_strip_cluster.size()));
    }
  }

  // Increment counters for wire cluster sizes, for clusters with only wire
  // signals
  if (!m_wire_cluster.empty() && m_strip_cluster.empty()) {
    if (m_wire_cluster.size() <= 5) {
      m_1D_wires.at(m_wire_cluster.size() - 1)++;
    } else {
      m_1D_wires.at(5)++;

      XTRACE(PROCESS, DEB,
             "More points than expected! Number of wire data points = %d\n",
             static_cast<int>(m_wire_cluster.size()));
    }
  }

  // Increment counters for wire cluster sizes, for clusters with only wire
  // signals
  if (m_wire_cluster.empty() && !m_strip_cluster.empty()) {
    if (m_strip_cluster.size() <= 5) {
      m_1D_strips.at(m_strip_cluster.size() - 1)++;
    } else {
      m_1D_strips.at(5)++;

      XTRACE(PROCESS, DEB,
             "More points than expected! Number of strip data points = %lu\n",
             m_strip_cluster.size());
    }
  }
}

bool operator<(const point &a, const point &b) {
  return (a.channel < b.channel);
}

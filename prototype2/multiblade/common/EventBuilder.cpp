/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <multiblade/common/EventBuilder.h>
#include <algorithm>
#include <fmt/format.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

namespace Multiblade {

EventBuilder::EventBuilder()
    : m_wire_cluster(0), m_strip_cluster(0), m_time_stamp(0), m_cluster_clock(0),
      m_first_signal(true), m_nevents(0) {
  resetCounters();
}

bool EventBuilder::addDataPoint(const uint16_t &channel,
                                const uint16_t &ADC,
                                const uint32_t &clock) {

  XTRACE(PROCESS, DEB, "Data-point received (%d, %d, %d)",
         static_cast<uint>(channel), static_cast<uint>(ADC),
         static_cast<uint>(clock));

  if (ADC < m_ADC_theshold) {
    XTRACE(PROCESS, DEB, "ADC-value below threshold. %d < %d",
           static_cast<uint>(ADC), static_cast<uint>(m_ADC_theshold));
    Stats.readouts_discarded++;
    return false;
  }

  // Sanity check. Recieved channel number must not exceed the sum of wire and
  // strip channels
  if (channel >= m_nwire_channels + m_nstrip_channels) {
    XTRACE(PROCESS, WAR,
           "Recieved channel number : %d - max channel-number : %d",
           static_cast<uint>(channel),
           static_cast<uint>(m_nwire_channels + m_nstrip_channels));
    return false;
  }

  if (m_first_signal) {

    m_cluster_clock = clock;
    m_first_signal = false;

    XTRACE(PROCESS, DEB, "First signal. Setting start clock to : %u", clock);
  }

  // Calculate the number of clock-cycles from the timestamp
  uint32_t clock_diff = clock - m_cluster_clock;

  // Check if we are within the time-window
  if (clock_diff < m_time_window) {

    // point is within time-window
    XTRACE(PROCESS, DEB, "Within time-window [%u < %u]", clock_diff,
           m_time_window);

    // Add point to cluster
    addPointToCluster(channel, ADC);

    return false;
  }

  // point is outside time-window - the cluster is complete.
  XTRACE(PROCESS, DEB, "Outside time-window [%u > %u]", clock_diff,
         m_time_window);

  // EventBuilder the stored clusters. True is returned if all checks
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

bool EventBuilder::processClusters() {

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

  XTRACE(PROCESS, DEB, "<<< Calculated position : Pos(%1.4f, %1.4f)>>>", m_wire_pos,
         m_strip_pos);

  if ((m_wire_pos < 0) && (m_strip_pos < 0)) {

    m_rejected_position++;

    XTRACE(PROCESS, WAR, "Both positions less than 0 - not an event!");

    return false;
  } else {

    // For monitoring and debugging
    incrementCounters(m_wire_cluster, m_strip_cluster);
  }

  return true;
}

bool EventBuilder::pointsAdjacent() {

  // Check if wire points are adjacent
  bool wires_adjacent = checkAdjacency(m_wire_cluster);
  // Check if strip points are adjacent
  bool strips_adjacent = checkAdjacency(m_strip_cluster);

  // Both sets have to be adjacent for a valid cluster
  bool adjacent = wires_adjacent && strips_adjacent;

  XTRACE(PROCESS, DEB, "Wires are%s adjacent, strips are%s adjacent!",
         (wires_adjacent ? "" : "not"), (strips_adjacent ? "" : "not"));

  return adjacent;
}

bool EventBuilder::checkAdjacency(std::vector<point> &cluster) {

  if (cluster.size() == 0) {
    return false;
  }

  if (cluster.size() <= 1)
    return true;

  // Sort the signals by channel number.
  std::sort(cluster.begin(), cluster.end());

// Cluster iterator

#if 0
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

double EventBuilder::calculatePosition(std::vector<point> &cluster) {

  if (m_use_weighted_average) {
    uint64_t sum_numerator {0};
    uint64_t sum_denominator {0};
    for (auto &it : cluster) {
      // printf("channel %d, adc: %d\n", it.channel, it.ADC);
      sum_numerator += it.channel * it.ADC;
      sum_denominator += it.ADC;
    }
    return (sum_denominator == 0 ? -1
                                 : static_cast<double>(sum_numerator) /
            static_cast<double>(sum_denominator));
  } else {
    uint16_t max_channel {0};
    uint16_t max_ADC {0};
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

void EventBuilder::lastPoint() {

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

std::vector<double> EventBuilder::getPosition() {
  std::vector<double> coordinates(3);
  coordinates[0] = m_wire_pos;
  coordinates[1] = m_strip_pos;
  coordinates[2] = getTimeStamp();

  return coordinates;
}

void EventBuilder::addPointToCluster(uint16_t channel, uint16_t ADC) {

  point point = {channel, ADC};
  if (channel < m_nwire_channels) {
    // XTRACE(DATA, DEB, "Add wire channel: %d\n", channel);
    m_wire_cluster.push_back(point);
  } else {
    // XTRACE(DATA, DEB, "Add strip channel: %d\n", channel);
    m_strip_cluster.push_back(point);
  }
}

void EventBuilder::resetCounters() {
  m_nevents = 0;
  m_rejected_adjacency = 0;
  m_rejected_position = 0;
  m_2D_wires = {{0, 0, 0, 0, 0, 0}};
  m_2D_strips = {{0, 0, 0, 0, 0, 0}};
  m_1D_wires = {{0, 0, 0, 0, 0, 0}};
  m_1D_strips = {{0, 0, 0, 0, 0, 0}};
}

void EventBuilder::incrementCounters(
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
             "More points than expected! Number of wire data points = %d",
             static_cast<int>(m_wire_cluster.size()));
    }
    if (m_strip_cluster.size() <= 5) {
      m_2D_strips.at(m_strip_cluster.size() - 1)++;
    } else {
      m_2D_strips.at(5)++;

      XTRACE(PROCESS, DEB,
             "More points than expected! Number of strip data points = %d",
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
             "More points than expected! Number of wire data points = %d",
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
             "More points than expected! Number of strip data points = %lu",
             m_strip_cluster.size());
    }
  }
}

bool operator<(const point &a, const point &b) {
  return (a.channel < b.channel);
}


void EventBuilder::print() const {

  fmt::memory_buffer out;

  uint64_t n2Dwireevents = sumArray(m_2D_wires);
  uint64_t n2Dstripevents = sumArray(m_2D_strips);
  uint64_t n1Dwireevents = sumArray(m_1D_wires);
  uint64_t n1Dstripevents = sumArray(m_1D_strips);

  fmt::format_to(out, "\n"
                      "Number of events recorded      : {:>10}\n"
                      "Number of 2D events            : {:>10} (wire), {:>10} (strip). These two numbers must be identical.\n"
                      "Number of 1D wire events       : {:>10}\n"
                      "Number of 1D strip events      : {:>10}\n"
                      "Total number of events         : {:>10}. Must match number of events recorded!\n",
                 m_nevents,
                 n2Dwireevents, n2Dstripevents,
                 n1Dwireevents,
                 n1Dstripevents,
                 n2Dwireevents + n1Dwireevents + n1Dstripevents);

  fmt::format_to(out, "\n"
                 "{:^52}{:^60}\n"
                 "{:^52}",
                 " ", "Events with channels per event",
                 " ");
  for (int i = 0; i < 6; i++)
    fmt::format_to(out, "{:>10}", (i < 5 ? std::to_string(i + 1) : ">5"));
  fmt::format_to(out, "\n"
                 "{:^52}{:->60}\n",
                 " ", "-");
  fmt::format_to(out, "2D events (both wire and strip signals) : \n");
  fmt::format_to(out, "{}",
                 printClusterAbsolute(m_2D_wires,
                       "Number of events with wires fired per event  : "));
  fmt::format_to(out, "{}", printClusterAbsolute(m_2D_strips,
                       "Number of events with strips fired per event : "));
  fmt::format_to(out, "{}", printClusterPercentage(m_2D_wires,
                         "Percentage of wires fired per event          : "));
  fmt::format_to(out, "{}", printClusterPercentage(m_2D_strips,
                         "Percentage of strips fired per event         : "));

  fmt::format_to(out, "1D wire events : \n");
  fmt::format_to(out, "{}", printClusterAbsolute(m_1D_wires,
                       "Number of events with wires fired per event  : "));
  fmt::format_to(out, "{}", printClusterPercentage(m_1D_wires,
                         "Percentage of wires fired per event          : "));
  fmt::format_to(out, "1D strip events : \n");
  fmt::format_to(out, "{}", printClusterAbsolute(m_1D_strips,
                       "Number of events with strips fired per event : "));
  fmt::format_to(out, "{}", printClusterPercentage(m_1D_strips,
                         "Percentage of strips fired per event         : "));

  fmt::format_to(out, "\n"
                        "Number of rejected clusters :\n"
                        "     Adacency : {:>10}\n"
                        "     Position : {:>10}\n",
                        m_rejected_adjacency,
                        m_rejected_position);

  uint64_t sum_wire =
      m_2D_wires[5] + m_1D_wires[5];
  uint64_t sum_strip =
      m_2D_strips[5] + m_1D_strips[5];

  fmt::format_to(out, "\n"
                           "Number of clusters with more than 5 points per wire and or strip : \n"
                           "     Wires  : {:>10}\n"
                           "     Strips : {:>10}\n",
                           sum_wire, sum_strip);

  std::cout << fmt::to_string(out);
}

std::string EventBuilder::printClusterAbsolute(const std::array<uint64_t, 6>& array,
                                                std::string text) {
  fmt::memory_buffer out;
  fmt::format_to(out, "     {}", text);
  for (int i = 0; i < 6; i++)
    fmt::format_to(out, "{:>10}", array[i]);
  fmt::format_to(out, "\n");
  return fmt::to_string(out);
}

std::string EventBuilder::printClusterPercentage(const std::array<uint64_t, 6>& array,
                                                  std::string text) {
  fmt::memory_buffer out;
  fmt::format_to(out, "     {}", text);
  uint64_t sum = sumArray(array);
  //std::cout << std::fixed << std::setprecision(4);
  for (int i = 0; i < 6; i++)
    fmt::format_to(out, "{:>10}", 1. * array[i] / sum * 100.);
  fmt::format_to(out, "\n");
  return fmt::to_string(out);
}

uint64_t EventBuilder::sumArray(const std::array<uint64_t, 6> &array) {
  uint64_t sum = 0;
  for (uint i = 0; i < 6; i++) {
    sum += array[i];
  }
  return sum;
}

}

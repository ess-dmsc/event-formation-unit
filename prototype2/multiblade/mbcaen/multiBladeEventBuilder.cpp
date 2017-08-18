//
// Created by soegaard on 6/2/17.
//

#include <algorithm>
#include "multiBladeEventBuilder.h"

multiBladeEventBuilder::multiBladeEventBuilder()
: m_clock_d(16e-9),
  m_time_window(185),
  m_nwire_channels(32),
  m_nstrip_channels(32),
  m_use_weighted_average(true),
  m_wire_cluster(0),
  m_strip_cluster(0),
  m_cluster_clock(0),
  m_first_signal(true),
  m_verbose(false),
  m_nevents(0)
{
    resetCounters();
}

multiBladeEventBuilder::~multiBladeEventBuilder() {}

bool multiBladeEventBuilder::addDataPoint(const uint8_t& channel, const uint64_t& ADC, const uint32_t& clock)
{

#ifdef TRACE
    std::cout << "Data-point received (" << int(channel) << ", " << ADC << ", " << clock << ")\n";
#endif

    // Increment the counter for number of data-points received.
    m_datapoints_received++;

    // Sanity check. Recieved channel number must not exceed the sum of wire and strip channels
    if (channel >= m_nwire_channels+m_nstrip_channels){
        std::cerr << "Recieved channel number : " << static_cast<uint>(channel) << std::endl;
        std::cerr << "which is larger than the total number of possible channels : "
                  << static_cast<uint>(m_nwire_channels+m_nstrip_channels) << std::endl;
        return false;
    }


    if (m_first_signal)
    {

#ifdef TRACE
        std::cout << "First signal. Setting start clock to : " << clock << std::endl;
#endif

        m_cluster_clock = clock;
        m_first_signal = false;
    }

    // Calculate the number of clock-cycles from the timestamp
    uint32_t clock_diff = clock - m_cluster_clock;

    // Check if we are within the time-window
    if (clock_diff < m_time_window) {

        // Datapoint is within time-window

#ifdef TRACE
        std::cout << "Within time-window [" << clock_diff << " < " << m_time_window << "]" << std::endl;
#endif

        addPointToCluster(channel, ADC);

        return false;
    }

    // Datapoint is outside time-window - the cluster is complete.

#ifdef TRACE
    std::cout << "Outside time-window [" << clock_diff << " > " << m_time_window << "]" << std::endl;
#endif

    // multiBladeEventBuilder the stored clusters. True is returned if all checks pass.
    bool position_determined = processClusters();

    // Position calculated and monitoring info stored. Clear vectors for next cluster.
    m_wire_cluster.clear();
    m_strip_cluster.clear();

    // Current data corresponds to next cluster, so store it in the vectors and set the new time-stamp.
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
    m_time_stamp = static_cast<double>(m_cluster_clock) * m_clock_d;

#ifdef TRACE
    std::cout << "Calculated position : Pos(" << m_wire_pos << ", " << m_strip_pos << ")\n";
#endif

    if ((m_wire_pos < 0) && (m_strip_pos < 0)) {

        m_rejected_position++;

#ifdef TRACE
        std::cout << "Both positions less than 0 - not an event" << std::endl;
#endif

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

#ifdef TRACE
    std::cout << "Wires are " << (wires_adjacent ? "" : "not") << " adjacent, "
              << "strips are " << (strips_adjacent ? "" : "not") << " adjacent!" << std::endl;
#endif

    return adjacent;
}

bool multiBladeEventBuilder::checkAdjacency(std::vector<datapoint> cluster) {

    // Check if cluster contains more than 1 signal. If not - then exit, since there will be nothing to do.
    if (cluster.size() <= 1)
        return true;

    // Sort the signals by channel number.
    std::sort(cluster.begin(), cluster.end());

    // Cluster iterator
    std::vector<datapoint>::iterator it1 = cluster.begin();
    // Loop until the second last data-point
    while (it1 != --cluster.end()) {
        // Get the next element relative to the main iterator
        std::vector<datapoint>::iterator it2 = std::next(it1);

        // Get the channel numbers
        uint8_t channel1 = it1->channel;
        uint8_t channel2 = it2->channel;

        // Calculate the difference in channel number
        int16_t diff = channel2 - channel1;

        // Check if the difference is larger than 1. If so, they are not adjacent ...
        if (diff > 1) {

            // Clear the cluster -- ie. remove it from the analysis.
            cluster.clear();
            // Break the while-loop
            return false;

            // Possible use if non-adjacent points are to be removed and processed separately.
            // ... erase the non-adjacent element
            //cluster.erase(it2);
        } else
            // Move iterator to next element
            it1++;
    }

    return true;
}

double multiBladeEventBuilder::calculatePosition(std::vector<datapoint> cluster) {

    double position = -1.;

    if (cluster.size() == 0)
        return position;

    if (m_use_weighted_average) {
        uint64_t sum_numerator = 0;
        uint64_t sum_denominator = 0;
        for (std::vector<datapoint>::iterator it = cluster.begin(); it != cluster.end(); ++it) {
            sum_numerator += it->channel * it->ADC;
            sum_denominator += it->ADC;
        }

        position = static_cast<double>(sum_numerator) / static_cast<double>(sum_denominator);
    } else {
        uint8_t max_channel = 0;
        uint64_t max_ADC = 0;
        for (std::vector<datapoint>::iterator it = cluster.begin(); it != cluster.end(); ++it) {
            if (it->ADC > max_ADC) {
                max_ADC = it->ADC;
                max_channel = it->channel;
            }
        }

        position = static_cast<double>(max_channel);
    }

    return position;
}

void multiBladeEventBuilder::lastPoint() {

    if (processClusters())
        m_nevents++;

    // Position calculated and monitoring info stored. Clear vectors for next cluster.
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

void multiBladeEventBuilder::addPointToCluster(uint8_t channel, uint64_t ADC) {

    datapoint point = {channel, ADC};
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
    m_2D_wires  = {{0, 0, 0, 0, 0, 0}};
    m_2D_strips = {{0, 0, 0, 0, 0, 0}};
    m_1D_wires  = {{0, 0, 0, 0, 0, 0}};
    m_1D_strips = {{0, 0, 0, 0, 0, 0}};
}

void multiBladeEventBuilder::incrementCounters(std::vector<datapoint> m_wire_cluster, std::vector<datapoint> m_strip_cluster) {

    // Increment counters for wire and strip cluster sizes, for clusters with both wire and strip signals
    if (m_wire_cluster.size() && m_strip_cluster.size()) {
        if (m_wire_cluster.size() <= 5) {
            m_2D_wires.at(m_wire_cluster.size() - 1)++;
        } else {
            m_2D_wires.at(5)++;
#ifdef TRACE
            std::cerr << "<addDataPoint> More datapoints than expected! Number of wire data points = "
                      << m_wire_cluster.size() << std::endl;
#endif
        }
        if (m_strip_cluster.size() <= 5) {
            m_2D_strips.at(m_strip_cluster.size() - 1)++;
        } else {
            m_2D_strips.at(5)++;
#ifdef TRACE
            std::cerr << "<addDataPoint> More datapoints than expected! Number of strip data points = "
                      << m_strip_cluster.size() << std::endl;
#endif
        }
    }

    // Increment counters for wire cluster sizes, for clusters with only wire signals
    if (m_wire_cluster.size() && !m_strip_cluster.size()) {
        if (m_wire_cluster.size() <= 5) {
            m_1D_wires.at(m_wire_cluster.size() - 1)++;
        } else {
            m_1D_wires.at(5)++;
#ifdef TRACE
            std::cerr << "<addDataPoint> More datapoints than expected! Number of wire data points = "
                      << m_wire_cluster.size() << std::endl;
#endif
        }
    }

    // Increment counters for wire cluster sizes, for clusters with only wire signals
    if (!m_wire_cluster.size() && m_strip_cluster.size()) {
        if (m_strip_cluster.size() <= 5) {
            m_1D_strips.at(m_strip_cluster.size() - 1)++;
        } else {
        m_1D_strips.at(5)++;
#ifdef TRACE
            std::cerr << "<addDataPoint> More datapoints than expected! Number of strip data points = "
                      << m_strip_cluster.size() << std::endl;
#endif
        }
    }
}

bool operator< (const datapoint& a, const datapoint& b) {
    return (a.channel < b.channel);
}

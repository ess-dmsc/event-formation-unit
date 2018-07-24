/// Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <array>
#include <iomanip>

#include "DumpEventBuilderInfo.h"

dumpEventBuilderInfo::dumpEventBuilderInfo()
#if 0
: m_2D_wire_events(0),
  m_2D_strip_events(0),
  m_1D_wire_events(0),
  m_1D_strip_events(0)
#endif
{
}

void dumpEventBuilderInfo::print(multiBladeEventBuilder p) {

  // clusterPoints(p);
  printOverview(p);
  printClusterInfo(p);
  printRejected(p);
  printExcessivePoints(p);
}

void dumpEventBuilderInfo::printOverview(multiBladeEventBuilder p) {

  uint64_t n2Dwireevents = sumArray(p.get2DWireClusterCounter());
  uint64_t n2Dstripevents = sumArray(p.get2DStripClusterCounter());
  uint64_t n1Dwireevents = sumArray(p.get1DWireClusterCounter());
  uint64_t n1Dstripevents = sumArray(p.get1DStripClusterCounter());

  std::cout << "\n";
  std::cout << "Number of data-points recieved : " << std::setw(10)
            << p.getNumberOfDatapointsReceived() << std::endl;
  std::cout << "Number of events recorded      : " << std::setw(10)
            << p.getNumberOfEvents() << std::endl;
  std::cout << "Number of 2D events            : " << std::setw(10)
            << n2Dwireevents << " (wire), " << std::setw(10) << n2Dstripevents
            << " (strip). "
            << "These two numbers must be identical." << std::endl;
  std::cout << "Number of 1D wire events       : " << std::setw(10)
            << n1Dwireevents << std::endl;
  std::cout << "Number of 1D strip events      : " << std::setw(10)
            << n1Dstripevents << std::endl;
  std::cout << "Total number of events         : " << std::setw(10)
            << n2Dwireevents + n1Dwireevents + n1Dstripevents
            << " Must match number of events recorded! " << std::endl;
}

void dumpEventBuilderInfo::printClusterInfo(multiBladeEventBuilder p) {

  std::cout << "\n";
  std::cout << "2D events (both wire and strip signals) : \n";
  printClusterAbsolute(p.get2DWireClusterCounter(),
                       "Number of events with wires fired per event  : ");
  printClusterAbsolute(p.get2DStripClusterCounter(),
                       "Number of events with strips fired per event : ");
  printClusterPercentage(p.get2DWireClusterCounter(),
                         "Percentage of wires fired per event          : ");
  printClusterPercentage(p.get2DStripClusterCounter(),
                         "Percentage of strips fired per event         : ");

  std::cout << "1D wire events : \n";
  printClusterAbsolute(p.get1DWireClusterCounter(),
                       "Number of events with wires fired per event  : ");
  printClusterPercentage(p.get1DWireClusterCounter(),
                         "Percentage of wires fired per event          : ");
  std::cout << "1D strip events : \n";
  printClusterAbsolute(p.get1DStripClusterCounter(),
                       "Number of events with strips fired per event : ");
  printClusterPercentage(p.get1DStripClusterCounter(),
                         "Percentage of strips fired per event         : ");
}

void dumpEventBuilderInfo::printRejected(multiBladeEventBuilder p) {

  std::cout << "\n";
  std::cout << "Number of rejected clusters :" << std::endl;
  std::cout << "     Adacency : " << std::setw(10)
            << p.getNumberOfAdjacencyRejected() << std::endl;
  std::cout << "     Position : " << std::setw(10)
            << p.getNumberOfPositionRejected() << std::endl;
}

void dumpEventBuilderInfo::printExcessivePoints(multiBladeEventBuilder p) {

  uint64_t sum_wire =
      p.get2DWireClusterCounter()[5] + p.get1DWireClusterCounter()[5];
  uint64_t sum_strip =
      p.get2DStripClusterCounter()[5] + p.get1DStripClusterCounter()[5];

  std::cout << "\n";
  std::cout
      << "Number of clusters with more than 5 points per wire and or strip : "
      << std::endl;
  std::cout << "     Wires  : " << std::setw(10) << sum_wire << std::endl;
  std::cout << "     Strips : " << std::setw(10) << sum_strip << std::endl;
}

void dumpEventBuilderInfo::printClusterAbsolute(std::array<uint64_t, 6> array,
                                                std::string text) {

  std::cout << "     " << text;
  printArray(array);
  std::cout << "\n";
}

void dumpEventBuilderInfo::printClusterPercentage(std::array<uint64_t, 6> array,
                                                  std::string text) {

  std::cout << "     " << text;
  printArrayPercentage(array);
  std::cout << "\n";
}

void dumpEventBuilderInfo::printArray(std::array<uint64_t, 6> array) {

  for (int i = 0; i < 6; i++)
    std::cout << std::setw(10) << array[i] << " ("
              << (i < 5 ? std::to_string(i + 1) : ">5") << ")";
}

void dumpEventBuilderInfo::printArrayPercentage(std::array<uint64_t, 6> array) {

  uint64_t sum = sumArray(array);

  std::cout << std::fixed << std::setprecision(4);
  for (int i = 0; i < 6; i++)
    std::cout << std::setw(10) << 1. * array[i] / sum * 100. << " ("
              << (i < 5 ? std::to_string(i + 1) : ">5") << ")";
}

uint64_t dumpEventBuilderInfo::sumArray(std::array<uint64_t, 6> array) {

  uint64_t sum = 0;

  for (uint i = 0; i < 6; i++)
    sum += array[i];

  return sum;
}

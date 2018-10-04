/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include "mbcommon/MultiBladeEventBuilder.h"

class DumpEventBuilderInfo {

public:
  /// Default and only constructor
  DumpEventBuilderInfo();

  /// Call this to wite the information to stdout
  void print(MultiBladeEventBuilder p);

private:
  /// \name Counters for number of channels hit per cluster/event.
  /// Two counters for 2D clusters (wire and strips signals) and two counters for
  /// 1D clusters.
  /// element 6 corresponds to more than 5 signals for either wire or strip in a
  /// cluster.
  //@{
  // std::array<uint64_t, 6> m_2D_wire_counts;
  // std::array<uint64_t, 6> m_2D_strip_counts;
  // std::array<uint64_t, 6> m_1D_wire_counts;
  // std::array<uint64_t, 6> m_1D_strip_counter;
  //@}

  /// \name Countes for number of clusters/events.
  /// 2D and 1D counters. Note the two 2D counters should be identical.
  //@{
  // uint64_t m_2D_wire_events   = 0;
  // uint64_t m_2D_strip_events  = 0;
  // uint64_t m_1D_wire_events  = 0;
  // uint64_t m_1D_strip_events = 0;
  //@}

  /// Writes overview information
  void printOverview(MultiBladeEventBuilder p);

  /// \name Prints information of all processed clusters/events
  /// This print-out calls several function.
  //@{
  void printClusterInfo(MultiBladeEventBuilder p);
  void printClusterAbsolute(std::array<uint64_t, 6>, std::string text);
  void printClusterPercentage(std::array<uint64_t, 6>, std::string text);
  void printArray(std::array<uint64_t, 6> array);
  void printArrayPercentage(std::array<uint64_t, 6> array);
  //@}

  /// Prints number of events with more than 5 signals per cluster/event for
  /// either wire or strip
  void printExcessivePoints(MultiBladeEventBuilder p);

  /// Prints number of rejected clusters/events
  void printRejected(MultiBladeEventBuilder p);

  /// Summs the contents of a std::array of uint64_t with dimension 6
  uint64_t sumArray(std::array<uint64_t, 6> array);
};

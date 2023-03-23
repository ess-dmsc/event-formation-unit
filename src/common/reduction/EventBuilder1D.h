// Copyright (C) 2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Time-boxed event builder for Multi-Blade
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/clustering/GapClusterer.h>

static constexpr uint64_t latency{2010}; // ns == 2.01us
// expect readouts in a plane to be at least this close

const uint8_t Plane{0};

class EventBuilder1D {
public:
  EventBuilder1D();

  /// \todo pass by rvalue?
  /// \brief add a hit to the event
  /// \param hit to be added to the event
  void insert(Hit hit);

  /// \brief form clusters, match them, and create events
  /// \param full_flush boolean determines if all readouts should be clustered
  ///         and matched, or if recent readouts should remain, in case future
  ///         readouts belong to the same event. WARNING, setting this to true
  ///         results in major performance loss.
  void flush(bool full_flush = false);

  /// \brief clear all stored hits from the HitVectors HitsX and HitsY
  void clearHits();

  /// \brief flushes both clusterers, ClustererX and ClustererY, forming cluster
  void flushClusterers();

  HitVector Hits;

  /// \todo parametrize
  GapClusterer Clusterer;

  // final vector of reconstructed events
  std::vector<Event> Events;

}; // class

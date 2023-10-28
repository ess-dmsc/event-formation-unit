// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Cluster2D.h
/// \brief Cluster2D class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/Hit2DVector.h>

/// \class Cluster2D Cluster2D.h
/// \brief A container of 2D hits, bounds and weight.
///        Hit2Ds can be added, but not removed. Coordinates and timestamps
///        are treated as having an uncertainty of 1 when evaluating dimensions,
///        thus including the endpoints.
///
/// \note  This class does not have defaulted virtual destructor as it can
///        inhibit automatic implicit move-semantics for the HitVector.

class Cluster2D {
public:
  /// \todo should be protected?
  /// \note This variable is left public, because event reduction/analysis
  /// strategies
  ///       must be able to sort hits in their preferred way without copying the
  ///       contents
  Hit2DVector hits;

public:
  /// \brief adds hit to cluster, accumulates mass and recalculates bounds
  ///        no validation is enforced, duplicates possible
  ///        no particular time or spatial ordering is expected
  ///        hits should have non 0-weight if center of mass is to be meaningful
  ///        invalidates plane if planes don't match, but still adds it
  /// \param hit to be added
  void insert(const Hit2D &hit);

  /// \brief merges another cluster into this one
  ///        moves the hits from the other cluster, rendering it empty
  ///        recalculates bounds and aggregates sums
  ///        invalidates plane if planes don't match, but still merges
  /// \param other cluster to be merged
  /// \post other cluster is cleared
  void merge(Cluster2D &other);

  /// \brief clears hits and resets calculated values
  void clear();

  /// \returns true if cluster contains no hits
  bool empty() const;

  /// \returns true if cluster contains hits and all are on the same plane
  bool valid() const;

  /// \brief check if cluster span exceeds the maximum allowed gap
  /// \param MaxAllowedGap the maximum gap allowed
  /// \returns true if a gap is detected
  //bool hasGap(uint8_t MaxAllowedGap) const;

  /// \returns number of hits in cluster
  size_t hitCount() const;

  /// \returns lowest coordinate, undefined in case of empty cluster
  uint16_t xCoordStart() const;
  /// \returns highest coordinate, undefined in case of empty cluster
  uint16_t xCoordEnd() const;
  /// \returns earliest coordinate added to cluster, undefined in case of empty
  /// cluster
  uint16_t xCoordEarliest() const;
  /// \returns latest coordinate added to cluster, undefined in case of empty
  /// cluster
  uint16_t xCoordLatest() const;
  /// \returns coordinate span, 0 in case of empty cluster
  uint16_t xCoordSpan() const;
  /// \returns lowest coordinate, undefined in case of empty cluster

  uint16_t yCoordStart() const;
  /// \returns highest coordinate, undefined in case of empty cluster
  uint16_t yCoordEnd() const;
  /// \returns earliest coordinate added to cluster, undefined in case of empty
  /// cluster
  uint16_t yCoordEarliest() const;
  /// \returns latest coordinate added to cluster, undefined in case of empty
  /// cluster
  uint16_t yCoordLatest() const;
  /// \returns coordinate span, 0 in case of empty cluster
  uint16_t yCoordSpan() const;

  /// \returns earliest timestamp, undefined in case of empty cluster
  uint64_t timeStart() const;
  /// \returns latest timestamp, undefined in case of empty cluster
  uint64_t timeEnd() const;
  /// \returns time span, 0 in case of empty cluster
  uint64_t timeSpan() const;

  /// \returns pre-calculated sum of each hit's weight
  double weightSum() const;

  /// \returns pre-calculated sum of each hit's weight squared
  double weight2Sum() const;

  /// \returns pre-calculated sum of each hit's weight*coord
  double xCoordMass() const;
  /// \returns center of mass in the coordinate dimension
  ///          can be NaN if weight sum is zero
  double xCoordCenter() const;

  /// \returns pre-calculated sum of each hit's weight*coord
  double yCoordMass() const;
  /// \returns center of mass in the coordinate dimension
  ///          can be NaN if weight sum is zero
  double yCoordCenter() const;

  /// \returns pre-calculated sum of each hit's weight*time
  double timeMass() const;
  /// \returns center of mass in the time dimension
  ///          can be NaN if weight sum is zero
  double timeCenter() const;

  /// \returns pre-calculated sum of each hit's weight*weigtht*coord
  double xCoordMass2() const;
  /// \returns center of masss quared in the coordinate dimension
  ///          can be NaN if weight sum is zero
  double xCoordCenter2() const;

  /// \returns pre-calculated sum of each hit's weight*weigtht*coord
  double yCoordMass2() const;
  /// \returns center of masss quared in the coordinate dimension
  ///          can be NaN if weight sum is zero
  double yCoordCenter2() const;

  /// \returns pre-calculated sum of each hit's weight*weight*time
  double timeMass2() const;
  /// \returns center of mass squared in the time dimension
  ///          can be NaN if weight sum is zero
  double timeCenter2() const;

  /// \brief calculates the overlapping time span of two clusters
  /// \param other cluster to be compared
  /// \returns overlapping time span inclusive of end points
  uint64_t timeOverlap(const Cluster2D &other) const;

  /// \brief calculates the time gap of two clusters
  /// \param other cluster to be compared
  /// \returns time gap between clusters
  uint64_t timeGap(const Cluster2D &other) const;

  /// \returns string describing cluster bounds and weights
  /// \param verbose also print hits
  std::string to_string(const std::string &prepend, bool verbose) const;

private:
  /// \todo uint8 might not be enough, if detectors have more independent
  /// modules/segments

  uint16_t x_coord_start_{Hit2D::InvalidCoord};
  uint16_t x_coord_end_{0};
  uint16_t x_coord_earliest_{Hit2D::InvalidCoord};
  uint16_t x_coord_latest_{Hit2D::InvalidCoord};

  uint16_t y_coord_start_{Hit2D::InvalidCoord};
  uint16_t y_coord_end_{0};
  uint16_t y_coord_earliest_{Hit2D::InvalidCoord};
  uint16_t y_coord_latest_{Hit2D::InvalidCoord};

  uint64_t time_start_{0xFFFFFFFFFFFFFFFFULL};
  uint64_t time_end_{0};

  double weight_sum_{0.0};   ///< sum of weight
  double x_coord_mass_{0.0}; ///< sum of coord*weight
  double y_coord_mass_{0.0}; ///< sum of coord*weight
  double time_mass_{0.0};    ///< sum of time*weight

  double weight2_sum_{0.0};
  double x_coord_mass2_{0.0}; ///< sum of coord*weight*weight
  double y_coord_mass2_{0.0}; ///< sum of coord*weight*weight
  double time_mass2_{0.0};    ///< sum of time*weight*weight

  int utpc_idx_min_{0};
  int utpc_idx_max_{0};
};

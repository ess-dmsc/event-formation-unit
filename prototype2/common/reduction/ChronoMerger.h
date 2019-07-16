/** Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file **/
//===----------------------------------------------------------------------===//
///
/// \file ChronoMerger.h
/// \brief ChronoMerger class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/reduction/NeutronEvent.h>
#include <cstddef>
#include <vector>
#include <list>

/// \class ChronoMerger ChronoMerger.h
/// \brief A smart-queue for merging neutron event data from multiple
///        semi-synchronized sources or processing pipelines. The queue
///        uses the criterion of maximum latency to guarantee that all
///        antecedent events from every pipeline have been collected before
///        data is released. If sort is called judiciously, this can
///        guarantee that data always comes out of the queue in chronological
///        order.

class ChronoMerger {
public:
  /// \brief Constructor, which also configures the behaviour of the queue.
  /// \param maximum_latency An empirically derived time delay after which it is
  ///        guaranteed that events will be delivered. This may be similar to
  ///        the observed latency of the readout system, but in practice could be
  ///        higher depending on buffer sizes in clustering pipelines.
  /// \param modules number of independent pipelines from which data needs
  ///        to be synchronized. A latency horizon is observed for each indexed
  ///        pipeline, and only when all pipelines are known to have moved past
  ///        the same time-point will events be released.
  explicit ChronoMerger(uint64_t maximum_latency, size_t modules);

  /// \brief Inserts one NeutronEvent into queue.
  /// \param module Identifies the pipeline from which the event originates.
  /// \param event One event to be added to the queue.
  void insert(size_t module, NeutronEvent event);

  /// \brief Splice an external NeutronEvent queue into the Merger's queue.
  /// \param module Identifies the pipeline from which events originate. All events
  ///               in merged queue must originate from the same pipeline for
  ///               promised guarantees to hold.
  /// \param events Queue of events that will be spliced. The original queue will be
  ///               rendered empty.
  void insert(size_t module, std::list<NeutronEvent>& events);

  /// \brief Forcibly syncs up the time horizons of two pipelines, picking the later
  ///        of the two horizons. This is to be used when external is sufficient to
  ///        establish that one pipeline's time progress implies the same for another.
  ///        This can be used for partial "flushing" of data.
  /// \param module1 id of first pipeline
  /// \param module2 id of second pipeline
  void sync_up(size_t module1, size_t module2);

  /// \brief Sorts the queue chronologically. This should be performed at some point
  ///        after inserting some data and before attempting to pop any off the queue.
  void sort();

  /// \brief Resets the time horizons to 0. This can be done after flushing if
  ///        pipeline are to be restarted with a new clock.
  void reset();

  /// \returns true if queue is empty
  bool empty() const;

  /// \returns timestamp of earliest event in queue
  /// \pre queue must have been sorted for this to be correct
  uint64_t earliest() const;

  /// \returns the global time horizon for all tracked pipelines. The horizon for each
  ///          pipeline is the latest time-point seen in among received events. The
  ///          global horizon is the earliest of these pipeline horizons, i.e. only
  ///          as far as the most lagging pipeline can see.
  uint64_t horizon() const;

  /// \returns true if pipeline is non-empty and earliest event in queue is outside
  ///          the maximum latency window in relation to the global time horizon.
  /// \pre queue must have been sorted for this evaluation to be correct
  bool ready() const;

  /// \returns the earliest event in queue.
  /// \pre queue must have been sorted prior to calling this
  /// \post earliest event will be removed from queue
  NeutronEvent pop_earliest();

  /// \brief prints queue config and contents for debug purposes
  std::string debug(const std::string& prepend, bool verbose) const;

private:
  /// Queue of neutron events
  std::list<NeutronEvent> queue_;

  /// Queue of neutron events
  std::vector<uint64_t> latest_;

  uint64_t maximum_latency_;
};


/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Gem detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/Detector.h>
#include <common/RingBuffer.h>
#include <gdgem/nmx/AbstractBuilder.h>
#include <gdgem/NMXStats.h>
#include <gdgem/NMXConfig.h>
#include <common/SPSCFifo.h>
#include <common/reduction/clustering/AbstractClusterer.h>
#include <common/reduction/matching/AbstractMatcher.h>
#include <common/monitor/Histogram.h>
#include <common/EV42Serializer.h>
#include <gdgem/nmx/TrackSerializer.h>

struct NMXSettings {
  std::string ConfigFile;
  std::string CalibrationFile;
  std::string fileprefix;
  /// \todo REMOVE eventually
  unsigned int PMin{0};
  unsigned int PMax{1279};
  unsigned int PWidth{0};
};

using namespace memory_sequential_consistent; // Lock free fifo

class GdGemBase : public Detector {
public:
  GdGemBase(BaseSettings const & settings, NMXSettings & LocalSettings);
  ~GdGemBase() {delete eth_ringbuf;}


  /// \brief detector specific threads
  void input_thread();
  void processing_thread();

  /// \brief detector specific commands
  int getCalibration(std::vector<std::string> cmdargs, char *output,
                     unsigned int *obytes);
protected:

  /** \todo figure out the right size  of the .._max_entries  */
  static constexpr int eth_buffer_max_entries {2000};
  static constexpr int eth_buffer_size {9000};
  static constexpr int kafka_buffer_size {12400};


  /** Shared between input_thread and processing_thread*/
  CircularFifo<unsigned int, eth_buffer_max_entries> input2proc_fifo;
  RingBuffer<eth_buffer_size> *eth_ringbuf;

  struct NMXSettings NMXSettings;
  Gem::NMXConfig nmx_opts;

  std::shared_ptr<Gem::AbstractBuilder> builder_;
  std::shared_ptr<AbstractClusterer> clusterer_x_;
  std::shared_ptr<AbstractClusterer> clusterer_y_;
  std::shared_ptr<AbstractMatcher> matcher_;

  Gem::NMXStats stats_;

  Hists hists_{std::numeric_limits<uint16_t>::max(),
               std::numeric_limits<uint16_t>::max()};

  ReducedEvent neutron_event_;

  uint64_t previous_full_time_{0};
  uint64_t recent_pulse_time_{0};
  bool have_pulse_time_{false};

  uint64_t truncated_time_;
  uint32_t pixelid_;

  bool sample_next_track_ {false};

  void apply_configuration();
  void cluster_plane(HitVector& hits, std::shared_ptr<AbstractClusterer> clusterer, bool flush);
  void perform_clustering(bool flush);
  void process_events(EV42Serializer&, Gem::TrackSerializer&);
};

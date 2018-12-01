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
#include <gdgem/NMXConfig.h>
#include <libs/include/SPSCFifo.h>
#include <common/clustering/AbstractClusterer.h>
#include <common/clustering/AbstractMatcher.h>
#include <common/Hists.h>
#include <common/EV42Serializer.h>
#include <gdgem/nmx/TrackSerializer.h>

struct NMXSettings {
  std::string ConfigFile;
  std::string CalibrationFile;
  std::string fileprefix;
};

using namespace memory_sequential_consistent; // Lock free fifo

class GdGemBase : public Detector {
public:
  GdGemBase(BaseSettings const & settings, NMXSettings & LocalNMXSettings);
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

  // Careful also using this for other NMX pipeline

  struct {
    // Input Counters
    int64_t rx_packets;
    int64_t rx_bytes;
    int64_t fifo_push_errors;
    // \todo Morten: explain. What is going on here?
    int64_t pad_a[5]; // cppcheck-suppress unusedStructMember

    // Processing thread
    int64_t processing_idle;
    int64_t fifo_seq_errors;

    // Parser stats
    int64_t lost_frames;
    int64_t bad_frames;
    int64_t good_frames;
    int64_t readouts_error_bytes;
    int64_t readouts_total;

    // Builder Counters
    int64_t readouts_bad_geometry;
    int64_t readouts_bad_adc;
    int64_t readouts_good;

    // Clustering
    int64_t clusters_total;
    int64_t clusters_x_only;
    int64_t clusters_y_only;
    int64_t clusters_xy;

    // Analysis
    int64_t events_bad_utpc;
    int64_t events_filter_rejects;
    int64_t events_geom_errors;
    int64_t events_good;
    int64_t readouts_in_good_events;

    // Producer
    int64_t tx_bytes;

    // Kafka stats below are common to all detectors
    int64_t kafka_produce_fails;
    int64_t kafka_ev_errors;
    int64_t kafka_ev_others;
    int64_t kafka_dr_errors;
    int64_t kafka_dr_noerrors;
  } __attribute__((aligned(64))) mystats;

  struct NMXSettings NMXSettings;
  Gem::NMXConfig nmx_opts;

  std::shared_ptr<Gem::AbstractBuilder> builder_;
  std::shared_ptr<AbstractClusterer> clusterer_x_;
  std::shared_ptr<AbstractClusterer> clusterer_y_;
  std::shared_ptr<AbstractMatcher> matcher_;
  Hists hists_{std::numeric_limits<uint16_t>::max(),
               std::numeric_limits<uint16_t>::max()};

  std::shared_ptr<Gem::utpcAnalyzer> utpc_analyzer_;
  Gem::utpcResults utpc_;

  uint64_t previous_full_time_{0};
  uint64_t recent_pulse_time_{0};
  bool have_pulse_time_{false};

  uint64_t truncated_time_;
  uint32_t pixelid_;

  bool sample_next_track_ {false};

  void apply_configuration();
  void cluster_plane(HitContainer& hits, std::shared_ptr<AbstractClusterer> clusterer, bool flush);
  void perform_clustering(bool flush);
  void process_events(EV42Serializer&, Gem::TrackSerializer&);
};

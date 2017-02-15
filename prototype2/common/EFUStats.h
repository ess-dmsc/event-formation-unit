/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief runtime statistics
 */

#pragma once

#include <cinttypes>
#include <libs/include/Timer.h>

class EFUStats {
public:
  /** @brief Constructor starts by clear()'ing all counters */
  EFUStats();

  /** @brief clear counters and also sample() */
  void clear();

  /** @brief set the mask determining what output is printed
   * @param mask bit mask
   */
  void set_mask(unsigned int mask);

  /** @brief send stats to time series database */
  void sendstats();

  /** @brief print out all active statistics */
  void report();

  typedef struct {
    // Input Counters
    uint64_t rx_packets;
    uint64_t rx_bytes;
    uint64_t fifo1_push_errors;
    uint64_t fifo1_free;
    uint64_t
        pad_a[4]; /**< @todo next counter is now on different cache boundary */

    // Processing Counters
    uint64_t rx_readouts;
    uint64_t rx_error_bytes;
    uint64_t rx_discards;
    uint64_t rx_idle1;
    uint64_t geometry_errors;
    uint64_t fifo2_push_errors;
    uint64_t fifo2_free;
    uint64_t
        padb[1]; /**< @todo next counter is now on different cache boundary */

    // Output Counters
    uint64_t rx_events;
    uint64_t rx_idle2;
    uint64_t tx_bytes;
  } stat_t;

  stat_t stats, stats_old;

private:
  Timer usecs_elapsed, runtime; /**< used for rate calculations */
  unsigned int report_mask{0};  /**< bitmask for active statistics */

  /** @brief print out packet related statistics */
  void packet_stats();

  /** @brief print out event related statistics */
  void event_stats();

  /** @brief print out fifo related statistics */
  void fifo_stats();
};

/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief runtime statistics 
 */

#pragma once
#include <algorithm>
#include <cinttypes>
#include <libs/include/Timer.h>

class Stats {
private:
   Timer usecs_elapsed, time;

  /** @todo comment */
  void reset() {
    ib = i;
    pb = p;
    ob = o;
    usecs_elapsed.now();
  }

  /** @todo comment */
  void clear() {
    std::fill_n((char*)&i, sizeof(i), 0);
    std::fill_n((char*)&p, sizeof(p), 0);
    std::fill_n((char*)&o, sizeof(o), 0);
    reset();
  }

  /** @todo comment */
  void packet_stats() {
    auto usecs = usecs_elapsed.timeus();
    uint64_t ipps = (i.rx_packets - ib.rx_packets) * 1000000 / usecs;
    uint64_t iMbps = 8*(i.rx_bytes - ib.rx_bytes) / usecs;
    uint64_t pkeps = (p.rx_events - pb.rx_events) * 1000 / usecs;
    uint64_t oMbps = 8*(o.tx_bytes - ob.tx_bytes) / usecs;

    printf(" | I - %12" PRIu64 " B, %8" PRIu64 " pkt/s, %5" PRIu64 " Mb/s" \
           " | P - %5" PRIu64 " kev/s" \
           " | O - %5" PRIu64 " Mb/s"
           , i.rx_bytes , ipps , iMbps , pkeps , oMbps);
  }

  /** @todo comment */
  void event_stats() {
    auto usecs = usecs_elapsed.timeus();
    uint64_t pkeps = (p.rx_events - pb.rx_events) * 1000 / usecs;

    printf(" | I - %12" PRIu64 " pkts" \
           " | P - %12" PRIu64 " events, %8" PRIu64 " kev/s, %12" PRIu64 " discards, %12" PRIu64 " errors" \
           , i.rx_packets, p.rx_events , pkeps , p.rx_discards, p.rx_errors);
  }

  /** @todo comment */
  void fifo_stats() {
    printf(" | I - %12" PRIu64 " fifo free" \
           " | P - %12" PRIu64 " fifo free"
           , i.fifo_free, p.fifo_free);
  }

public:

  /** @todo comment */
  Stats() { clear(); }


  void report(unsigned int mask) {
    if (mask)
      printf("%5" PRIu64 , time.timeus()/1000000);
    if (mask & 0x01)
      packet_stats();

    if (mask & 0x02)
      event_stats();

    if (mask & 0x04)
      fifo_stats();

    if (mask)
      printf("\n");
    reset();
  }

  typedef struct {
    // Counters
    uint64_t rx_packets;
    uint64_t rx_bytes;
    uint64_t push_errors;
    uint64_t  fifo_free;
  } input_stat_t;

  typedef struct {
    // Counters
    uint64_t rx_events;
    uint64_t rx_errors;
    uint64_t rx_discards;
    uint64_t idle;
    uint64_t push_errors;
    uint64_t  fifo_free;
  } processing_stat_t;

  typedef struct {
    // Counters
    uint64_t rx_events;
    uint64_t idle;
    uint64_t tx_bytes;
  } output_stat_t;
 
  input_stat_t i, ib;
  processing_stat_t p, pb;
  output_stat_t o, ob;
};

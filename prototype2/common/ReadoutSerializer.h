/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief flatbuffer serialization
 */

#pragma once

#include "mo01_nmx_generated.h"
#include <common/Producer.h>

class ReadoutSerializer {
public:
  /** @brief Create the ReadoutSerializer
   * @param maxentries the number of readout tuples to buffer before sending to Kafka
   * @param producer Kafka producer to use for publishing the readout data
   */
  ReadoutSerializer(size_t maxentries, Producer &producer);

  /** @brief empty destructor */
  ~ReadoutSerializer();

  /** @brief function to add a readout tuple to an array for later publishing to Kafka
   * @param plane arbitrary data multiplexer (x, y, module, rack, ...)
   * @param channel datasource identifier (strip, wire, grid, ...)
   * @param time timestamp
   * @param adc digitizer adc value (charge, voltage, ...)
   */
  int addEntry(uint16_t plane, uint16_t channel, uint32_t time, uint16_t adc);

  /** @brief publish data to Kafka broker and clear internal counters */
  int produce();

  /** @brief return the number of queues samples */
  size_t getNumEntries(){return entries;};

private:
  size_t maxlen{0}; /**< maximum number of entries in array */
  flatbuffers::FlatBufferBuilder builder; /**< google flatbuffer builder */
  Producer &producer; /**< wrapper for Kafka producer */

  // Will be used to create MONHit
  size_t entries{0}; /**< current number of queues etries */
  std::vector<uint16_t> planes; /**< described above  */
  std::vector<uint32_t> times; /**< described above */
  std::vector<uint16_t> channels; /**< described above */
  std::vector<uint16_t> adcs; /**< described above */
};

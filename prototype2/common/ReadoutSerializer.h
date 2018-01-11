/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief flatbuffer serialization
 */

#pragma once

#ifdef FLATBUFFERS
#include <../streaming-data-types/build/schemas/mo01_nmx_generated.h>
#else
#pragma message("FLATBUFFERS not defined, using old schemas")
#include <common/mo01_nmx_generated.h>
#endif
#include <common/Producer.h>

class ReadoutSerializer {
public:
  /** @todo document */
  ReadoutSerializer(size_t maxarraylength, Producer &prod);

  /** @todo document */
  ~ReadoutSerializer();

  /** @todo document */
  int addEntry(uint16_t plane, uint16_t channel, uint32_t time, uint16_t adc);

  /** @todo document */
  int produce();

  size_t getNumEntries(){return entries;};

private:
  size_t maxlen{0};
  flatbuffers::FlatBufferBuilder builder;
  Producer &producer;

  // Will be used to create MONHit
  size_t entries{0};
  std::vector<uint16_t> planes;
  std::vector<uint32_t> times;
  std::vector<uint16_t> channels;
  std::vector<uint16_t> adcs;
};

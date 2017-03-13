/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief flatbuffer serialization
 */

#pragma once

#include <../monitors/schemas/mon_efu_generated.h>
#include <common/Producer.h>
#include <libs/include/gccintel.h>

class HistSerializer {
public:

  /** @todo document */
  HistSerializer(size_t maxarraylength);

  /** @todo document */
  ~HistSerializer();

  /** @todo document */
  int serialize(uint32_t * xhist, uint32_t * yhist, size_t entries, char **buffer);

private:
  flatbuffers::FlatBufferBuilder builder;
  size_t maxlen{0};
  uint8_t *xarrptr{nullptr};
  uint8_t *yarrptr{nullptr};
};

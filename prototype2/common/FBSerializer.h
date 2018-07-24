/// Copyright (C) 2016-2018 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief flatbuffer serialization
///
//===----------------------------------------------------------------------===//

#pragma once

#include "ev42_events_generated.h"
#include <common/Producer.h>

class FBSerializer {
public:
  /** @todo document */
  FBSerializer(size_t maxarraylength, Producer &prod);

  /** @todo document */
  ~FBSerializer();

  /** @todo document */
  int serialize(uint64_t time, uint64_t seqno, size_t entries, char **buffer);

  /** @todo document */
  int addevent(uint32_t time, uint32_t pixel);

  /** @todo document */
  int produce();

private:
  flatbuffers::FlatBufferBuilder builder;
  size_t maxlen{0};
  uint8_t *timeptr{nullptr};
  uint8_t *pixelptr{nullptr};

  EventMessage *eventMsg;

  char *fbBufferPointer = nullptr;
  size_t fbSize = 0;
  flatbuffers::uoffset_t *timeLenPtr;
  flatbuffers::uoffset_t *pixelLenPtr;

  Producer &producer;

  size_t events{0};
  size_t seqno{1};
};

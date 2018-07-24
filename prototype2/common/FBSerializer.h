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

//TODO: rename this
class FBSerializer {
public:
  //TODO: source name as param
  //TODO: repalce producer with functor for sending off
  /** \todo document */
  FBSerializer(size_t maxarraylength, Producer &prod);

  //TODO: more getters

  /** \todo document */
  void set_pulse_time(uint64_t time);

  /** \todo document */
  uint64_t get_pulse_time() const;

  //TODO: make private
  /** \todo document */
  size_t serialize(size_t entries, char **buffer);

  /** \todo document */
  size_t addevent(uint32_t time, uint32_t pixel);

  /** \todo document */
  size_t produce();

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
  //TODO make 0
  uint64_t seqno{1};
};

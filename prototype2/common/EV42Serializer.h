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
#include <common/Buffer.h>
#include <functional>

class EV42Serializer {
public:
  /** \todo document */
  EV42Serializer(size_t max_array_length, std::string source_name);

  void set_callback(std::function<void(Buffer)> cb);

  /** \todo document */
  void set_pulse_time(uint64_t time);

  /** \todo document */
  uint64_t get_pulse_time() const;

  /** \todo document */
  size_t addevent(uint32_t time, uint32_t pixel);

  /** \todo document */
  size_t events() const;

  /** \todo document */
  uint64_t current_message_id() const;

  /** \todo document */
  size_t produce();

  //TODO: make private
  /** \todo document */
  Buffer serialize();

private:
  // \todo should this not be predefined in terms of jumbo frame?
  size_t max_events_{0};
  size_t events_{0};

  // \todo maybe should be mutated directly in buffer?
  uint64_t message_id_{1};

  std::function<void(Buffer)> callback;

  // All of this is the flatbuffer
  flatbuffers::FlatBufferBuilder builder;
  uint8_t *timeptr{nullptr};
  uint8_t *pixelptr{nullptr};

  EventMessage *eventMsg;

  char *fbBufferPointer {nullptr};
  size_t fbSize {0};
  flatbuffers::uoffset_t *timeLenPtr;
  flatbuffers::uoffset_t *pixelLenPtr;
};

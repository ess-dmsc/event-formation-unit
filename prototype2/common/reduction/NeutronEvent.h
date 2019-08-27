/** Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file **/
//===----------------------------------------------------------------------===//
///
/// \file NeutronEvent.h
/// \brief NeutronEvent class definitions
///
//===----------------------------------------------------------------------===//

#pragma once

/// \class NeutronEvent NeutronEvent.h
/// \brief The final reduced tuple for event formation -- (time, pixel_id)
///        This struct is intended for queueing up and sorting before
///        subtracting the most recent pulse time serializing them for
///        transmission. This is the most atomic piece of information that
///        can come out of an event-processing pipeline.

#include <cstdint>
#include <string>

struct NeutronEvent {
  uint64_t time;
  uint32_t pixel_id;

  std::string to_string() const;
};


/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file NeutronEvent.h
/// \brief NeutronEvent class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>
#include <string>

struct NeutronEvent {
  uint64_t time;
  uint32_t pixel_id;

  std::string to_string() const;
};


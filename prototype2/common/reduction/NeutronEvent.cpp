/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file NeutronEvent.h
/// \brief NeutronEvent class implementation
///
//===----------------------------------------------------------------------===//

#include <common/reduction/NeutronEvent.h>
#include <fmt/format.h>

std::string NeutronEvent::to_string() const {
  return fmt::format("t:{:>20}  pix:{:>10}", time, pixel_id);
}

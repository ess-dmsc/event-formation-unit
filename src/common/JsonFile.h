// Copyright (C) 2019-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief reads and writes JSON files to/from nlohmann types
///
/// See https://nlohmann.github.io/json/doxygen/index.html
//===----------------------------------------------------------------------===//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <nlohmann/json.hpp>
#pragma GCC diagnostic pop
#include <fmt/format.h>
#include <fstream>

inline nlohmann::json from_json_file(const std::string& fname)
{
  nlohmann::json j;
  std::ifstream ifs(fname, std::ofstream::in);
  if (ifs.fail()) {
    throw std::runtime_error(fmt::format("file permission error or missing json file {}", fname));
  }
  if (ifs.good())
    ifs >> j;

  return j;
}

inline void to_json_file(const nlohmann::json& j, const std::string& fname)
{
  std::ofstream(fname, std::ofstream::trunc) << j.dump(1);
}

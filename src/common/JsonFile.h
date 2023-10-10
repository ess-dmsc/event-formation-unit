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

inline nlohmann::json from_json_file(const std::string &fname) {
  nlohmann::json j;
  std::ifstream ifs(fname, std::ofstream::in);
  if (ifs.fail()) {
    throw std::runtime_error(
        fmt::format("file permission error or missing json file {}", fname));
  }
  if (ifs.good())
    ifs >> j;

  return j;
}

inline void to_json_file(const nlohmann::json &j, const std::string &fname) {
  std::ofstream(fname, std::ofstream::trunc) << j.dump(1);
}


inline void json_change_key(nlohmann::json &object, const std::string& old_key, const std::string& new_key) {
    // get iterator to old key; TODO: error handling if key is not present
    nlohmann::json::iterator it = object.find(old_key);
    // create null value for new key and swap value from old key
    std::swap(object[new_key], it.value());
    // delete value at old key (cheap, because the value is null after swap)
    object.erase(it);
}

/// \brief Given a nlohmann json object and a list of known required fields
/// return a string with the missing fields.
inline void json_check_keys(std::string Prefix, nlohmann::json &object,
  std::vector<std::string> RequiredFields) {
  std::string Missing{""};

  for (const auto & Field : RequiredFields) {
    if (not object.contains(Field)) {
      if (Missing.size() != 0) {
        Missing += " ";
      }
      Missing += Field;
    }
  }
  if (Missing.size() != 0) {
    std::string ErrMsg = fmt::format("{}: missing mandatory keys - {}", Prefix, Missing);
    throw std::runtime_error(ErrMsg);
  }
}

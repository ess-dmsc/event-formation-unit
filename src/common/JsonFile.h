// Copyright (C) 2019 - 2025 European Spallation Source, ERIC. See LICENSE file
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


namespace Json
{

/// \brief Load and return a json object from a file
/// \param fname  The name of the json file
///
/// \return the loaded json object
inline nlohmann::json fromFile(const std::string &fname) {
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

/// \brief Store a json object on disk
/// \param object  The json object
/// \param fname   The name of the json file
inline void toFile(const nlohmann::json &object, const std::string &fname) {
  std::ofstream(fname, std::ofstream::trunc) << object.dump(1);
}

/// \brief Change the name of key in a json object
/// \param object   The json object
/// \param old_key  The current name of the key
/// \param new_key  The new key name
inline void changeKey(nlohmann::json &object, const std::string& old_key, const std::string& new_key) {
    /// Get iterator to old key
    /// \todo error handling if key is not present
    nlohmann::json::iterator it = object.find(old_key);
    // create null value for new key and swap value from old key
    std::swap(object[new_key], it.value());
    // delete value at old key (cheap, because the value is null after swap)
    object.erase(it);
}

/// \brief Given a json object and a list of known required fields, return a
///        string with the missing fields.
/// \param Prefix  Prefix used by all field keys
/// \param object  The json object
/// \param RequiredFields  List of required fields
inline void checkKeys(const std::string &Prefix, const nlohmann::json &object,
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

} // namespace name

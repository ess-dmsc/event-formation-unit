#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <nlohmann/json.hpp>
#pragma GCC diagnostic pop
#include <fstream>

inline nlohmann::json from_json_file(const std::string& fname)
{
  nlohmann::json j;
  std::ifstream ifs(fname, std::ofstream::in);
  if (ifs.good())
    ifs >> j;
  return j;
}

inline void to_json_file(const nlohmann::json& j, const std::string& fname)
{
  std::ofstream(fname, std::ofstream::trunc) << j.dump(1);
}

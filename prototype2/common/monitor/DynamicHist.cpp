/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/monitor/DynamicHist.h>
#include <fmt/format.h>
#include <sstream>

std::string DynamicHist::debug() const {
  std::stringstream ss;
  for (size_t i = 0; i < hist.size(); ++i)
    ss << "[" << i << "]=" << hist[i];
  return ss.str();
}

std::string DynamicHist::visualize(bool non_empty_only) const {
  if (hist.empty())
    return {};

  size_t vmax{hist[0]};
  size_t start{0}, end{0};
  bool print{false};
  for (uint32_t i = 0; i < hist.size(); i++) {
    const auto &val = hist[i];
    vmax = std::max(vmax, val);
    if (val > 0) {
      end = i;
      if (!print) {
        start = i;
        print = true;
      }
    }
  }

  std::string largesti = fmt::format("{}", end);
  std::string pad = "{:<" + fmt::format("{}", largesti.size()) + "}";

  // \todo parametrize this
  size_t nstars{60};

  std::stringstream ss;
  for (size_t i = start; i <= end; i++) {
    auto val = hist[i];
    if (!non_empty_only || (val > 0))
      ss << fmt::format(pad, i) << ": "
         << fmt::format("{:<62}", std::string((nstars * val) / vmax, '*'))
         << val << "\n";
  }
  return ss.str();
}
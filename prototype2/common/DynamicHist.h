/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#pragma once
#include <string>
#include <vector>

class DynamicHist {
public:
  std::vector<size_t> hist;

  inline void bin(size_t i) {
    if (hist.size() <= i)
      hist.resize(i + 1, 0);
    hist[i]++;
  }

  inline bool empty() const {
    return hist.empty();
  }

  std::string debug() const;
  std::string visualize(bool non_empty_only = false) const;
};

// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Maintain and update runtime status
///
/// Should be run periodically (every N seconds)
//===----------------------------------------------------------------------===//

#include <common/Trace.h>
#include <cstdint>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

class RuntimeStat {
public:

  RuntimeStat(std::vector<int64_t> Counters) :
     OldStats(Counters) {
       if (Counters.size() == 0) {
         throw std::runtime_error("RuntimeStat array must be nonzero");
       }
  }

  /// \brief Get  vector of current counters and compare with old values
  /// if the values change the 'stage' is deemed 'up' and the corresponding
  /// bit is set.
  uint32_t getRuntimeStatus(std::vector<int64_t> Stats) {
    uint32_t Status{0};
    if ((Stats.size() > 32) or (Stats.size() != OldStats.size())) {
      XTRACE(PROCESS, ERR, "Runtime stat size mismatch (%zu)", Stats.size());
      return 0;
    }

    for (int i = 0; i < (int)Stats.size(); i++) {
      if (Stats[i] - OldStats[i] != 0) {
        Status |= 1<<i; // Set stage flag
      }
      OldStats[i] = Stats[i];
    }
    return Status;
  }

private:
  std::vector<int64_t> OldStats;
};

// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Maintain and update runtime status
///
/// A runtime stat is a set of (maximum 32) status flags that can be set
/// or clear. Each flag indicates a good/bad status of a 'feature'
///
/// The runtime status query should be run periodically (every N seconds)
/// it can be used in the EFUs to report if a pipeline stage is running
/// At the moment acticity is determined by observing a change in counts
/// between each poll.
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <cstdint>
#include <stdexcept>
#include <vector>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

class RuntimeStat {
public:
  RuntimeStat(std::vector<int64_t> Counters) : OldStats(Counters) {
    if (Counters.size() == 0) {
      throw std::runtime_error("RuntimeStat array must have nonzero size");
    }
  }

  /// \brief Get  vector of current counters and compare with old values
  /// if the values change the 'stage' is deemed 'up' and the corresponding
  /// bit is set.
  uint32_t getRuntimeStatusMask(std::vector<int64_t> Stats) {
    uint32_t Status{0};
    if ((Stats.size() > 32) or (Stats.size() != OldStats.size())) {
      XTRACE(PROCESS, ERR, "Runtime stat size mismatch (%zu)", Stats.size());
      return 0;
    }

    for (int i = 0; i < (int)Stats.size(); i++) {
      if (Stats[i] - OldStats[i] != 0) {
        Status |= 1 << i; // Set stage flag
      }
      OldStats[i] = Stats[i];
    }
    return Status;
  }

private:
  std::vector<int64_t> OldStats;
};

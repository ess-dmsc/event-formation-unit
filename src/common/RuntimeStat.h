// Copyright (C) 2020-2025 European Spallation Source, ERIC. See LICENSE file
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
/// At the moment activity is determined by observing a change in counts
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
  /// \brief Constructor used to initialize the runtime status
  /// \param Counters vector of counters to be used for runtime status
  /// A copy of the counters is stored internally
  /// \note The size of the vector must be <= 32
  RuntimeStat(const std::vector<int64_t> &Counters) {
    checkCountersSize(Counters);
    OldStats = Counters;
  }

  /// \brief Move constructor used to initialize the runtime status
  /// \param Counters vector of counters to be used for runtime status
  /// Optimized to move rvalue reference (temporary object) to internal storage
  /// \note The size of the vector must be <= 32
  RuntimeStat(std::vector<int64_t> &&Counters) {
    checkCountersSize(Counters);
    OldStats = std::move(Counters);
  }

  /// \brief Get  vector of current counters and compare with old values
  /// if the values change the 'stage' is deemed 'up' and the corresponding
  /// bit is set.
  uint32_t getRuntimeStatusMask(const std::vector<int64_t> &Stats) {
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

  inline void checkCountersSize(const std::vector<int64_t> &Counters) {
    if (Counters.empty()) {
      throw std::runtime_error("RuntimeStat array must have nonzero size");
    }

    if (Counters.size() > 32) {
      throw std::runtime_error("RuntimeStat array must have size <= 32");
    }
  }
};

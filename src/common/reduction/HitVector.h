/** Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file **/
///===--------------------------------------------------------------------===///
///
/// \file HitVector.h
/// \brief HitVector alias and convenience functions
///
///===--------------------------------------------------------------------===///

#pragma once

#include <common/reduction/Hit.h>

struct HitAllocatorBase {
  static char *s_MemBegin;
  static char *s_MemEnd;
};

template <class T> struct HitAllocator : public HitAllocatorBase {
  typedef T value_type;

  HitAllocator() = default;
  template <class U> constexpr HitAllocator(const HitAllocator<U> &) noexcept {}

  T *allocate(std::size_t n) {
    char* p = s_MemBegin;
    s_MemBegin += n * sizeof(T);
    if (s_MemBegin < s_MemEnd)
      return (T*)p;
    //return nullptr;
    throw std::bad_alloc();
  }
  void deallocate(T *, std::size_t) noexcept { /* do nothing*/
  }
};

template <class T, class U>
bool operator==(const HitAllocator<T> &, const HitAllocator<U> &) {
  return true;
}
template <class T, class U>
bool operator!=(const HitAllocator<T> &, const HitAllocator<U> &) {
  return false;
}

using HitVector = std::vector<Hit, HitAllocator<Hit>>;
//using HitVector = std::vector<Hit>;

/// \brief convenience function for sorting Hits by increasing time
inline void sort_chronologically(HitVector &hits) {
  std::sort(hits.begin(), hits.end(),
            [](const Hit &hit1, const Hit &hit2) {
              return hit1.time < hit2.time;
            });
}

/// \brief convenience function for sorting Hits by increasing coordinate
inline void sort_by_increasing_coordinate(HitVector &hits) {
  std::sort(hits.begin(), hits.end(),
            [](const Hit &hit1, const Hit &hit2) {
              return hit1.coordinate < hit2.coordinate;
            });
}

/// \brief convenience function for sorting Hits by decreasing weight
inline void sort_by_decreasing_weight(HitVector &hits) {
  std::sort(hits.begin(), hits.end(),
            [](const Hit &hit1, const Hit &hit2) {
              return hit1.weight > hit2.weight;
            });
}

/// \brief convenience function for printing vector of Hits
std::string to_string(const HitVector &vec, const std::string &prepend);

/// \brief convenience function for printing vector of Hits
std::string visualize(const HitVector &vec, const std::string &prepend,
                      uint16_t max_columns = 0, size_t max_rows = 0);

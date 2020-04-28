/** Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file **/
///===--------------------------------------------------------------------===///
///
/// \file HitVector.h
/// \brief HitVector alias and convenience functions
///
///===--------------------------------------------------------------------===///

#pragma once

#include <common/PoolAllocator.h>
#include <common/reduction/Hit.h>

#define ENABLE_GREEDY_HIT_ALLOCATOR 0

struct GreedyHitStorage {
  static char *MemBegin;
  static char *MemEnd;
};

template <class T> struct GreedyHitAllocator {
  using value_type = T;

  GreedyHitAllocator() = default;
  template <class U>
  constexpr GreedyHitAllocator(const GreedyHitAllocator<U> &) noexcept {}

  T *allocate(std::size_t n) {
    RelAssertMsg (ENABLE_GREEDY_HIT_ALLOCATOR, "Remember to enable");
    char *p = GreedyHitStorage::MemBegin;
    GreedyHitStorage::MemBegin += n * sizeof(T);
    if (GreedyHitStorage::MemBegin < GreedyHitStorage::MemEnd)
      return (T *)p;
    throw std::bad_alloc();
  }
  void deallocate(T *, std::size_t) noexcept {}
};

template <class T, class U>
bool operator==(const GreedyHitAllocator<T> &, const GreedyHitAllocator<U> &) {
  return true;
}
template <class T, class U>
bool operator!=(const GreedyHitAllocator<T> &, const GreedyHitAllocator<U> &) {
  return false;
}

//-----------------------------------------------------------------------------

template <typename T, typename Alloc = std::allocator<T>> class MyVector {
public:
  typedef std::vector<T, Alloc> Vector;

  Vector Vec;

  typedef T value_type;
  typedef Alloc allocator_type;
  typedef typename Vector::iterator iterator;
  typedef typename Vector::const_iterator const_iterator;
  typedef typename Vector::reference reference;
  typedef typename Vector::const_reference const_reference;
  typedef typename Vector::size_type size_type;
  typedef typename Vector::difference_type difference_type;
  typedef typename Vector::pointer pointer;
  typedef typename Vector::const_pointer const_pointer;
  typedef typename Vector::reverse_iterator reverse_iterator;
  typedef typename Vector::const_reverse_iterator const_reverse_iterator;

  // 1024 is enough to avoid allocs in "bat" example.
  enum { MinReserveCount = 1024 };

  MyVector() { reserve(MinReserveCount); }
  MyVector(Alloc &alloc) : Vec(alloc) { reserve(MinReserveCount); }

  iterator begin() noexcept { return Vec.begin(); }
  const_iterator begin() const noexcept { return Vec.begin(); }
  iterator end() noexcept { return Vec.end(); }
  const_iterator end() const noexcept { return Vec.end(); }

  reverse_iterator rbegin() noexcept { return Vec.rbegin(); }
  const_reverse_iterator rbegin() const noexcept { return Vec.rbegin(); }
  reverse_iterator rend() noexcept { return Vec.rend(); }
  const_reverse_iterator rend() const noexcept { return Vec.rend(); }

  size_type size() const noexcept { return Vec.size(); }
  size_type max_size() const noexcept { return Vec.max_size(); }
  size_type capacity() const noexcept { return Vec.capacity(); }
  bool empty() const noexcept { return Vec.empty(); }

  void reserve(size_type n) {
    Vec.reserve(n > (size_type)MinReserveCount ? n
                                               : (size_type)MinReserveCount);
  }

  reference operator[](size_type n) { return Vec.operator[](n); }
  const_reference operator[](size_type n) const { return Vec.operator[](n); }

  reference front() { return Vec.front(); }
  const_reference front() const { return Vec.front(); }
  reference back() { return Vec.back(); }
  const_reference back() const { return Vec.back(); }

  value_type *data() noexcept { return Vec.data(); }
  const value_type *data() const noexcept { return Vec.data(); }

  void push_back(const value_type &x) { Vec.push_back(x); }

  void push_back(value_type &&x) { Vec.push_back(std::move(x)); }

  template <class... Args> void emplace_back(Args &&... args) {
    Vec.emplace_back(std::forward<Args>(args)...);
  }

  void pop_back() { Vec.pop_back(); }

  // TODO why are we using insert()?
  template <class InputIterator>
  iterator insert(const_iterator position, InputIterator first,
                  InputIterator last) {
    return Vec.insert(position, first, last);
  }

  // make sure we reserve enough, as clear() is (possibly) needed to re-use a
  // vector after it has been std::move'd
  void clear() noexcept {
    Vec.clear();
    reserve(MinReserveCount);
  }
  void resize(size_type sz) { Vec.resize(sz); }
  void resize(size_type sz, const value_type &c) { Vec.resize(sz, c); }
};

//-----------------------------------------------------------------------------

struct HitVectorStorage {
  enum : size_t { Bytes_1GB = 1024 * 1024 * 1024 };
  using AllocConfig =
      PoolAllocatorConfig<Hit, Bytes_1GB, MyVector<Hit>::MinReserveCount>;
  static AllocConfig::PoolType *Pool;
  static PoolAllocator<AllocConfig> Alloc;
  static std::size_t MaxAllocCount;
};

template <class T> struct HitVectorAllocator {
  using value_type = T;

  HitVectorAllocator() = default;
  template <class U>
  constexpr HitVectorAllocator(const HitVectorAllocator<U> &) noexcept {}

  T *allocate(std::size_t n) {
    /// \todo (mortenhs): This don't work when a vector is (default)
    // copy-constructed because it will only have the capacity of it's source
    // vector.
    if (0) {
      RelAssertMsg(n >= MyVector<Hit>::MinReserveCount,
                   "Reserve not properly called somewhere. Could be a move-"
                   "semantic issue.");
    }
    if (0 && n > HitVectorStorage::MaxAllocCount) {
      XTRACE(MAIN, CRI, "HitVector max size %zu", n);
      HitVectorStorage::MaxAllocCount = n;
    }

    return HitVectorStorage::Alloc.allocate(n);
  }
  void deallocate(T *p, std::size_t n) noexcept {
    HitVectorStorage::Alloc.deallocate(p, n);
  }
};

template <class T, class U>
bool operator==(const HitVectorAllocator<T> &, const HitVectorAllocator<U> &) {
  return true;
}
template <class T, class U>
bool operator!=(const HitVectorAllocator<T> &, const HitVectorAllocator<U> &) {
  return false;
}

//-----------------------------------------------------------------------------

// using HitVector = std::vector<Hit, GreedyHitAllocator<Hit>>;
// using HitVector = std::vector<Hit>;
// using HitVector = MyVector<Hit>;
// using HitVector = MyVector<Hit, GreedyHitAllocator<Hit>>;
using HitVector = MyVector<Hit, HitVectorAllocator<Hit>>;

/// \brief convenience function for sorting Hits by increasing time
inline void sort_chronologically(HitVector &hits) {
  std::sort(hits.begin(), hits.end(), [](const Hit &hit1, const Hit &hit2) {
    return hit1.time < hit2.time;
  });
}

/// \brief convenience function for sorting Hits by increasing coordinate
inline void sort_by_increasing_coordinate(HitVector &hits) {
  std::sort(hits.begin(), hits.end(), [](const Hit &hit1, const Hit &hit2) {
    return hit1.coordinate < hit2.coordinate;
  });
}

/// \brief convenience function for sorting Hits by decreasing weight
inline void sort_by_decreasing_weight(HitVector &hits) {
  std::sort(hits.begin(), hits.end(), [](const Hit &hit1, const Hit &hit2) {
    return hit1.weight > hit2.weight;
  });
}

/// \brief convenience function for printing vector of Hits
std::string to_string(const HitVector &vec, const std::string &prepend);

/// \brief convenience function for printing vector of Hits
std::string visualize(const HitVector &vec, const std::string &prepend,
                      uint16_t max_columns = 0, size_t max_rows = 0);

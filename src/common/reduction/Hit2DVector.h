// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
///===--------------------------------------------------------------------===///
///
/// \file Hit2DVector.h
/// \brief Hit2DVector alias and convenience functions
///
///===--------------------------------------------------------------------===///

#pragma once

#include <common/debug/Trace.h>
#include <common/memory/PoolAllocator.h>
#include <common/reduction/Hit2D.h>
#include <utility>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

#define ENABLE_GREEDY_HIT_ALLOCATOR 0

struct GreedyHit2DStorage {
  static char *MemBegin;
  static char *MemEnd;
};

template <class T> struct GreedyHit2DAllocator {
  using value_type = T;

  GreedyHit2DAllocator() = default;
  template <class U>
  constexpr GreedyHit2DAllocator(const GreedyHit2DAllocator<U> &) noexcept {}

  T *allocate(std::size_t n) {
    RelAssertMsg(ENABLE_GREEDY_HIT_ALLOCATOR, "Remember to enable");
    char *p = GreedyHit2DStorage::MemBegin;
    GreedyHit2DStorage::MemBegin += n * sizeof(T);
    if (GreedyHit2DStorage::MemBegin < GreedyHit2DStorage::MemEnd)
      return (T *)p;
    throw std::bad_alloc();
  }
  void deallocate(T *, std::size_t) noexcept {}
};

template <class T, class U>
bool operator==(const GreedyHit2DAllocator<T> &, const GreedyHit2DAllocator<U> &) {
  return true;
}
template <class T, class U>
bool operator!=(const GreedyHit2DAllocator<T> &, const GreedyHit2DAllocator<U> &) {
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

  void push_back(const value_type &x) {
    XTRACE(DATA, DEB, "pushing back constant value");
    Vec.push_back(std::move(x));
  }

  void push_back(value_type &&x) { Vec.push_back(std::move(x)); }

  template <class... Args> void emplace_back(Args &&...args) {
    Vec.emplace_back(std::forward<Args>(args)...);
  }

  void pop_back() { Vec.pop_back(); }

  /// \todo why are we using insert()?
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

struct Hit2DVectorStorage {
  enum : size_t { Bytes_1GB = 1024 * 1024 * 1024 };
  using AllocConfig =
      PoolAllocatorConfig<Hit2D, Bytes_1GB, MyVector<Hit2D>::MinReserveCount, false,
                          true>;
  static AllocConfig::PoolType *Pool;
  static PoolAllocator<AllocConfig> Alloc;
  static std::size_t MaxAllocCount;
};

template <class T> struct Hit2DVectorAllocator {
  using value_type = T;

  Hit2DVectorAllocator() = default;
  template <class U>
  constexpr Hit2DVectorAllocator(const Hit2DVectorAllocator<U> &) noexcept {}

  T *allocate(std::size_t n) {
    /// \todo (mortenhs): This don't work when a vector is (default)
    // copy-constructed because it will only have the capacity of it's source
    // vector.
    if (0) {
      RelAssertMsg(n >= MyVector<Hit2D>::MinReserveCount,
                   "Reserve not properly called somewhere. Could be a move-"
                   "semantic issue.");
    }
    if (0 && n > Hit2DVectorStorage::MaxAllocCount) {
      XTRACE(MAIN, CRI, "Hit2DVector max size %zu", n);
      Hit2DVectorStorage::MaxAllocCount = n;
    }

    return Hit2DVectorStorage::Alloc.allocate(n);
  }
  void deallocate(T *p, std::size_t n) noexcept {
    Hit2DVectorStorage::Alloc.deallocate(p, n);
  }
};

/// \todo Does not seem to be required
template <class T, class U>
bool operator==(const Hit2DVectorAllocator<T> &, const Hit2DVectorAllocator<U> &) {
  return true;
}

/// \todo don't know the logic behind this but it seems to be necessary for
/// the project to compile, yet is not used or at least does not have
/// any test coverage
template <class T, class U>
bool operator!=(const Hit2DVectorAllocator<T> &, const Hit2DVectorAllocator<U> &) {
  return false;
}

//-----------------------------------------------------------------------------

using Hit2DVector = MyVector<Hit2D, Hit2DVectorAllocator<Hit2D>>;

/// \brief convenience function for sorting Hit2Ds by increasing time
inline void sort_chronologically(Hit2DVector&& hits) {
  std::sort(hits.begin(), hits.end(), [](const Hit2D& hit1, const Hit2D& hit2) {
    return hit1.time < hit2.time;
  });
}

/// \brief convenience function for sorting Hit2Ds by increasing time
inline void sort_partial_chronologically(Hit2DVector& hits) {
  if(hits.size()>20){
    std::partial_sort(hits.begin(), hits.begin() + 20, hits.end(),
                    [](const Hit2D& hit1, const Hit2D& hit2) {
                      return hit1.time < hit2.time;
                    });
  }
  else{
    std::sort(hits.begin(), hits.end(),
                    [](const Hit2D& hit1, const Hit2D& hit2) {
                      return hit1.time < hit2.time;
                    });
  }
  
}

/// \brief convenience function for sorting Hits by increasing coordinate
inline void sortByIncreasingXCoordinate(Hit2DVector &hits) {
  std::sort(hits.begin(), hits.end(), [](const Hit2D &hit1, const Hit2D &hit2) {
    return hit1.x_coordinate < hit2.x_coordinate;
  });
}

/// \brief convenience function for sorting Hits by decreasing weight
inline void sort_by_decreasing_weight(Hit2DVector &hits) {
  std::sort(hits.begin(), hits.end(), [](const Hit2D &hit1, const Hit2D &hit2) {
    return hit1.weight > hit2.weight;
  });
}

/// \brief convenience function for printing vector of Hit2Ds
std::string to_string(const Hit2DVector &vec, const std::string &prepend);

/// \brief convenience function for printing vector of Hit2Ds
std::string visualize(const Hit2DVector &vec, const std::string &prepend,
                      uint16_t max_columns = 0, size_t max_rows = 0);

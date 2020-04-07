/** Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file **/
///===--------------------------------------------------------------------===///
///
/// \file HitVector.h
/// \brief HitVector alias and convenience functions
///
///===--------------------------------------------------------------------===///

#pragma once

#include <common/reduction/Hit.h>
#include <common/PoolAllocator.h>

// TODO rename to GreedyLinearAllocator
struct HitAllocatorBase {
  static char *s_MemBegin;
  static char *s_MemEnd;
};

template <class T> struct HitAllocator : public HitAllocatorBase {
  using value_type = T;

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

//-----------------------------------------------------------------------------

#include <algorithm> // TODO remove

template <typename T, typename Alloc = std::allocator<T>>
class MyVector {
public:
  typedef std::vector<T, Alloc> Vector;

  Vector m_Vec;

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

  enum { kMinReserveCount = 16 };

  MyVector() { reserve(kMinReserveCount); }
  MyVector(Alloc &alloc) : m_Vec(alloc) { reserve(kMinReserveCount); }

  iterator begin() noexcept { return m_Vec.begin(); }
  const_iterator begin() const noexcept { return m_Vec.begin(); }
  iterator end() noexcept { return m_Vec.end(); }
  const_iterator end() const noexcept { return m_Vec.end(); }

  reverse_iterator rbegin() noexcept { return m_Vec.rbegin(); }
  const_reverse_iterator rbegin() const noexcept { return m_Vec.rbegin(); }
  reverse_iterator rend() noexcept { return m_Vec.rend(); }
  const_reverse_iterator rend() const noexcept { return m_Vec.rend(); }

  size_type size() const noexcept { return m_Vec.size(); }
  size_type max_size() const noexcept { return m_Vec.max_size(); }
  size_type capacity() const noexcept { return m_Vec.capacity(); }
  bool empty() const noexcept { return m_Vec.empty(); }

  void reserve(size_type n) { m_Vec.reserve(std::max(n, (size_type)kMinReserveCount)); }

  reference operator[](size_type n) { return m_Vec.operator[](n); }
  const_reference operator[](size_type n) const { return m_Vec.operator[](n); }

  reference front() { return m_Vec.front(); }
  const_reference front() const { return m_Vec.front(); }
  reference back() { return m_Vec.back(); }
  const_reference back() const { return m_Vec.back(); }

  value_type *data() noexcept { return m_Vec.data(); }
  const value_type *data() const noexcept { return m_Vec.data(); }

  void push_back(const value_type &x) {
      m_Vec.push_back(x);
  }

  void push_back(value_type &&x) {
      m_Vec.push_back(std::move(x));
  }

  template <class... Args> void emplace_back(Args &&... args) {
      m_Vec.emplace_back(std::forward<Args>(args)...);
  }

  void pop_back() { m_Vec.pop_back(); }

  // TODO why are we using insert()?
  template <class InputIterator>
  iterator insert(const_iterator position, InputIterator first,
                  InputIterator last) {
    return m_Vec.insert(position, first, last);
  }

  void clear() noexcept { m_Vec.clear(); }
  void resize(size_type sz) { m_Vec.resize(sz); }
  void resize(size_type sz, const value_type &c) { m_Vec.resize(sz, c); }
};

//-----------------------------------------------------------------------------

struct HitVectorStorage {
  using PoolCfg =
      PoolAllocatorConfig<Hit, 1024 * 1024 * 1024, MyVector<Hit>::kMinReserveCount>;
  static PoolCfg::PoolType* s_Pool;
  static PoolAllocator<PoolCfg> s_Alloc;
};

template <class T> struct HitVectorAllocator : public HitVectorStorage {
  using value_type = T;

  HitVectorAllocator() = default;
  template <class U>
  constexpr HitVectorAllocator(const HitVectorAllocator<U> &) noexcept {}

  T *allocate(std::size_t n) { return s_Alloc.allocate(n); }
  void deallocate(T *p, std::size_t n) noexcept { s_Alloc.deallocate(p, n); }
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

//using HitVector = std::vector<Hit, HitAllocator<Hit>>;
//using HitVector = std::vector<Hit>;
//using HitVector = MyVector<Hit>;
//using HitVector = MyVector<Hit, HitAllocator<Hit>>;
using HitVector = MyVector<Hit, HitVectorAllocator<Hit>>;

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

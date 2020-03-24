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

#include <algorithm> // TODO remove

template <typename T, typename Alloc = std::allocator<T>>
class MyVector {
public:
  typedef std::vector<T, Alloc> Vector;

  Vector m_Vec;

  typedef T value_type;
  typedef Alloc allocator_type;
  typedef typename allocator_type::reference reference;
  typedef typename allocator_type::const_reference const_reference;
  typedef typename Vector::iterator iterator;
  typedef typename Vector::const_iterator const_iterator;
  typedef typename allocator_type::size_type size_type;
  typedef typename allocator_type::difference_type difference_type;
  typedef typename allocator_type::pointer pointer;
  typedef typename allocator_type::const_pointer const_pointer;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  //static const size_type kMinReserveCount = 16;
  enum { kMinReserveCount = 16 };

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
    if (m_Vec.capacity() > m_Vec.size()) {
      m_Vec.push_back(x);
    } else {
      m_Vec.reserve(m_Vec.size() + kMinReserveCount); // TODO growth policy
      m_Vec.push_back(x);
    }
  }

  void push_back(value_type &&x) {
    if (m_Vec.capacity() > m_Vec.size()) {
      m_Vec.push_back(std::move(x));
    } else {
      m_Vec.reserve(m_Vec.size() + kMinReserveCount); // TODO growth policy
      m_Vec.push_back(std::move(x));
    }
  }

  template <class... Args> void emplace_back(Args &&... args) {
    if (m_Vec.capacity() > m_Vec.size()) {
      m_Vec.emplace_back(std::forward<Args>(args)...);
    } else {
      m_Vec.reserve(m_Vec.size() + kMinReserveCount); // TODO growth policy
      m_Vec.emplace_back(std::forward<Args>(args)...);
    }
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

//using HitVector = std::vector<Hit, HitAllocator<Hit>>;
//using HitVector = std::vector<Hit>;
using HitVector = MyVector<Hit>;

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

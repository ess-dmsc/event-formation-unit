// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Thread-safe vector wrapper for Statistics class
///
/// This class provides a thread-safe wrapper around std::vector specifically
/// designed for the Statistics class usage patterns. It uses a shared_mutex
/// for reader-writer synchronization to allow multiple concurrent readers.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <mutex>
#include <shared_mutex>
#include <vector>
#include <functional>
#include <algorithm>
#include <type_traits>

/// \brief Thread-safe vector wrapper with reader-writer locking
/// \tparam T The type of elements stored in the vector
template<typename T>
class ThreadSafeVector {
public:
  using size_type = typename std::vector<T>::size_type;
  using const_reference = typename std::vector<T>::const_reference;
  using reference = typename std::vector<T>::reference;

  /// \brief Default constructor
  ThreadSafeVector() = default;

  /// \brief Copy constructor (deleted - cannot copy vectors containing references)
  ThreadSafeVector(const ThreadSafeVector&) = delete;

  /// \brief Copy assignment operator (deleted - cannot copy vectors containing references)
  ThreadSafeVector& operator=(const ThreadSafeVector&) = delete;

  /// \brief Move constructor (thread-safe)
  ThreadSafeVector(ThreadSafeVector&& other) noexcept {
    std::unique_lock<std::shared_mutex> lock(other.mutex_);
    data_ = std::move(other.data_);
  }

  /// \brief Move assignment operator (thread-safe)
  ThreadSafeVector& operator=(ThreadSafeVector&& other) noexcept {
    if (this != &other) {
      std::unique_lock<std::shared_mutex> lock1(mutex_);
      std::unique_lock<std::shared_mutex> lock2(other.mutex_);
      data_ = std::move(other.data_);
    }
    return *this;
  }

  /// \brief Get the size of the vector (thread-safe)
  /// \return The number of elements in the vector
  size_type size() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return data_.size();
  }

  /// \brief Check if the vector is empty (thread-safe)
  /// \return true if the vector is empty, false otherwise
  bool empty() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return data_.empty();
  }

  /// \brief Add an element to the end of the vector (thread-safe)
  /// \param value The value to add
  template<typename U>
  void push_back(U&& value) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    data_.push_back(std::forward<U>(value));
  }

  /// \brief Emplace an element at the end of the vector (thread-safe)
  /// \param args Arguments to construct the element
  template<typename... Args>
  void emplace_back(Args&&... args) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    data_.emplace_back(std::forward<Args>(args)...);
  }

  /// \brief Get a const reference to an element at the specified index (thread-safe)
  /// \param index The index of the element (0-based)
  /// \return Const reference to the element
  /// \throws std::out_of_range if index is invalid
  const_reference at(size_type index) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return data_.at(index);
  }

  /// \brief Get a reference to an element at the specified index (thread-safe)
  /// \param index The index of the element (0-based)
  /// \return Reference to the element
  /// \throws std::out_of_range if index is invalid
  reference at(size_type index) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return data_.at(index);
  }

  /// \brief Execute a function with read-only access to the entire vector
  /// \param func Function that takes const reference to the vector
  /// \return The result of the function call
  template<typename Func>
  auto withReadLock(Func&& func) const -> decltype(func(std::declval<const std::vector<T>&>())) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return func(data_);
  }

  /// \brief Execute a function with read-write access to the entire vector
  /// \param func Function that takes reference to the vector
  /// \return The result of the function call
  template<typename Func>
  auto withWriteLock(Func&& func) -> decltype(func(std::declval<std::vector<T>&>())) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return func(data_);
  }

  /// \brief Find an element that matches a predicate (thread-safe)
  /// \param predicate Function that returns true for matching elements
  /// \return Pointer to the matching element, or nullptr if not found
  template<typename Predicate>
  const T* find_if(Predicate&& predicate) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto it = std::find_if(data_.begin(), data_.end(), std::forward<Predicate>(predicate));
    return (it != data_.end()) ? &(*it) : nullptr;
  }

  /// \brief Check if any element matches a predicate (thread-safe)
  /// \param predicate Function that returns true for matching elements
  /// \return true if a matching element exists, false otherwise
  template<typename Predicate>
  bool any_of(Predicate&& predicate) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return std::any_of(data_.begin(), data_.end(), std::forward<Predicate>(predicate));
  }

private:
  mutable std::shared_mutex mutex_;  ///< Reader-writer mutex for thread safety
  std::vector<T> data_;              ///< The underlying vector
};
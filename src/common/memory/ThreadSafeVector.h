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
  ThreadSafeVector(ThreadSafeVector&& Other) noexcept {
    std::unique_lock<std::shared_mutex> Lock(Other.Mutex);
    Data = std::move(Other.Data);
  }

  /// \brief Move assignment operator (thread-safe)
  ThreadSafeVector& operator=(ThreadSafeVector&& Other) noexcept {
    if (this != &Other) {
      std::unique_lock<std::shared_mutex> Lock1(Mutex);
      std::unique_lock<std::shared_mutex> Lock2(Other.Mutex);
      Data = std::move(Other.Data);
    }
    return *this;
  }

  /// \brief Get the size of the vector (thread-safe)
  /// \return The number of elements in the vector
  size_type size() const {
    std::shared_lock<std::shared_mutex> Lock(Mutex);
    return Data.size();
  }

  /// \brief Check if the vector is empty (thread-safe)
  /// \return true if the vector is empty, false otherwise
  bool empty() const {
    std::shared_lock<std::shared_mutex> Lock(Mutex);
    return Data.empty();
  }

  /// \brief Add an element to the end of the vector (thread-safe)
  /// \param Value The value to add
  template<typename UType>
  void push_back(UType&& Value) {
    std::unique_lock<std::shared_mutex> Lock(Mutex);
    Data.push_back(std::forward<UType>(Value));
  }

  /// \brief Emplace an element at the end of the vector (thread-safe)
  /// \param Args Arguments to construct the element
  template<typename... ArgsT>
  void emplace_back(ArgsT&&... Args) {
    std::unique_lock<std::shared_mutex> Lock(Mutex);
    Data.emplace_back(std::forward<ArgsT>(Args)...);
  }

  /// \brief Get a const reference to an element at the specified index (thread-safe)
  /// \param Index The index of the element (0-based)
  /// \return Const reference to the element
  /// \throws std::out_of_range if Index is invalid
  const_reference at(size_type Index) const {
    std::shared_lock<std::shared_mutex> Lock(Mutex);
    return Data.at(Index);
  }

  /// \brief Get a reference to an element at the specified index (thread-safe)
  /// \param Index The index of the element (0-based)
  /// \return Reference to the element
  /// \throws std::out_of_range if Index is invalid
  reference at(size_type Index) {
    std::unique_lock<std::shared_mutex> Lock(Mutex);
    return Data.at(Index);
  }

  /// \brief Execute a function with read-only access to the entire vector
  /// \param Func Function that takes const reference to the vector
  /// \return The result of the function call
  template<typename FuncT>
  auto withReadLock(FuncT&& Func) const -> decltype(Func(std::declval<const std::vector<T>&>())) {
    std::shared_lock<std::shared_mutex> Lock(Mutex);
    return Func(Data);
  }

  /// \brief Execute a function with read-write access to the entire vector
  /// \param Func Function that takes reference to the vector
  /// \return The result of the function call
  template<typename FuncT>
  auto withWriteLock(FuncT&& Func) -> decltype(Func(std::declval<std::vector<T>&>())) {
    std::unique_lock<std::shared_mutex> Lock(Mutex);
    return Func(Data);
  }

  /// \brief Find an element that matches a predicate (thread-safe)
  /// \param Predicate Function that returns true for matching elements
  /// \return Pointer to the matching element, or nullptr if not found
  template<typename PredicateT>
  const T* find_if(PredicateT&& Predicate) const {
    std::shared_lock<std::shared_mutex> Lock(Mutex);
    auto It = std::find_if(Data.begin(), Data.end(), std::forward<PredicateT>(Predicate));
    return (It != Data.end()) ? &(*It) : nullptr;
  }

  /// \brief Check if any element matches a predicate (thread-safe)
  /// \param Predicate Function that returns true for matching elements
  /// \return true if a matching element exists, false otherwise
  template<typename PredicateT>
  bool any_of(PredicateT&& Predicate) const {
    std::shared_lock<std::shared_mutex> Lock(Mutex);
    return std::any_of(Data.begin(), Data.end(), std::forward<PredicateT>(Predicate));
  }

private:
  mutable std::shared_mutex Mutex;  ///< Reader-writer mutex for thread safety
  std::vector<T> Data;              ///< The underlying vector
};
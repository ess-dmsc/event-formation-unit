// Copyright (C) 2023-2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Simple implementation for the observer pattern with template
/// observable
//===----------------------------------------------------------------------===//

#pragma once

#include <algorithm>
#include <cstdint>
#include <locale>
#include <memory>
#include <vector>

namespace Observer {

/**
 * @brief A template class for observing data events.
 * 
 * This class defines a template for observing data events of type `DataEvent`.
 * Subclasses of this class must implement the `applyData` method to handle the data event.
 * 
 * @tparam DataEvent The type of data event to observe.
 */
template <typename DataEvent> class DataEventObserver {
public:
  /**
   * @brief Applies the data event.
   * 
   * This method is called when a data event occurs. Subclasses must implement this method
   * to handle the data event.
   * 
   * @param event The data event to apply.
   */
  virtual void applyData(const DataEvent &event) = 0;
};

/**
 * @brief Template class for an observable data event.
 * 
 * This class allows other classes to subscribe to and receive data events of type `DataEvent`.
 * It maintains a list of data event listeners and provides methods to subscribe and publish data events.
 * 
 * @tparam DataEvent The type of data event.
 */
template <typename DataEvent> class DataEventObservable {
private:
  std::vector<DataEventObserver<DataEvent> *> dataEventListeners;

public:
  /**
   * @brief Default constructor.
   */
  DataEventObservable<DataEvent>(){};

  /**
   * @brief Default destructor.
   */
  virtual ~DataEventObservable<DataEvent>() = default;

  /**
   * @brief Subscribes a data event listener.
   * 
   * Adds the specified listener to the list of data event listeners, if it is not already subscribed.
   * 
   * @param listener A pointer to the data event listener.
   */
  inline void subscribe(DataEventObserver<DataEvent> *listener) {

    // Only add listener if it is not already subscribed
    if (std::find(dataEventListeners.begin(), dataEventListeners.end(),
                  listener) == dataEventListeners.end()) {
      dataEventListeners.push_back(listener);
    }
  }

  /**
   * @brief Publishes a data event to all subscribed listeners.
   * 
   * Calls the `applyData` method of each subscribed listener, passing the specified data event.
   * 
   * @param event The data event to be published.
   */
  inline void publishData(const DataEvent &event) const {
    for (const auto &listener : dataEventListeners) {
      listener->applyData(event);
    }
  }
};

} // namespace Observer
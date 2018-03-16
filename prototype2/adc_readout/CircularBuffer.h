/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Circular buffer based on a lock less queue.
 */

#pragma once

#include "readerwriterqueue.h"
#include <cassert>
#include <memory>

namespace SpscBuffer {

/// @brief A dummy data deallocator. Used so that we can use a contigous set of memory in the buffer.
template <class DataType> struct NopDeleter {
  void operator()(DataType *Ptr __attribute__((unused))) const {}
};

template <class DataType>
using ElementPtr = std::unique_ptr<DataType, NopDeleter<DataType>>;

/// @brief Sets the name of the queue type in use to Queue. Note that using the non-blocking version, (i.e. setting SPSC_NO_WAIT) is MUCH(!) faster than the blocking version.
template <class DataType>
#ifdef SPSC_NO_WAIT
using Queue = moodycamel::ReaderWriterQueue<ElementPtr<DataType>>;
#else
using Queue = moodycamel::BlockingReaderWriterQueue<ElementPtr<DataType>>;
#endif

/// @brief Single producer, single consumer lock less data buffer/queue.
/// This class actually implements two different queues; one for elements containing data and one for empty ones.
/// You must always process the elements (and return them to the correct queue) in the order that you have received them.
template <class DataType> class CircularBuffer {
public:
  /// @param[in] Elements Number of elements in the buffer/queue.
  CircularBuffer(std::int32_t Elements)
      : DataBuffer(new DataType[Elements]), DataQueue(Elements),
        EmptyQueue(Elements) {
    for (int i = 0; i < Elements; i++) {
      EmptyQueue.enqueue(ElementPtr<DataType>(&DataBuffer[i]));
    }
#ifndef NDEBUG
    ProducerElemPtr = nullptr;
    ConsumerElemPtr = nullptr;
#endif
  }
  /// @brief Get empty element that can then be used to store data.
  /// @param[out] Element The pointer to the element.
  /// @return Returns true if the pointer can be used to access an empty element, false otherwise.
  bool tryGetEmpty(ElementPtr<DataType> &Element) {
    bool Success = EmptyQueue.try_dequeue(Element);
#ifndef NDEBUG
    if (Success and ProducerElemPtr != nullptr) {
      assert(false);
    } else {
      ProducerElemPtr = Element.get();
    }
#endif
    return Success;
  };

#ifndef SPSC_NO_WAIT
  /// @brief Same as CircularBuffer::tryGetEmpty() but blocks for a specific amount of time.
  /// @param[out] Element The pointer to the element.
  /// @param[in] TimeoutUSecs The amount of time to wait for an empty element before failing.
  /// @return Returns true if the pointer can be used to access an empty element, false otherwise.
  bool waitGetEmpty(ElementPtr<DataType> &Element, std::int64_t TimeoutUSecs) {
    bool Success = EmptyQueue.wait_dequeue_timed(Element, TimeoutUSecs);
#ifndef NDEBUG
    if (Success and ProducerElemPtr != nullptr) {
      assert(false);
    } else {
      ProducerElemPtr = Element.get();
    }
#endif
    return Success;
  };
#endif
  
  /// @brief Put non empty element in the data queue.
  /// @param[in] Element The pointer to the element.
  /// @return Returns true if the element was put in the queue, false otherwise. If false is returned, you are then still responsible for the pointer.
  bool tryPutData(ElementPtr<DataType> &&Element) {
#ifndef NDEBUG
    DataType *TempPtr = Element.get();
#endif
    bool Success =
        DataQueue.try_enqueue(std::forward<ElementPtr<DataType>>(Element));
#ifndef NDEBUG
    if (Success and ProducerElemPtr != TempPtr) {
      assert(false);
    } else {
      ProducerElemPtr = nullptr;
    }
#endif
    return Success;
  }
  // End producer

  /// @brief Get data element.
  /// @param[out] Element The pointer to the element.
  /// @return Returns true if the pointer can be used to access a data element, false otherwise.
  bool tryGetData(ElementPtr<DataType> &Element) {
    bool Success = DataQueue.try_dequeue(Element);
#ifndef NDEBUG
    if (Success and ConsumerElemPtr != nullptr) {
      assert(false);
    } else {
      ConsumerElemPtr = Element.get();
    }
#endif
    return Success;
  }

#ifndef SPSC_NO_WAIT
  /// @brief Blocking version of CircularBuffer::tryGetData().
  /// @param[out] Element The pointer to the element.
  /// @param[in] TimeoutUSecs The number of micro seconds to block (wait) for new data before failing (returning false).
  /// @return Returns true if the pointer can be used to access a data element, false otherwise.
  bool waitGetData(ElementPtr<DataType> &Element, std::int64_t TimeoutUSecs) {
    bool Success = DataQueue.wait_dequeue_timed(Element, TimeoutUSecs);
#ifndef NDEBUG
    if (Success and ConsumerElemPtr != nullptr) {
      assert(false);
    } else {
      ConsumerElemPtr = Element.get();
    }
#endif
    return Success;
  }
#endif
  
  /// @brief Put empty (processed) element back in the empty queue.
  /// @param[in] Element The pointer to the element.
  /// @return Returns true if the element was put in the empty queue, false otherwise.
  bool tryPutEmpty(ElementPtr<DataType> &&Element) {
#ifndef NDEBUG
    DataType *TempPtr = Element.get();
#endif
    bool Success =
        EmptyQueue.try_enqueue(std::forward<ElementPtr<DataType>>(Element));
#ifndef NDEBUG
    if (Success and ConsumerElemPtr != TempPtr) {
      assert(false);
    } else {
      ConsumerElemPtr = nullptr;
    }
#endif
    return Success;
  }
  // End consumer
private:
#ifndef NDEBUG
  DataType *ProducerElemPtr;
  DataType *ConsumerElemPtr;
#endif
  std::unique_ptr<DataType[]> DataBuffer;
  Queue<DataType> DataQueue;
  Queue<DataType> EmptyQueue;
};
} // namespace SpscBuffer

#pragma once

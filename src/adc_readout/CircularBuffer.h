/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Circular buffer based on a lock less queue.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cassert>
#include <memory>
#include <readerwriterqueue/readerwriterqueue.h>

namespace SpscBuffer {

template <class DataType> using ElementPtr = DataType *;

/// \brief Sets the name of the queue type in use to Queue. Note that using the
/// non-blocking version, (i.e. setting SPSC_NO_WAIT) is MUCH(!) faster than the
/// blocking version.
template <class DataType>
#ifdef SPSC_NO_WAIT
using Queue = moodycamel::ReaderWriterQueue<ElementPtr<DataType>>;
#else
using Queue = moodycamel::BlockingReaderWriterQueue<ElementPtr<DataType>>;
#endif

/// \brief Single producer, single consumer lock less data buffer/queue.
/// This class actually implements two different queues; one for elements
/// containing data and one for empty ones.
/// You must always process the elements (and return them to the correct queue)
/// in the order that you have received them.
template <class DataType> class CircularBuffer {
public:
  /// \param[in] Elements Number of elements in the buffer/queue.
  explicit CircularBuffer(std::int32_t Elements)
      : DataBuffer(new DataType[Elements]), DataQueue(Elements),
        EmptyQueue(Elements) {
    for (int i = 0; i < Elements; i++) {
      EmptyQueue.enqueue(ElementPtr<DataType>(&DataBuffer[i]));
    }
  }
  /// \brief Get empty element that can then be used to store data.
  /// \param[out] Element The pointer to the element.
  /// \return Returns true if the pointer can be used to access an empty
  /// element, false otherwise.
  bool tryGetEmpty(ElementPtr<DataType> &Element) {
    bool Success = EmptyQueue.try_dequeue(Element);
#ifndef NDEBUG
    if (Success) {
      assert(ProducerElemPtr == nullptr);
      ProducerElemPtr = Element;
    }
#endif
    return Success;
  };

#ifndef SPSC_NO_WAIT
  /// \brief Same as CircularBuffer::tryGetEmpty() but blocks for a specific
  /// amount of time.
  /// \param[out] Element The pointer to the element.
  /// \param[in] TimeoutUSecs The amount of time to wait for an empty element
  /// before failing.
  /// \return Returns true if the pointer can be used to access an empty
  /// element, false otherwise.
  bool waitGetEmpty(ElementPtr<DataType> &Element, std::int64_t TimeoutUSecs) {
    bool Success = EmptyQueue.wait_dequeue_timed(Element, TimeoutUSecs);
#ifndef NDEBUG
    if (Success) {
      assert(ProducerElemPtr == nullptr);
      ProducerElemPtr = Element;
    }
#endif
    return Success;
  };
#endif

  /// \brief Put non empty element in the data queue.
  /// \param[in] Element The pointer to the element.
  /// \return Returns true if the element was put in the queue, false otherwise.
  /// If false is returned, you are then still responsible for the pointer.
  bool tryPutData(ElementPtr<DataType> &&Element) {
#ifndef NDEBUG
    DataType *TempPtr = Element;
#endif
    bool Success =
        DataQueue.try_enqueue(std::forward<ElementPtr<DataType>>(Element));
#ifndef NDEBUG
    if (Success) {
      assert(ProducerElemPtr == TempPtr);
      ProducerElemPtr = nullptr;
    }
#endif
    return Success;
  }
  // End producer

  /// \brief Get data element.
  /// \param[out] Element The pointer to the element.
  /// \return Returns true if the pointer can be used to access a data element,
  /// false otherwise.
  bool tryGetData(ElementPtr<DataType> &Element) {
    bool Success = DataQueue.try_dequeue(Element);
#ifndef NDEBUG
    if (Success) {
      assert(ConsumerElemPtr == nullptr);
      ConsumerElemPtr = Element;
    }
#endif
    return Success;
  }

#ifndef SPSC_NO_WAIT
  /// \brief Blocking version of CircularBuffer::tryGetData().
  /// \param[out] Element The pointer to the element.
  /// \param[in] TimeoutUSecs The number of micro seconds to block (wait) for
  /// new data before failing (returning false).
  /// \return Returns true if the pointer can be used to access a data element,
  /// false otherwise.
  bool waitGetData(ElementPtr<DataType> &Element, std::int64_t TimeoutUSecs) {
    bool Success = DataQueue.wait_dequeue_timed(Element, TimeoutUSecs);
#ifndef NDEBUG
    if (Success) {
      assert(ConsumerElemPtr == nullptr);
      ConsumerElemPtr = Element;
    }
#endif
    return Success;
  }
#endif

  /// \brief Put empty (processed) element back in the empty queue.
  /// \param[in] Element The pointer to the element.
  /// \return Returns true if the element was put in the empty queue, false
  /// otherwise.
  bool tryPutEmpty(ElementPtr<DataType> &&Element) {
#ifndef NDEBUG
    DataType *TempPtr = Element;
#endif
    bool Success =
        EmptyQueue.try_enqueue(std::forward<ElementPtr<DataType>>(Element));
#ifndef NDEBUG
    if (Success) {
      assert(ConsumerElemPtr == TempPtr);
      ConsumerElemPtr = nullptr;
    }
#endif
    return Success;
  }
  // End consumer
private:
#ifndef NDEBUG
  DataType *ProducerElemPtr{nullptr};
  DataType *ConsumerElemPtr{nullptr};
#endif
  std::unique_ptr<DataType[]> DataBuffer;
  Queue<DataType> DataQueue;
  Queue<DataType> EmptyQueue;
};
} // namespace SpscBuffer

#pragma once

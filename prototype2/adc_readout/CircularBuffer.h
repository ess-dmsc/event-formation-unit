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

template <class DataType> struct NopDeleter {
  void operator()(DataType *Ptr __attribute__((unused))) const {}
};

template <class DataType>
using ElementPtr = std::unique_ptr<DataType, NopDeleter<DataType>>;

// Note, none blocking operation is MUCH faster
template <class DataType>
#ifdef SPSC_NO_WAIT
using Queue = moodycamel::ReaderWriterQueue<ElementPtr<DataType>>;
#else
using Queue = moodycamel::BlockingReaderWriterQueue<ElementPtr<DataType>>;
#endif

template <class DataType> class CircularBuffer {
public:
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
  // Producer
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

  // Consumer
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

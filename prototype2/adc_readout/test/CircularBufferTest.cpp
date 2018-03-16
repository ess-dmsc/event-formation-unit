/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Unit tests.
 */

#include <gtest/gtest.h>
#include <chrono>
#include <random>
#include "../CircularBuffer.h"
#include <thread>
#ifdef __linux__
#include <pthread.h>
#endif

using namespace SpscBuffer;

TEST(GetEmpty, GetSingle) {
  ElementPtr<int> SomePtr(nullptr);
  CircularBuffer<int> SomeBuffer(10);
  EXPECT_TRUE(SomeBuffer.tryGetEmpty(SomePtr));
  EXPECT_NE(SomePtr.get(), nullptr);
}

TEST(GetEmpty, GetMultiple) {
  ElementPtr<int> SomePtr(nullptr);
  int Elements = 10;
  CircularBuffer<int> SomeBuffer(Elements);
  for (int y = 0; y < Elements; y++) {
    EXPECT_TRUE(SomeBuffer.tryGetEmpty(SomePtr));
    EXPECT_NE(SomePtr.get(), nullptr);
    SomeBuffer.tryPutData(std::move(SomePtr));
    SomePtr.reset();
  }
  SomePtr.reset();
  EXPECT_FALSE(SomeBuffer.tryGetEmpty(SomePtr));
  EXPECT_EQ(SomePtr.get(), nullptr);
}

#if 0
//These tests take too long
TEST(GetEmpty, WaitGetSingle) {
  ElementPtr<int> somePtr(nullptr);
  CircularBuffer<int> someBuffer(10);
  EXPECT_TRUE(someBuffer.wait_get_empty(somePtr, 1000));
  EXPECT_NE(somePtr.get(), nullptr);
}

TEST(GetEmpty, WaitGetMultiple) {
  ElementPtr<int> somePtr(nullptr);
  int elements = 10;
  CircularBuffer<int> someBuffer(elements);
  for (int y = 0; y < elements; y++) {
    EXPECT_TRUE(someBuffer.tryGetEmpty(somePtr));
    EXPECT_NE(somePtr.get(), nullptr);
    someBuffer.tryPutData(std::move(somePtr));
    somePtr.reset();
  }
  somePtr.reset();
  std::int64_t uSecTimeout = 1000;
  std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
  EXPECT_FALSE(someBuffer.wait_get_empty(somePtr, uSecTimeout));
  std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
  std::int64_t actualTimeout = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
  EXPECT_GE(actualTimeout, uSecTimeout);
  EXPECT_LE(actualTimeout, uSecTimeout * 2);
  EXPECT_EQ(somePtr.get(), nullptr);
}
#endif

TEST(PutElem, PutOne) {
  ElementPtr<int> SomePtr(nullptr);
  int Elements = 10;
  CircularBuffer<int> SomeBuffer(Elements);
  SomeBuffer.tryGetEmpty(SomePtr);
  EXPECT_TRUE(SomeBuffer.tryPutData(std::move(SomePtr)));
  EXPECT_EQ(SomePtr.get(), nullptr);
}

TEST(PutElem, PutMultiple) {
  ElementPtr<int> SomePtr(nullptr);
  int Elements = 15; //One less than a power of 2 value due to the implementation of the queue
  CircularBuffer<int> SomeBuffer(Elements);
  for (int j = 0; j < Elements; j++) {
    SomeBuffer.tryGetEmpty(SomePtr);
    EXPECT_TRUE(SomeBuffer.tryPutData(std::move(SomePtr)));
    EXPECT_EQ(SomePtr.get(), nullptr);
  }
  int SomeValue = 5;
  ElementPtr<int> SomeOtherPtr(static_cast<int*>(&SomeValue));
  EXPECT_FALSE(SomeBuffer.tryPutData(std::move(SomeOtherPtr)));
  EXPECT_NE(SomeOtherPtr.get(), nullptr);
}

TEST(GetElem, GetOneFail) {
  ElementPtr<int> SomePtr(nullptr);
  int Elements = 15; //One less than a power of 2 value due to the implementation of the queue
  CircularBuffer<int> SomeBuffer(Elements);
  EXPECT_FALSE(SomeBuffer.tryGetData(SomePtr));
  EXPECT_EQ(SomePtr.get(), nullptr);
}

TEST(GetElem, GetOneSuccess) {
  ElementPtr<int> SomePtr(nullptr);
  int Elements = 15; //One less than a power of 2 value due to the implementation of the queue
  CircularBuffer<int> SomeBuffer(Elements);
  SomeBuffer.tryGetEmpty(SomePtr);
  SomeBuffer.tryPutData(std::move(SomePtr));
  EXPECT_TRUE(SomeBuffer.tryGetData(SomePtr));
  EXPECT_NE(SomePtr.get(), nullptr);
}

TEST(GetElem, GetMultiple) {
  ElementPtr<int> SomePtr(nullptr);
  int Elements = 15; //One less than a power of 2 value due to the implementation of the queue
  CircularBuffer<int> SomeBuffer(Elements);
  for (int i = 0; i < Elements; i++) {
    SomeBuffer.tryGetEmpty(SomePtr);
    SomeBuffer.tryPutData(std::move(SomePtr));
  }
  for (int u = 0; u < Elements; u++) {
    SomePtr.reset();
    EXPECT_TRUE(SomeBuffer.tryGetData(SomePtr));
    EXPECT_NE(SomePtr.get(), nullptr);
    SomeBuffer.tryPutEmpty(std::move(SomePtr));
  }
  SomePtr.reset();
  EXPECT_FALSE(SomeBuffer.tryGetData(SomePtr));
  EXPECT_EQ(SomePtr.get(), nullptr);
}

TEST(PutGetElem, One) {
  std::default_random_engine Generator;
  std::uniform_int_distribution<int> Distribution(2,100000);
  
  ElementPtr<int> SomePtr(nullptr);
  int Elements = 15; //One less than a power of 2 value due to the implementation of the queue
  CircularBuffer<int> SomeBuffer(Elements);
  SomeBuffer.tryGetEmpty(SomePtr);
  int *TempPtr = SomePtr.get();
  int SomeInt = Distribution(Generator);
  *SomePtr = SomeInt;
  SomeBuffer.tryPutData(std::move(SomePtr));
  EXPECT_EQ(SomePtr.get(), nullptr);
  ElementPtr<int> AnotherPtr(nullptr);
  SomeBuffer.tryGetData(AnotherPtr);
  EXPECT_EQ(AnotherPtr.get(), TempPtr);
  EXPECT_EQ(*AnotherPtr, SomeInt);
}

TEST(PutGetElem, Multiple) {
  std::default_random_engine Generator;
  std::uniform_int_distribution<int> Distribution(2,100000);
  
  ElementPtr<int> SomePtr(nullptr);
  int Elements = 15; //One less than a power of 2 value due to the implementation of the queue
  std::vector<int> Data;
  std::vector<int*> DataPtr;
  CircularBuffer<int> SomeBuffer(Elements);
  for (int e = 0; e < Elements; e++) {
    int SomeInt = Distribution(Generator);
    Data.push_back(SomeInt);
    EXPECT_TRUE(SomeBuffer.tryGetEmpty(SomePtr));
    *SomePtr = SomeInt;
    DataPtr.push_back(SomePtr.get());
    SomeBuffer.tryPutData(std::move(SomePtr));
  }
  
  ElementPtr<int> AnotherPtr(nullptr);
  for (int g = 0; g < Elements; g++) {
    SomeBuffer.tryGetData(AnotherPtr);
    EXPECT_EQ(AnotherPtr.get(), DataPtr[g]);
    EXPECT_EQ(*AnotherPtr, Data[g]);
    SomeBuffer.tryPutEmpty(std::move(AnotherPtr));
  }
}

TEST(PutGetElem, EmptyReturn) {
  ElementPtr<int> SomePtr(nullptr);
  int Elements = 15; //One less than a power of 2 value due to the implementation of the queue
  std::vector<int*> DataPtr;
  CircularBuffer<int> SomeBuffer(Elements);
  
  for (int e = 0; e < Elements; e++) {
    EXPECT_TRUE(SomeBuffer.tryGetEmpty(SomePtr));
    DataPtr.push_back(SomePtr.get());
    SomeBuffer.tryPutData(std::move(SomePtr));
  }
  
  for (int g = 0; g < Elements; g++) {
    SomeBuffer.tryGetData(SomePtr);
    SomeBuffer.tryPutEmpty(std::move(SomePtr));
  }
  
  for (int g = 0; g < Elements; g++) {
    SomeBuffer.tryGetEmpty(SomePtr);
    EXPECT_EQ(SomePtr.get(), DataPtr[g]);
    SomeBuffer.tryPutData(std::move(SomePtr));
  }
}

template <int N>
std::int32_t ThreadTestFunction(std::int32_t Elements, std::int64_t ProducerDelayMs, std::int64_t ConsumerDelayMs, std::int32_t RunTime) {
  CircularBuffer<std::int32_t[N]> Buffer(Elements);
  std::int32_t DataErrors = 0;
  using sys_clk = std::chrono::system_clock;
  sys_clk::time_point StopTime = sys_clk::now() + std::chrono::duration<int, std::ratio<1, 1000>>(RunTime);
  std::int32_t ProducerCounter, ConsumerCounter;
  
  auto Producer = [&]() {
    std::int32_t Counter = 0;
    ElementPtr<std::int32_t[N]> ProducerData(nullptr);
    std::chrono::duration<std::int64_t, std::milli> SleepTime(ProducerDelayMs);
    while (sys_clk::now() < StopTime) {
      if (Buffer.tryGetEmpty(ProducerData)) {
        for (int i = 0; i < N; i++) {
          (*ProducerData.get())[i] = Counter++;
        }
        while (not Buffer.tryPutData(std::move(ProducerData))) {}
        if (ProducerDelayMs != 0) {
          std::this_thread::sleep_for(SleepTime);
        }
      }
    }
    ProducerCounter = Counter;
  };
  
  auto Consumer = [&]() {
    std::int32_t Counter = 0;
    ElementPtr<std::int32_t[N]> ConsumerData(nullptr);
    std::chrono::duration<std::int64_t, std::milli> SleepTime(ConsumerDelayMs);
    while (sys_clk::now() < StopTime) {
      if (Buffer.tryGetData(ConsumerData)) {
        for (int i = 0; i < N; i++) {
          if ((*ConsumerData.get())[i] != Counter++) {
            DataErrors++;
          }
        }
        while (not Buffer.tryPutEmpty(std::move(ConsumerData))) {}
        if (ConsumerDelayMs != 0) {
          std::this_thread::sleep_for(SleepTime);
        }
      }
    }
    ConsumerCounter = Counter;
  };
  ElementPtr<std::int32_t[N]> TempData(nullptr);
#ifdef __linux__
  cpu_set_t ProducerCpu, ConsumerCpu;
  CPU_ZERO(&ProducerCpu);
  CPU_ZERO(&ConsumerCpu);
  CPU_SET(0, &ProducerCpu);
  CPU_SET(1, &ConsumerCpu);
#endif
  std::thread ProducerThread(Producer);
  std::thread ConsumerThread(Consumer);
#ifdef __linux__
  pthread_setaffinity_np(ProducerThread.native_handle(), sizeof(cpu_set_t), &ProducerCpu);
  pthread_setaffinity_np(ConsumerThread.native_handle(), sizeof(cpu_set_t), &ConsumerCpu);
#endif
  ConsumerThread.join();
  ProducerThread.join();
  
//  std::cout << "Prod/cons ctr: " << prod_ctr << " / " << cons_ctr << std::endl;
  return DataErrors;
}

const std::int32_t RunTimeMs = 50;
const std::int32_t ShortQueue = 3;
const std::int32_t LongQueue = 1000000;
TEST(ProducerConsumer, Size1_ShortQueue_ConsumerDelay) {
  EXPECT_EQ(ThreadTestFunction<1>(ShortQueue, 0, 2, RunTimeMs), 0);
}
TEST(ProducerConsumer, Size1_LongQueue_ConsumerDelay) {
  EXPECT_EQ(ThreadTestFunction<1>(LongQueue, 0, 2, RunTimeMs), 0);
}
TEST(ProducerConsumer, Size1_ShortQueue_ProducerDelay) {
  EXPECT_EQ(ThreadTestFunction<1>(ShortQueue, 2, 0, RunTimeMs), 0);
}
TEST(ProducerConsumer, Size1_LongQueue_ProducerDelay) {
  EXPECT_EQ(ThreadTestFunction<1>(LongQueue, 2, 0, RunTimeMs), 0);
}
TEST(ProducerConsumer, Size1_LongQueue_NoDelay) {
  EXPECT_EQ(ThreadTestFunction<1>(LongQueue, 0, 0, RunTimeMs), 0);
}
TEST(ProducerConsumer, Size1_ShortQueue_NoDelay) {
  EXPECT_EQ(ThreadTestFunction<1>(ShortQueue, 0, 0, RunTimeMs), 0);
}

TEST(ProducerConsumer, Size4_ShortQueue_ConsumerDelay) {
  EXPECT_EQ(ThreadTestFunction<4>(ShortQueue, 0, 2, RunTimeMs), 0);
}
TEST(ProducerConsumer, Size4_LongQueue_ConsumerDelay) {
  EXPECT_EQ(ThreadTestFunction<4>(LongQueue, 0, 2, RunTimeMs), 0);
}
TEST(ProducerConsumer, Size4_ShortQueue_ProducerDelay) {
  EXPECT_EQ(ThreadTestFunction<4>(ShortQueue, 2, 0, RunTimeMs), 0);
}
TEST(ProducerConsumer, Size4_LongQueue_ProducerDelay) {
  EXPECT_EQ(ThreadTestFunction<4>(LongQueue, 2, 0, RunTimeMs), 0);
}
TEST(ProducerConsumer, Size4_LongQueue_NoDelay) {
  EXPECT_EQ(ThreadTestFunction<4>(LongQueue, 0, 0, RunTimeMs), 0);
}
TEST(ProducerConsumer, Size4_ShortQueue_NoDelay) {
  EXPECT_EQ(ThreadTestFunction<4>(ShortQueue, 0, 0, RunTimeMs), 0);
}

TEST(ProducerConsumer, Size2048_ShortQueue_ConsumerDelay) {
  EXPECT_EQ(ThreadTestFunction<2048>(ShortQueue, 0, 2, RunTimeMs), 0);
}
TEST(ProducerConsumer, Size2048_LongQueue_ConsumerDelay) {
  EXPECT_EQ(ThreadTestFunction<2048>(LongQueue, 0, 2, RunTimeMs), 0);
}
TEST(ProducerConsumer, Size2048_ShortQueue_ProducerDelay) {
  EXPECT_EQ(ThreadTestFunction<2048>(ShortQueue, 2, 0, RunTimeMs), 0);
}
TEST(ProducerConsumer, Size2048_LongQueue_ProducerDelay) {
  EXPECT_EQ(ThreadTestFunction<2048>(LongQueue, 2, 0, RunTimeMs), 0);
}
TEST(ProducerConsumer, Size2048_LongQueue_NoDelay) {
  EXPECT_EQ(ThreadTestFunction<2048>(LongQueue, 0, 0, RunTimeMs), 0);
}
TEST(ProducerConsumer, Size2048_ShortQueue_NoDelay) {
  EXPECT_EQ(ThreadTestFunction<2048>(ShortQueue, 0, 0, RunTimeMs), 0);
}

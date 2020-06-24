/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include "UdpConnection.h"
#include <ciso646>
#include <functional>
#include <iostream>
#include <common/Timer.h>

#include <mach/thread_policy.h>





/// from https://github.com/elazarl/cpu_affinity/blob/master/affinity_darwin.c
/// "The Unlicense" license:
/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/
#include <stdio.h>
#include <mach/thread_policy.h>
#include <mach/task_info.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/thread_policy.h>
#include <mach/thread_act.h>
#include <pthread.h>



/*
int pthread_create_with_cpu_affinity(pthread_t *restrict thread, int cpu,
                                     const pthread_attr_t *restrict attr,
                                     void *(*start_routine)(void *), void *restrict arg) {
  int rv = pthread_create_suspended_np(thread, attr, start_routine, arg);
  mach_port_t mach_thread = pthread_mach_thread_np(*thread);
  if (rv != 0) {
    return rv;
  }




  thread_affinity_policy_data_t policy_data = { 3 };
  mach_port_t myThread = pthread_mach_thread_np(pthread_self())
  thread_policy_set((myThread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy_data, THREAD_AFFINITY_POLICY_COUNT);
  return 0;
}*/



using UdpEndpoint = asio::ip::udp::endpoint;
using namespace std::chrono_literals;

// struct QueryResult {
//  explicit QueryResult(asio::ip::udp::resolver::iterator &&Endpoints)
//      : EndpointIterator(std::move(Endpoints)) {
//    while (EndpointIterator != asio::ip::udp::resolver::iterator()) {
//      auto CEndpoint = *EndpointIterator;
//      EndpointList.push_back(CEndpoint);
//      ++EndpointIterator;
//    }
//    std::sort(EndpointList.begin(), EndpointList.end(), [](auto &a, auto &b) {
//      return a.address().is_v6() < b.address().is_v6();
//    });
//  }
//  asio::ip::udp::endpoint getNextEndpoint() {
//    if (NextEndpoint < EndpointList.size()) {
//      return EndpointList[NextEndpoint++];
//    }
//    return {};
//  }
//  bool isDone() const { return NextEndpoint >= EndpointList.size(); }
//  asio::ip::udp::resolver::iterator EndpointIterator;
//  std::vector<UdpEndpoint> EndpointList;
//  unsigned int NextEndpoint{0};
//};

void* TransmitThreadStart(void* arg)
{
  UdpConnection* This = (UdpConnection*)arg;
  This->transmitThread();
  return nullptr;
}

static const int kTransmitQueueSize = 50;

UdpConnection::UdpConnection(std::string DstAddress, std::uint16_t DstPort,
                             asio::io_service &__attribute__((unused)) Service)
    : Address(std::move(DstAddress)), Port(DstPort)
      // Socket(Service, UdpEndpoint()), Resolver(Service),
      // ReconnectTimeout(Service), HeartbeatTimeout(Service),
      // DataPacketTimeout(Service),
      // TransmitBuffer(std::make_unique<DataPacket>(MaxPacketSize)),
      ,
      RemoteEndpoint(Address, Port),
      DataSource(Socket::Endpoint("0.0.0.0", 0), RemoteEndpoint),
      SharedStandbyBuffer(
          nullptr /*std::make_unique<DataPacket>(MaxPacketSize)*/) {
  // resolveDestination();

  DataSource.setBufferSizes(KernelTxBufferSize, 0);
  DataSource.printBufferSizes();

  IdlePacket.Head.Version = Protocol::VER_1;
  IdlePacket.Head.Type = PacketType::Idle;
  IdlePacket.Head.ReadoutLength = 20;
  IdlePacket.Head.ReadoutCount = 0;
  IdlePacket.Head.ClockMode = Clock::Clk_Ext;
  IdlePacket.Head.OversamplingFactor = 1;
  IdlePacket.Idle.TimeStamp = {0, 0};
  IdlePacket.fixEndian();

  for (int i = 0; i < kTransmitQueueSize; i++) {
    FreePackets.push_back(new DataPacket(MaxPacketSize));
    TransmitRequests.push_back(nullptr);
  }
  SharedStandbyBuffer = FreePackets.back();
  FreePackets.pop_back();
  TransmitRequests.clear();
  


//kern_return_t thread_policy_set(thread_act_t thread, thread_policy_flavor_t flavor, thread_policy_t policy_info, mach_msg_type_number_t policy_infoCnt);

  //thread_affinity_policy_data_t policy_data = { /*int cpu*/ 3 };
  //mach_port_t myThread = pthread_mach_thread_np(pthread_self());
  //thread_policy_set(myThread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy_data, THREAD_AFFINITY_POLICY_COUNT);

  pthread_t thread1;
  if (pthread_create_suspended_np(&thread1, NULL, TransmitThreadStart, this) !=
      0) {
    abort();
  }
  mach_port_t mach_thread1 = pthread_mach_thread_np(thread1);
  int wantedCpu = 7;
  thread_affinity_policy_data_t policyData1 = { 1 << wantedCpu };
  int res = thread_policy_set(mach_thread1, THREAD_AFFINITY_POLICY,
                    (thread_policy_t)&policyData1, THREAD_AFFINITY_POLICY_COUNT);
  if (res != 0) {
    fprintf(stderr, "bad thread_policy_set %i", res);
  }

  //thread_resume(mach_thread1);

  TransmitThread = std::thread([this]() { this->transmitThread(); });
}

void UdpConnection::resolveDestination() {
  // asio::ip::udp::resolver::query Query(asio::ip::udp::v4(), Address,
  //                                     std::to_string(Port));
  // auto ResolveHandlerGlue = [this](auto &Error, auto EndpointIterator) {
  //  this->handleResolve(Error, EndpointIterator);
  //};
  // Resolver.async_resolve(Query, ResolveHandlerGlue);
}

void UdpConnection::reconnectWait() {
  // auto HandlerGlue = [this](auto &) { this->resolveDestination(); };
  // ReconnectTimeout.expires_after(1s);
  // ReconnectTimeout.async_wait(HandlerGlue);
}

// void UdpConnection::tryConnect(QueryResult AllEndpoints) {
//  UdpEndpoint CurrentEndpoint = AllEndpoints.getNextEndpoint();
//  auto HandlerGlue = [this, AllEndpoints](auto &Error) {
//  //  this->handleConnect(Error, AllEndpoints);
//  //};
//  //Socket.async_connect(CurrentEndpoint, HandlerGlue);
//}

// void UdpConnection::handleResolve(
//    const asio::error_code &Error,
//    asio::ip::udp::resolver::iterator EndpointIter) {
//  //if (Error) {
//  //  reconnectWait();
//  //  return;
//  //}
//  //QueryResult AllEndpoints(std::move(EndpointIter));
//  //tryConnect(AllEndpoints);
//}

// void UdpConnection::handleConnect(const asio::error_code &Error,
//                                  QueryResult const &AllEndpoints) {
//  //if (!Error) {
//  //  startHeartbeatTimer();
//  //
//  //  //////// here socket is connected
//  //  TransmitThread = std::thread([this]() { this->transmitThread(); });
////
//  //  return;
//  //}
//  //Socket.close();
//  //if (AllEndpoints.isDone()) {
//  //  reconnectWait();
//  //  return;
//  //}
//  //tryConnect(AllEndpoints);
//}
//
void UdpConnection::startHeartbeatTimer() {
  // HeartbeatTimeout.expires_after(4s);
  // HeartbeatTimeout.async_wait([this](auto &Error) {
  //  if (not Error) {
  //    transmitHeartbeat();
  //    startHeartbeatTimer();
  //  }
  //});
}

void UdpConnection::startPacketTimer() {
  // DataPacketTimeout.expires_after(500ms);
  // DataPacketTimeout.async_wait([this](auto &Error) {
  //  if (not Error) {
  //    swapAndTransmitSharedStandbyBuffer();
  //    startHeartbeatTimer();
  //  }
  //});
}

void UdpConnection::transmitHeartbeat() {
  TimeStamp IdleTS{CurrentRefTimeNS + RefTimeDeltaNS,
                   TimeStamp::ClockMode::External};
  auto IdleRawTS = RawTimeStamp{IdleTS.getSeconds(), IdleTS.getSecondsFrac()};
  IdleRawTS.fixEndian();
  IdlePacket.Head.ReferenceTimeStamp = IdleRawTS;
  IdlePacket.Idle.TimeStamp = IdleRawTS;

  // DON't SEND IDLE WHILE TESTING MULTITHREAD SENDING
  // transmitPacket(&IdlePacket, sizeof(IdlePacket));
}

void UdpConnection::transmitPacket(const void *DataPtr,
                                   const size_t __attribute__((unused)) Size) {

  // bool wasTransmitBuffer =
  //     (DataPtr == this->TransmitBuffer.get()->Buffer.get());
  //
  // Socket.async_send(
  //     asio::buffer(DataPtr, Size), [this, wasTransmitBuffer](auto &, auto) {
  //       if (wasTransmitBuffer) {
  //         int prevTransmitUse = this->TransmitBufferInUse.exchange(0);
  //         //fprintf(stdout, "transmitPacket() release
  //         TransmitBufferInUse\n"); RelAssertMsg(prevTransmitUse == 1, "we
  //         release it");
  //       }
  //
  //       this->incNumSentPackets();
  //     });

  TransmitRequestsAccess.lock();
  TransmitRequests.push_back((DataPacket *)DataPtr);
  TransmitRequestsAccess.unlock();
}

extern bool RunLoop;

void UdpConnection::transmitThread() {
  
  static const bool ContinuousSpeedTest = false;
  if (ContinuousSpeedTest) {
    fprintf(stdout, "ContinuousSpeedTest\n");
  }
  static const bool RepeatPacketSpeedTest = false;
  if (RepeatPacketSpeedTest) {
    fprintf(stdout, "RepeatPacketSpeedTest\n");
  }
  bool FirstData = true;
  Timer FirstDataTimer;
  uint64_t SendCount = 0;

  std::vector<DataPacket *> LocalQueue;
  LocalQueue.reserve (kTransmitQueueSize);

#define LOCAL_QUEUE 1

  while (RunLoop) {
    bool empty = false;
    DataPacket *data = nullptr;

    TransmitRequestsAccess.lock();
    empty = (TransmitRequests.size() == 0);
    if (!empty) {
#if LOCAL_QUEUE
      for (DataPacket *request : TransmitRequests) {
        LocalQueue.push_back(request);
      }
      TransmitRequests.clear();
#else
      data = TransmitRequests.front();
      TransmitRequests.pop_front();
#endif
    }
    TransmitRequestsAccess.unlock();

    if (empty) {
      // this->TransmitBufferInUse.exchange(0); // the queue is empty
      usleep(FirstData ? 10000 : 1);
      continue;
    } else {
      if (FirstData) {
        FirstData = false;
        FirstDataTimer.now();
      }
    }

#if LOCAL_QUEUE
    for (DataPacket *request : LocalQueue) {
      data = request;
#endif
      void *DataBuf = data->Buffer.get();
      std::size_t DataSize = data->Size;

      if (ContinuousSpeedTest) {
        for (;;) {
          DataSource.send(DataBuf, DataSize);

          if ((SendCount++ & ((1 << 18) - 1)) == 0) {
            uint64_t AccumSize = DataSize * SendCount;
            double Secs = FirstDataTimer.timeus() / 1e6;
            double GbPerSec = AccumSize / (1024 * 1024 * 1024 * Secs);
            double PkgPerSec = SendCount / Secs;
            fprintf(stderr, "GbPerSec %0.3f, pkg/sec %0.3f, pkg bytes %u\n",
                    GbPerSec, PkgPerSec, (uint32_t)DataSize);
          }
        }
      } else if (RepeatPacketSpeedTest) {
        DataSource.send(DataBuf, DataSize);

        if ((SendCount++ & ((1 << 17) - 1)) == 0) {
          uint64_t AccumSize = DataSize * SendCount;
          double Secs = FirstDataTimer.timeus() / 1e6;
          double GbPerSec = AccumSize / (1024 * 1024 * 1024 * Secs);
          double PkgPerSec = SendCount / Secs;
          fprintf(stderr, "GbPerSec %0.3f, pkg/sec %0.3f, pkg bytes %u\n",
                  GbPerSec, PkgPerSec, (uint32_t)DataSize);
        }
      } else {
        DataSource.send(DataBuf, DataSize);

        if ((SendCount++ & ((1 << 16) - 1)) == 0) {
          uint64_t AccumSize = DataSize * SendCount;
          double Secs = FirstDataTimer.timeus() / 1e6;
          double GbPerSec = AccumSize / (1024 * 1024 * 1024 * Secs);
          double PkgPerSec = SendCount / Secs;
          fprintf(stderr, "GbPerSec %0.3f, pkg/sec %0.3f, pkg bytes %u\n",
                  GbPerSec, PkgPerSec, (uint32_t)DataSize);
        }
      }

      // // we assume we're always transmit "TransmitBuffer".
      // int prevTransmitUse = this->TransmitBufferInUse.exchange(0);
      // // fprintf(stdout, "transmitPacket() release TransmitBufferInUse\n");
      // RelAssertMsg(prevTransmitUse == 1, "we release it");

      this->incNumSentPackets();
#if LOCAL_QUEUE
    }
#endif

#if LOCAL_QUEUE
    FreePacketsAccess.lock();
    for (DataPacket *request : LocalQueue) {
      FreePackets.push_back(request);
    }
    FreePacketsAccess.unlock();
    LocalQueue.clear();
#else
    // we're done sending so release the data packet.
    if (RepeatPacketSpeedTest) {
      TransmitRequestsAccess.lock();
      TransmitRequests.push_back(data);
      TransmitRequestsAccess.unlock();
    } else {
      FreePacketsAccess.lock();
      FreePackets.push_back(data);
      FreePacketsAccess.unlock();
    }
#endif
  }
}

// used by the generators
void UdpConnection::addSamplingRun(void const *const DataPtr, size_t Bytes,
                                   TimeStamp Timestamp) {
  if (CurrentRefTimeNS == 0) {
    CurrentRefTimeNS = Timestamp.getTimeStampNS();
  }

  // JonasNilsson:
  // I believe the code is trying to simulate the reference pulse timestamps of
  // the ESS accelerator. Those occur at fixed time intervals. Hence, when the
  // code determines that we are past the point in time at which we should have
  // a new reference pulse timestamp, it updates that timestamp.
  while (Timestamp.getTimeStampNS() > CurrentRefTimeNS + RefTimeDeltaNS) {
    CurrentRefTimeNS += RefTimeDeltaNS;
  }

  // int prevSharedStandbyUse = SharedStandbyInUse.exchange (1);
  // //fprintf(stdout, "addSamplingRun() lock SharedStandbyInUse\n");
  // RelAssertMsg (prevSharedStandbyUse == 0, "");

  bool Success =
      SharedStandbyBuffer->addSamplingRun(DataPtr, Bytes, CurrentRefTimeNS);
  std::pair<size_t, size_t> BufferSizes = SharedStandbyBuffer->getBufferSizes();
  size_t Size = BufferSizes.first;
  size_t MaxSize = BufferSizes.second;


  // int prevSharedStandbyUse2 = SharedStandbyInUse.exchange(0);
  // //fprintf(stdout, "addSamplingRun() release SharedStandbyInUse\n");
  // RelAssertMsg (prevSharedStandbyUse2 == 1, "we release it");

  if (not Success or MaxSize - Size < 20) {
    swapAndTransmitSharedStandbyBuffer();
    // assert(
    //    SharedStandbyBuffer->addSamplingRun(DataPtr, Bytes,
    //    CurrentRefTimeNS));
  } else {
    // startPacketTimer();
  }
  ++SamplingRuns;
  // startHeartbeatTimer();
}

void UdpConnection::swapAndTransmitSharedStandbyBuffer() {

  //// spin wait for transmit buffer to be free.
  // int prevTransmitUse = 0;
  // while (!TransmitBufferInUse.compare_exchange_strong(prevTransmitUse, 1)) {
  //  prevTransmitUse = 0;
  //};
  ////TransmitBufferInUse.exchange (1);
  ////fprintf(stdout, "swapAndTransmitSharedStandbyBuffer() lock
  ///TransmitBufferInUse\n");
  // RelAssertMsg (prevTransmitUse == 0, "");
  //
  // int prevSharedStandbyUse = SharedStandbyInUse.exchange (1);
  ////fprintf(stdout, "swapAndTransmitSharedStandbyBuffer() lock
  ///SharedStandbyInUse\n");
  // RelAssertMsg (prevSharedStandbyUse == 0, "");

  // std::swap(TransmitBuffer, SharedStandbyBuffer);
  // SharedStandbyBuffer->resetPacket();

  // int prevSharedStandbyUse2 = SharedStandbyInUse.exchange (0);
  ////fprintf(stdout, "swapAndTransmitSharedStandbyBuffer() release
  ///SharedStandbyInUse\n");
  // RelAssertMsg (prevSharedStandbyUse2 == 1, "we release it");

  DataPacket *TransmitBuffer = SharedStandbyBuffer;
  SharedStandbyBuffer = nullptr;
  /*auto DataInfo =*/ TransmitBuffer->formatPacketForSend(PacketCount);
  PacketCount++;
  //transmitPacket(DataInfo.first, DataInfo.second);
  transmitPacket((void*)TransmitBuffer, 0);


  // allocate new standby buffer
  while (SharedStandbyBuffer == nullptr) {
    //usleep(10000); // test
    FreePacketsAccess.lock();
    if (FreePackets.size()) {
      SharedStandbyBuffer = FreePackets.back();
      FreePackets.pop_back();
    }
    FreePacketsAccess.unlock();

    if (SharedStandbyBuffer == nullptr) {
      usleep(1);
    }
  }

  // std::swap(TransmitBuffer, SharedStandbyBuffer);
  SharedStandbyBuffer->resetPacket();
}

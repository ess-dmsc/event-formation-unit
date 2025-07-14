// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief ESS Detector interface implementations
///
//===----------------------------------------------------------------------===//

#include <common/detector/Detector.h>
#include <common/system/SocketImpl.h>

// clang-format off
const std::string Detector::METRIC_RECEIVE_PACKETS = "receive.packets";
const std::string Detector::METRIC_RECEIVE_BYTES = "receive.bytes";
const std::string Detector::METRIC_RECEIVE_DROPPED = "receive.dropped";
const std::string Detector::METRIC_THREAD_INPUT_IDLE = "thread.input_idle";
const std::string Detector::METRIC_TRANSMIT_CALIBMODE_PACKETS = "transmit.calibmode_packets";
// clang-format on

void Detector::inputThread() {
  XTRACE(INPUT, DEB, "Starting inputThread");
  SocketImpl::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                             EFUSettings.DetectorPort);

  UDPReceiver dataReceiver(local);
  dataReceiver.setBufferSizes(EFUSettings.TxSocketBufferSize,
                              EFUSettings.RxSocketBufferSize);
  dataReceiver.printBufferSizes();
  dataReceiver.setRecvTimeout(0, EFUSettings.SocketRxTimeoutUS);

  LOG(INIT, Sev::Info, "Detector input thread started on {}:{}",
      local.IpAddress, local.Port);

  while (runThreads) {
    int readSize;
    unsigned int rxBufferIndex = RxRingbuffer.getDataIndex();

    RxRingbuffer.setDataLength(rxBufferIndex, 0);
    auto DataPtr = RxRingbuffer.getDataBuffer(rxBufferIndex);
    if ((readSize =
             dataReceiver.receive(DataPtr, RxRingbuffer.getMaxBufSize())) > 0) {
      RxRingbuffer.setDataLength(rxBufferIndex, readSize);
      XTRACE(INPUT, DEB, "Received an udp packet of length %d bytes", readSize);
      ITCounters.RxPackets++;
      ITCounters.RxBytes += readSize;

      if (CalibrationMode) {
        MonitorSerializer->serialize((uint8_t *)DataPtr, readSize);
        MonitorSerializer->produce();
        ITCounters.CalibModePackets++;
        continue;
      }

      if (InputFifo.push(rxBufferIndex)) {
        RxRingbuffer.getNextBuffer();
      } else {
        ITCounters.FifoPushErrors++;
      }
    } else {
      ITCounters.RxIdle++;
    }
  }
  XTRACE(INPUT, ALW, "Stopping input thread.");
  return;
}

void Detector::startThreads() {
  for (auto &tInfo : Threads) {
    tInfo.thread = std::thread(tInfo.func);
  }
}

void Detector::stopThreads() {
  runThreads.store(false);
  for (auto &tInfo : Threads) {
    if (tInfo.thread.joinable()) {
      tInfo.thread.join();
    }
  }
}

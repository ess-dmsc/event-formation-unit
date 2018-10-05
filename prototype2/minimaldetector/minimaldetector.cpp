/* Copyright (C) 2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// plugin for a minimalistic detector doing basically nothing
//===----------------------------------------------------------------------===//

#include <common/Detector.h>
#include <libs/include/Socket.h>

const char *classname = "Demo Detector";

/// \brief Needed even if there are no custom CLI commands to register
void SetCLIArguments(CLI::App __attribute__((unused)) & parser) { }

/// \brief the detector interface specification
class DemoDetector : public Detector {
public:
  DemoDetector(BaseSettings settings);
  ~DemoDetector();
  void main_thread();
  const char *detectorname();

private:
  struct {
    // Input Counters
    int64_t rx_packets;
  } mystats;
};

PopulateCLIParser PopulateParser{SetCLIArguments};

///
DemoDetector::~DemoDetector() { }

///
DemoDetector::DemoDetector(BaseSettings settings) : Detector("Demo", settings) {
  Stats.setPrefix("efu.demo");
  Stats.create("rx_packets", mystats.rx_packets);

  std::function<void()> mainFunc = [this]() { DemoDetector::main_thread(); };
  Detector::AddThreadFunction(mainFunc, "detectorthread");
}

///
const char *DemoDetector::detectorname() { return classname; }

///
void DemoDetector::main_thread() {
  /** Connection setup */
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort);
  UDPReceiver eth_receive(local);
  eth_receive.setRecvTimeout(0, 100000); /// secs, usecs

  constexpr int RxBufferSize = 9000;
  char RxBuffer[RxBufferSize];

  while (true) {
    /// receive packets
    int rdsize;
    if ((rdsize = eth_receive.receive(RxBuffer, RxBufferSize)) > 0) {
      mystats.rx_packets++;

      /// do processing
    }
    // Checking for exit
    if (not runThreads) {
      return;
    }
  }
}

DetectorFactory<DemoDetector> Factory;

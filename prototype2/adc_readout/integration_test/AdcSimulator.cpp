/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Simple application for simulating chopper TDC and monitor event data production.
 */

#include <fstream>
#include "UDPServer.h"
#include "PacketGenerator.h"
#include <chrono>
#include <thread>
#include <csignal>
#include <iomanip>
#include <algorithm>
#include <iostream>

bool RunLoop = true;

void signalHandler(int signal) {
  std::cout << "Got exit signal:" << signal << std::endl;
  RunLoop = false;
}

int main() {
  const std::vector<int> ActiveChannels{0, 1, 2, 3};
  const int OversamplingFactor{4};
  
  std::signal(SIGINT, signalHandler);
  
  ADC::PacketGenerator AdcGenerator(OversamplingFactor);
  
  UDPServer server(2048, 65535);
  
  while (not server.IsOk()) {
  }
  const int NrOfSamples = 4468;
  const int NrOfOriginalSamples = OversamplingFactor * NrOfSamples;
  const double TimeFracMax = 88052500.0/2;
  double TimeStep = NrOfOriginalSamples / TimeFracMax;
  auto now = std::chrono::system_clock::now().time_since_epoch();
  std::uint32_t NowSeconds = std::chrono::duration_cast<std::chrono::seconds>(now).count();
  double NowSecondsFrac = (std::chrono::duration_cast<std::chrono::milliseconds>(now).count() / 1000.0) - NowSeconds;
  while (RunLoop) {
    for (auto &CurrentChannel : ActiveChannels) {
      auto AdcPacket = AdcGenerator.GeneratePacket(NowSeconds, NowSecondsFrac, CurrentChannel);
      server.TransmitPacket(AdcPacket.Pointer, AdcPacket.Size);
    }
    NowSecondsFrac += TimeStep;
    if (NowSecondsFrac > 1.0) {
      NowSecondsFrac -= 1.0;
      ++NowSeconds;
    }
    auto TestNow = std::chrono::system_clock::now().time_since_epoch();
    double TestNowSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(TestNow).count() / 1000.0;
    if ((NowSeconds + NowSecondsFrac) > TestNowSeconds) {
      std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<1>>(TimeStep));
    }
  }
  std::cout << "Waiting for transmit thread to exit!" << std::endl;
  return 0;
}

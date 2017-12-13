//
// Created by Jonas Nilsson on 2017-11-08.
//

#include <gtest/gtest.h>
#include "../AdcReadout.h"
#include "TestUDPServer.h"
#include <random>

class AdcReadoutStandIn : public AdcReadout {
public:
  AdcReadoutStandIn() : AdcReadout({}) {};
  using Detector::Threads;
  using AdcReadout::toParsingQueue;
};

TEST(AdcReadout, StartThreads) {
  AdcReadoutStandIn Readout;
  Readout.StartThreads();
  EXPECT_EQ(Readout.Threads.size(), 2);
  for (auto &t : Readout.Threads) {
    EXPECT_TRUE(t.thread.joinable());
  }
  Readout.StopThreads();
}

TEST(ADC_Readout, SinglePacket) {
  AdcReadoutStandIn Readout;
  Readout.Threads.at(0).thread = std::thread(Readout.Threads.at(0).func);
  TestUDPServer server(2048, 65535, 1, 100);
  SpscBuffer::ElementPtr<InData> elem;
  EXPECT_TRUE(Readout.toParsingQueue.waitGetData(elem, 10000));
  Readout.StopThreads();
}

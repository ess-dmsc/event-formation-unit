/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Simple application for simulating chopper TDC and monitor event data
 * production.
 */

#include "SamplingTimer.h"
#include "UdpConnection.h"
#include <CLI/CLI.hpp>

#include <csignal>
#include <iostream>

bool RunLoop = true;

void signalHandler(int signal) {
  std::cout << "Got exit signal:" << signal << std::endl;
  RunLoop = false;
  static int countSIGINT = 0;
  if (signal == SIGINT && countSIGINT++ == 3) {
    std::cout << "many repeats of SIGINT, aborting." << std::endl;
    abort();
  }
}

enum class RunMode { SIMPLE, DELAY_LINE_TIME, DELAY_LINE_AMP, CONTINOUS };

struct SimSettings {
  std::string EFUAddress{"localhost"};
  std::uint16_t Port1{65535};
  std::uint16_t Port2{65534};
  bool SecondFPGA{false};
  RunMode Mode{RunMode::SIMPLE};
  double EventRate{1000};
  double NoiseRate{0.5};
};

void addCLIOptions(CLI::App &Parser, SimSettings &Settings) {
  Parser
      .add_option("--efu_addr", Settings.EFUAddress,
                  "Address to which the data should be transmitted.")
      ->default_str("localhost");
  Parser
      .add_option(
          "--port1", Settings.Port1,
          "UDP port #1 (from FPGA 1) to which the data should be transmitted.")
      ->default_str("65535");
  Parser
      .add_option(
          "--port2", Settings.Port1,
          "UDP port #2 (from FPGA 2) to which the data should be transmitted.")
      ->default_str("65534");
  Parser.add_flag("--two_fpgas", Settings.SecondFPGA,
                  "Simulate a second FPGA (data source).");
  std::vector<std::pair<std::string, RunMode>> ModeMap{
      {"simple", RunMode::SIMPLE},
      {"delay_line_time", RunMode::DELAY_LINE_TIME},
      {"delay_line_amp", RunMode::DELAY_LINE_AMP},
      {"continous", RunMode::CONTINOUS},
  };
  Parser
      .add_option("--mode", Settings.Mode,
                  "Set the simulation (data generation) mode.")
      ->transform(CLI::CheckedTransformer(ModeMap, CLI::ignore_case))
      ->default_str("continous");

  Parser
      .add_option("--event_rate", Settings.EventRate,
                  "Event rate when mode is set to delay-line time or "
                  "delay-line amplitude.")
      ->default_str("1000");

  Parser
      .add_option(
          "--noise_rate", Settings.NoiseRate,
          "Noise rate per channel. Applies to all modes except continous.")
      ->default_str("0.5");
}

bool ShouldCreateGenerator(SamplerType Type, UdpConnection *TargetAdcBox,
                           UdpConnection *AdcBox2, SimSettings UsedSettings) {
  if (0) { // only create first generator
    static uint32_t callcount = 0;
    if (callcount++ != 0) {
      return false;
    }
  }

  if (!UsedSettings.SecondFPGA && TargetAdcBox == AdcBox2)
    return false;

  if (UsedSettings.Mode == RunMode::SIMPLE or
      UsedSettings.Mode == RunMode::DELAY_LINE_AMP or
      UsedSettings.Mode == RunMode::DELAY_LINE_TIME) {
    if (Type == SamplerType::PoissonDelay) {
      return true;
    }
  }
  if (UsedSettings.Mode == RunMode::DELAY_LINE_AMP and
      Type == SamplerType::AmpEventDelay) {
    return true;
  }
  if (UsedSettings.Mode == RunMode::CONTINOUS) {
    if (Type == SamplerType::Continous) {
      return true;
    }
  }
  return false;
}

int main(const int argc, char *argv[]) {
  SimSettings UsedSettings;
  CLI::App CLIParser{"Detector simulator"};
  addCLIOptions(CLIParser, UsedSettings);
  try {
    CLIParser.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    CLIParser.exit(e);
    return 0;
  }
  const std::vector<int> ActiveChannels{0, 1, 2, 3};

  std::signal(SIGINT, signalHandler);

  UdpConnection AdcBox1(UsedSettings.EFUAddress, UsedSettings.Port1);
  UdpConnection AdcBox2(UsedSettings.EFUAddress, UsedSettings.Port2);

  std::vector<UdpConnection *> UdpConnections = {&AdcBox1};
  if (UsedSettings.SecondFPGA) {
    UdpConnections.push_back(&AdcBox2);
  }
  std::vector<TimePointNano> TriggerTime_UdpHeartBeat;
  TriggerTime_UdpHeartBeat.resize(UdpConnections.size());

  std::vector<PoissonDelay> PoissionGenerators;
  std::vector<TimePointNano> TriggerTime_Poisson;

  std::vector<AmpEventDelay> AmpDelayGenerators;
  std::vector<TimePointNano> TriggerTime_AmpDelay;

  std::vector<ContinousSamplingTimer> ContinousGenerators;
  std::vector<TimePointNano> TriggerTime_Continous;

  // Box 1 timers
  {
    UdpConnection *CurAdc = &AdcBox1;

    if (ShouldCreateGenerator(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      PoissionGenerators.emplace_back(
          PoissonDelay::Create(CurAdc, 0, 0,
                               {{"rate", UsedSettings.NoiseRate},
                                {"offset", 1.0},
                                {"amplitude", 2000.0}}));
      TriggerTime_Poisson.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      PoissionGenerators.emplace_back(
          PoissonDelay::Create(CurAdc, 0, 1,
                               {{"rate", UsedSettings.NoiseRate},
                                {"offset", 10.0},
                                {"amplitude", 3000.0}}));
      TriggerTime_Poisson.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      PoissionGenerators.emplace_back(
          PoissonDelay::Create(CurAdc, 0, 2,
                               {{"rate", UsedSettings.NoiseRate},
                                {"offset", 100.0},
                                {"amplitude", 4000.0}}));
      TriggerTime_Poisson.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      PoissionGenerators.emplace_back(
          PoissonDelay::Create(CurAdc, 0, 3,
                               {{"rate", UsedSettings.NoiseRate},
                                {"offset", 1000.0},
                                {"amplitude", 5000.0}}));
      TriggerTime_Poisson.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::Continous, CurAdc, &AdcBox2,
                              UsedSettings)) {
      ContinousGenerators.emplace_back(ContinousSamplingTimer::Create(
          CurAdc, 0, 0, {{"offset", 1000.0}, {"amplitude", 1000.0}}));
      TriggerTime_Continous.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::AmpEventDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      AmpDelayGenerators.emplace_back(
          AmpEventDelay::Create(CurAdc, 0, UsedSettings.EventRate));
      TriggerTime_AmpDelay.emplace_back();
    }
  }

  // Box 2 timers
  {
    UdpConnection *CurAdc = &AdcBox2;

    if (ShouldCreateGenerator(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      PoissionGenerators.emplace_back(
          PoissonDelay::Create(CurAdc, 1, 0,
                               {{"rate", UsedSettings.NoiseRate},
                                {"offset", 1.0},
                                {"amplitude", 2000.0}}));
      TriggerTime_Poisson.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      PoissionGenerators.emplace_back(
          PoissonDelay::Create(CurAdc, 1, 1,
                               {{"rate", UsedSettings.NoiseRate},
                                {"offset", 10.0},
                                {"amplitude", 3000.0}}));
      TriggerTime_Poisson.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      PoissionGenerators.emplace_back(
          PoissonDelay::Create(CurAdc, 1, 2,
                               {{"rate", UsedSettings.NoiseRate},
                                {"offset", 100.0},
                                {"amplitude", 4000.0}}));
      TriggerTime_Poisson.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      PoissionGenerators.emplace_back(
          PoissonDelay::Create(CurAdc, 1, 3,
                               {{"rate", UsedSettings.NoiseRate},
                                {"offset", 1000.0},
                                {"amplitude", 5000.0}}));
      TriggerTime_Poisson.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::Continous, CurAdc, &AdcBox2,
                              UsedSettings)) {
      ContinousGenerators.emplace_back(ContinousSamplingTimer::Create(
          CurAdc, 1, 0, {{"offset", 1000.0}, {"amplitude", 500.0}}));
      TriggerTime_Continous.emplace_back();
    }
  }

  // Stats printer
  auto PrintStats = [&AdcBox1, &AdcBox2]() {
    std::cout << "Sampling runs generated by FPGA simulator 1: "
              << AdcBox1.getNrOfRuns();
    std::cout << "\nSampling runs generated by FPGA simulator 2: "
              << AdcBox2.getNrOfRuns() << "\n";
  };
  TimeDurationNano PrintStatsInterval(5'000'000'000ull);
  TimePointNano TriggerTime_PrintStats;

  bool runTriggers = false;
  while (RunLoop) {

    auto TimeNow = std::chrono::high_resolution_clock::now();
    TimeStamp TimeTS = MakeTimeStampFromClock(TimeNow);

    // Poission
    for (int32_t i = 0, count = PoissionGenerators.size(); i < count; i++) {
      auto &self = PoissionGenerators[i];
      auto &TriggerTime = TriggerTime_Poisson[i];
      if (TriggerTime <= TimeNow) {
        TriggerTime = TimeNow + self.calcDelaTime();
        if (runTriggers) {
          self.genSamplesAndQueueSend(TimeTS);
        }
      }
    }

    // AmpDelay
    for (int32_t i = 0, count = AmpDelayGenerators.size(); i < count; i++) {
      auto &self = AmpDelayGenerators[i];
      auto &TriggerTime = TriggerTime_AmpDelay[i];
      if (TriggerTime <= TimeNow) {
        TriggerTime = TimeNow + self.calcDelaTime();
        if (runTriggers) {
          self.genSamplesAndQueueSend(TimeTS);
        }
      }
    }

    // Continous
    for (int32_t i = 0, count = ContinousGenerators.size(); i < count; i++) {
      auto &self = ContinousGenerators[i];
      auto &TriggerTime = TriggerTime_Continous[i];
      if (TriggerTime <= TimeNow) {
        TriggerTime = TimeNow + self.calcDelaTime();
        if (runTriggers) {
          self.genSamplesAndQueueSend(TimeTS);
        }
      }
    }

    // PrintStats
    {
      auto &TriggerTime = TriggerTime_PrintStats;
      if (TriggerTime <= TimeNow) {
        TriggerTime = TimeNow + PrintStatsInterval;
        if (runTriggers) {
          PrintStats();
        }
      }
    }

    // UdpConnection HeatBeat
    for (int32_t i = 0, count = UdpConnections.size(); i < count; i++) {
      auto &self = *UdpConnections[i];
      auto &TriggerTime = TriggerTime_UdpHeartBeat[i];
      if (TriggerTime <= TimeNow) {
        TriggerTime = TimeNow + self.HeartbeatInterval;
        if (runTriggers) {
          self.transmitHeartbeat();
        }
      }
    }

    // UdpConnection FlushIdleData
    for (int32_t i = 0, count = UdpConnections.size(); i < count; i++) {
      auto &self = *UdpConnections[i];
      if (self.shouldFlushIdleDataPacket(TimeNow)) {
        if (runTriggers) {
          self.flushIdleDataPacket(TimeNow);
        }
      }
    }

    runTriggers = true;
  }

  std::cout << "Waiting for transmit thread(s) to exit" << std::endl;

  return 0;
}

/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief Simple application for simulating chopper TDC and monitor event data
 * production.
 */

#include "AmpEventDelay.h"
#include "ContinousSamplingTimer.h"
#include "PoissonDelay.h"
#include "SampleRunGenerator.h"
#include "UdpConnection.h"
#include <CLI/CLI.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>

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

using namespace std::chrono_literals;

template <class T> struct VecFixed {
  T *Elms;
  int32_t Count;
  int32_t Capacity;

  VecFixed(int32_t Capacity) : Capacity(Capacity) {
    Elms = (T *)malloc(sizeof(T) * Capacity);
    Count = 0;
  }

  ~VecFixed() {
    for (int32_t i = Count - 1; i >= 0; --i)
      Elms[i].~T();
    free(Elms);
  }

  T *allocate() {
    RelAssertMsg(Count < Capacity, "");
    return &Elms[Count++];
  }

  T &operator[](int32_t i) {
    RelAssertMsg(i < Count, "");
    return Elms[i];
  }

  T *begin() { return Elms; }
  T *end() { return Elms + Count; }
};

void CreatePoissonGenerator(UdpConnection *UdpCon, int BoxNr, int ChNr,
                            std::map<std::string, double> Settings,
                            PoissonDelay *out) {
  double Settings_offset = Settings.at("offset");
  double Settings_amplitude = Settings.at("amplitude");
  double Settings_rate = Settings.at("rate");

  SampleRunGenerator SampleGen(100, 50, 20, 1.0, Settings_offset, BoxNr, ChNr);

  SamplingTimerData TimerData{
      SamplerType::PoissonDelay, UdpCon,       SampleGen, Settings_offset,
      Settings_amplitude,        Settings_rate};

  std::random_device RandomDevice; 
  PoissonDelayData data = {
      TimerData, std::default_random_engine(RandomDevice()),
      std::exponential_distribution<double>(Settings_rate)};

  new (out) PoissonDelay{data};
}

void CreateAmpEventDelayGenerator(UdpConnection *UdpCon, int BoxNr,
                                  double EventRate, AmpEventDelay *out) {

  const int NrOfSamples{100};

  double Settings_offset = 0.0;
  double Settings_amplitude = 0.0;
  double Settings_rate = EventRate;
  SampleRunGenerator SampleGen(NrOfSamples, 50, 20, 1.0, Settings_offset, 0, 0);

  SamplingTimerData TimerData = {
      SamplerType::AmpEventDelay, UdpCon,       SampleGen, Settings_offset,
      Settings_amplitude,         Settings_rate};

  std::random_device RandomDevice;
  PoissonDelayData PoissonData = {
      TimerData, std::default_random_engine(RandomDevice()),
      std::exponential_distribution<double>(Settings_rate)};

  SampleRunGenerator AnodeGen(NrOfSamples, 50, 20, 1.0, 500, BoxNr, 0);
  SampleRunGenerator XPosGen(NrOfSamples, 50, 20, 1.0, 500, BoxNr, 1);
  SampleRunGenerator YPosGen(NrOfSamples, 50, 20, 1.0, 500, BoxNr, 2);

  AmpEventDelayData data = {AnodeGen, XPosGen, YPosGen, PoissonData};
  new (out) AmpEventDelay{data};
}

void CreateContinousGenerator(UdpConnection *UdpCon, int BoxNr, int ChNr,
                              std::map<std::string, double> Settings,
                              ContinousSamplingTimer *out) {

  double Settings_offset = Settings.at("offset");
  double Settings_amplitude = Settings.at("amplitude");
  double Settings_rate = 0;

  int NrOfSamples = 4468;
  int OversamplingFactor = 4;
  if (0) { // high overhead test
    NrOfSamples = 100;
    OversamplingFactor = 1;
  }
  int NrOfOriginalSamples = NrOfSamples * OversamplingFactor;

  SampleRunGenerator SampleGen(NrOfSamples, 50, 20, 1.0, Settings_offset, BoxNr,
                               ChNr);

  const double TimeFracMax = 88052500.0 / 2;
  std::chrono::duration<size_t, std::nano> TimeStepNano =
      std::chrono::duration<size_t, std::nano>(static_cast<std::uint64_t>(
          (NrOfOriginalSamples / TimeFracMax) * 1e9));

  SamplingTimerData TimerData{
      SamplerType::Continous, UdpCon,       SampleGen, Settings_offset,
      Settings_amplitude,     Settings_rate};

  ContinousSamplingTimerData data = {TimerData, TimeStepNano};

  new (out) ContinousSamplingTimer{data};
}

bool ShouldCreateGenerator(SamplerType Type, UdpConnection *TargetAdcBox,
                           UdpConnection *AdcBox2, SimSettings UsedSettings) {
  if (0) {
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

  std::vector<UdpConnection *> UdpConnections = {&AdcBox1, &AdcBox2};
  std::vector<TimePointNano> TriggerTime_UdpHeartBeat;
  TriggerTime_UdpHeartBeat.resize(2);

  VecFixed<PoissonDelay> PoissionGenerators(20);
  std::vector<TimePointNano> TriggerTime_Poisson(20);

  VecFixed<AmpEventDelay> AmpDelayGenerators(20);
  std::vector<TimePointNano> TriggerTime_AmpDelay(20);

  VecFixed<ContinousSamplingTimer> ContinousGenerators(20);
  std::vector<TimePointNano> TriggerTime_Continous(20);

  // Box 1 timers
  {
    UdpConnection *CurAdc = &AdcBox1;

    if (ShouldCreateGenerator(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      CreatePoissonGenerator(CurAdc, 0, 0,
                             {{"rate", UsedSettings.NoiseRate},
                              {"offset", 1.0},
                              {"amplitude", 2000.0}},
                             PoissionGenerators.allocate());
      TriggerTime_Poisson.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      CreatePoissonGenerator(CurAdc, 0, 1,
                             {{"rate", UsedSettings.NoiseRate},
                              {"offset", 10.0},
                              {"amplitude", 3000.0}},
                             PoissionGenerators.allocate());
      TriggerTime_Poisson.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      CreatePoissonGenerator(CurAdc, 0, 2,
                             {{"rate", UsedSettings.NoiseRate},
                              {"offset", 100.0},
                              {"amplitude", 4000.0}},
                             PoissionGenerators.allocate());
      TriggerTime_Poisson.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      CreatePoissonGenerator(CurAdc, 0, 3,
                             {{"rate", UsedSettings.NoiseRate},
                              {"offset", 1000.0},
                              {"amplitude", 5000.0}},
                             PoissionGenerators.allocate());
      TriggerTime_Poisson.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::Continous, CurAdc, &AdcBox2,
                              UsedSettings)) {
      CreateContinousGenerator(CurAdc, 0, 0,
                               {{"offset", 1000.0}, {"amplitude", 1000.0}},
                               ContinousGenerators.allocate());
      TriggerTime_Continous.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::AmpEventDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      CreateAmpEventDelayGenerator(CurAdc, 0, UsedSettings.EventRate,
                                   AmpDelayGenerators.allocate());
      TriggerTime_AmpDelay.emplace_back();
    }
  }

  // Box 2 timers
  {
    UdpConnection *CurAdc = &AdcBox2;

    if (ShouldCreateGenerator(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      CreatePoissonGenerator(CurAdc, 1, 0,
                             {{"rate", UsedSettings.NoiseRate},
                              {"offset", 1.0},
                              {"amplitude", 2000.0}},
                             PoissionGenerators.allocate());
      TriggerTime_Poisson.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      CreatePoissonGenerator(CurAdc, 1, 1,
                             {{"rate", UsedSettings.NoiseRate},
                              {"offset", 10.0},
                              {"amplitude", 3000.0}},
                             PoissionGenerators.allocate());
      TriggerTime_Poisson.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      CreatePoissonGenerator(CurAdc, 1, 2,
                             {{"rate", UsedSettings.NoiseRate},
                              {"offset", 100.0},
                              {"amplitude", 4000.0}},
                             PoissionGenerators.allocate());
      TriggerTime_Poisson.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                              UsedSettings)) {
      CreatePoissonGenerator(CurAdc, 1, 3,
                             {{"rate", UsedSettings.NoiseRate},
                              {"offset", 1000.0},
                              {"amplitude", 5000.0}},
                             PoissionGenerators.allocate());
      TriggerTime_Poisson.emplace_back();
    }

    if (ShouldCreateGenerator(SamplerType::Continous, CurAdc, &AdcBox2,
                              UsedSettings)) {
      CreateContinousGenerator(CurAdc, 1, 0,
                               {{"offset", 1000.0}, {"amplitude", 500.0}},
                               ContinousGenerators.allocate());
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
  std::chrono::duration<size_t, std::nano> PrintStatsInterval(5'000'000'000ull);
  TimePointNano TriggerTime_PrintStats;

  bool runTriggers = false;
  while (RunLoop) {

    // Get TimeStep for current time
    auto TimeNow = std::chrono::high_resolution_clock::now();
    TimeStamp TimeTS = [&] {
      auto NowSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                            TimeNow.time_since_epoch())
                            .count();
      double NowSecFrac = (std::chrono::duration_cast<std::chrono::nanoseconds>(
                               TimeNow.time_since_epoch())
                               .count() /
                           1e9) -
                          NowSeconds;
      std::uint32_t Ticks = std::lround(NowSecFrac * (88052500 / 2.0));

      RawTimeStamp rts{static_cast<uint32_t>(NowSeconds), Ticks};
      return TimeStamp(rts, TimeStamp::ClockMode::External);
    }();

    // Poission
    for (int32_t i = 0, count = PoissionGenerators.Count; i < count; i++) {
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
    for (int32_t i = 0, count = AmpDelayGenerators.Count; i < count; i++) {
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
    for (int32_t i = 0, count = ContinousGenerators.Count; i < count; i++) {
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

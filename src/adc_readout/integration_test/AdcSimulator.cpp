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
  static int repeat2 = 0;
  if (signal == 2 && repeat2++ == 3) {
    std::cout << "many repeats of sig 2, aborting." << std::endl;
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

  T* begin() { return Elms; }
  T* end() { return Elms + Count; }
};

void SetUpPoissonGenerator(asio::io_service &Service, UdpConnection *UdpCon,
                           int BoxNr, int ChNr,
                           std::map<std::string, double> Settings,
                           PoissonDelay *out) {

  double Settings_offset = Settings.at("offset");
  double Settings_amplitude = Settings.at("amplitude");
  double Settings_rate = Settings.at("rate");

  SampleRunGenerator SampleGen(100, 50, 20, 1.0, Settings_offset, BoxNr, ChNr);

  SamplingTimerData TimerData{
      SamplerType::PoissonDelay, &Service,           UdpCon,       SampleGen,
      Settings_offset,           Settings_amplitude, Settings_rate};
  PoissonDelayData data = {TimerData};
  new (out) PoissonDelay{data};
}

void SetUpContinousGenerator(asio::io_service &Service, UdpConnection *UdpCon,
                             int BoxNr, int ChNr,
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

  SamplingTimerData TimerData{
      SamplerType::Continous, &Service,           UdpCon,       SampleGen,
      Settings_offset,        Settings_amplitude, Settings_rate};
  ContinousSamplingTimerData data = {TimerData, NrOfOriginalSamples};
  new (out) ContinousSamplingTimer{data};
}

void SetUpAmpPosGenerator(asio::io_service &Service, UdpConnection *UdpCon,
                          int BoxNr, double EventRate, AmpEventDelay *out) {

  const int NrOfSamples{100};

  double Settings_offset = 0.0;
  double Settings_amplitude = 0.0;
  double Settings_rate = EventRate;
  SampleRunGenerator SampleGen(NrOfSamples, 50, 20, 1.0, Settings_offset, 0, 0);

  SamplingTimerData TimerData = {SamplerType::AmpEventDelay,
                                 &Service,
                                 UdpCon,
                                 SampleGen,
                                 Settings_offset,
                                 Settings_amplitude,
                                 Settings_rate};
  PoissonDelayData PoissonData = {TimerData};

  SampleRunGenerator AnodeGen(NrOfSamples, 50, 20, 1.0, 500, BoxNr, 0);
  SampleRunGenerator XPosGen(NrOfSamples, 50, 20, 1.0, 500, BoxNr, 1);
  SampleRunGenerator YPosGen(NrOfSamples, 50, 20, 1.0, 500, BoxNr, 2);

  AmpEventDelayData data = {AnodeGen, XPosGen, YPosGen, PoissonData};
  new (out) AmpEventDelay{data};
}

class StatsTimer {
public:
  StatsTimer(std::function<void()> OnTimer, asio::io_service &Service)
      : TimerFunc(std::move(OnTimer)), Timer(Service) {}

  void start() {
    Timer.expires_after(5s);
    Timer.async_wait([this](auto &Error) {
      if (not Error) {
        TimerFunc();
        start();
      };
    });
  }
  void stop() { Timer.cancel(); }

private:
  std::function<void()> TimerFunc;
  asio::system_timer Timer;
};

bool ShouldCreateTimer(SamplerType Type, UdpConnection *TargetAdcBox,
                       UdpConnection *AdcBox2, SimSettings UsedSettings) {
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

  asio::io_service Service;
  asio::io_service::work Worker(Service);

  UdpConnection AdcBox1(UsedSettings.EFUAddress, UsedSettings.Port1, Service);
  UdpConnection AdcBox2(UsedSettings.EFUAddress, UsedSettings.Port2, Service);

  VecFixed<PoissonDelay> PoissionTimers(20);
  VecFixed<AmpEventDelay> AmpDelayTimers(20);
  VecFixed<ContinousSamplingTimer> ContinousTimers(20);

  // Box 1 timers
  {
    UdpConnection *CurAdc = &AdcBox1;

    if (ShouldCreateTimer(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                          UsedSettings)) {
      SetUpPoissonGenerator(Service, CurAdc, 0, 0,
                            {{"rate", UsedSettings.NoiseRate},
                             {"offset", 1.0},
                             {"amplitude", 2000.0}},
                            PoissionTimers.allocate());
    }

    if (ShouldCreateTimer(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                          UsedSettings)) {
      SetUpPoissonGenerator(Service, CurAdc, 0, 1,
                            {{"rate", UsedSettings.NoiseRate},
                             {"offset", 10.0},
                             {"amplitude", 3000.0}},
                            PoissionTimers.allocate());
    }

    if (ShouldCreateTimer(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                          UsedSettings)) {
      SetUpPoissonGenerator(Service, CurAdc, 0, 2,
                            {{"rate", UsedSettings.NoiseRate},
                             {"offset", 100.0},
                             {"amplitude", 4000.0}},
                            PoissionTimers.allocate());
    }

    if (ShouldCreateTimer(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                          UsedSettings)) {
      SetUpPoissonGenerator(Service, CurAdc, 0, 3,
                            {{"rate", UsedSettings.NoiseRate},
                             {"offset", 1000.0},
                             {"amplitude", 5000.0}},
                            PoissionTimers.allocate());
    }

    if (ShouldCreateTimer(SamplerType::Continous, CurAdc, &AdcBox2,
                          UsedSettings)) {
      SetUpContinousGenerator(Service, CurAdc, 0, 0,
                              {{"offset", 1000.0}, {"amplitude", 1000.0}},
                              ContinousTimers.allocate());
    }

    if (ShouldCreateTimer(SamplerType::AmpEventDelay, CurAdc, &AdcBox2,
                          UsedSettings)) {
      SetUpAmpPosGenerator(Service, CurAdc, 0, UsedSettings.EventRate,
                           AmpDelayTimers.allocate());
    }
  }

  // Box 2 timers
  {
    UdpConnection *CurAdc = &AdcBox2;

    if (ShouldCreateTimer(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                          UsedSettings)) {
      SetUpPoissonGenerator(Service, CurAdc, 1, 0,
                            {{"rate", UsedSettings.NoiseRate},
                             {"offset", 1.0},
                             {"amplitude", 2000.0}},
                            PoissionTimers.allocate());
    }

    if (ShouldCreateTimer(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                          UsedSettings)) {
      SetUpPoissonGenerator(Service, &AdcBox2, 1, 1,
                            {{"rate", UsedSettings.NoiseRate},
                             {"offset", 10.0},
                             {"amplitude", 3000.0}},
                            PoissionTimers.allocate());
    }

    if (ShouldCreateTimer(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                          UsedSettings)) {
      SetUpPoissonGenerator(Service, &AdcBox2, 1, 2,
                            {{"rate", UsedSettings.NoiseRate},
                             {"offset", 100.0},
                             {"amplitude", 4000.0}},
                            PoissionTimers.allocate());
    }

    if (ShouldCreateTimer(SamplerType::PoissonDelay, CurAdc, &AdcBox2,
                          UsedSettings)) {
      SetUpPoissonGenerator(Service, &AdcBox2, 1, 3,
                            {{"rate", UsedSettings.NoiseRate},
                             {"offset", 1000.0},
                             {"amplitude", 5000.0}},
                            PoissionTimers.allocate());
    }

    if (ShouldCreateTimer(SamplerType::Continous, CurAdc, &AdcBox2,
                          UsedSettings)) {
      SetUpContinousGenerator(Service, &AdcBox2, 1, 0,
                              {{"offset", 1000.0}, {"amplitude", 500.0}},
                              ContinousTimers.allocate());
    }
  }

  // for (auto &t : PoissionTimers)
  //  t.start();

  //for (auto &t : AmpDelayTimers)
  //  t.start();
  //for (auto &t : ContinousTimers)
  //  t.start();

  auto PrintStats = [&AdcBox1, &AdcBox2]() {
    std::cout << "Sampling runs generated by FPGA simulator 1: "
              << AdcBox1.getNrOfRuns();
    std::cout << "\nSampling runs generated by FPGA simulator 2: "
              << AdcBox2.getNrOfRuns() << "\n";
  };

  StatsTimer Stats(PrintStats, Service);
  Stats.start();

  std::thread AsioThread([&Service]() { Service.run(); });
  // std::thread AsioThread([]{});
  // Service.run();

  while (RunLoop) {
    // std::this_thread::sleep_for(500ms);

    for (auto &self : PoissionTimers) {
      // /// start()
      // double DelayTimeSec = self.RandomDistribution(self.RandomGenerator);
      // std::chrono::duration<size_t, std::nano> DelayTimeNs(
      //     static_cast<size_t>(DelayTimeSec /** 1e6*/));

      // self.EventTimer.expires_after(DelayTimeNs);
      // // self.EventTimer.async_wait([this](auto &Error) {
      // //  if (not Error) {
      // //    genSamplesAndEnqueueSend();
      // //    start();
      // //  }
      // //});

      //RelAssertMsg(PoissionTimers.Count == 1, "");

      /// genSamplesAndEnqueueSend
      auto WantedTimeNow = std::chrono::system_clock::now();
      auto NowSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                            WantedTimeNow.time_since_epoch())
                            .count();
      double NowSecFrac = (std::chrono::duration_cast<std::chrono::nanoseconds>(
                               WantedTimeNow.time_since_epoch())
                               .count() /
                           1e9) -
                          NowSeconds;
      std::uint32_t Ticks = std::lround(NowSecFrac * (88052500 / 2.0));

      RawTimeStamp rts{static_cast<uint32_t>(NowSeconds), Ticks};
      TimeStamp Time(rts, TimeStamp::ClockMode::External);

      ////////////////

      //static // Adding this static will recycle the first generate(), which will be fast!
      std::pair<void *, std::size_t> SampleRun =
          self.data.TimerData.SampleGen.generate(
              self.data.TimerData.Settings_amplitude, Time);

      self.data.TimerData.UdpCon->addSamplingRun(SampleRun.first,
                                                 SampleRun.second, Time);

    }
  }

  std::cout << "Waiting for transmit thread to exit!" << std::endl;
  Service.stop();
  if (AsioThread.joinable()) {
    AsioThread.join();
  }
  return 0;
}

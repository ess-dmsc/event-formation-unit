/** Copyright (C) 2019 European Spallation Source ERIC */

#include <string>
#include <vector>
#include <CLI/CLI.hpp>
#include <h5cpp/hdf5.hpp>
#include "WaveformData.h"
#include "FPGASim.h"
#include "DataModulariser.h"
#include "PoissonDelay.h"

bool RunLoop = true;

void signalHandler(int signal) {
  std::cout << "Got exit signal:" << signal << std::endl;
  RunLoop = false;
}

struct StreamSettings {
  std::string EFUAddress{"localhost"};
  std::uint16_t Port{65535};
  double EventRate{1000};
  std::string NeXuSFile;
  std::string WaveformPath;
};

void addCLIOptions(CLI::App &Parser, StreamSettings &Settings) {
  Parser
      .add_option("--efu_addr", Settings.EFUAddress,
                  "Address to which the data should be transmitted.")
      ->default_str("localhost");
  Parser
      .add_option(
          "--port", Settings.Port,
          "UDP port to which the data should be transmitted.")
      ->default_str("65535");
  Parser
      .add_option(
          "--event_rate", Settings.EventRate,
          "Events (waveforms) transmitted per second.")
      ->default_str("1000");
  Parser
      .add_option("--data_file", Settings.NeXuSFile,
                  "Path to (NeXus) file to be used as a data source.")->required(true);
  Parser
      .add_option("--nexus_path", Settings.WaveformPath,
                  "Path to group that has the waveform data. E.g. \"/entry/waveform_channel_\"")->required(true);
}

class FileSampler {
public:
  FileSampler(std::string const &FileName, std::string const &NexusPath) {
    InFile = hdf5::file::open(FileName,hdf5::file::AccessFlags::READONLY);
    RootNode = InFile.root();
    for (int i = 0; i < 4; i++) {
      auto Group = RootNode.get_group(NexusPath + std::to_string(i));
      Channels.emplace_back(Group);
    }
    for (auto &Ch : Channels) {
      CurrentTimestamp.emplace_back(std::make_pair(CurrentTimestamp.size(), Ch.getTimestamp()));
    }
  }
  std::pair<void const * const, std::size_t> generate() {
    auto MinimumTs = std::min_element(CurrentTimestamp.begin(), CurrentTimestamp.end(), [](auto const &A, auto const &B) {
        return A.second < B.second;
    });
    auto &CurrentChannel = Channels.at(MinimumTs->first);
    if (CurrentChannel.outOfData()) {
      RunLoop = false;
      return {nullptr, 0};
    }
    auto TempPair = Modulariser.modularise(CurrentChannel.getWaveform(), CurrentChannel.getTimestamp(), MinimumTs->first);
    CurrentChannel.nextWaveform();
    MinimumTs->second = CurrentChannel.getTimestamp();
    return TempPair;
  }
private:
  hdf5::file::File InFile;
  hdf5::node::Group RootNode;
  std::vector<WaveformData> Channels;
  std::vector<std::pair<int,std::uint64_t>> CurrentTimestamp;
  DataModulariser Modulariser;
};

auto SetUpContGenerator(asio::io_service &Service, FPGASim *FPGAPtr, StreamSettings &Settings) {
  auto SampleGen = std::make_shared<FileSampler>(Settings.NeXuSFile, Settings.WaveformPath);
  auto Glue = [Settings, SampleGen, FPGAPtr](RawTimeStamp const &) {
    auto SampleRun = SampleGen->generate();
    FPGAPtr->addSamplingRun(SampleRun.first, SampleRun.second);
  };
  return std::make_shared<PoissonDelay>(Glue, Service, Settings.EventRate);
}

using namespace std::chrono_literals;

class StatsTimer {
public:
  StatsTimer(std::function<void()> OnTimer, asio::io_service &Service)
      : TimerFunc(std::move(OnTimer)), Timer(Service){};
  void start() {
    Timer.expires_after(5s);
    auto Handler = [this](auto &Error) { this->handleEventTimer(Error); };
    Timer.async_wait(Handler);
  }
  void stop() { Timer.cancel(); }

private:
  void handleEventTimer(const asio::error_code &Error) {
    if (not Error) {
      TimerFunc();
      start();
    }
  };
  std::function<void()> TimerFunc;
  asio::system_timer Timer;
};

int main(const int argc, char *argv[]) {
  StreamSettings UsedSettings;
  CLI::App CLIParser{"Adc file streamer"};
  addCLIOptions(CLIParser, UsedSettings);
  try {
    CLIParser.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    CLIParser.exit(e);
    return 0;
  }

  asio::io_service Service;
  asio::io_service::work Worker(Service);

  auto AdcBox = std::make_shared<FPGASim>(UsedSettings.EFUAddress,
                                           UsedSettings.Port, Service);

  std::shared_ptr<SamplingTimer> DataTimer = SetUpContGenerator(Service, AdcBox.get(), UsedSettings);
  DataTimer->start();

  auto PrintStats = [&AdcBox]() {
    std::cout << "--------------------------\n";
    std::cout << "Sampling runs:   " << AdcBox->getNrOfRuns() << "\n";
    std::cout << "Created packets: " << AdcBox->getNrOfPackets() << "\n";
    std::cout << "Sent packets:    " << AdcBox->getNrOfSentPackets() << "\n";
  };

  StatsTimer Stats(PrintStats, Service);
  Stats.start();

  std::thread AsioThread([&Service]() { Service.run(); });

  while (RunLoop) {
    std::this_thread::sleep_for(500ms);
  }

  std::cout << "\nWaiting for transmit thread to exit!" << std::endl;
  Service.stop();
  if (AsioThread.joinable()) {
    AsioThread.join();
  }
  std::cout << "\nFinal tally:\n";
  std::cout << "Sampling runs:   " << AdcBox->getNrOfRuns() << "\n";
  std::cout << "Created packets: " << AdcBox->getNrOfPackets() << "\n";
  std::cout << "Sent packets:    " << AdcBox->getNrOfSentPackets() << "\n";
  return 0;
}

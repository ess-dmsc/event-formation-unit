/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/EFUArgs.h>
#include <common/Trace.h>
#include <cstdio>
#include <iostream>
#include <regex>
#include <string>
#include <fstream>

EFUArgs::EFUArgs() {
  CLIParser.set_help_flag(); //Removes the default help flag
  HelpOption = CLIParser.add_flag("-h,--help", "Print this help message and exit")->group("EFU Options")->configurable(false);
  CLIParser
      .add_option("-a,--logip", GraylogConfig.address,
                  "Graylog server IP address")
      ->group("EFU Options")
      ->set_default_val("127.0.0.1");
  CLIParser
      .add_option("-b,--broker_addr", EFUSettings.KafkaBrokerAddress,
                  "Kafka broker address")
      ->group("EFU Options")
      ->set_default_val("localhost");
  CLIParser
      .add_option("-k,--broker_port", EFUSettings.KafkaBrokerPort,
                  "Kafka broker port")
      ->group("EFU Options")
      ->set_default_val("9092");
  CLIParser
      .add_option("-t,--broker_topic", EFUSettings.KafkaTopic,
                  "Kafka broker topic")
      ->group("EFU Options")
      ->set_default_val("Detector_data");
  CLIParser
      .add_option("-c,--core_affinity",
                  [this](std::vector<std::string> Input) {
                    return parseAffinityStrings(Input);
                  },
                  "Thread to core affinity. Ex: \"-c input_t:4\"")
      ->group("EFU Options");
  DetectorOption = CLIParser.add_option("-d,--det", det, "Detector name")
                       ->group("EFU Options")
                       ->required();
  CLIParser
      .add_option("-i,--dip", EFUSettings.DetectorAddress,
                  "IP address of receive interface")
      ->group("EFU Options")
      ->set_default_val("0.0.0.0");
  CLIParser
      .add_option("-p,--port", EFUSettings.DetectorPort, "TCP/UDP receive port")
      ->group("EFU Options")
      ->set_default_val("9000");
  CLIParser
      .add_option("-m,--cmdport", EFUSettings.CommandServerPort,
                  "Command parser tcp port")
      ->group("EFU Options")
      ->set_default_val("8888");
  CLIParser
      .add_option("-g,--graphite", EFUSettings.GraphiteAddress,
                  "IP address of graphite metrics server")
      ->group("EFU Options")
      ->set_default_val("127.0.0.1");
  CLIParser
      .add_option("-o,--gport", EFUSettings.GraphitePort, "Graphite tcp port")
      ->group("EFU Options")
      ->set_default_val("2003");
  CLIParser
      .add_option("-s,--stopafter", EFUSettings.StopAfterSec,
                  "Terminate after timeout seconds")
      ->group("EFU Options")
      ->set_default_val("4294967295"); // 0xffffffffU
  WriteConfigOption = CLIParser.add_option("--write_config", ConfigFileName, "Write CLI options with default values to config file.")->group("EFU Options")->configurable(false);
  ReadConfigOption = CLIParser.set_config("--read_config", "", "Read CLI options from config file.", false)->group("EFU Options")->excludes(WriteConfigOption);
}

bool EFUArgs::parseAffinityStrings(
    std::vector<std::string> ThreadAffinityStrings) {
  bool CoreIntegerCorrect = false;
  int CoreNumber = 0;
  try {
    CoreNumber = std::stoi(ThreadAffinityStrings.at(0));
    CoreIntegerCorrect = true;
  } catch (std::invalid_argument &e) {
    // No nothing
  }
  if (ThreadAffinityStrings.size() == 1 and CoreIntegerCorrect) {
    ThreadAffinity.emplace_back(ThreadCoreAffinitySetting{
        "implicit_affinity", static_cast<std::uint16_t>(CoreNumber)});
  } else {
    std::string REPattern = "([^:]+):(\\d{1,2})";
    std::regex AffinityRE(REPattern);
    std::smatch AffinityRERes;
    for (auto &AffinityStr : ThreadAffinityStrings) {
      if (not std::regex_match(AffinityStr, AffinityRERes, AffinityRE)) {
        return false;
      }
      ThreadAffinity.emplace_back(ThreadCoreAffinitySetting{
          AffinityRERes[0],
          static_cast<std::uint16_t>(std::stoi(AffinityRERes[1]))});
    }
  }
  return true;
}

void EFUArgs::printSettings() {
  XTRACE(INIT, ALW, "Starting event processing pipeline2\n");
  XTRACE(INIT, ALW, "  Log IP:        %s\n", GraylogConfig.address.c_str());
  XTRACE(INIT, ALW, "  Detector:      %s\n", det.c_str());
  //    XTRACE(INIT, ALW, "  CPU Offset:    %d\n", cpustart);
  XTRACE(INIT, ALW, "  Config file:   %s\n", EFUSettings.ConfigFile.c_str());
  XTRACE(INIT, ALW, "  IP addr:       %s\n",
         EFUSettings.DetectorAddress.c_str());
  XTRACE(INIT, ALW, "  UDP Port:      %d\n", EFUSettings.DetectorPort);
  XTRACE(INIT, ALW, "  Kafka broker:  %s\n",
         EFUSettings.KafkaBrokerAddress.c_str());
  XTRACE(INIT, ALW, "  Graphite:      %s\n",
         EFUSettings.GraphiteAddress.c_str());
  XTRACE(INIT, ALW, "  Graphite port: %d\n", EFUSettings.GraphitePort);
  XTRACE(INIT, ALW, "  Command port:  %d\n", EFUSettings.CommandServerPort);
  XTRACE(INIT, ALW, "  Stopafter:     %u\n", EFUSettings.StopAfterSec);
}

void EFUArgs::printHelp() { std::cout << CLIParser.help(); }

EFUArgs::Status EFUArgs::parseFirstPass(const int argc, char *argv[]) {
  try {
    CLIParser.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
  }
  if ((*HelpOption and not *DetectorOption) or (not *HelpOption and not *DetectorOption)) {
    printHelp();
    return Status::EXIT;
  }
  CLIParser.reset();
  return Status::CONTINUE;
}

EFUArgs::Status EFUArgs::parseSecondPass(const int argc, char *argv[]) {
  try {
    CLIParser.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    CLIParser.exit(e);
    return Status::EXIT;
  }
  if (*HelpOption and *DetectorOption) {
    printHelp();
    return Status::EXIT;
  }
  if (*WriteConfigOption) {
    std::ofstream ConfigFile(ConfigFileName, std::ios::binary);
    if (not ConfigFile.is_open()) {
      std::cout << "Failed to open config file for writing." << std::endl;
      return Status::EXIT;
    }
    ConfigFile << CLIParser.config_to_str(true);
    ConfigFile.close();
  }
  return Status::CONTINUE;
}

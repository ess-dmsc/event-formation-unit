// Copyright (C) 2018-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Multi-Blade prototype detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#include <CLI/CLI.hpp>
#include <cinttypes>
#include <common/EFUArgs.h>
#include <common/EV42Serializer.h>
#include <common/Producer.h>
#include <common/Trace.h>
#include <common/TimeString.h>

#include <unistd.h>

#include <common/Socket.h>
#include <common/TSCTimer.h>
#include <common/Timer.h>

#include <h5cpp/hdf5.hpp>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

struct {
  std::string FileName;
  std::string KafkaBroker{"172.30.242.20:9092"};
  std::string KafkaTopic{"FREIA_detector"};
  int KafkaBufferSize {124000}; /// entries ~ 1MB
} Config;

CLI::App app{"Read event_id from hdf5 files and send to Kafka"};

int main(int argc, char *argv[]) {
  app.add_option("-f, --file", Config.FileName, "FileWriter HDF5");
  CLI11_PARSE(app, argc, argv);

  Producer eventprod(Config.KafkaBroker, Config.KafkaTopic);
  auto Produce = [&eventprod](auto DataBuffer, auto Timestamp) {
    eventprod.produce(DataBuffer, Timestamp);
  };

  EV42Serializer flatbuffer(Config.KafkaBufferSize, "multiblade", Produce);

  uint64_t efu_time = 1000000000LU * (uint64_t)time(NULL); // ns since 1970
  flatbuffer.pulseTime(efu_time);

  auto AnotherFile = hdf5::file::open(Config.FileName);
  auto RootGroup = AnotherFile.root();
  auto Dataset = RootGroup.get_dataset("/experiment/data/event_id");
  hdf5::dataspace::Simple Dataspace(Dataset.dataspace());
  auto Dimensions = Dataspace.current_dimensions();
  auto MaxDimensions = Dataspace.maximum_dimensions();

  std::cout << "Dataset dimensions\n";
  std::cout << "   Current | Max\n";

  for (unsigned int i = 0; i < Dimensions.size(); i++) {
    std::cout << "i:" << i << "      " << Dimensions[i] << " | "
              << MaxDimensions[i] << "\n";
  }

  std::cout << "\nAll elements\n";
  std::vector<uint32_t> AllElements(Dataspace.size());
  Dataset.read(AllElements);
  for (uint32_t Value : AllElements) {
    printf("%u, ", Value);
    flatbuffer.addEvent(0, Value);
  }
  printf("\n");
  flatbuffer.produce();

  return 0;
}

// Copyright (C) 2021 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief A streamer of efu events (only pixel ids) from hdf5 files
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <CLI/CLI.hpp>
#include <cinttypes>
#include <common/kafka/EV42Serializer.h>
#include <common/kafka/Producer.h>
#include <h5cpp/hdf5.hpp>
#include <unistd.h>

struct {
  std::string FileName;
  std::string KafkaBroker{"172.30.242.20:9092"};
  std::string KafkaTopic{"freia_detector"};
  int KafkaBufferSize {124000}; /// entries ~ 1MB
} Config;

CLI::App app{"Read event_id from hdf5 files and send to Kafka"};

int main(int argc, char *argv[]) {
  app.add_option("-f, --file", Config.FileName, "FileWriter HDF5");
  app.add_option("-b, --broker", Config.KafkaBroker, "Kafka broker");
  app.add_option("-t, --topic", Config.KafkaTopic, "Kafka topic");
  CLI11_PARSE(app, argc, argv);

  Producer eventprod(Config.KafkaBroker, Config.KafkaTopic);
  auto Produce = [&eventprod](auto DataBuffer, auto Timestamp) {
    eventprod.produce(DataBuffer, Timestamp);
  };

  EV42Serializer flatbuffer(Config.KafkaBufferSize, "hdf5pixel", Produce);

  uint64_t efu_time = 1000000000LU * (uint64_t)time(NULL); // ns since 1970
  flatbuffer.pulseTime(efu_time);

  auto HDF5File = hdf5::file::open(Config.FileName);
  auto RootGroup = HDF5File.root();
  auto Dataset = RootGroup.get_dataset("/experiment/data/event_id");
  hdf5::dataspace::Simple Dataspace(Dataset.dataspace());
  std::vector<uint32_t> AllElements(Dataspace.size());
  Dataset.read(AllElements);

  for (uint32_t Value : AllElements) {
    flatbuffer.addEvent(0, Value);
  }

  flatbuffer.produce();
  sleep(1);
  return 0;
}

// GCOVR_EXCL_STOP

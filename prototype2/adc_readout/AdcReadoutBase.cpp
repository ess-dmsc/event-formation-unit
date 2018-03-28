/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief ADC readout detector module.
 */

#include "AdcReadoutBase.h"
#include "AdcSettings.h"
#include "PeakFinder.h"
#include "SampleProcessing.h"
#include "libs/include/Socket.h"

AdcReadoutBase::AdcReadoutBase(BaseSettings Settings,
                               AdcSettings &ReadoutSettings)
    : Detector("AdcReadout", Settings), toParsingQueue(MessageQueueSize),
      ReadoutSettings(ReadoutSettings), GeneralSettings(Settings) {
  std::function<void()> inputFunc = [this]() { this->inputThread(); };
  Detector::AddThreadFunction(inputFunc, "input");
  std::map<std::string, TimeStampLocation> TimeStampLocationMap{
      {"Start", TimeStampLocation::Start},
      {"Middle", TimeStampLocation::Middle},
      {"End", TimeStampLocation::End}};

  std::function<void()> processingFunc = [this]() { this->parsingThread(); };
  Detector::AddThreadFunction(processingFunc, "parsing");
  Stats.setPrefix("adc_readout");
  Stats.create("input.bytes.received", AdcStats.input_bytes_received);
  Stats.create("parser.errors", AdcStats.parser_errors);
  Stats.create("parser.packets.total", AdcStats.parser_packets_total);
  Stats.create("parser.packets.idle", AdcStats.parser_packets_idle);
  Stats.create("parser.packets.data", AdcStats.parser_packets_data);
  Stats.create("parser.packets.stream", AdcStats.parser_packets_stream);
  Stats.create("processing.packets.lost", AdcStats.processing_packets_lost);
  AdcStats.processing_packets_lost = -1; // To compensate for the first error.

  if (ReadoutSettings.PeakDetection) {
    Processors.emplace_back(
        std::unique_ptr<AdcDataProcessor>(new PeakFinder(getProducer())));
  }
  if (ReadoutSettings.SerializeSamples) {
    std::unique_ptr<AdcDataProcessor> Processor(
        new SampleProcessing(getProducer(), ReadoutSettings.Name));
    dynamic_cast<SampleProcessing *>(Processor.get())
        ->setTimeStampLocation(
            TimeStampLocationMap.at(ReadoutSettings.TimeStampLocation));
    dynamic_cast<SampleProcessing *>(Processor.get())
        ->setMeanOfSamples(ReadoutSettings.TakeMeanOfNrOfSamples);
    dynamic_cast<SampleProcessing *>(Processor.get())
        ->setSerializeTimestamps(ReadoutSettings.SampleTimeStamp);
    Processors.emplace_back(std::move(Processor));
  }
}

std::shared_ptr<Producer> AdcReadoutBase::getProducer() {
  if (ProducerPtr == nullptr) {
    ProducerPtr = std::shared_ptr<Producer>(
        new Producer(GeneralSettings.KafkaBroker, GeneralSettings.KafkaTopic));
  }
  return ProducerPtr;
}

void AdcReadoutBase::inputThread() {
  std::int64_t BytesReceived = 0;
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPServer mbdata(local);
  mbdata.setbuffers(0, 2000000);
  mbdata.printbuffers();
  mbdata.settimeout(0, 100000); // One tenth of a second (100ms)
  ElementPtr DataElement;
  bool outCome;

  while (Detector::runThreads) {
    if (nullptr == DataElement) {
      outCome = toParsingQueue.waitGetEmpty(DataElement, 500);
      if (not outCome) {
        continue;
      }
    }
    int ReceivedBytes = mbdata.receive(static_cast<void *>(DataElement->Data),
                                       DataElement->MaxLength); // Fix cast
    DataElement->Length = ReceivedBytes;
    if (ReceivedBytes > 0) {
      BytesReceived += DataElement->Length;
      AdcStats.input_bytes_received = BytesReceived;
      toParsingQueue.tryPutData(std::move(DataElement));
    }
  }
}

void AdcReadoutBase::parsingThread() {
  ElementPtr DataElement;
  bool GotElement = false;
  while (Detector::runThreads) {
    GotElement = toParsingQueue.waitGetData(DataElement, 1000);
    if (GotElement) {
      try {
        PacketData ParsedAdcData = parsePacket(*DataElement);
        if (ParsedAdcData.GlobalCount != ++LastGlobalCount) {
          ++AdcStats.processing_packets_lost;
          LastGlobalCount = ParsedAdcData.GlobalCount;
        }
        ++AdcStats.parser_packets_total;
        if (PacketType::Data == ParsedAdcData.Type) {
          ++AdcStats.parser_packets_data;
        } else if (PacketType::Idle == ParsedAdcData.Type) {
          ++AdcStats.parser_packets_idle;
        } else if (PacketType::Stream == ParsedAdcData.Type) {
          ++AdcStats.parser_packets_stream;
        }
        for (auto &Processor : Processors) {
          (*Processor).processPacket(ParsedAdcData);
        }
      } catch (ParserException &e) {
        ++AdcStats.parser_errors;
      }
      while (not toParsingQueue.tryPutEmpty(std::move(DataElement)) and
             Detector::runThreads) {
        // Do nothing
      }
    }
  }
}

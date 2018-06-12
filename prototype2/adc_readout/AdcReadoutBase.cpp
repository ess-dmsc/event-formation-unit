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

AdcReadoutBase::AdcReadoutBase(BaseSettings const &Settings,
                               AdcSettings &ReadoutSettings)
    : Detector("AdcReadout", Settings), ReadoutSettings(ReadoutSettings),
      GeneralSettings(Settings) {
  // Note: Must call this->inputThread() instead of
  // AdcReadoutBase::inputThread() for the unit tests to work
  std::function<void()> inputFunc = [this]() { this->inputThread(); };
  Detector::AddThreadFunction(inputFunc, "input");
  const int NrOfChannels{4};
  for (int y = 0; y < NrOfChannels; y++) {
    DataModuleQueues.emplace_back(new Queue(MessageQueueSize));
    // Note: Must call this->processingThread() instead of
    // AdcReadoutBase::processingThread() for the unit tests to work
    std::function<void()> processingFunc = [this, y]() {
      this->processingThread(*this->DataModuleQueues.at(y));
    };
    Detector::AddThreadFunction(processingFunc,
                                "processing_" + std::to_string(y));
  }
  Stats.setPrefix("adc_readout" + ReadoutSettings.GrafanaNameSuffix);
  Stats.create("input.bytes.received", AdcStats.input_bytes_received);
  Stats.create("parser.errors", AdcStats.parser_errors);
  Stats.create("parser.unknown_channel", AdcStats.parser_unknown_channel);
  Stats.create("parser.packets.total", AdcStats.parser_packets_total);
  Stats.create("parser.packets.idle", AdcStats.parser_packets_idle);
  Stats.create("parser.packets.data", AdcStats.parser_packets_data);
  Stats.create("processing.packets.lost", AdcStats.processing_packets_lost);
  Stats.create("current_ts", AdcStats.current_ts_seconds);
  AdcStats.processing_packets_lost = -1; // To compensate for the first error.
}

std::shared_ptr<Producer> AdcReadoutBase::getProducer() {
  if (ProducerPtr == nullptr) {
    ProducerPtr = std::shared_ptr<Producer>(
        new Producer(GeneralSettings.KafkaBroker, GeneralSettings.KafkaTopic));
  }
  return ProducerPtr;
}

SamplingRun *AdcReadoutBase::GetDataModule(int Channel) {
  SpscBuffer::ElementPtr<SamplingRun> ReturnModule{nullptr};
  bool Success = DataModuleQueues.at(Channel)->tryGetEmpty(ReturnModule);
  if (Success) {
    return ReturnModule;
  }
  return nullptr;
}

bool AdcReadoutBase::QueueUpDataModule(SamplingRun *Data) {
  return DataModuleQueues.at(Data->Channel)
      ->tryPutData(SpscBuffer::ElementPtr<SamplingRun>(Data));
}

void AdcReadoutBase::inputThread() {
  std::int64_t BytesReceived = 0;
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(),
                         EFUSettings.DetectorPort);
  UDPReceiver mbdata(local);
  mbdata.setBufferSizes(0, 2000000);
  mbdata.printBufferSizes();
  mbdata.setRecvTimeout(0,
                        100000); // secs, usecs, One tenth of a second (100ms)
  InData ReceivedPacket;
  std::function<SamplingRun *(int)> DataModuleProducer(
      [this](int Channel) { return this->GetDataModule(Channel); });
  std::function<bool(SamplingRun *)> QueingFunction([this](SamplingRun *Data) {
    this->AdcStats.current_ts_seconds = Data->TimeStamp.Seconds;
    return this->QueueUpDataModule(Data);
  });
  PacketParser Parser(QueingFunction, DataModuleProducer);
  while (Detector::runThreads) {
    int ReceivedBytes = mbdata.receive(static_cast<void *>(ReceivedPacket.Data),
                                       ReceivedPacket.MaxLength); // Fix cast
    ReceivedPacket.Length = ReceivedBytes;
    if (ReceivedBytes > 0) {
      BytesReceived += ReceivedPacket.Length;
      AdcStats.input_bytes_received = BytesReceived;
      try {
        try {
          auto PacketInfo = Parser.parsePacket(ReceivedPacket);
          if (PacketInfo.GlobalCount != ++LastGlobalCount) {
            ++AdcStats.processing_packets_lost;
            LastGlobalCount = PacketInfo.GlobalCount;
          }
          ++AdcStats.parser_packets_total;
          if (PacketType::Data == PacketInfo.Type) {
            ++AdcStats.parser_packets_data;
          } else if (PacketType::Idle == PacketInfo.Type) {
            ++AdcStats.parser_packets_idle;
          }
        } catch (ParserException &e) {
          ++AdcStats.parser_errors;
        }
      } catch (ModuleProcessingException &E) {
        AdcStats.processing_buffer_full++;
        while (Detector::runThreads and
               not QueueUpDataModule(std::move(E.UnproccesedData))) {
        }
      }
    }
  }
}

void AdcReadoutBase::processingThread(Queue &DataModuleQueue) {
  std::vector<std::unique_ptr<AdcDataProcessor>> Processors;

  if (ReadoutSettings.PeakDetection) {
    Processors.push_back(
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

  bool GotModule = false;
  DataModulePtr Data;
  const std::int64_t TimeoutUSecs = 1000000;
  while (Detector::runThreads) {
    GotModule = DataModuleQueue.waitGetData(Data, TimeoutUSecs);
    if (GotModule) {
      try {
        for (auto &Processor : Processors) {
          (*Processor).processData(*Data);
        }
      } catch (ParserException &e) {
        ++AdcStats.parser_errors;
      }
      while (not DataModuleQueue.tryPutEmpty(std::move(Data)) and
             Detector::runThreads) {
        // Do nothing
      }
    }
  }
}

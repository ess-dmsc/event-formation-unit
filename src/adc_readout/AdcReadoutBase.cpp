/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  \brief ADC readout detector module.
 *  \todo This file should contain more comments
 *  \todo consider not creating stats dynamically
 */

#include "AdcReadoutBase.h"
#include "AdcSettings.h"
#include "PeakFinder.h"
#include "SampleProcessing.h"
#include "UDPClient.h"
#include <common/Log.h>
#include <iostream>
#include <memory>

AdcReadoutBase::AdcReadoutBase(BaseSettings const &Settings,
                               AdcSettings const &ReadoutSettings)
    : Detector("AdcReadout", Settings), ReadoutSettings(ReadoutSettings),
      GeneralSettings(Settings), Service(std::make_shared<asio::io_service>()),
      Worker(*Service) {
  // Note: Must call this->inputThread() instead of
  // AdcReadoutBase::inputThread() for the unit tests to work
  std::function<void()> inputFunc = [this]() { this->inputThread(); };
  Detector::AddThreadFunction(inputFunc, "input");

  Stats.setPrefix("adc_readout" + GeneralSettings.GraphiteRegion, "");
  Stats.create("input.bytes.received", AdcStats.InputBytesReceived);
  Stats.create("parser.errors", AdcStats.ParseErrors);
  Stats.create("parser.packets.total", AdcStats.ParserPacketsTotal);
  Stats.create("parser.packets.idle", AdcStats.ParserPacketsIdle);
  Stats.create("parser.packets.data", AdcStats.ParserPacketsData);
  Stats.create("current_ts", AdcStats.CurrentTsMs);
}

std::shared_ptr<Producer> AdcReadoutBase::getProducer() {
  std::lock_guard<std::mutex> Guard(ProducerMutex);
  if (ProducerPtr == nullptr) {
    ProducerPtr = std::make_shared<Producer>(GeneralSettings.KafkaBroker,
                                             GeneralSettings.KafkaTopic);
  }
  return ProducerPtr;
}

std::shared_ptr<DelayLineProducer>
AdcReadoutBase::getDelayLineProducer(OffsetTime UsedOffset) {
  std::lock_guard<std::mutex> Guard(DelayLineProducerMutex);
  if (DelayLineProducerPtr == nullptr) {
    std::string UsedTopic = GeneralSettings.KafkaTopic;
    if (not ReadoutSettings.DelayLineKafkaTopic.empty()) {
      UsedTopic = ReadoutSettings.DelayLineKafkaTopic;
    }
    DelayLineProducerPtr = std::make_shared<DelayLineProducer>(
        GeneralSettings.KafkaBroker, UsedTopic, ReadoutSettings, UsedOffset);
    Stats.create("events.delay_line", DelayLineProducerPtr->getNrOfEvents());
  }
  return DelayLineProducerPtr;
}

void AdcReadoutBase::stopThreads() {
  Service->stop();
  Detector::stopThreads();
}

uint64_t DataQueueIsEmpty = 0;
uint64_t DequeueModuleIsNull = 0;

SamplingRun *
AdcReadoutBase::getDataModuleFromQueue(ChannelID const Identifier) {

  // where we bind a queue in DataModuleQueues to processing in
  // processingThread().
  if (DataModuleQueues.find(Identifier) == DataModuleQueues.end()) {
    DataModuleQueues.emplace(
        std::make_pair(Identifier, std::make_unique<Queue>(MessageQueueSize)));
    auto ProcessingID = DataModuleQueues.size() - 1;
    auto ThreadName = "processing_" + std::to_string(ProcessingID);
    auto TempEventCounter = std::make_shared<std::int64_t>();
    *TempEventCounter = 0;
    Stats.create("events.adc" + std::to_string(Identifier.SourceID) + "_ch" +
                     std::to_string(Identifier.ChannelNr),
                 (*TempEventCounter));
    std::function<void()> processingFunc = [this, ThreadName, Identifier,
                                            TempEventCounter]() {
      this->processingThread(*this->DataModuleQueues.at(Identifier),
                             TempEventCounter);
    };
    Detector::AddThreadFunction(processingFunc, ThreadName);
    auto &NewThread = Detector::Threads.at(Detector::Threads.size() - 1);
    NewThread.thread = std::thread(NewThread.func);
    LOG(INIT, Sev::Debug,
        "Lazily launching processing thread for channel {} of ADC #{}.",
        Identifier.ChannelNr, Identifier.SourceID);
  }

  SpscBuffer::ElementPtr<SamplingRun> ReturnModule{nullptr};
  bool Success = DataModuleQueues.at(Identifier)->tryGetEmpty(ReturnModule);
  if (Success) {
    if (ReturnModule == nullptr) {
      DequeueModuleIsNull++;
    }
    return ReturnModule;
  }
  DataQueueIsEmpty++;
  return nullptr;
}

bool AdcReadoutBase::queueUpDataModule(SamplingRun *Data) {
  return DataModuleQueues.at(Data->Identifier)
      ->tryPutData(SpscBuffer::ElementPtr<SamplingRun>(Data));
}

/// \todo better packet counting semantics needed? Normally
/// all packets should be counted so that
/// packets_total == packets_idle + _data + _errors
void AdcReadoutBase::parsePacketWithStats(InData const &Packet,
                                    PacketParser &Parser) {
  if (Packet.Length > 0) {
    AdcStats.InputBytesReceived += Packet.Length;
    try {
      try {
        PacketInfo pi = Parser.parsePacket(Packet);
        ++AdcStats.ParserPacketsTotal;
        if (PacketType::Data == pi.Type) {
          ++AdcStats.ParserPacketsData;
        } else if (PacketType::Idle == pi.Type) {
          ++AdcStats.ParserPacketsIdle;
        }
      } catch (ParserException &e) {
        ++AdcStats.ParseErrors;
      }
    } catch (ModuleProcessingException &E) {
      AdcStats.ProcessingBufferFull++;
      while (Detector::runThreads and
             not queueUpDataModule(std::move(E.UnproccesedData))) {
      }
    }
  }

  static uint64_t DumpExceptionsCount = 0;  
  if (ReadoutSettings.DumpParserExceptionsCount && (DumpExceptionsCount++ & ((1 << 14) - 1)) == 0) {
    printf("\n");
    for (int i = 0; i < static_cast<int>(ParserException::Type::Count); i++) {
      printf("%s = %llu\n", ParserException::TypeNames[i],
             ParserException::ErrorTypeCount[i]);
    }
    printf("DataQueueIsEmpty = %llu\n"
           "DequeueModuleIsNull = %llu\n",
           DataQueueIsEmpty, DequeueModuleIsNull);
  }
}

void AdcReadoutBase::inputThread() {
  std::unique_ptr<UDPClient> UdpReceiver;

  std::function<SamplingRun *(ChannelID)> PullDataModuleFromQueue(
      [this](auto Identifier) {
        return this->getDataModuleFromQueue(Identifier);
      });

  std::function<bool(SamplingRun *)> PushDataModuleToQueue(
      [this](SamplingRun *Data) {
        this->AdcStats.CurrentTsMs =
            TimestampOffset.calcTimestampNS(Data->StartTime.getTimeStampNS()) /
            1000000;
        return this->queueUpDataModule(Data);
      });

  PacketParser Parser1(PushDataModuleToQueue, PullDataModuleFromQueue, 0);

  auto PacketParser = [&Parser1, this](auto Packet) {
    this->parsePacketWithStats(Packet, Parser1);
  };

  auto GetPacketsForConfig = [&](auto Packet) {
    try {
      ConfigInfo Config = parseHeaderForConfigInfo(Packet);
      TimestampOffset = OffsetTime(ReadoutSettings.TimeOffsetSetting,
                                   ReadoutSettings.ReferenceTime,
                                   Config.BaseTime.getTimeStampNS());
      LOG(INIT, Sev::Info, "Config packet received, starting data processing.");
      UdpReceiver->setPacketHandler(PacketParser);
    } catch (ParserException &E) {
      LOG(INIT, Sev::Error,
          "Failed to extract config data from packet due to the following "
          "error: {}",
          E.what());
    }
  };

  UdpReceiver = std::make_unique<UDPClient>(
      Service, EFUSettings.DetectorAddress, EFUSettings.DetectorPort,
      GetPacketsForConfig);

  LOG(INIT, Sev::Info,
      "Waiting for data packet to be used for parser configuration.");

  Service->run();
}

void AdcReadoutBase::processingThread(
    Queue &DataModuleQueue, std::shared_ptr<std::int64_t> EventCounter) {

  std::vector<std::unique_ptr<AdcDataProcessor>> Processors;

  auto UsedOffset = TimestampOffset;

  LOG(INIT, Sev::Info, "Process PeakDetection = {}", ReadoutSettings.PeakDetection);
  if (ReadoutSettings.PeakDetection) {
    Processors.emplace_back(
        std::make_unique<PeakFinder>( /// \todo likeky to continously alloc/free
                                      /// PulseParameters and likely leak
                                      /// EventData.
            getProducer(), ReadoutSettings.Name, UsedOffset));
  }

  LOG(INIT, Sev::Info, "Process SerializeSamples = {}",
      ReadoutSettings.SerializeSamples);
  if (ReadoutSettings.SerializeSamples) {
    auto Processor =
        std::make_unique<SampleProcessing>( /// \todo this is likely to
                                            /// continuously alloc/free
                                            /// ProcessedSamples.
            getProducer(), ReadoutSettings.Name, UsedOffset);
    Processor->setTimeStampLocation(
        TimeStampLocationMap.at(ReadoutSettings.TimeStampLocation));
    Processor->setMeanOfSamples(ReadoutSettings.TakeMeanOfNrOfSamples);
    Processor->setSerializeTimestamps(ReadoutSettings.SampleTimeStamp);
    Processors.emplace_back(std::move(Processor));
  }

  LOG(INIT, Sev::Info, "Process DelayLineDetector = {}",
      ReadoutSettings.DelayLineDetector);
  if (ReadoutSettings.DelayLineDetector) {
    Processors.emplace_back(std::make_unique<DelayLineProcessing>(
        getDelayLineProducer(UsedOffset), ReadoutSettings.Threshold));
  }

  DataModulePtr Data = nullptr;
  const std::int64_t TimeoutUSecs = 20000;
  while (Detector::runThreads) {
    bool GotModule = DataModuleQueue.waitGetData(Data, TimeoutUSecs);
    if (GotModule) {
      try {
        for (auto &Processor : Processors) {
          (*Processor).processData(*Data);
        }
        ++(*EventCounter);
      } catch (ParserException &e) {
        ++AdcStats.ParseErrors;
      }
      while (not DataModuleQueue.tryPutEmpty(std::move(Data)) and
             Detector::runThreads) {
        // Do nothing
      }
    }
  }
}

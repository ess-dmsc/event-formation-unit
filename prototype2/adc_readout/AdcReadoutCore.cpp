/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief ADC readout detector module.
 */

#include "AdcReadoutCore.h"
#include "libs/include/Socket.h"
#include "AdcSettings.h"

AdcReadoutCore::AdcReadoutCore(BaseSettings Settings, AdcSettingsStruct &AdcSettings) : Detector("AdcReadout", Settings), toParsingQueue(100), ProducerPtr(new Producer(Settings.KafkaBroker, Settings.KafkaTopic)), AdcSettings(AdcSettings) {
  std::function<void()> inputFunc = [this](){AdcReadoutCore::inputThread();};
  Detector::AddThreadFunction(inputFunc, "input");

  std::function<void()> processingFunc = [this](){AdcReadoutCore::parsingThread();};
  Detector::AddThreadFunction(processingFunc, "parsing");
  Stats.setPrefix("adc_readout");
  Stats.create("input.bytes.received", AdcStats.input_bytes_received);
  Stats.create("parser.errors.unknown", AdcStats.parser_errors_unknown);
  Stats.create("parser.errors.feedf00d", AdcStats.parser_errors_feedf00d);
  Stats.create("parser.errors.filler", AdcStats.parser_errors_filler);
  Stats.create("parser.errors.beefcafe", AdcStats.parser_errors_beefcafe);
  Stats.create("parser.errors.dlength", AdcStats.parser_errors_dlength);
  Stats.create("parser.errors.abcd", AdcStats.parser_errors_abcd);
  Stats.create("parser.errors.hlength", AdcStats.parser_errors_hlength);
  Stats.create("parser.errors.type", AdcStats.parser_errors_type);
  Stats.create("parser.errors.ilength", AdcStats.parser_errors_ilength);
  Stats.create("parser.packets.total", AdcStats.parser_packets_total);
  Stats.create("parser.packets.idle", AdcStats.parser_packets_idle);
  Stats.create("parser.packets.data", AdcStats.parser_packets_data);
  Stats.create("parser.packets.error", AdcStats.parser_packets_error);
  Stats.create("processing.packets.lost", AdcStats.processing_packets_lost);
  AdcStats.processing_packets_lost = -1; //To compensate for the first error.
  
  Processor = std::unique_ptr<AdcDataProcessor>(new PeakFinder(ProducerPtr));
  
}

void AdcReadoutCore::inputThread() {
  std::uint64_t BytesReceived = 0;
  Socket::Endpoint local(EFUSettings.DetectorAddress.c_str(), EFUSettings.DetectorPort);
  UDPServer mbdata(local);
  mbdata.setbuffers(0, 2000000);
  mbdata.printbuffers();
  mbdata.settimeout(0, 100000); // One tenth of a second
  ElementPtr DataElement;
  bool outCome;
  
  while (Detector::runThreads) {
    if (nullptr == DataElement) {
      outCome = toParsingQueue.waitGetEmpty(DataElement, 500);
      if (not outCome) {
        continue;
      }
    }
    DataElement->Length = mbdata.receive(static_cast<void*>(DataElement->Data), DataElement->MaxLength); //Fix cast
    if (DataElement->Length > 0) {
      BytesReceived += DataElement->Length;
      AdcStats.input_bytes_received = BytesReceived;
      toParsingQueue.tryPutData(std::move(DataElement));
    }
  }
}

void AdcReadoutCore::addParserError(ParserException::Type ExceptionType) {
  switch (ExceptionType) {
    case ParserException::Type::UNKNOWN:
      ++AdcStats.parser_errors_unknown;
      break;
    case ParserException::Type::TRAILER_FEEDF00D:
      ++AdcStats.parser_errors_feedf00d;
      break;
    case ParserException::Type::TRAILER_0x55:
      ++AdcStats.parser_errors_filler;
      break;
    case ParserException::Type::DATA_BEEFCAFE:
      ++AdcStats.parser_errors_beefcafe;
      break;
    case ParserException::Type::DATA_LENGTH:
      ++AdcStats.parser_errors_dlength;
      break;
    case ParserException::Type::DATA_ABCD:
      ++AdcStats.parser_errors_abcd;
      break;
    case ParserException::Type::HEADER_LENGTH:
      ++AdcStats.parser_errors_hlength;
      break;
    case ParserException::Type::HEADER_TYPE:
      ++AdcStats.parser_errors_type;
      break;
    case ParserException::Type::IDLE_LENGTH:
      ++AdcStats.parser_errors_ilength;
      break;
    default:
      ++AdcStats.parser_errors_unknown;
  }
}

void AdcReadoutCore::parsingThread() {
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
        }
        (*Processor)(ParsedAdcData);
//        ProcessingFunction(ParsedAdcData);
      } catch (ParserException &e) {
        addParserError(e.getErrorType());
        ++AdcStats.parser_packets_error;
      }
      while (not toParsingQueue.tryPutEmpty(std::move(DataElement)) and Detector::runThreads) {
        //Do nothing
      }
    }
  }
}

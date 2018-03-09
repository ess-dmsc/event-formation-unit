/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Sample processing of the ADC data.
 */

#pragma once

#include <memory>
#include <deque>
#include <vector>
#include <map>
#include "AdcSettings.h"
#include "AdcDataProcessor.h"

enum class TimeStampLocation {
  Start,
  Middle,
  End,
};

struct ProcessedSamples {
  int Channel;
  std::uint64_t TimeStamp;
  double TimeDelta;
  std::vector<std::uint16_t> Samples;
  std::vector<std::uint64_t> TimeStamps;
};

std::uint64_t CalcSampleTimeStamp(RawTimeStamp const &Start, RawTimeStamp const &End, TimeStampLocation const Location);

class ChannelProcessing {
public:
  ChannelProcessing() = default;
  ProcessedSamples operator()(DataModule const &Samples);
  void setMeanOfSamples(int NrOfSamples);
  int getMeanOfSamples() const {return MeanOfNrOfSamples;};
  void setTimeStampLocation(TimeStampLocation Location);
  TimeStampLocation getTimeStampLocation() const {return TSLocation;};
  void reset();
private:
  int MeanOfNrOfSamples{1};
  int SumOfSamples{0};
  int NrOfSamplesSummed{0};
  RawTimeStamp TimeStampOfFirstSample;
  TimeStampLocation TSLocation{TimeStampLocation::Middle};
};

class SampleProcessing : public AdcDataProcessor {
public:
  SampleProcessing(std::shared_ptr<ProducerBase> Prod, std::string const &Name);
  ~SampleProcessing() = default;
  virtual void operator()(PacketData const &Data) override;
  void setMeanOfSamples(int NrOfSamples);
  void setTimeStampLocation(TimeStampLocation Location);
  TimeStampLocation getTimeStampLocation() const {return TSLocation;};
protected:
  std::map<TimeStampLocation, int> TimeLocSerialisationMap{{TimeStampLocation::Start, 1}, {TimeStampLocation::Middle, 2}, {TimeStampLocation::End, 3}};
  std::string AdcName;
  virtual void serializeAndTransmitData(ProcessedSamples const &Data);
  std::map<int, std::uint64_t> MessageCounters;
  std::map<int, ChannelProcessing> ProcessingInstances;
  int MeanOfNrOfSamples{1};
  TimeStampLocation TSLocation{TimeStampLocation::Middle};
};


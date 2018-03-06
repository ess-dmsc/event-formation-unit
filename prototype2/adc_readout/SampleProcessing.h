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
#include "AdcDataProcessor.h"

enum class TimeStampLocation {
  Start,
  Middle,
  End,
};

struct ProcessedSamples {
  int ChannelNr;
  std::uint64_t TimeStamp;
  double TimeDelta;
  std::vector<std::uint16_t> Samples;
  std::vector<std::uint64_t> TimeStamps;
};

std::uint64_t CalcSampleTimeStamp(const RawTimeStamp &Start, const RawTimeStamp &End, const TimeStampLocation Location);

class ChannelProcessing {
public:
  ChannelProcessing();
  ProcessedSamples operator()(const DataModule &Samples);
  void setMeanOfSamples(int NrOfSamples);
  void setTimeStampLocation(TimeStampLocation Location);
  void reset();
private:
  int MeanOfNrOfSamples{1};
  int SumOfSamples{0};
  int NrOfSamplesSummed{0};
  RawTimeStamp TimeStampOfFirstSample;
  TimeStampLocation TSLocation{TimeStampLocation::Start};
};

class SampleProcessing : public AdcDataProcessor {
public:
  SampleProcessing(std::shared_ptr<Producer> Prod);
  virtual void operator()(const PacketData &Data) override;
  void setMeanOfSamples(int NrOfSamples);
  void setTimeStampLocation(TimeStampLocation Location);
  TimeStampLocation getTimeStampLocation() const {return TSLocation;};
protected:
  void serializeData(const ProcessedSamples &Data) const;
  void transmitData(const std::uint8_t &DataPtr, const size_t Size) const;
  std::map<int, ChannelProcessing> ProcessingInstances;
  int MeanOfNrOfSamples{1};
  TimeStampLocation TSLocation{TimeStampLocation::Start};
};


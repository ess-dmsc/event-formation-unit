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

std::uint64_t CalcSampleTimeStamp(RawTimeStamp const &Start, RawTimeStamp const &End, TimeStampLocation const Location);

class ChannelProcessing {
public:
  ChannelProcessing();
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
  SampleProcessing(std::shared_ptr<ProducerBase> Prod);
  virtual void operator()(PacketData const &Data) override;
  void setMeanOfSamples(int NrOfSamples);
  void setTimeStampLocation(TimeStampLocation Location);
  TimeStampLocation getTimeStampLocation() const {return TSLocation;};
protected:
  virtual void serializeAndTransmitData(ProcessedSamples const &Data) const;
  std::map<int, ChannelProcessing> ProcessingInstances;
  int MeanOfNrOfSamples{1};
  TimeStampLocation TSLocation{TimeStampLocation::Middle};
};


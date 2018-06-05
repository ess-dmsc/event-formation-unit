/** Copyright (C) 2018 European Spallation Source ERIC */

/** @file
 *
 *  @brief Sample processing of the ADC data.
 */

#pragma once

#include "AdcDataProcessor.h"
#include "AdcSettings.h"
#include <deque>
#include <map>
#include <memory>
#include <vector>

/// @brief For keeping track of where the time stamp originates in oversampled
/// data.
enum class TimeStampLocation {
  Start,
  Middle,
  End,
};

/// @brief Contains data that is to be serialized into a flatbuffer.
struct ProcessedSamples {
  ProcessedSamples() = default;
  ProcessedSamples(size_t NrOfSamples)
      : Samples(NrOfSamples), TimeStamps(NrOfSamples) {}
  int Channel;
  std::uint64_t TimeStamp;
  double TimeDelta;
  std::vector<std::uint16_t> Samples;
  std::vector<std::uint64_t> TimeStamps;
};

/// @brief Selects the Start, End or a midpoint between them (based on Location)
/// and returns that time point as nanoseconds.
/// @param[in] Start Time stamp of first sample.
/// @param[in] End Time stamp of last sample.
/// @param[in] Location Which timestamp to return.
/// @return The selected timestamp converted to nanoseconds.
std::uint64_t CalcSampleTimeStamp(RawTimeStamp const &Start,
                                  RawTimeStamp const &End,
                                  TimeStampLocation const Location);

/// @brief Does processing of individual samples before serialisation.
/// Implements oversampling functionality, i.e. taking the mean of x number of
/// samples.
class ChannelProcessing {
public:
  ChannelProcessing() = default;
  /// @brief Called with the data that is to be processed.
  /// @note Does not actually check that it is called with data from the correct
  /// channel. The caller has to make sure that this is the case.
  /// @param[in] Samples The datamodule (sampling run) to proccess.
  /// @return Processed (oversampled) data sample points.
  ProcessedSamples processModule(DataModule const &Samples);

  /// @brief Sets oversampling factor. Default on instantiation is 1, i.e. no
  /// oversampling.
  void setMeanOfSamples(int NrOfSamples);

  /// @brief Get oversampling factor.
  int getMeanOfSamples() const { return MeanOfNrOfSamples; };

  /// @brief Sets the time stamp location for overasmpled samples.
  /// @param[in] Location Three different values are possible; Start, Middle,
  /// End.
  void setTimeStampLocation(TimeStampLocation Location);

  /// @brief Get the time stamp location setting.
  TimeStampLocation getTimeStampLocation() const { return TSLocation; };

  /// @brief Resets the oversampling state.
  /// Should be called between the processing of non-contigous sample runs. Is
  /// called by ChannelProcessing::setMeanOfSamples().
  void reset();

private:
  int MeanOfNrOfSamples{1};
  int SumOfSamples{0};
  int NrOfSamplesSummed{0};
  RawTimeStamp TimeStampOfFirstSample;
  TimeStampLocation TSLocation{TimeStampLocation::Middle};
};

/// @brief Handles processing of sample data, serialization and transmission
/// (prouction) to a Kafka broker.
class SampleProcessing : public AdcDataProcessor {
public:
  /// @param[in] Prod Shared pointer to Kafka producer instance.
  /// @param[in] Name Name of the data source. Used when setting the name of the
  /// source of the flatbuffer.
  SampleProcessing(std::shared_ptr<ProducerBase> Prod, std::string const &Name);
  ~SampleProcessing() = default;

  /// @brief Called to actually process, serialise and transmit the (already)
  /// parsed data.
  /// @note Will NOT concatenate sample runs from the same channel. Samples from
  /// each data module in Data will be put in a seperate flatbuffer.
  /// @param[in] Data Parsed data to process.
  virtual void processData(DataModule const &Data) override;

  void setMeanOfSamples(int NrOfSamples);
  void setTimeStampLocation(TimeStampLocation Location);
  void setSerializeTimestamps(bool SerializeTimeStamps);
  TimeStampLocation getTimeStampLocation() const { return TSLocation; };

protected:
  std::map<TimeStampLocation, int> TimeLocSerialisationMap{
      {TimeStampLocation::Start, 1},
      {TimeStampLocation::Middle, 2},
      {TimeStampLocation::End, 3}};
  std::string AdcName;

  /// @brief Does the actual serialisation and transmission of the data.
  /// @note Should only be called by SampleProcessing::processPacket().
  virtual void serializeAndTransmitData(ProcessedSamples const &Data);
  std::uint64_t MessageCounter{0};
  ChannelProcessing ProcessingInstance;
  int MeanOfNrOfSamples{1};
  bool SampleTimestamps{false};
  TimeStampLocation TSLocation{TimeStampLocation::Middle};
};

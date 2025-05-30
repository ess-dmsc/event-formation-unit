// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator base class of artificial ESS readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <generators/functiongenerators/DistributionGenerator.h>
#include <common/readout/ess/Parser.h>
#include <common/system/SocketImpl.h>
#include <common/testutils/DataFuzzer.h>
#include <common/time/ESSTime.h>
#include <common/types/DetectorType.h>

#include <CLI/CLI.hpp>

#include <cstdint>
#include <memory>

///
/// \class ReadoutGeneratorBase
/// \brief Base class for UDP data generator for ESS readout data.
///
class ReadoutGeneratorBase {
public:
  /// \brief default frequency for all generators. It can be changed with command line parameter
  /// q, --frequency.
  static constexpr uint16_t DefaultFrequency{ 14 };

  ///
  /// \struct GeneratorSettings
  /// \brief Struct that holds the generator settings.
  ///
  // clang-format off
  struct GeneratorSettings {
    bool Debug{false};                                      ///< Print debug info

    uint32_t NFibers{2};                                    ///< Number of fibers
    uint32_t FiberMask{0x00ffffff};                         ///< Mask out unused fibers

    DetectorType Detector{DetectorType::RESERVED};          ///< Data type as specified in the ESS Readout ICD
    std::string IpAddress{"127.0.0.1"};                     ///< IP address for UDP transmission
    uint16_t UDPPort{9000};                                 ///< UDP port for transmission
    uint64_t NumberOfPackets{0};                            ///< Number of packets to transmit (0 means all packets)
    uint64_t SpeedThrottle{0};                              ///< Speed throttle for transmission
    uint64_t PktThrottle{0};                                ///< Packet throttle for transmission

    /// \todo This should be the default mode and obsolete per packet generation
    /// \todo Frequency should be a double instead of integer.
    uint16_t Frequency{ DefaultFrequency };                ///< Frequency of time updates for each packet

    uint8_t headerVersion{1};             ///< Header version
    bool Loop{false};                     ///< Flag to keep looping the same file forever
    bool Randomise{false};                ///< Flag to randomise header and data
    uint32_t KernelTxBufferSize{1000000}; ///< Kernel transmit buffer size
  } Settings;
  // clang-format on

  ///
  /// \brief Constructor for ReadoutGeneratorBase.
  /// \param detectorType The type of detector.
  ///
  ReadoutGeneratorBase(DetectorType detectorType=DetectorType::RESERVED);

  /// \brief Destructor for ReadoutGeneratorBase.
  virtual ~ReadoutGeneratorBase() = default;

  ///
  /// \brief Creates a packet ready for UDP transmission.
  /// Method will create as many network packets possible with in a pulse duration. Each packet will
  /// be populated with as many readout as possible.
  /// \param socket, interface to transmit object.
  /// \param pulseTimeDuration. Duration of a of a pulse in nano seconds
  void generatePackets(SocketInterface *socket, const esstime::TimeDurationNano &pulseTimeDuration);

  ///
  /// \brief Sets the readout data size.
  /// \param ReadoutSize The size of the readout data.
  ///
  void setReadoutDataSize(uint8_t ReadoutSize);

  ///
  /// \brief Sets number of readouts per packet.
  /// Method can be used to set number of readout to a different number
  /// that are possible to include in a packet. The number will be validated
  /// and if the value is to large program will throw an exception.
  /// \param ReadoutCount Readout count
  ///
  void setReadoutPerPacket(uint32_t ReadoutCount);

  ///
  /// \brief Process command line arguments, update settings.
  /// Exit application if -h defined or in case of parsing error.
  ///
  /// \param argc The number of command-line arguments.
  /// \param argv The command-line arguments.
  ///
  void argParse(int argc, char *argv[]);

  ///
  /// \brief Sets up a generator. Internal Buffers, socket, etc. are instantiated
  /// 
  /// \param generator A pointer to a readout generator that can return a value for readout
  /// time (input value) and a function/distribution value (output value).
  /// Input parameter should be DistributionGenerator when neutron arrival follows a probability distribution
  /// Input parameter should be LinearDistribution when neutrons are expect a specific intervals.
  /// 
  void initialize(std::shared_ptr<FunctionGenerator> readoutGenerator);

  ///
  /// \brief Start the transmission loop for the generator.
  ///
  void transmitLoop();

  static constexpr int BufferSize{8972}; ///< Size of the buffer
  uint8_t Buffer[BufferSize];            ///< Buffer for the packet

protected:
  CLI::App app{"UDP data generator for ESS readout data"};

  ///
  /// \brief Get a tuple containing the high and the low readout time. If the
  /// --ToF option is set the value will include a time of flight distribution
  /// calculated by the DistributionGenerator
  /// \param timeOfFlight  If specified, use this value for the time of flight
  /// \return Readout time pair [high, low].
  virtual std::pair<uint32_t, uint32_t> generateReadoutTime(double timeOfFlight);

  ///
  /// \overload
  virtual std::pair<uint32_t, uint32_t> generateReadoutTime();

  ///
  /// \return a time of flight value from the distribution generator
  double getTimeOffFlight() {
    return distributionGenerator->getValue();
  }

  // clang-format off
  static constexpr int MAX_TIME_DRIFT_NS{20}; ///< Maximum allowed pulse time drift
  const uint32_t TimeLowOffset{20000};        ///< Time offset for readout generation (ticks)
  const uint32_t PrevTimeLowOffset{10000};    ///< Previous time offset for readout generation (ticks)
  uint8_t ReadoutDataSize{0};                 ///< Size of the readout data
  uint16_t ReadoutPerPacket{0};               ///< Number of readouts
  uint64_t Packets{0};                        ///< Number of packets
  uint32_t SeqNum{0};                         ///< Sequence number
  uint16_t DataSize{0};                       ///< Number of data bytes in packet
  uint8_t HeaderSize{0};                      ///< Size of the header
  // clang-format on

  DataFuzzer Fuzzer; ///< Data fuzzer
  UDPTransmitter *DataSource{nullptr}; ///< Data source

private:
  ESSReadout::Parser::HeaderVersion headerVersion{
    ESSReadout::Parser::HeaderVersion::V0}; ///< Header version

  ///
  /// \brief Generates the header of the packet.
  ///
  void generateHeader();

  ///
  /// \brief Fills out the specified buffer with readouts.
  ///
  virtual void generateData() = 0;

  ///
  /// \brief Finishes the packet by incrementing the sequence number and
  /// performing fuzzing.
  ///
  void finishPacket();

  // clang-format off
  esstime::ESSTime pulseTime;                    ///< Pulse time
  esstime::ESSTime prevPulseTime;                ///< Previous pulse time
  esstime::ESSTime readoutTime;                  ///< Readout time
  esstime::TimeDurationNano pulseFrequencyNs{0}; ///< Pulse frequency in nanoseconds
  // clang-format on

  /// \brief For TOF distribution calculations
  /// TofDist could be calculated from default values in Settings struct
  /// by setting Frequency to default.
  std::shared_ptr<FunctionGenerator> distributionGenerator{};
  static constexpr double TicksPerMs{ esstime::ESSTime::ESSClockFreqHz/1000.0 };
};
// GCOVR_EXCL_STOP

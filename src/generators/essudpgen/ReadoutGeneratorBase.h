// Copyright (C) 2021 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator base class of artificial ESS readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <common/readout/ess/Parser.h>
#include <common/system/SocketImpl.h>
#include <common/testutils/DataFuzzer.h>
#include <common/time/ESSTime.h>
#include <common/types/DetectorType.h>
#include <flatbuffers/stl_emulation.h>
#include <generators/functiongenerators/DistributionGenerator.h>
#include <generators/functiongenerators/FunctionGenerator.h>

#include <CLI/CLI.hpp>

#include <cstdint>
#include <memory>

///
/// \class ReadoutGeneratorBase
/// \brief Base class for UDP data generator for ESS readout data.
///
class ReadoutGeneratorBase {
public:
  /// \brief default frequency for all generators. It can be changed with
  /// command line parameter q, --frequency.
  static constexpr uint16_t DEFAULT_FREQUENCY{14};

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
    uint16_t Frequency{ DEFAULT_FREQUENCY };                ///< Frequency of time updates for each packet

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
  ReadoutGeneratorBase(DetectorType detectorType = DetectorType::RESERVED);

  /// \brief Destructor for ReadoutGeneratorBase.
  virtual ~ReadoutGeneratorBase() = default;

  ///
  /// \brief Creates multiple packets with header, readout data and transmit it
  /// over UDP within one pulse time. Another part of the method controls the
  /// pulse time. Method will create as many network packets as possible with in
  /// a pulse duration where each packet will be populated with as many readout
  /// as possible with in the BufferSize limit. \param socket, interface to
  /// transmit object. \param pulseTimeDuration. Duration of a of a pulse in
  /// nano seconds
  void generatePackets(SocketInterface *socket,
                       const esstime::TimeDurationNano &pulseTimeDuration);

  ///
  /// \brief Sets the readout data size.
  /// \param ReadoutSize The size of the readout data.
  ///
  void setReadoutDataSize(uint8_t ReadoutSize);

  ///
  /// \brief Sets number of readouts per packet.
  /// Method can be used to set number of readout to a different number
  /// that are possible to include in a packet.
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
  /// \brief Sets up a generator. Internal Buffers, socket, etc. are
  /// instantiated
  ///
  /// \param readoutGenerator A unique pointer to a FunctionGenerator that
  /// provides time of flight distribution for readout time calculations. Use a
  /// DistributionGenerator implementation when neutron arrival follows a
  /// probability distribution, or a LinearDistribution when neutrons are
  /// expected at specific intervals. \throws std::runtime_error Header version
  /// is not V0 or V1.
  ///
  void initialize(std::unique_ptr<FunctionGenerator> readoutGenerator);

  ///
  /// \brief Start the transmission loop for the generator.
  ///
  void transmitLoop();

  static constexpr int BufferSize{8972}; ///< Size of the buffer
  uint8_t Buffer[BufferSize];            ///< Buffer for the packet

protected:
  CLI::App app{"UDP data generator for ESS readout data"};

  /// \brief Get a tuple containing the high and the low readout time which is
  /// generated by the readout time generator.
  ///
  /// \return Readout time pair [high, low].
  /// \throws std::runtime_error if readout time generator is not initialized.
  std::pair<uint32_t, uint32_t> generateReadoutTime() const;

  /// \brief Get the time of flight from the readout time generator.
  /// \return Time of flight in double precision.
  /// \note The unit depends on the generator configuration, typically in ms.
  /// \throws std::runtime_error if readout time generator is not initialized.
  esstime::TimeDurationNano getTimeOfFlightNS(esstime::ESSTime &) const;

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

  uint32_t numberOfReadouts{0}; ///< Number of readouts genearated
  // clang-format on

  DataFuzzer Fuzzer;                            ///< Data fuzzer
  std::unique_ptr<UDPTransmitter> DataSource{}; ///< Data source

private:
  ESSReadout::Parser::HeaderVersion headerVersion{
      ESSReadout::Parser::HeaderVersion::V0}; ///< Header version

  ///
  /// \brief Generates the header of the packet.
  /// \throws std::runtime_error if readouts per packet and readout data size
  /// will exceed buffer size.
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
  std::unique_ptr<FunctionGenerator> readoutTimeGenerator;
  static constexpr double TicksPerMs{esstime::ESSTime::ESSClockFreqHz / 1000.0};
};
// GCOVR_EXCL_STOP

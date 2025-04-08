// Copyright (C) 2024 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Generator of artificial CAEN readouts
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <caen/readout/DataParser.h>
#include <generators/essudpgen/ReadoutGeneratorBase.h>
#include <generators/functiongenerators/DistributionGenerator.h>

namespace Caen {

class ReadoutGenerator : public ReadoutGeneratorBase {
public:
  // Settings local to CAEN data generator
  struct {
    std::string Detector;

    /// \brief Generate nicely distributed tof data
    bool Tof{false};

    /// \brief If true, generate data for four amplitudes
    bool Loki{false};

    /// \brief If true, use bitmap images as pixel masks for the data
    bool Bitmaps{false};

    /// \brief Print debug info
    bool Debug{false};

    // Masks used to restrict generated data
    uint32_t AmplitudeMask{0xffff};  // Amplitudes
    uint8_t  FiberVals{24};
    uint32_t FiberMask{0xffffff};    // Fibers 0 - 23
    uint8_t  FENVals{16};
    uint32_t FENMask{0xffff};        // FENs 0 - 16
    uint8_t  GroupVals{8};
    uint32_t GroupMask{0xffff};      // Groups 0 - 7
  } CaenSettings;

  ReadoutGenerator();

  void setTypeByName(const std::string &Name) {
      Settings.TypeOverride = NameToType[Name];
      printf("Detector %s has type %u\n", Name.c_str(),
             Settings.TypeOverride);
  }

 protected:
  ///
  /// \brief Generate EFU readout data for the specified detector and mode.
  void generateData() override;

 private:
  ///
  /// \brief Generate random readout data
  void generateRandomData();

  ///
  /// \brief Generate readout data using bitmap mask to display a text.
  ///
  /// --------------------------------------------------------------------------
  /// Produce readout data that uses bitmaps to mask data, in order to produce
  /// images ot text characters. Asymmetrical letters will reveal mirror flips
  /// along both horizontal and vertical axes.
  ///
  /// We utilize the property that group ids G0 in [0, 1, 2, 3] produce even
  /// pixel strips of width 7, whereas group ids G1 in [4, 5, 6, 7] will
  /// produce uneven pixel strips of height 7.
  ///
  /// For a given FEN id, if we then combine two groups from each set, for
  /// example [0, 4], we will get a continuous strip with a pixel height of 14.
  ///
  /// Finally, we may then combine two consecutive FEN ids, for example 2 and 4,
  /// to get a a continuous pixel strip of height of 28. This then enables us to
  /// use bitmaps of size 28x28 to draw characters or images by masking randomly
  /// generated pixels.
  void generateMaskedData();

  ///
  ///  If the CLI option "--debug" is specified, print out debug IO
  void printDebug(const DataParser::CaenReadout &ReadoutData);

  ///
  /// \brief If the CLI option "--debug" is specified, test that all amplitudes
  /// can be generated for the entire range of straws and positions.
  void testAmplitudes() const;

  ///
  /// \brief Generate a straw id determined by the four amplitudes
  ///
  /// \param A First amplitude
  /// \param B Second amplitude
  /// \param C Third amplitude
  /// \param D Fourth amplitude
  ///
  /// \return the straw id
  double straw(int16_t A, int16_t B, int16_t C, int16_t D) const;

  ///
  /// \brief Generate a straw position determined by the four amplitudes
  ///
  /// \param A First amplitude
  /// \param B Second amplitude
  /// \param C Third amplitude
  /// \param D Fourth amplitude
  ///
  /// \return the straw position
  double pos(int16_t A, int16_t B, int16_t C, int16_t D) const;

  /// \brief From a straw id `s` and straw position `p`, construct the four
  ///        amplitudes associated with the id and the position.
  ///
  /// \param s Straw id
  /// \param p Straw position
  ///
  /// \return a tuple containing the four amplitudes A, B, C, and D
  std::tuple<int16_t, int16_t, int16_t, int16_t> amplitudes(size_t s, size_t p) const;

  ///
  /// \brief For a given readout index, return a pointer to the readout buffer
  /// at a given index.
  ///
  /// \param Index The readout index
  ///
  /// \return A pointer to the readout data
  DataParser::CaenReadout *getReadoutDataPtr(size_t Index);

  ///\brief For TOF distribution calculations
  DistributionGenerator TofDist{1000.0/14};
  float TicksPerMs{88552.0};
};
} // namespace Caen
// GCOVR_EXCL_STOP

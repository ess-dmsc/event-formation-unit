// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Common abstraction for Charge Division calibration (CDCalibration)
/// Implementing ideas from ownCloud folder
/// 'DM/detectors/02 instruments/01 common/03 Calibration/Charge division'
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/JsonFile.h>
#include <string>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Caen {

class CDCalibration {
public:
  CDCalibration(std::string Name) : Name(Name) { };

  /// \brief load json from file into the jsion root object
  CDCalibration(std::string Name, std::string CalibrationFile);

  /// \brief parse the calibration and validate internal consistency
  /// \retval True if file is valid, else False.
  void parseCalibration();

  /// \brief apply the position correction
  /// \param Pos the uncorrected position along the charge division unit
  /// \param GroupIndex which group are we in
  /// \param UnitIndex which polynomial of the N (GroupSize)
  /// \return the corrected position
  double posCorrection(int GroupIndex, int UnitIndex, double Pos);

  /// \brief intervals are vectors of vectors
  std::vector<std::vector<std::pair<double, double>>> Intervals;

  /// \brief coefficients are vectors of vectors of vectors
  std::vector<std::vector<std::vector<double>>> Calibration;

  // Grafana Counters
  struct {
    int64_t ClampLow{0};
    int64_t ClampHigh{0};
  } Stats;

  struct {
    // New abstraction: Groups is used in stead of Tubes(LOKI), Triplets(BIFROST),
    // TubePair(MIRACLES) etc.
    int Groups{0};
    // New abstraction: GroupSize is used instead of Straws(LOKI), Tubes(BIFROST)
    int GroupSize{0};

  } Parms;

  std::string ConfigFile{""};
  nlohmann::json root;

private:

  ///\brief log and trace then throw runtime exception
  void throwException(std::string Message);

  ///\brief Do an initial sanity check of the provided json file
  /// called from parseCaibration()
  void consistencyCheck();

  ///\brief Load the parameters into a suitable structure
  void loadCalibration();

  ///\brief validate that the supplied intervals are consistent
  ///\param Index groupindex used for error messages
  ///\param Parameter the parameter section object
  void validateIntervals(int Index, nlohmann::json Parameter);

  ///\brief helper function to validate points in an interval are within
  /// the unit interval.
  bool inUnitInterval(std::pair<double, double> & Pair);

  ///\brief validate that the provided polynomial coefficients have the
  /// expected sizes. More checks can be added for example we could calculate
  /// how large a fraction of the unit interval would clamp to high or low
  /// values and complain if the fraction is too large.
  ///\param Index groupindex used for error messages
    ///\param Parameter the parameter section object
  void validatePolynomials(int Index, nlohmann::json Parameter);

  ///\brief helper function to check that the returned value is an object.
  /// \todo not torally sure when it is expected to be this. For example if
  /// the returned value can be parsed as a string it is not an object.
  /// might be removed in the future if not useful.
  nlohmann::json getObjectAndCheck(nlohmann::json JsonObject, std::string Property);

  std::string Name{""}; ///< Detector/instrument name prvided in constructor

  std::string Message; /// Used for throwing exceptions.
};
} // namespace Caen

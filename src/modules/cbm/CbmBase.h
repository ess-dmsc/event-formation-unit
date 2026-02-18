// Copyright (C) 2022 - 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CBM detector base plugin interface definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/kafka/EV44Serializer.h>
#include <common/kafka/serializer/DA00HistogramSerializer.h>
#include <common/memory/HashMap2D.h>
#include <memory>
#include <modules/cbm/Counters.h>
#include <modules/cbm/SchemaDetails.h>
#include <modules/cbm/geometry/Config.h>
#include <modules/cbm/readout/Parser.h>

namespace cbm {

/// A class representing the CbmBase detector.
///
/// This class inherits from the Detector base class and provides additional
/// functionality specific to the CbmBase detector.
class CbmBase : public Detector {
public:
  /// Constructor.
  ///
  /// Constructs a CbmBase object with the given settings.
  ///
  /// \param settings The settings for the CbmBase detector.
  CbmBase(BaseSettings const &settings);

  /// Destructor.
  ~CbmBase() = default;

  /// The processing thread function.
  ///
  /// This function is responsible for the processing logic of the CbmBase
  /// detector.
  void processingThread();

  /// Struct representing counters.
  ///
  /// This struct holds counters for various operations related to the CbmBase
  /// detector.
  struct Counters Counters {};

protected:
  std::unique_ptr<Config> CbmConfiguration;
  std::unique_ptr<Parser> CbmParser;

private:
  std::unique_ptr<HashMap2D<SchemaDetails>> SchemaMap;
};

} // namespace cbm

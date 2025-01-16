// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of the PixelEventHandler class handles pixel data
/// events and epoch ESS pulse time events.
//===----------------------------------------------------------------------===//

#pragma once

#include "common/reduction/Hit2D.h"
#include "common/reduction/Hit2DVector.h"
#include "common/reduction/clustering/Abstract2DClusterer.h"
#include <common/kafka/EV44Serializer.h>
#include <common/reduction/clustering/Hierarchical2DClusterer.h>
#include <memory>
#include <modules/timepix3/Counters.h>
#include <modules/timepix3/dataflow/DataObserverTemplate.h>
#include <modules/timepix3/dto/TimepixDataTypes.h>
#include <modules/timepix3/geometry/Config.h>
#include <modules/timepix3/geometry/Timepix3Geometry.h>

namespace Timepix3 {

///
/// \brief The PixelEventHandler class handles pixel data events and epoch ESS
/// pulse time events.
///
/// This class is an implementation of the Observer::DataEventObserver interface
/// for the timepixDTO::ESSGlobalTimeStamp type. It provides methods for
/// applying pixel data and epoch ESS pulse time data, as well as pushing data
/// to Kafka.
///
/// The class can split the 2D frame into sub frames and process them in
/// parallel.
///
/// The PixelEventHandler class also contains private member variables for
/// counters, geometry, serializer, and lastEpochESSPulseTime. It uses a vector
/// of Hierarchical2DClusterer objects and a vector of Hit2DVector objects for
/// clustering pixel hits.
///
/// \see Observer::DataEventObserver
///
class PixelEventHandler
    : public Observer::DataEventObserver<timepixDTO::ESSGlobalTimeStamp> {

private:
  Counters &statCounters; /// < Reference to the Counters object for tracking
                          /// statistics.
  std::shared_ptr<Timepix3Geometry>
      geometry; /// < Shared pointer to the Timepix3Geometry object for geometry
                /// information.
  EV44Serializer &serializer; /// < Reference to the EV44Serializer object for
                              /// serialization.

  const Config
      &TimepixConfiguration; /// < Reference to the Timepix configuration.

  std::chrono::nanoseconds
      FrequencyPeriodNs; /// < Frequency period in nanoseconds.

public:
  ///
  /// \brief Constructs a new PixelEventHandler object with the specified
  /// counters, geometry, and serializer.
  ///
  /// This constructor initializes the PixelEventHandler object with the
  /// specified counters, geometry, and serializer.
  ///
  /// \param counters The Counters object for tracking statistics.
  /// \param geometry The shared pointer to the Timepix3Geometry object for
  /// geometry information.
  /// \param serializer The EV44Serializer object for serialization.
  ///
  PixelEventHandler(Counters &counters,
                    std::shared_ptr<Timepix3Geometry> geometry,
                    EV44Serializer &serializer,
                    const Config &TimepixConfiguration);

  ///
  /// \brief Destroys the PixelEventHandler object.
  ///
  /// This destructor is virtual to allow proper cleanup of derived classes.
  ///
  virtual ~PixelEventHandler(){};

  ///
  /// \brief Applies the epoch ESS pulse time data to the PixelEventHandler.
  ///
  /// This method is an implementation of the applyData method from the
  /// Observer::DataEventObserver interface. It takes a
  /// timepixDTO::ESSGlobalTimeStamp object as input and applies the epoch ESS
  /// pulse time data to the PixelEventHandler.
  ///
  /// \param epochEssPulseTime The timepixDTO::ESSGlobalTimeStamp object
  /// containing the epoch ESS pulse time data.
  ///

  ///
  /// \brief Pushes the data to Kafka.
  ///
  /// This method pushes the data to Kafka.
  ///
  int publishEvents(Cluster2DContainer &);

  void
  applyData(const timepixDTO::ESSGlobalTimeStamp &epochEssPulseTime) override;
};

} // namespace Timepix3
// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of the PixelEventHandler class handles pixel data
/// events and epoch ESS pulse time events.
//===----------------------------------------------------------------------===//

#pragma once

#include <chrono>
#include <common/kafka/EV44Serializer.h>
#include <common/reduction/Hit2DVector.h>
#include <common/reduction/clustering/Hierarchical2DClusterer.h>
#include <common/utils/EfuUtils.h>
#include <dataflow/DataObserverTemplate.h>
#include <dto/TimepixDataTypes.h>
#include <future>
#include <geometry/Timepix3Geometry.h>
#include <memory>
#include <modules/timepix3/Counters.h>
#include <vector>
#include "geometry/Config.h"

namespace Timepix3 {

/**
 * @brief The PixelEventHandler class handles pixel data events and epoch ESS
 * pulse time events.
 *
 * This class is an implementation of the Observer::DataEventObserver interface
 * for the timepixDTO::ESSGlobalTimeStamp type. It provides methods for applying
 * pixel data and epoch ESS pulse time data, as well as pushing data to Kafka.
 *
 * The class can split the 2D frame into sub frames and process them in
 * parallel.
 *
 * The PixelEventHandler class also contains private member variables for
 * counters, geometry, serializer, and lastEpochESSPulseTime. It uses a vector
 * of Hierarchical2DClusterer objects and a vector of Hit2DVector objects for
 * clustering pixel hits.
 *
 * @see Observer::DataEventObserver
 */
class PixelEventHandler
    : public Observer::DataEventObserver<timepixReadout::PixelReadout>,
      public Observer::DataEventObserver<timepixDTO::ESSGlobalTimeStamp> {

private:
  Counters &statCounters; /**< Reference to the Counters object for tracking
                             statistics. */
  std::shared_ptr<Timepix3Geometry>
      geometry; /**< Shared pointer to the Timepix3Geometry object for geometry
                   information. */
  EV44Serializer &serializer; /**< Reference to the EV44Serializer object for
                                 serialization. */

  const Config
      &TimepixConfiguration; /**< Reference to the Timepix configuration. */

  nanoseconds FrequencyPeriodNs; /**< Frequency period in nanoseconds. */

  std::unique_ptr<timepixDTO::ESSGlobalTimeStamp> lastEpochESSPulseTime =
      nullptr; /**< Unique pointer to the last epoch ESS pulse time. */

  std::vector<std::unique_ptr<Hierarchical2DClusterer>>
      clusterers; /**< Vector of unique pointers to Hierarchical2DClusterer
                     objects for clustering pixel hits. */
  std::vector<Hit2DVector> sub2DFrames; /**< Vector of Hit2DVector objects for
                                           storing clustered hits for sub frames
                                           in case of parrallel processing */

  /**
   * @brief Publishes the clustered events to the appropriate kafka topic.
   *
   * This method takes a Cluster2DContainer object as input and publishes the
   * clustered events to the appropriate kafka topic.
   *
   * @param clusters The Cluster2DContainer object containing the clustered
   * events.
   */
  void publishEvents(Cluster2DContainer &clusters);

  /**
   * @brief Clusters the pixel hits using the specified Hierarchical2DClusterer
   * object.
   *
   * This method takes a Hierarchical2DClusterer object and a Hit2DVector object
   * as input. It clusters the pixel hits using the specified clusterer and
   * stores the clustered hits in the windows vector.
   *
   * @param clusterer The Hierarchical2DClusterer object used for clustering.
   * @param hitsVector The Hit2DVector object containing the pixel hits to be
   * clustered.
   */
  void clusterHits(Hierarchical2DClusterer &clusterer, Hit2DVector &hitsVector);

  /**
   * @brief Calculates the global time based on the pixel time over threshold
   * (ToT), fine time over amplitude (fToA), and SPIDR time.
   *
   * This method takes the ToT, fToA, and SPIDR time as input and calculates the
   * global time.
   *
   * @param toa The pixel time over threshold (ToT).
   * @param fToA The fine time over amplitude (fToA).
   * @param spidrTime The SPIDR time.
   * @return The calculated global time.
   */
  uint64_t calculateGlobalTime(const uint16_t &toa, const uint8_t &fToA,
                               const uint32_t &spidrTime);

public:
  /**
   * @brief Constructs a new PixelEventHandler object with the specified
   * counters, geometry, and serializer.
   *
   * This constructor initializes the PixelEventHandler object with the
   * specified counters, geometry, and serializer.
   *
   * @param counters The Counters object for tracking statistics.
   * @param geometry The shared pointer to the Timepix3Geometry object for
   * geometry information.
   * @param serializer The EV44Serializer object for serialization.
   */
  PixelEventHandler(Counters &counters,
                    std::shared_ptr<Timepix3Geometry> geometry,
                    EV44Serializer &serializer, const Config &TimepixConfiguration);

  /**
   * @brief Destroys the PixelEventHandler object.
   *
   * This destructor is virtual to allow proper cleanup of derived classes.
   */
  virtual ~PixelEventHandler(){};

  /**
   * @brief Applies the pixel data to the PixelEventHandler.
   *
   * This method is an implementation of the applyData method from the
   * Observer::DataEventObserver interface. It takes a
   * timepixReadout::PixelReadout object as input and applies the pixel data to
   * the PixelEventHandler.
   *
   * @param pixelDataEvent The timepixReadout::PixelReadout object containing
   * the pixel data.
   */
  void applyData(const timepixReadout::PixelReadout &pixelDataEvent) override;

  /**
   * @brief Applies the epoch ESS pulse time data to the PixelEventHandler.
   *
   * This method is an implementation of the applyData method from the
   * Observer::DataEventObserver interface. It takes a
   * timepixDTO::ESSGlobalTimeStamp object as input and applies the epoch ESS
   * pulse time data to the PixelEventHandler.
   *
   * @param epochEssPulseTime The timepixDTO::ESSGlobalTimeStamp object
   * containing the epoch ESS pulse time data.
   */
  void
  applyData(const timepixDTO::ESSGlobalTimeStamp &epochEssPulseTime) override;

  /**
   * @brief Pushes the data to Kafka.
   *
   * This method pushes the data to Kafka.
   */
  void pushDataToKafka();
};

} // namespace Timepix3
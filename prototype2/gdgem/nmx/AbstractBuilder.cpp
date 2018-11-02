/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  \brief Bastract class for creating NMX hits
 */

#include "AbstractBuilder.h"

#include <common/Trace.h>

AbstractBuilder::AbstractBuilder(std::shared_ptr<AbstractClusterer> x,
                                 std::shared_ptr<AbstractClusterer> y) {
  clusterer_x = x;
  clusterer_y = y;
}

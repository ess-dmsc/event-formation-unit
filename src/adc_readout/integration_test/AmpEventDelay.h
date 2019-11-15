/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Header file for calculating the delay of an event.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "PoissonDelay.h"

class AmpEventDelay : public PoissonDelay {
public:
  AmpEventDelay(std::function<void(TimeStamp const &)> OnEvent,
                asio::io_service &AsioService, double EventRate)
      : PoissonDelay(std::move(OnEvent), AsioService, EventRate){};
};

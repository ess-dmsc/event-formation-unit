/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include "PoissonDelay.h"

class AmpEventDelay : public PoissonDelay {
public:
  AmpEventDelay(std::function<void(RawTimeStamp const &)> OnEvent,
                asio::io_service &AsioService, double EventRate)
      : PoissonDelay(std::move(OnEvent), AsioService, EventRate){};
};

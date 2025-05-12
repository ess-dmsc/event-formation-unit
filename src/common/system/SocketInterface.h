// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Socket interface for pure abstraction
///
//===----------------------------------------------------------------------===//

#pragma once

/// @brief Socket interface used for transmitting data
class SocketInterface {
public:
  /// Send data in buffer with specified length
  virtual int send(void const *dataBuffer, int dataLength) = 0;
};
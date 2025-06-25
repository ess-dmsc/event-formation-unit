// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Socket interface for pure abstraction
///
//===----------------------------------------------------------------------===//

#pragma once

/// \brief Socket interface used for transmitting data
/// Socket interface for data communication abstraction. Pure abstract class will be used for 
/// injection into generator method that creates multiple packets and unit test.
class SocketInterface {
public:
  virtual ~SocketInterface() = default;
  /// \brief Send data in buffer with specified length
  /// \param dataBuffer pointer to data buffer.
  /// \param dataLength length of buffer.
  virtual int send(void const *dataBuffer, int dataLength) = 0;
};
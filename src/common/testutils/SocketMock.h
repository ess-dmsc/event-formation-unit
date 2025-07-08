// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Socket mock used for generator unit test
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/system/SocketInterface.h>
#include <vector>

/// \brief Socket mock used for validating a single packet. Each call to send 
/// overwrites the content of internal buffer.
class SocketMock : public SocketInterface {
public:
  using Buffer = std::vector<char>;

  void Clear() { buffer.clear(); }

  const Buffer &GetData() const { return buffer; }

  int send(void const *dataBuffer, int dataLength) override {
    for (int i = 0; i < dataLength; i++) {
      buffer.emplace_back(((char *)dataBuffer)[i]);
    }
    return dataLength;
  }

  SocketMock() = default;
  ~SocketMock() = default;

private:
  Buffer buffer{};
};

/// \brief Socket mock used for validating a multiple packets. Each call to send 
/// adds data to a vector. Call clear to empty internal buffer
class MultipackageSocketMock : public SocketInterface {
public:
  using BufferItem = std::vector<char>;
  using BufferList = std::vector<BufferItem>;

  const BufferList &PackageList() const { return bufferList; }

  void Clear() { bufferList.clear(); }

  int send(void const *dataBuffer, int dataLength) override {
    BufferItem value{};
    value.reserve(dataLength);
    for (int i = 0; i < dataLength; i++) {
      value.emplace_back(((char *)dataBuffer)[i]);
    }

    bufferList.emplace_back(value);
    return dataLength;
  }

  MultipackageSocketMock() = default;
  ~MultipackageSocketMock() = default;

private:
  BufferList bufferList{};
};

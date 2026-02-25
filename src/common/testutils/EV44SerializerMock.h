// Copyright (C) 2025 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Mock for EV44Serializer to validate addEvent calls in instrument
///        tests. Uses GMock to intercept and verify time/pixel arguments.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/kafka/EV44Serializer.h>
#include <gmock/gmock.h>

/// \brief Mock EV44Serializer that intercepts addEvent() calls.
/// Use with testing::NiceMock<> to suppress warnings for unverified calls,
/// or raw EV44SerializerMock for strict verification.
class EV44SerializerMock : public EV44Serializer {
public:
  EV44SerializerMock() : EV44Serializer(0, "mock") {}
  MOCK_METHOD(size_t, addEvent, (int32_t time, int32_t pixel), (override));
};

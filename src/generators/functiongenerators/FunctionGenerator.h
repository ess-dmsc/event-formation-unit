// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Interface for function generators
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

class FunctionGenerator {
public:
    virtual ~FunctionGenerator() {}

    virtual double getValue(const double&) = 0;
};

// GCOVR_EXCL_STOP
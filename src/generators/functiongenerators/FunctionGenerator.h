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
    
    /// \brief Get the value at a given position in the generator function.
    virtual double getDistValue(const double&) = 0;
    
    /// \brief return a random value based on the distribution function
    virtual double getValue() = 0;
};

// GCOVR_EXCL_STOP
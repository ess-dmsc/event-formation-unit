// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Provide a printf based hexdump functionality
///
//===----------------------------------------------------------------------===//

#pragma once

#include <stdio.h>

/// \brief make a hexdump-like printout of a buffer, mainly debugging
/// \param DataPtr void pointer to the buffer containing data
/// \param DataLen length of the data
void hexDump(const void* DataPtr, size_t DataLen);

// Copyright (C) 2018-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief test-helper class for saving a buffer to file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <string>

/// \brief save buffer to file
void saveBuffer(std::string filename, void *buffer, uint64_t datasize);

/// \brief helper function to remove temporary files after test
void deleteFile(std::string filename);

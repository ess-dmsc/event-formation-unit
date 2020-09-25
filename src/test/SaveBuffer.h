/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <string>
#include <cinttypes>

/// \brief functionality moved away from DataSave class and into test
void saveBuffer(std::string filename, void *buffer, uint64_t datasize);

/// \brief helter function to remove temporary files after test
void deleteFile(std::string filename);

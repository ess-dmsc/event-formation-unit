/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Wrapper class for reading Jalousie data from binary file
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <cstdio>
#include <cstdint>
#include <string>

const uint64_t kNeutronDataType   = (0x0ULL)<<62;
const uint64_t kMetaDataType      = (0x2ULL)<<62;
const uint64_t kAdcDataType       = (0x3ULL)<<62;

const uint64_t kMetaDataIndex1{uint64_t(0x1) << 58};

const uint64_t kMetaDataSubIndex0 = (0x0ULL)<<54;
const uint64_t kMetaDataSubIndex1 = (0x1ULL)<<54;

const uint64_t kMetaDataSubIndex5 = (0x5ULL)<<54;
const uint64_t kMetaDataSubIndex6 = (0x6ULL)<<54;
const uint64_t kMetaDataSubIndex7 = (0x7ULL)<<54;
const uint64_t kMetaDataSubIndex8 = (0x8ULL)<<54;
const uint64_t kMetaDataSubIndex9 = (0x9ULL)<<54;
const uint64_t kMetaDataSubIndex10 =(0xAULL)<<54;

class ReaderJalousie {
public:
  ReaderJalousie(std::string filename);
  ~ReaderJalousie();
  int getNextReadout(char * buffer);
  int read(char *buffer, size_t bufferlen);

private:
  FILE * FilePtr{nullptr};
  bool HeaderRead{false};
  uint64_t Entries{0};
  uint32_t BoardId{0};
};
// GCOVR_EXCL_STOP

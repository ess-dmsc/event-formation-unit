// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Provide a printf based hexdump functionality
///
/// From https://gist.github.com/ccbrown
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <common/debug/Hexdump.h>

void hexDump(const void *DataPtr, size_t DataLen) {
  char ascii[17];
  size_t i, j;
  ascii[16] = '\0';
  for (i = 0; i < DataLen; ++i) {
    printf("%02X ", ((unsigned char *)DataPtr)[i]);
    if (((unsigned char *)DataPtr)[i] >= ' ' &&
        ((unsigned char *)DataPtr)[i] <= '~') {
      ascii[i % 16] = ((unsigned char *)DataPtr)[i];
    } else {
      ascii[i % 16] = '.';
    }
    if ((i + 1) % 8 == 0 || i + 1 == DataLen) {
      printf(" ");
      if ((i + 1) % 16 == 0) {
        printf("|  %s \n", ascii);
      } else if (i + 1 == DataLen) {
        ascii[(i + 1) % 16] = '\0';
        if ((i + 1) % 16 <= 8) {
          printf(" ");
        }
        for (j = (i + 1) % 16; j < 16; ++j) {
          printf("   ");
        }
        printf("|  %s \n", ascii);
      }
    }
  }
}
// GCOVR_EXCL_STOP

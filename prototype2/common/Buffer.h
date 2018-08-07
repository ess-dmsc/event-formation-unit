/// Copyright (C) 2016-2018 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief common definition of buffer
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <cstddef>

// \todo make specific types, maybe template?

struct Buffer {
  Buffer() {}

  Buffer(char *address, size_t sz)
      : buffer(address), size(sz) {}

  Buffer(uint16_t *address, size_t sz)
      : buffer(reinterpret_cast<char*>(address)), size(sz) {}

  Buffer(uint32_t *address, size_t sz)
      : buffer(reinterpret_cast<char*>(address)), size(sz) {}

  char *buffer{nullptr};
  size_t size{0};
};


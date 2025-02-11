// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief functions specific to gcc compilers and cpu architecture
///
//===----------------------------------------------------------------------===//

#pragma once

#ifdef __ARM_ARCH
  #include <common/system/arm.h>
#else
  #include <common/system/intel.h>
#endif

// gcc specific macros

/// branch prediction macros
#if 0
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif


#ifdef RELEASE
#define UNUSED
#else
#define UNUSED __attribute__((unused))
#endif

#define ALIGN(x) __attribute__((aligned(x)))

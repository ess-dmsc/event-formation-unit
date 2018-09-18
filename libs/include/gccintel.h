/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief functions specific to gcc compilers and intel cpus
///
//===----------------------------------------------------------------------===//

#pragma once

/// branch prediction macros
#if 0
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

/// read time stamp counter - runs at processer Hz
static __inline__ unsigned long long rdtsc(void) {
  unsigned hi, lo;
  __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
  return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

#ifdef RELEASE
#define UNUSED
#else
#define UNUSED __attribute__((unused))
#endif

#define ALIGN(x) __attribute__((aligned(x)))
//#define ALIGN(x)

// Copyright (C) 2020-2020 European Spallation Source, ERIC. See LICENSE file
#pragma once

#include <common/Expect.h>
#include <stdio.h>
#include <stdlib.h>

/// \brief macro RelAssertMsg() provides an assert with a message.
///        It's always on in Release builds, however it has only small perf
///        impact, as the condition expression is always expected to be true.
///        It's made to ensure correct failure modes in Google Test (compiled in
///        Debug) and when running large datasets, while cannot run well in
///        Debug builds.

#define HandleAssertFail(exp, file, line, msg)                                 \
  ((void)fprintf(stderr, "%s:%u: failed assertion `%s': \"%s\"\n", file, line, \
                 exp, msg),                                                    \
   abort())

// for now the asserts are always on!
// TODO make the asserts primarity for DEBUG and google test.
#define RelAssertMsg(exp, msg)                                                 \
  (UNLIKELY(!(exp)) ? HandleAssertFail(#exp, __FILE__, __LINE__, msg) : (void)0)

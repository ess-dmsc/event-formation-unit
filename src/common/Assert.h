#pragma once

#include <stdio.h>
#include <stdlib.h>

#define HandleAssertFail(exp, file, line, msg)                                 \
  ((void)fprintf(stderr, "%s:%u: failed assertion `%s': \"%s\"\n", file, line, \
                 exp, msg),                                                    \
   abort())

// for now the asserts are always on!
// TODO make the asserts primarity for DEBUG and google test.
#define RelAssertMsg(exp, msg)                                                 \
  (__builtin_expect(!(exp), 0)                                                 \
       ? HandleAssertFail(#exp, __FILE__, __LINE__, msg)                       \
       : (void)0)
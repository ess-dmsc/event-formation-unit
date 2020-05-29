// Copyright (C) 2020-2020 European Spallation Source, ERIC. See LICENSE file
#pragma once

/// \brief macros LIKELY and UNLIKELY are hints for codegeneration, when some
///        code path is unlikely or expectional.

#if 1

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#else

// option to disable
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)

#endif
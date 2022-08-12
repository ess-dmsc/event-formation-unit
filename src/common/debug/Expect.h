// Copyright (C) 2020-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief macros LIKELY and UNLIKELY are hints for codegeneration, when some
///        code path is unlikely or expectional.
//===----------------------------------------------------------------------===//

#pragma once

#if 1

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#else

// option to disable
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)

#endif

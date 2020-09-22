// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation (\todo put in header?
//===----------------------------------------------------------------------===//


#include <common/TSCTimer.h>

/** */
TSCTimer::TSCTimer(void) { t1 = rdtsc(); }

/** */
void TSCTimer::now(void) { t1 = rdtsc(); }

/** */
uint64_t TSCTimer::timetsc(void) { return (rdtsc() - t1); }

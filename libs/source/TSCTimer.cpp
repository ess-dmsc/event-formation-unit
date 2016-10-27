/** Copyright (C) 2016 European Spallation Source */

#include <libs/include/TSCTimer.h>

/** */
TSCTimer::TSCTimer(void) { t1 = rdtsc(); }

/** */
void TSCTimer::now(void) { t1 = rdtsc(); }

/** */
uint64_t TSCTimer::timetsc(void) { return (rdtsc() - t1); }

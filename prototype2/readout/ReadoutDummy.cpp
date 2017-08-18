/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Trace.h>
#include <libs/include/gccintel.h> // UNUSED macros
#include <readout/ReadoutDummy.h>

int ReadoutDummy::parse(const char *buffer, uint32_t size) {

  if (validate(buffer, size) < 0) {
    return -1;
  }

  if (type != detectortype) {
    XTRACE(PROCESS, WAR,
           "Type idendifier in packet (%d) does not match instrument (%d)\n",
           type, detectortype);
    return -1;
  }

  return 0;
}

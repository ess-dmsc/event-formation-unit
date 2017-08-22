/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <common/Trace.h>
#include <cstring>
#include <sonde/ideas/Data.h>

using namespace std;

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

/** @todo no error checking, assumes valid data and valid buffer */
int IDEASData::createevent(uint32_t time, uint32_t pixel_id, char *buffer) {

  if (pixel_id < 1) {
    XTRACE(PROCESS, WAR, "invalid pixel_id %d\n", pixel_id);
    return -1;
  }

  static_assert(sizeof(time) == 4, "time should be 32 bit");
  static_assert(sizeof(pixel_id) == 4, "pixelid should be 32 bit");

  std::memcpy(buffer + 0, &time, sizeof(time));
  std::memcpy(buffer + 4, &pixel_id, sizeof(pixel_id));
  return 0;
}

int IDEASData::receive(const char *buffer, int size) {

  assert(buffer != nullptr);
  assert(size > 0);

  return 1; /** @todo implement data parser */
}

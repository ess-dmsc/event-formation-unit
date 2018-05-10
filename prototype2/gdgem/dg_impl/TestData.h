#pragma once
#include <vector>

struct Hit
{
  unsigned int fec;
  unsigned int chip_id;
  unsigned int framecounter;
  unsigned int srs_timestamp;
  unsigned int channel;
  unsigned int bcid;
  unsigned int tdc;
  unsigned int adc;
  unsigned int overthreshold;
};

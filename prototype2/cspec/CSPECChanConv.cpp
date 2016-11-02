/** Copyright (C) 2016 European Spallation Source ERIC */

#include <algorithm>
#include <cspec/CSPECChanConv.h>

CSPECChanConv::CSPECChanConv() {
  std::fill_n(wirecal, adcsize, 0);
  std::fill_n(gridcal, adcsize, 0);
}

int CSPECChanConv::makecal(uint16_t *array, unsigned int min, unsigned int max,
                           unsigned int nb_channels) {
  if ((min >= max) || (min >= CSPECChanConv::adcsize) ||
      (max >= CSPECChanConv::adcsize))
    return -1;
  if (nb_channels > (max - min))
    return -1;

  for (unsigned int adc = 0; adc < CSPECChanConv::adcsize; adc++) {
    if (adc < min) {
      array[adc] = CSPECChanConv::adcsize - 1;
    } else if (adc > max) {
      array[adc] = CSPECChanConv::adcsize - 1;
    } else {
      array[adc] = ((adc - min) * nb_channels) / (max - min);
    }
  }
  return 0;
}

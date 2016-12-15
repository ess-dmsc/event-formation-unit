/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Lookup (calibration) tables for conversion of CSPEC adc to wire/grid
 * number
 */

#pragma once
#include <cinttypes>

class CSPECChanConv {
public:
  static const int adcsize = 16384;

  /** @todo document */
  CSPECChanConv();

  /** @todo document */
  uint16_t getwireid(unsigned int wire_adc) { return wirecal[wire_adc]; };

  /** @todo document */
  uint16_t getgridid(unsigned int grid_adc) { return gridcal[grid_adc]; };

  /** @todo document */
  int makewirecal(unsigned int min, unsigned int max,
                  unsigned int nb_channels) {
    return makecal(wirecal, min, max, nb_channels);
  }

  /** @todo document */
  int makegridcal(unsigned int min, unsigned int max,
                  unsigned int nb_channels) {
    return makecal(gridcal, min, max, nb_channels);
  }

  /** @brief loads wire and grid calibrations from efu_args
   */
  void load_calibration(uint16_t *wirecal_new, uint16_t *gridcal_new);

private:
  // Generate a linear map from adc values to N channels
  int makecal(uint16_t *array, unsigned int min, unsigned int max,
              unsigned int nb_channels);
  uint16_t wirecal[adcsize];
  uint16_t gridcal[adcsize];
};

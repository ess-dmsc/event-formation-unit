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
  CSPECChanConv();

  uint16_t getWireId(unsigned int wire_adc) { return wirecal[wire_adc]; };
  uint16_t getGridId(unsigned int grid_adc) { return gridcal[grid_adc]; };

  int makewirecal(unsigned int min, unsigned int max,
                  unsigned int nb_channels) {
    return makecal(wirecal, min, max, nb_channels);
  }

  int makegridcal(unsigned int min, unsigned int max,
                  unsigned int nb_channels) {
    return makecal(gridcal, min, max, nb_channels);
  }

  /** @brief loads wire and grid calibrations from efu_args
   */
  void load_calibration(uint16_t * wirecal_new, uint16_t * gridcal_new);

private:
  // Generate a linear map from adc values to N channels
  int makecal(uint16_t *array, unsigned int min, unsigned int max,
              unsigned int nb_channels);
  uint16_t wirecal[adcsize];
  uint16_t gridcal[adcsize];
};

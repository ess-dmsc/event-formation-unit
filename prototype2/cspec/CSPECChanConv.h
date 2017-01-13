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

  /** @brief constructor, sets calibration data to 0 */
  CSPECChanConv();

  /** @brief constructor, sets calibration data to 0
   * @todo document
   */
  CSPECChanConv(uint16_t initval);

  /** @brief return the wireid from an adc value using calibration data
   *  @param wire_adc readout value for wire position
   */
  uint16_t getwireid(unsigned int wire_adc) { return wirecal[wire_adc]; };

  /** @brief return the gridid from an adc value using calibration data
   *  @param grid_adc readout value for wire position
   */
  uint16_t getgridid(unsigned int grid_adc) { return gridcal[grid_adc]; };

  /** @brief generate linear wire calibration, used for testing
   *  @param min starting offset for wire ids
   *  @param max ending offset for wire ids
   *  @param nb_channels number of channels to generate
   */
  int makewirecal(unsigned int min, unsigned int max,
                  unsigned int nb_channels) {
    return makecal(wirecal, min, max, nb_channels);
  }

  /** @brief generate linear wire calibration, used for testing
   *  @param min starting offset for grid ids
   *  @param max ending offset for grid ids
   *  @param nb_channels number of channels to generate
   */
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
  uint16_t wirecal[adcsize]; /**< holds current calibration for wires */
  uint16_t gridcal[adcsize]; /**< holds current calibration for grids */
};

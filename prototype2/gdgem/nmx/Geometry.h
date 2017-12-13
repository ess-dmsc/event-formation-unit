/** Copyright (C) 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Specifies the geometry of an N-dimensional detector array
 * a calculation of global detector pixel id
 */

#pragma once

#include <cinttypes>
#include <vector>

class Geometry {
public:
  /** @brief Add dimension to geometry definition.
   * @param size Number of detector elements (pixels) in added dimension
   */
  void add_dimension(uint16_t size);

  /** @brief Returns the global detector pixel id
   * @param coords An array of pixel coordinates, must be of the same size
   * as number of dimensions, each value within defined bounds
   */
  uint32_t to_pixid(const std::vector<uint16_t> &coords) const;

  /** @brief Extracts individual coordinates from global pixel id,
   * returns true if pixelid is valid (within defined bounds).
   * @param pixid global pixel id
   * @param coords An array of pixel coordinates, must be of the same size
   * as number of dimensions
   */
  bool from_pixid(uint32_t pixid, std::vector<uint16_t> &coords) const;

private:
  std::vector<uint32_t> coefs_;
  std::vector<uint16_t> limits_;
  uint32_t maxid_{0};
};

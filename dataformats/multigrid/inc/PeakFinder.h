/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief class to find peaks from histogram data and generate
 * conversion tables for adc to wire and grid ids
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

class PeakData {
public:
  /** @brief Specification of a peak:
   * @param start start of peak_end
   * @param end end of peak + 1
   */
  PeakData(int start, int end) : start_(start), end_(end) {}
  int start_{0};
  int end_{0};
};

class PeakFinder {
public:
  /** @brief Constructor, sets private member variables
   * @param minimum_width the minimum width for valid peaks
   * @param signal_threshold sets the noise level
   * @param low_cut values below this index are also zeroed as noise
   */
  PeakFinder(int minimum_width, int signal_threshold, int low_cut);

  /** @brief calculate the peak positions from histogram data
   * @param histogram pre-populated histogram data
   */
  std::vector<PeakData *> &findpeaks(const std::vector<int> &data);

  /** @brief prints misc. stats from the peak list
   */
  void printstats(std::string info);

  /** @bried generate a calibration file based on the known peak positions
   * gaps are filled with zeroes which are invalid wire and grid positions
   * as these must start at 1. Each peak is filled with its sequence
   * number. NOTE that NO ASSUMPTIONS are made with regards to the expected
   * number of valid values.
   */
  void makecal(uint16_t *buffer, int table_size);

  int getminwidth() { return minwidth; }
  int getthresh() { return thresh; }
  int getcapped() { return capped; }

private:
  std::vector<PeakData *> peaks{};
  int minwidth{1}; /**< minimum width for identifying peaks */
  int thresh{-1};  /**< values above this are considered signals */
  int low{0};
  int capped{0}; /**< number of zero suppressions applied */

  enum State { gap = 1, peak = 2 };
  State state{gap};
};

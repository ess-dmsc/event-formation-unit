/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief class to find peaks from histogram data and generate
 * conversion tables for adc to wire and grid ids
 */

 #pragma once

 #include <vector>

 class PeakData{
 public:
   PeakData(int start, int end) : start_(start), end_(end) {}
   int start_{0};
   int end_{0};
 };

 class PeakFinder {
 public:
   /** @brief Constructor, sets private member variables
    * @param minimum_width the minimum width for valid peaks
    * @param signal_threshold sets the noise level
    */
   PeakFinder(int minimum_width, int signal_threshold);

   /** @brief calculate the peak positions from histogram data
    * @param histogram pre-populated histogram data
    */
   std::vector<PeakData *>& findpeaks(std::vector<int> data);

   int getminwidth() {return minwidth;}
   int getthresh() {return thresh;}
   int getcapped() {return capped;}

private:
  std::vector<PeakData *> peaks;
  int minwidth{1}; /**< minimum width for identifying peaks */
  int thresh{0}; /**< values above this are considered signals */
  int capped{0}; /**< number of zero suppressions applied */

  enum State {gap = 1, peak = 2};
  State state{gap};
};

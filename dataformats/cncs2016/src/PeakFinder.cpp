/** Copyright (C) 2016 European Spallation Source ERIC */

#include <PeakFinder.h>
#include <cstdio>

PeakFinder::PeakFinder(int minimum_width, int signal_threshold)
       : minwidth(minimum_width), thresh(signal_threshold) { }

std::vector<PeakData *>& PeakFinder::findpeaks(std::vector<int> data) {

  for (auto & d : data) {
    if (d <= thresh) {
      d = 0;
      capped++;
    }
  }

  int peak_start = 0;
  int peak_end = 0;

  for (unsigned int i = 0; i < data.size(); i++) {
    int val = data[i];

    //printf("%d, %d\n", i, val);
    switch (state) {

      // Skip over gaps
      case gap:
        if (val == 0) {
          //printf("gap 0, no chnge\n");
          state = gap;
          continue;
        } else {
          //printf("peak start %d\n", i);
          peak_start = i;
          state = peak;
          continue;
        }
      break;

      // Handle potential peaks
      case peak:
        if (val == 0) {
          if ((i < data.size() - 1) && (data[i + 1] > 0)) {
            //printf("single hole in data\n");
            continue; // intermittent zero
          }

          peak_end = i;
          if ((peak_end - peak_start >= minwidth) || (data[i - 1] > 0)) { // this is a peak
            //printf("adding peak. begin %d, end %d\n", peak_start, peak_end);
            peaks.push_back(new PeakData(peak_start, peak_end));
          } else {
            //printf("too narrow\n");
          }
          state = gap;
          continue;
        } else {
        }

      break;
    }
  }


  return peaks;
}

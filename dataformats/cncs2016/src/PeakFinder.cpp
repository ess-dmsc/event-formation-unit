/** Copyright (C) 2016 European Spallation Source ERIC */

#include <libs/include/StatCounter.h>
#include <Histogram.h>
#include <PeakFinder.h>
#include <cstdio>

PeakFinder::PeakFinder(int minimum_width, int signal_threshold)
       : minwidth(minimum_width), thresh(signal_threshold) { }

std::vector<PeakData *>& PeakFinder::findpeaks(const std::vector<int> data) {

  std::vector<int> datacopy = data;
  for (auto & d : datacopy) {

    if (d <= thresh) {
      d = 0;
      capped++;
    }
  }

  int peak_start = 0;
  int peak_end = 0;

  for (unsigned int i = 0; i < datacopy.size(); i++) {
    int val = datacopy[i];

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
          if ((i < datacopy.size() - 1) && (datacopy[i + 1] > 0)) {
            //printf("single hole in data\n");
            continue; // intermittent zero
          }

          peak_end = i;
          if ((peak_end - peak_start >= minwidth) || (datacopy[i - 1] > 0)) { // this is a peak
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
  //printf("Peaks found %lu\n", peaks.size());
  return peaks;
}

void PeakFinder::printstats() {
  StatCounter<int> width;
  StatCounter<int> gaps;
  StatCounter<int> start;
  StatCounter<int> end;
  for (auto p : peaks) {
    auto w = p->end_ - p->start_;
    start.add(p->start_);
    width.add(w);
    end.add(p->end_);
  }

  for (unsigned int i = 1; i < peaks.size(); i++) {
    auto g = peaks[i]->start_ - peaks[i-1]->end_;
    gaps.add(g);
  }

  printf("Peak start %d\n", start.min());
  printf("Peak end %d\n", end.max());
  printf("Min width: %d\n", width.min());
  printf("Max width: %d\n", width.max());
  printf("Avg width: %d\n", width.avg());
  printf("Min gap: %d\n", gaps.min());
  printf("Max gap: %d\n", gaps.max());
  printf("Avg gap: %d\n", gaps.avg());
}

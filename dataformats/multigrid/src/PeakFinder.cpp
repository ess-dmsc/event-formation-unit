/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <Histogram.h>
#include <PeakFinder.h>
#include <cstdio>
#include <libs/include/StatCounter.h>

PeakFinder::PeakFinder(int minimum_width, int signal_threshold, int low_cut)
    : minwidth(minimum_width), thresh(signal_threshold), low(low_cut) {}

PeakFinder::~PeakFinder() {
  peaks.clear();
}

std::vector<PeakData *> &PeakFinder::findpeaks(const std::vector<int> &data) {
  assert(data.size() != 0);
  peaks.clear(); // allow multiple calls to findpeaks with different data
  StatCounter<int> peakstats;

  std::vector<int> datacopy = data;

  for (auto d : datacopy) {
    if (d != 0) {
      peakstats.add(d);
    }
  }

  printf("histogram stats, min: %d, max %d, avg %d\n", peakstats.min(),
         peakstats.max(), peakstats.avg());

  if (thresh == -1) { /**< use automatic background level */
    printf("Automatic threshold\n");
    thresh = peakstats.avg() / 5; /**< @todo avoid hard coded value */
  }

  for (unsigned int i = 0; i < datacopy.size(); i++) {
    auto &d = datacopy[i];

    if ((d <= thresh) || (i < (unsigned int)low)) {
      d = 0;
      capped++;
    }
  }

  int peak_start = 0;
  int peak_end = 0;

  for (unsigned int i = 0; i < datacopy.size(); i++) {
    int val = datacopy[i];

    // printf("%d, %d\n", i, val);
    switch (state) {

    // Skip over gaps
    case gap:
      if (val == 0) {
        // printf("gap 0, no chnge\n");
        state = gap;
        continue;
      } else {
        // printf("peak start %d\n", i);
        peak_start = i;
        state = peak;
        continue;
      }
      break;

    // Handle potential peaks
    case peak:
      // printf("%d, %d\n", i, val);
      if (val == 0) {
        if ((i < datacopy.size() - 1) && (datacopy[i + 1] > 0)) {
          // printf("single hole in data\n");
          continue; // intermittent zero
        }

        peak_end = i;
        // if ((peak_end - peak_start >= minwidth) || (datacopy[i - 1] > 0)) {
        // // this is a peak
        if ((peak_end - peak_start >= minwidth)) { // this is a peak
          // printf("adding peak. begin %d, end %d\n", peak_start, peak_end);
          peaks.push_back(new PeakData(peak_start, peak_end));
        } else {
          // printf("too narrow\n");
        }
        state = gap;
        continue;
      }
      break;
    }
  }
  // printf("Peaks found %lu\n", peaks.size());
  return peaks;
}

void PeakFinder::printstats(std::string info) {
  StatCounter<int> width, gaps, start, end;

  for (auto p : peaks) {
    start.add(p->start_);
    end.add(p->end_);
    width.add(p->end_ - p->start_);
  }

  for (unsigned int i = 1; i < peaks.size(); i++) {
    auto g = peaks[i]->start_ - peaks[i - 1]->end_;
    gaps.add(g);
  }

  printf("%s\n", info.c_str());
  printf("Peaks:  %lu, start %d, end %d\n", peaks.size(), start.min(),
         end.max());
  printf("Widths: min %d, max %d, avg %d\n", width.min(), width.max(),
         width.avg());
  printf("Gaps:   min %d, max %d, avg %d\n", gaps.min(), gaps.max(),
         gaps.avg());
}

void PeakFinder::makecal(uint16_t *buffer, int table_size) {
  for (int i = 0; i < table_size; i++) {
    buffer[i] = 0;
  }

  int peakidx = 1;
  for (auto pk : peaks) {
    assert(pk->start_ < table_size);
    assert(pk->end_ < table_size);
    for (int j = pk->start_; j < pk->end_; j++) {
      buffer[j] = peakidx;
    }
    peakidx++;
  }
}

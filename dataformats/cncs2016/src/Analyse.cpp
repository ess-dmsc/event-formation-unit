/** Copyright (C) 2016 European Spallation Source ERIC */

#include <Analyse.h>
#include <Histogram.h>
#include <MapFile.h>
#include <PeakFinder.h>
#include <cassert>
#include <common/MultiGridGeometry.h>
#include <common/Trace.h>
#include <cspec/CSPECChanConv.h>
#include <cspec/CalibrationFile.h>
#include <errno.h>
#include <fcntl.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>

Analyze::Analyze(Args &opts)
    : ofile(opts.ofile), cfile(opts.cfile), low_cut(opts.hist_low) {
  static const int flags = O_TRUNC | O_CREAT | O_WRONLY;
  static const int mode = S_IRUSR | S_IWUSR;

  if ((eventdatafd = open((ofile + ".events").c_str(), flags, mode)) < 0) {
    perror("open() failed");
  }
  if ((histdatafd = open((ofile + ".hist").c_str(), flags, mode)) < 0) {
    perror("open() failed");
  }
  if ((csvdatafd = open((ofile + ".csv").c_str(), flags, mode)) < 0) {
    perror("open() failed");
  }
}

Analyze::~Analyze() {
  close(eventdatafd);
  close(histdatafd);
  close(csvdatafd);
}

int Analyze::populate(CSPECData &dat, int readouts) {
  static int seqno = 0;
  int valid = 0;
  for (int i = 0; i < readouts; i++) {
    if (dat.data[i].valid == 1) {
      valid++;
      seqno++;

      unsigned int wpos = dat.data[i].d[2]; /** wire 0 */
      unsigned int gpos = dat.data[i].d[6]; /** grid 0 */

      dprintf(eventdatafd,
              "%9d, %10d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d\n", seqno,
              dat.data[i].time, dat.data[i].d[0], dat.data[i].d[1],
              dat.data[i].d[2], dat.data[i].d[3], dat.data[i].d[4],
              dat.data[i].d[5], dat.data[i].d[6], dat.data[i].d[7]);

      global.add(wpos);
      local.add(wpos);
      w0pos.add(wpos);
      g0pos.add(gpos);
    }
  }
  return valid;
}

int Analyze::readfile(std::string filename) {
  CSPECChanConv conv;
  MultiGridGeometry CNCS(1, 2, 48, 4, 16, 1, 1);

  CSPECData dat(200000, &conv, &CNCS); // Default signal thresholds

  MapFile file(filename);

  stats.readouts = dat.receive(file.getaddress(), file.getsize());

  stats.discards = dat.input_filter();
  stats.events = populate(dat, stats.readouts);
  assert(stats.readouts - stats.discards == stats.events);

  return stats.events;
}

int Analyze::batchreader(std::string dir, std::string prefix,
                         std::string postfix, int begin, int end) {
  char buffer[1000];

  const char *fmt1 =
      "%-40s, %5s, %10s, %10s, %10s, %10s, %7s, %7s, %6s, %12s, %12s, %12s\n";
  const char *fmt2 =
      "%-40s, %5d, %10d, %10d, %10d, %10d, %7d, %7d, %6d, %12d, %12d, %12d\n";
  printf("#Loading files %s(%d-%d)%s\n", prefix.c_str(), begin, end,
         postfix.c_str());
  printf("#From directory %s\n\n", dir.c_str());
  printf(fmt1, "#Filename", "index", "readouts", "discards", "events", "ev_gbl",
         "nonzero", "firstnz", "lastnz", "nonzero_gbl", "firstnz_glbl",
         "lastnz_glbl");

  dprintf(csvdatafd, "#Loading files %s(%d-%d)%s\n", prefix.c_str(), begin, end,
          postfix.c_str());
  dprintf(csvdatafd, "#From directory %s\n\n", dir.c_str());
  dprintf(csvdatafd, fmt1, "#Filename", "index", "readouts", "discards",
          "events", "ev_gbl", "nonzero", "firstnz", "lastnz", "nonzero_gbl",
          "firstnz_glbl", "lastnz_glbl");

  for (int i = begin; i <= end; i++) {
    sprintf(buffer, "%s%03d%s", prefix.c_str(), i, postfix.c_str());
    auto filename = std::string(buffer);
    sprintf(buffer, "%s%s", dir.c_str(), filename.c_str());
    auto pathname = std::string(buffer);

    local.clear();
    auto events = readfile(pathname);
    if (events > 0) {
      for (int j = 0; j < 4000; j++) {
        if (local.hist[j] >= low_cut) {
          dprintf(histdatafd, "%5d, %5d, %5d\n", i, j, local.hist[j]);
        }
      }
      local.analyze(0);  // was 150
      global.analyze(0); // was 150

      if (i % 1000 == 0) {
        printf("%d", i);
      }

      if (i % 10 == 0) {
        printf(".");
      }

      fflush(stdout);

      dprintf(csvdatafd, fmt2, filename.c_str(), i, stats.readouts,
              stats.discards, local.entries, global.entries, local.nonzero,
              local.firstnonzero, local.lastnonzero, global.nonzero,
              global.firstnonzero, global.lastnonzero);
    } else if (events == 0) {
      printf("# %s no valid events, ignored\n", filename.c_str());
      dprintf(csvdatafd, "# %s no valid events, ignored\n", filename.c_str());
    } else {
      printf("# %s file error, ignored\n", filename.c_str());
      dprintf(csvdatafd, "# %s file error, ignored\n", filename.c_str());
    }
  }
  printf("\n");
  return 0;
}

void Analyze::makecal() {

  PeakFinder wires(2, 0, 150); /**< min peak width 2, threshold 150*/
  PeakFinder grids(2, 0, 150); /**< min peak width 2, threshold 150*/

  wires.findpeaks(w0pos.hist);
  wires.printstats(std::string("\nw0pos statistics"));

  grids.findpeaks(g0pos.hist);
  grids.printstats(std::string("\ng0pos statistics"));

  if (!cfile.empty()) {
    printf("Writing calibration to file\n");
    uint16_t wcal[CSPECChanConv::adcsize];
    uint16_t gcal[CSPECChanConv::adcsize];

    wires.makecal(wcal, CSPECChanConv::adcsize);
    grids.makecal(gcal, CSPECChanConv::adcsize);

    CalibrationFile calibfile;
    calibfile.save(std::string(cfile), (char *)wcal, (char *)gcal);
  }
}

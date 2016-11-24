/** Copyright (C) 2016 European Spallation Source ERIC */

#include <cassert>
#include <common/MultiGridGeometry.h>
#include <common/Trace.h>
#include <cspec/CSPECData.h>
#include <cspec/CSPECChanConv.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <string>

#define UNUSED __attribute__((unused))

class Histogram {
public:
  static const unsigned int histsize = 16384;
  int hist[histsize];
  int entries{0};
  int firstnonzero = -1;
  int lastnonzero = -1;
  int nonzero = 0;

  Histogram() {
    clear();
  }

  void add(unsigned int value) {
    assert(value < histsize);
    hist[value]++;
    entries++;
  }

  void clear() {
    memset(hist, 0, sizeof(hist));
    entries = 0;
    firstnonzero = -1;
    lastnonzero = -1;
    nonzero = 0;
  }

  void analyze() {
    firstnonzero = -1;
    lastnonzero = -1;
    nonzero = 0;
    //printf("tot entries: %6d - ", entries);
    for (unsigned int i = 150; i < histsize; i++) {
      if ((hist[i] > 0) and (firstnonzero == -1)) {
        firstnonzero = i;
      }
      if (hist[i] > 0) {
        nonzero++;
        lastnonzero = i;
      }
    }
    //printf("nonzero %6d, firstnonzero %5d, lastnonzero %5d, span %5d\n",
    //       nonzero, firstnonzero, lastnonzero, lastnonzero-firstnonzero);
  }
};


void populate(CSPECData & dat, int events, Histogram& wglobal, Histogram& wlocal) {
  for (int i = 0; i < events; i++) {
     unsigned int wpos = dat.data[i].d[2];
     //unsigned int gpos = dat.data[i].d[6];
     wglobal.add(wpos);
     wlocal.add(wpos);
  }
}


int readfile(char * filename, Histogram& wglobal, Histogram& wlocal) {
  struct stat sb;
  CSPECChanConv conv;
  conv.makewirecal(0, CSPECChanConv::adcsize - 1, 128); // Linear look-up table
  conv.makegridcal(0, CSPECChanConv::adcsize - 1, 96);  // Linear look-up table

  MultiGridGeometry CNCS(2, 48, 4, 16);

  CSPECData dat(100000, &conv, &CNCS); // Default signal thresholds

  int fd = open(filename, O_RDONLY);
  if (fd < 0)
    return -1;

  auto res = fstat(fd, &sb);
  assert(res != -1);

  auto addr = mmap(NULL, sb.st_size, PROT_READ,MAP_PRIVATE, fd, 0);
  assert(addr != MAP_FAILED);

  auto events = dat.receive((const char *)addr, sb.st_size);

  res = munmap(addr, sb.st_size);
  assert(res == 0);

  populate(dat, events, wglobal, wlocal);
  return events;
}


class Args {
public:
  /** @brief constructor for program arguments parsed via getopt_long()
   * @param argc Argument count - typically taken from main()
   * @param argv Argument array - typically taken from main()
   */
  Args(int argc, char *argv[]);

  std::string dir{"/home/morten/cncsdata/vanadium_july_27/"};
  std::string prefix{"2016_07_26_1005_sample_"};
  std::string postfix{".bin"};
  int start{643};
  int end{765};
};

Args::Args(int argc, char * argv[]) {
    optind = 1; // global variable used by getopt

    while (1) {
      static struct option long_options[] = {
          {"help", no_argument, 0, 'h'},
          {"dir", required_argument, 0, 'd'},
          {"prefix", required_argument, 0, 'p'},
          {"postfix", required_argument, 0, 'o'},
          {"start", required_argument, 0, 's'},
          {"end", required_argument, 0, 'e'},
          {0, 0, 0, 0}};

      int option_index = 0;

      int c =
          getopt_long(argc, argv, "d:p:o:s:e:h", long_options, &option_index);
      if (c == -1)
        break;

      switch (c) {
      case 'd':
        dir.assign(optarg);
        break;
      case 'p':
        prefix.assign(optarg);
        break;
      case 'o':
        postfix.assign(optarg);
        break;
      case 's':
        start = atoi(optarg);
        break;
      case 'e':
        end = atoi(optarg);
        break;
      case 'h':
      default:
        printf("Usage: analyze [OPTIONS]\n");
        printf(" --dir, -d directory     path to data including last '/' \n");
        printf(" --prefix, -p prefix     first part of filename \n");
        printf(" --postfix -o postfix    last part of filename \n");
        printf(" --start, -s number      sequence number of first file \n");
        printf(" --end -e number         sequence number of last file \n");
        printf(" -h                      help - prints this message \n");
        return;
      }
    }

}



int main(UNUSED int argc, UNUSED char * argv[]) {
  char filename[200];
  char pathname[500];
  Args opts(argc, argv);

  Histogram global, local;

  const char * fmt1 = " %-40s, %5s, %10s, %10s, %7s, %7s, %6s, %12s, %12s, %12s\n";
  const char * fmt2 = " %-40s, %5d, %10d, %10d, %7d, %7d, %6d, %12d, %12d, %12d\n";

  printf("#Loading files %s(%d-%d)%s\n", opts.prefix.c_str(), opts.start, opts.end, opts.postfix.c_str());
  printf("#From directory %s\n", opts.dir.c_str());
  printf("\n");

  printf(fmt1, "#Filename",  "index", "events", "ev_gbl", "nonzero", "firstnz", "lastnz", "nonzero_gbl", "firstnz_glbl", "lastnz_glbl");
  for (int i = opts.start; i <= opts.end; i++) {
    sprintf(filename,"%s%03d%s", opts.prefix.c_str(), i, opts.postfix.c_str());
    sprintf(pathname, "%s%s", opts.dir.c_str(), filename);


    local.clear();
    auto events = readfile(pathname, global, local);
    if (events > 0) {
      local.analyze();
      global.analyze();
      printf(fmt2, filename, i, local.entries, global.entries,
             local.nonzero, local.firstnonzero, local.lastnonzero,
             global.nonzero, global.firstnonzero, global.lastnonzero );
    } else {
      printf("# %s error, ignored\n", filename);
    }
  }
}

/** Copyright (C) 2016 European Spallation Source ERIC */

#include <cassert>
#include <common/MultiGridGeometry.h>
#include <common/Trace.h>
#include <cspec/CSPECData.h>
#include <cspec/CSPECChanConv.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <string>
#include <unistd.h>
//#include <matplotlibcpp.h>

typedef struct {
  unsigned int readouts;
  unsigned int discards;
  unsigned int events;
} raw_stat_t;

#define UNUSED __attribute__((unused))

class Histogram {
public:
  static const unsigned int histsize = 16384;
  int hist[histsize];
  int entries{0};
  int firstnonzero = -1;
  int lastnonzero = -1;
  int nonzero = 0;

  Histogram() { clear(); }

  void add(unsigned int value) {
    assert(value < histsize);
    hist[value]++;
    entries++;
  }

  void clear() {
    memset(hist, 0, sizeof(hist));
    entries = 0;
  }

  void analyze() {
    firstnonzero = -1;
    lastnonzero = -1;
    nonzero = 0;

    for (unsigned int i = 150; i < histsize; i++) {
      if ((hist[i] > 0) and (firstnonzero == -1)) {
        firstnonzero = i;
      }
      if (hist[i] > 0) {
        nonzero++;
        lastnonzero = i;
      }
    }
  }
};


int  populate(CSPECData & dat, int readouts, Histogram& wglobal, Histogram& wlocal, int fd) {
  static int seqno = 0;
  int valid = 0;
  for (int i = 0; i < readouts; i++) {
     if (dat.data[i].valid == 1) {
       valid++;
       seqno++;
       unsigned int wpos = dat.data[i].d[2];
       //unsigned int gpos = dat.data[i].d[6];
       #if 1
       dprintf(fd, "%9d, %10d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d\n", seqno, dat.data[i].time,
               dat.data[i].d[0], dat.data[i].d[1], dat.data[i].d[2], dat.data[i].d[3],
               dat.data[i].d[4], dat.data[i].d[5], dat.data[i].d[6], dat.data[i].d[7] );
      #endif
       wglobal.add(wpos);
       wlocal.add(wpos);
     }
  }
  return valid;
}

int readfile(char * filename, Histogram& wglobal, Histogram& wlocal, int ofd,
             raw_stat_t * stat) {

  struct stat sb;
  int fd;
  CSPECChanConv conv;
  MultiGridGeometry CNCS(2, 48, 4, 16);

  CSPECData dat(200000, &conv, &CNCS); // Default signal thresholds

  if ((fd = open(filename, O_RDONLY)) < 0) {
    perror("open() failed");
    return -1;
  }

  auto res = fstat(fd, &sb);
  assert(res != -1);

  auto addr = mmap(NULL, sb.st_size, PROT_READ,MAP_PRIVATE, fd, 0);
  assert(addr != MAP_FAILED);

  stat->readouts = dat.receive((const char *)addr, sb.st_size);
  stat->discards = dat.input_filter();
  stat->events = populate(dat, stat->readouts, wglobal, wlocal, ofd);
  assert(stat->readouts - stat->discards == stat->events);

  res = munmap(addr, sb.st_size);
  assert(res == 0);
  close(fd);

  return stat->events;
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
  std::string ofile{"tmp.dat"};
  int start{0};
  int end{0};
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
          {"output", required_argument, 0, 'f'},
          {0, 0, 0, 0}};

      int option_index = 0;

      int c =
          getopt_long(argc, argv, "d:p:o:s:e:f:h", long_options, &option_index);
      if (c == -1)
        break;

      switch (c) {
      case 'd':
        dir.assign(optarg);
        break;
      case 'f':
        ofile.assign(optarg);
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
        exit(0);
      }
    }

}



int main(UNUSED int argc, UNUSED char * argv[]) {
  char filename[200];
  char pathname[500];
  Args opts(argc, argv);
  raw_stat_t stats;
  int ofd;

  Histogram global, local;

  const char * fmt1 = "%-40s, %5s, %10s, %10s, %10s, %10s, %7s, %7s, %6s, %12s, %12s, %12s\n";
  const char * fmt2 = "%-40s, %5d, %10d, %10d, %10d, %10d, %7d, %7d, %6d, %12d, %12d, %12d\n";

  printf("#Loading files %s(%d-%d)%s\n", opts.prefix.c_str(), opts.start, opts.end, opts.postfix.c_str());
  printf("#From directory %s\n", opts.dir.c_str());
  printf("\n");

  if ((ofd = open(opts.ofile.c_str(), O_TRUNC|O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR)) < 0) {
    perror("open() failed");
  }

  printf(fmt1, "#Filename",  "index", "readouts", "discards", "events", "ev_gbl",
               "nonzero", "firstnz", "lastnz", "nonzero_gbl", "firstnz_glbl",
               "lastnz_glbl");

  for (int i = opts.start; i <= opts.end; i++) {
    sprintf(filename,"%s%03d%s", opts.prefix.c_str(), i, opts.postfix.c_str());
    sprintf(pathname, "%s%s", opts.dir.c_str(), filename);

    local.clear();
    auto events = readfile(pathname, global, local, ofd, &stats);
    if (events > 0) {
      local.analyze();
      global.analyze();
      printf(fmt2, filename, i, stats.readouts, stats.discards, local.entries, global.entries,
             local.nonzero, local.firstnonzero, local.lastnonzero,
             global.nonzero, global.firstnonzero, global.lastnonzero );
    } else if (events == 0) {
      printf("# %s no valid events, ignored\n", filename);
    } else {
      printf("# %s file error, ignored\n", filename);
    }
  }
  close(ofd);
}

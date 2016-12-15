/** Copyright (C) 2016 European Spallation Source ERIC */

#include <Args.h>
#include <getopt.h>
#include <stdlib.h>

Args::Args(int argc, char *argv[]) {
  optind = 1; // global variable used by getopt

  while (1) {
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"calibfile", required_argument, 0, 'c'},
        {"dir", required_argument, 0, 'd'},
        {"prefix", required_argument, 0, 'p'},
        {"postfix", required_argument, 0, 'o'},
        {"start", required_argument, 0, 's'},
        {"end", required_argument, 0, 'e'},
        {"output", required_argument, 0, 'f'},
        {"histlow", required_argument, 0, 'l'},
        {0, 0, 0, 0}};

    int option_index = 0;

    int c = getopt_long(argc, argv, "d:p:o:s:e:f:l:c:h", long_options,
                        &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 'c':
      cfile.assign(optarg);
      break;
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
    case 'l':
      hist_low = atoi(optarg);
      break;
    case 'h':
    default:
      printf("Usage: analyze [OPTIONS]\n");
      printf(" --dir, -d directory      path to data including last '/' \n");
      printf(" --prefix, -p prefix      first part of filename \n");
      printf(" --postfix, -o postfix    last part of filename \n");
      printf(" --start, -s number       sequence number of first file \n");
      printf(" --end, -e number         sequence number of last file \n");
      printf(" --histlow, -l number     low threshold for dumping hist. data "
             "to file \n");
      printf(" --calibfile, -c file     wire and grid calibration file to load "
             "before parsing \n");
      printf(" --output, f file         output filename");
      printf(" --help, -h               help - prints this message \n");
      exit(0);
    }
  }
}

/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <Args.h>
#include <getopt.h>
#include <stdlib.h>

Args::Args(int argc, char *argv[]) {
  optind = 1; // global variable used by getopt

  while (1) {
    static struct option long_options[] = {
        // clang-format off
        {"help",      no_argument,       0, 'h'},

        {"basedir",   required_argument, 0, 'b'},
        {"dir",       required_argument, 0, 'd'},
        {"prefix",    required_argument, 0, 'p'},
        {"postfix",   required_argument, 0, 'o'},
        {"start",     required_argument, 0, 's'},
        {"end",       required_argument, 0, 'e'},

        {"output",    required_argument, 0, 'f'},
        {"histlow",   required_argument, 0, 'l'},

        {"runspec",   required_argument, 0, 'j'},
        {"runfile",   no_argument,       0, 'r'},
        {0,           0,                 0,   0}};
    // clang-format on

    int option_index = 0;

    int c = getopt_long(argc, argv, "b:d:p:o:s:e:f:l:c:j:r:h", long_options,
                        &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 'b':
      basedir.assign(optarg);
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
    case 'j':
      runspec.assign(optarg);
      break;
    case 'r':
      runfile.assign(optarg);
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
      printf(" --runfile, -r file       use <file> for batch processing \n");
      printf(" --runspec, -j file       get runspec from <file> \n");
      printf(" --basedir, -b directory  path to base directory of raw data \n");
      printf(" --dir, -d directory      path to data including last '/' relative to basedir \n");

      printf(" --prefix, -p prefix      first part of filename \n");
      printf(" --postfix, -o postfix    last part of filename \n");
      printf(" --start, -s number       sequence number of first file \n");
      printf(" --end, -e number         sequence number of last file \n");

      printf(" --output, f file         output filename/dir");

      printf(" --histlow, -l number     low threshold for dumping hist. data "
             "to file \n");
      printf(" --help, -h               help - prints this message \n");
      exit(0);
    }
  }
}

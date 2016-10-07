/** Copyright (C) 2016 European Spallation Source */

#include <EFUArgs.h>
#include <cstdio>
#include <getopt.h>
#include <iostream>
#include <string>
#include <unistd.h>

EFUArgs::EFUArgs(int argc, char *argv[]) {
  using namespace std;

  while (1) {
    static struct option long_options[] = {
       {"help", no_argument, 0, 'h'},
       {"det", required_argument, 0, 'd'},
       {"port", required_argument, 0, 'p'},
       {0, 0, 0, 0}
     };

    int option_index = 0;

    int c = getopt_long(argc, argv, "d:p:h", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 0:
      if (long_options[option_index].flag != 0)
        break;
    case 'd':
      det.assign(optarg);
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 'h':
    default:
      printf("Usage: efu2 [OPTIONS]\n");
      printf(" --det -d name           detector name \n");
      printf(" --port -p port          udp port \n");
      printf(" -h                      help - prints this message \n");
      exit(1);
    }
  }
  cout << "Starting event processing pipeline2" << endl;
  cout << "  Detector: " << det << endl;
  cout << "  UDP Port: " << port << endl;
}

/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/EFUArgs.h>
#include <common/Trace.h>
#include <cstdio>
#include <getopt.h>
#include <iostream>
#include <string>

EFUArgs *efu_args; /** global var */

EFUArgs::EFUArgs(int argc, char *argv[]) {
  using namespace std;

  optind = 1; // global variable used by getopt

  while (1) {
    static struct option long_options[] = {
        // clang-format off
        {"help",      no_argument,       0, 'h'},
        {"broker",    required_argument, 0, 'b'},
        {"cpu",       required_argument, 0, 'c'},
        {"det",       required_argument, 0, 'd'},
        {"dip",       required_argument, 0, 'i'},
        {"dport",     required_argument, 0, 'p'},
        {"reports",   required_argument, 0, 'r'},
        {"stopafter", required_argument, 0, 's'},
        {"graphite",  required_argument, 0, 'g'},
        {"gport",     required_argument, 0, 'o'},
        {"file",      required_argument, 0, 'f'},
        {"logip",     required_argument, 0, 'a'},
        {"cmdport",   required_argument, 0, 'm'},
        {0, 0, 0, 0}
      };
    // clang-format on

    int option_index = 0;

    int c = getopt_long(argc, argv, "a:b:c:d:f:g:m:o:i:p:r:s:h", long_options,
                        &option_index);
    if (c == -1)
      break;

    switch (c) {
    // case 0: // currently not using flags
    //  if (long_options[option_index].flag != 0)
    //    break;
    case 'a':
      graylog_ip.assign(optarg);
      break;
    case 'b':
      broker.assign(optarg);
      break;
    case 'c':
      cpustart = atoi(optarg);
      break;
    case 'd':
      det.assign(optarg);
      break;
    case 'g':
      graphite_ip_addr.assign(optarg);
      break;
    case 'o':
      graphite_port = atoi(optarg);
      break;
    case 'm':
      cmdserver_port = atoi(optarg);
      break;
    case 'i':
      ip_addr.assign(optarg);
      break;
    case 'f':
      config_file.assign(optarg);
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 'r':
      reportmask = (unsigned int)strtoul(optarg, 0, 0);
      break;
    case 's':
      stopafter = atoi(optarg);
      break;
    case 'h':
    default:
      printf("Usage: efu2 [OPTIONS]\n");
      printf(" --broker, -b broker      Kafka broker string \n");
      printf(" --cpu, -c lcore          lcore id of first thread \n");
      printf(" --det, -d name           detector name \n");
      printf(" --dip, -i ipaddr         ip address of receive interface \n");
      printf(" --port, -p port          udp port \n");
      printf(" --file, -f configfile    pipeline-specific config file \n");
      printf(
          " --graphite, -g ipaddr       ip address of graphite metrics server \n");
      printf(" --gport, -o port         Graphite tcp port \n");
      printf(" --cmdport, -m port       command parser tcp port\n");
      printf(" --stopafter, -s timeout  terminate after timeout seconds \n");
      printf(" --help, -h               help - prints this message \n");
      stopafter = 0;
      return;
    }
  }

  XTRACE(INIT, ALW, "Starting event processing pipeline2\n");
  XTRACE(INIT, ALW, "  Detector:      %s\n", det.c_str());
  XTRACE(INIT, ALW, "  CPU Offset:    %d\n", cpustart);
  XTRACE(INIT, ALW, "  IP addr:       %s\n", ip_addr.c_str());
  XTRACE(INIT, ALW, "  UDP Port:      %d\n", port);
  XTRACE(INIT, ALW, "  Kafka broker:  %s\n", broker.c_str());
  XTRACE(INIT, ALW, "  Graphite:      %s\n", graphite_ip_addr.c_str());
  XTRACE(INIT, ALW, "  Graphite port: %d\n", graphite_port);
  XTRACE(INIT, ALW, "  Stopafter:     %u\n", stopafter);
}

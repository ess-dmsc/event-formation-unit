/** Copyright (C) 2016 European Spallation Source */

#include <DGArgs.h>
#include <cstdio>
#include <getopt.h>
#include <iostream>
#include <unistd.h>

DGArgs::DGArgs(int argc, char *argv[]) {

  using namespace std;

  int c;
  while (1) {
    static struct option long_options[] = {
        {"ipaddr", required_argument, 0, 'i'},
        {"data", required_argument, 0, 'd'},
        {"port", required_argument, 0, 'p'},
        {"size", required_argument, 0, 's'},
        {"tuples", required_argument, 0, 't'},
        {"update", required_argument, 0, 'u'},
        {"sndbuf", required_argument, 0, 'x'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}};

    int option_index = 0;

    c = getopt_long(argc, argv, "d:i:p:s:t:u:hx", long_options, &option_index);

    if (c == -1)
      break;
    switch (c) {
    case 0:
      if (long_options[option_index].flag != 0)
        break;
    case 'd':
      buflen = atoi(optarg);
      break;
    case 'i':
      dest_ip.assign(optarg);
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 's':
      txGB = atoi(optarg);
      break;
    case 't':
      vmmtuples = atoi(optarg);
      break;
    case 'u':
      updint = atoi(optarg);
      break;
    case 'x':
      sndbuf = atoi(optarg);
      break;
    case 'h':
    default:
      printf("Usage: bulkdatagen [OPTIONS] \n");
      printf(" --tuples -t tuples     number of data tuples in each UDP packet "
             "\n");
      printf(" --size -s size         size in GB of transmitted data \n");
      printf(" --ipaddr -i ipaddr     destination ip address \n");
      printf(" --port -p port         UDP destination port \n");
      printf(" --data -d len          size of Tx/Tx buffer in bytes (max 9000) "
             "\n");
      printf(" --update -u interval   update interval (seconds) \n");
      printf(" --sndbuf -x len        kernel tx buffer size \n");
      printf(" -h                     help - prints this message \n");
      exit(1);
    }
  }
  printf("Generating a bulk data stream\n");
  printf("  number of bytes:        %d GB\n", txGB);
  printf("  data tuples per packet: %d\n", vmmtuples);
  printf("Network properties\n");
  printf("  destination ip address: %s\n", dest_ip.c_str());
  printf("  destination udp port:   %d\n", port);
  printf("  tx buffer size:         %dB\n", buflen);
  printf("  sndbuf:                 %dB\n", sndbuf);
  printf("Other properties\n");
  printf("  update interval:        %ds\n", updint);
}

/** Copyright (C) 2016 European Spallation Source */

#include <EFUArgs.h>
#include <cstdio>
#include <getopt.h>
#include <iostream>
#include <string>
#include <unistd.h>

EFUArgs::EFUArgs(int argc, char *argv[]) {
  using namespace std;

  int c;

  while (1) {
    static struct option long_options[] = {
        {"datasz", required_argument, 0, 'd'},
        {"broker", required_argument, 0, 'b'},
        {"nokafka", no_argument, 0, 'n'},
        {"help", no_argument, 0, 'h'},
        {"port", required_argument, 0, 'p'},
        {"reduction", required_argument, 0, 'r'},
        {"stopafter", required_argument, 0, 's'},
        {"update", required_argument, 0, 'u'},
        {"rcvbuf", required_argument, 0, 'x'},
        {0, 0, 0, 0}};

    int option_index = 0;

    c = getopt_long(argc, argv, "b:d:p:r:s:u:x:hn", long_options,
                    &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0:
      if (long_options[option_index].flag != 0)
        break;
    case 'd':
      buflen = atoi(optarg);
      break;
    case 'b':
      broker.assign(optarg);
    case 'n':
      kafka = false;
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 'r':
      reduction = atoi(optarg);
      break;
    case 's':
      stopafter = atoi(optarg);
      break;
    case 'u':
      updint = atoi(optarg);
      break;
    case 'x':
      rcvbuf = atoi(optarg);
      break;
    case 'h':
    default:
      printf("Usage: efu [OPTIONS]\n");
      printf(
          " --datasz -d len         size of Rx buffer in bytes (max 9000) \n");
      printf(" --broker -b ipaddr:port address of Kafka broker \n");
      printf(" --nokafka -n            bypass Kafka \n");
      printf(" --port -p port          UDP receive port \n");
      printf(" --reduction -r val      data reduction when processing \n");
      printf(" --stopafter -s interval stop after intercal seconds \n");
      printf(" --update -u interval    update interval (sec) \n");
      printf(" --rcvbuf -x size        socket rcvbuf size (kernel) \n");
      printf(" -h                      help - prints this message \n");
      exit(1);
    }
  }
  cout << "Starting event processing pipeline" << endl;
  cout << "  data reduction ratio: " << reduction << endl;
  cout << "Network properties" << endl;
  cout << "  receive udp port:     " << port << endl;
  cout << "  rx buffer size:       " << buflen << " bytes" << endl;
  cout << "  Kafka broker:         " << broker << endl;
  cout << "  Kafka enabled:        " << (kafka ? "yes" : "no") << endl;
  cout << "  rcvbuf:               " << rcvbuf << endl;
  cout << "Other properties" << endl;
  cout << "  update interval:      " << updint << " sec" << endl;
  cout << "  stopafter:            "
       << (stopafter == 0xffffffff ? "never"
                                   : (std::to_string(stopafter)) + " sec")
       << endl;
}

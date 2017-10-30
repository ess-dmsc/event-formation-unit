#include <Args.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>

Args::Args(int argc, char *argv[]) {
  int c;
  while ((c = getopt(argc, argv, "hi:p:b:t:")) != -1)
    switch (c) {
    case 'i':
      dest_ip = optarg;
      break;

    case 'p':
      port = atoi(optarg);
      break;

    case 't':
      txpps = atoi(optarg);
      break;

    case 'b':
      buflen = atoi(optarg);
      break;

    case 'h':
    default:
      std::cout << "Usage: udptx [OPTIONS]" << std::endl;
      std::cout << " -i ipaddr      destination ip address" << std::endl;
      std::cout << " -p port        UDP destination port" << std::endl;
      std::cout << " -b buflen      size of Tx/Tx buffer in bytes (max 9000)"
           << std::endl;
      std::cout << " -h             help - prints this message" << std::endl;
      exit(1);
    }

  std::cout << "ipaddr:   " << dest_ip << std::endl;
  std::cout << "udp port: " << port << std::endl;
  std::cout << "txbuffer: " << buflen << std::endl;
}

#include <Args.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>

Args::Args(int argc, char *argv[]) {
  using namespace std;
  int c;
  while ((c = getopt(argc, argv, "hi:p:b:")) != -1)
    switch (c) {
    case 'i':
      dest_ip = optarg;
      break;

    case 'p':
      port = atoi(optarg);
      break;

    case 'b':
      buflen = atoi(optarg);
      break;

    case 'h':
    default:
      cout << "Usage: udptx [OPTIONS]" << endl;
      cout << " -i ipaddr      destination ip address" << endl;
      cout << " -p port        UDP destination port" << endl;
      cout << " -b buflen      size of Tx/Tx buffer in bytes (max 9000)"
           << endl;
      cout << " -h             help - prints this message" << endl;
      exit(1);
    }

  cout << "ipaddr:   " << dest_ip << endl;
  cout << "udp port: " << port << endl;
  cout << "txbuffer: " << buflen << endl;
}

#include <DGArgs.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>

DGArgs::DGArgs(int argc, char *argv[]) {
  using namespace std;
  int c;
  while ((c = getopt(argc, argv, "i:n:p:b:s:u:h")) != -1)
    switch (c) {
    case 'b':
      buflen = atoi(optarg);
      break;
    case 'i':
      dest_ip.assign(optarg);
      break;
    case 'n':
      vmmtuples = atoi(optarg);
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 's':
      txGB = atol(optarg);
      break;
    case 'u':
      updint = atol(optarg);
      break;

    case 'h':
    default:
      cout << "Usage: bulkdatagen [OPTIONS]" << endl;
      cout << " -n tuples      number of data tuples in each UDP packet"
           << endl;
      cout << " -s size        size in GB of transmitted data" << endl;
      cout << " -i ipaddr       destination ip address" << endl;
      cout << " -p port        UDP destination port" << endl;
      cout << " -b buflen      size of Tx/Tx buffer in bytes (max 9000)"
           << endl;
      cout << " -u interval    update interval (seconds)" << endl;
      cout << " -h             help - prints this message" << endl;
      exit(1);
    }
  cout << "Generating a bulk data stream" << endl;
  cout << "  number of bytes:        " << txGB << "GB" << endl;
  cout << "  data tuples per packet: " << vmmtuples << endl;
  cout << "Network properties" << endl;
  cout << "  destination ip address: " << dest_ip << endl;
  cout << "  destination udp port:   " << port << endl;
  cout << "  tx buffer size:         " << buflen << "B" << endl;
  cout << "Other properties" << endl;
  cout << "  update interval:        " << updint << "s" << endl;
}

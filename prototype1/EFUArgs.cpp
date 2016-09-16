#include <EFUArgs.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>

EFUArgs::EFUArgs(int argc, char *argv[]) {
  using namespace std;
  int c;
  while ((c = getopt(argc, argv, "b:p:r:u:h")) != -1)
    switch (c) {
    case 'b':
      buflen = atoi(optarg);
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 'r':
      reduction = atoi(optarg);
      break;
    case 'u':
      updint = atoi(optarg);
      break;
    case 'h':
    default:
      cout << "Usage: bulkdatagen [OPTIONS]" << endl;
      cout << " -p port        UDP destination port" << endl;
      cout << " -r reduction   data reduction when processing" << endl;
      cout << " -b buflen      size of Tx/Tx buffer in bytes (max 9000)"
           << endl;
      cout << " -h             help - prints this message" << endl;
      cout << " -u interval    update interval (sec)" << endl;
      exit(1);
    }
  cout << "Starting event processing pipeline" << endl;
  cout << "  data reduction ratio:   " << reduction << endl;
  cout << "Network properties" << endl;
  cout << "  receive udp port:       " << port << endl;
  cout << "  rx buffer size:         " << buflen << " bytes" << endl;
  cout << "Other properties" << endl;
  cout << "  update interval:        " << updint << " sec" << endl;
}

#include <DGArgs.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>

DGArgs::DGArgs(int argc, char *argv[]) {
  using namespace std;
  int c;
  while ((c = getopt(argc, argv, "hn:s:p:b:")) != -1)
    switch (c) {
    case 'n':
      vmmtuples = atoi(optarg);
      break;

    case 'p':
      port = atoi(optarg);
      break;

    case 'b':
      buflen = atoi(optarg);
      break;

    case 's':
      txGB = atol(optarg);
      break;

    case 'h':
    default:
      cout << "Usage: bulkdatagen [OPTIONS]" << endl;
      cout << " -n tuples      number of data tuples in each UDP packet"
           << endl;
      cout << " -s size        size in GB of transmitted data" << endl;
      cout << " -p port        UDP destination port" << endl;
      cout << " -b buflen      size of Tx/Tx buffer in bytes (max 9000)"
           << endl;
      cout << " -h             help - prints this message" << endl;
      exit(1);
    }
}

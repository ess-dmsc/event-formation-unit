#include <EFUArgs.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>

EFUArgs::EFUArgs(int argc, char *argv[]) {
  using namespace std;
  int c;
  while ((c = getopt(argc, argv, "hn:s:p:b:")) != -1)
    switch (c) {
    case 'p':
      port = atoi(optarg);
      break;

    case 'b':
      buflen = atoi(optarg);
      break;

    case 'h':
    default:
      cout << "Usage: bulkdatagen [OPTIONS]" << endl;
      cout << " -p port        UDP destination port" << endl;
      cout << " -b buflen      size of Tx/Tx buffer in bytes (max 9000)"
           << endl;
      cout << " -h             help - prints this message" << endl;
      exit(1);
    }
}

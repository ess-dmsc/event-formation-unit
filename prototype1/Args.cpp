#include <Args.h>
#include <cstdio>
#include <iostream>
#include <unistd.h>

Args::Args(int argc, char *argv[]) {
  using namespace std;
  int c;
  while ((c = getopt(argc, argv, "hn:s:p:")) != -1)
    switch (c) {
    case 'n':
      vmmtuples = atoi(optarg);
      break;

    case 'p':
      port = atoi(optarg);
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
      cout << " -h             help - prints this message" << endl;
      exit(1);
    }

  cout << "Generating " << txGB << " GB of bulk data packets" << endl;
  cout << "containing " << vmmtuples << " nmx data tuples per packet" << endl;
  cout << "for udp port " << port << endl;
}

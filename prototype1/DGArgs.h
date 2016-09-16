

class DGArgs {
public:
  DGArgs(int argc, char *argv[]);

  unsigned long long txGB{10};
  int vmmtuples{20};
  int port{9000};
  int buflen{9000};
};



class Args {
public:
  Args(int argc, char *argv[]);

  long long txGB{10};
  int vmmtuples{20};
  int port{9000};
};

#include <string>

class Args {
public:
  Args(int argc, char *argv[]);

  std::string dest_ip{"127.0.0.1"};
  int port{9000};
  int buflen{9000};
};

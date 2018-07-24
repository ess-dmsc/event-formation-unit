/// Copyright (C) 2016-2018 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief class for handling configuration and command line options
///
//===----------------------------------------------------------------------===//

#include <string>

class Args {
public:
  Args(int argc, char *argv[]);

  std::string dest_ip{"127.0.0.1"};
  int port{9000};
  int buflen{9000};
  uint32_t txpps{1}; /**< transmit packets per second */
};

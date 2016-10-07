/** Copyright (C) 2016 European Spallation Source */

#include <string>

class EFUArgs {
public:
  EFUArgs(int argc, char *argv[]);

  std::string det{"nmx"}; /**< detector name */
};

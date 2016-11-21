/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Command parser
 */

#include <common/EFUArgs.h>
#include <map>
#include <string>
#include <vector>

typedef int (*function_ptr)(std::vector<std::string> cmdargs, char *output,
                            unsigned int *osize);

class Parser {
public:
  static const unsigned int max_command_size = 100;
  enum error { OK = 0, EUSIZE, EOSIZE, ENOTOKENS, EBADCMD, EBADARGS };
  /** @todo document
   */
  Parser();

  /** @todo document
   */
  int registercmd(std::string cmd_name, function_ptr cmd_fn);

  /** @todo document
   */
  int parse(char *input, unsigned int isize, char *output, unsigned int *osize);

private:
  std::map<std::string, function_ptr> commands;
};

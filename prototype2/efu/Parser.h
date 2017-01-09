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

  /** @brief Create parser with the currently fixed commands
   */
  Parser();

  /** @brief used to register new commands with the Parser
   *  @param cmd_name name of command
   *  @param function pointer to command implementation
   */
  int registercmd(std::string cmd_name, function_ptr cmd_fn);

  /** @brief parse a command
   *  @param[in] input char array holding command and its arguments
   *  @param[in] isize input size in bytes
   *  @param[out] output reply for the command
   *  @param[out] osize output size in bytes
   */
  int parse(char *input, unsigned int isize, char *output, unsigned int *osize);

private:
  std::map<std::string, function_ptr> commands; /**< map of all commands */
};

// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Command parser
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <map>
#include <string>
#include <vector>

using cmdFunction =
    std::function<int(std::vector<std::string>, char *, unsigned int *)>;

class Parser {
public:
  static const unsigned int max_command_size = 100;
  enum error { OK = 0, EUSIZE, EOSIZE, ENOTOKENS, EBADCMD, EBADARGS };

  /// \brief Create parser with the currently fixed commands
  Parser(std::shared_ptr<Detector> detector, Statistics &stats,
         int &keep_running);

  /// \brief used to register new commands with the Parser
  /// \param cmd_name name of command
  /// \param function pointer to command implementation
  int registercmd(const std::string &cmd_name, cmdFunction cmd_fn);

  void clearCommands();

  /// \brief parse a command
  /// \param[in] input char array holding command and its arguments
  /// \param[in] isize input size in bytes
  /// \param[out] output reply for the command
  /// \param[out] osize output size in bytes
  int parse(char *input, unsigned int isize, char *output, unsigned int *osize);

  std::map<std::string, cmdFunction> commands; ///< map of all commands
};

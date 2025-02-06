// Copyright (C) 2016 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Command parser implementation
///
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <cassert>
#include <common/Version.h>
#include <common/debug/Log.h>
#include <common/detector/EFUArgs.h>
#include <cstring>
#include <efu/Parser.h>
#include <efu/Server.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

//=============================================================================
static int stat_get_count(const std::vector<std::string> &cmdargs, char *output,
                          unsigned int *obytes,
                          std::shared_ptr<Detector> detector,
                          Statistics stats) {
  auto nargs = cmdargs.size();

  LOG(CMD, Sev::Debug, "STAT_GET_COUNT");
  if (nargs != 1) {
    LOG(CMD, Sev::Warning, "STAT_GET_COUNT: wrong number of arguments");
    return -Parser::EBADARGS;
  }

  *obytes = snprintf(output, SERVER_BUFFER_SIZE, "STAT_GET_COUNT %d",
                     detector->statsize() + (int)stats.size());

  return Parser::OK;
}

//=============================================================================
static int stat_get(const std::vector<std::string> &cmdargs, char *output,
                    unsigned int *obytes, std::shared_ptr<Detector> detector,
                    Statistics stats) {
  auto nargs = cmdargs.size();
  LOG(CMD, Sev::Debug, "STAT_GET");
  if (nargs != 2) {
    LOG(CMD, Sev::Warning, "STAT_GET: wrong number of arguments");
    return -Parser::EBADARGS;
  }
  auto index = atoi(cmdargs.at(1).c_str());
  std::string name;
  int64_t value;
  // Detector stats
  if (index <= detector->statsize()) {
    name = detector->statname(index);
    value = detector->statvalue(index);
  } else { // User supplied stats
    index = index - detector->statsize();
    name = stats.name(index);
    value = stats.value(index);
  }
  LOG(CMD, Sev::Debug, "STAT_GET {} {}", name, value);

  *obytes = snprintf(output, SERVER_BUFFER_SIZE, "STAT_GET %s %" PRIi64,
                     name.c_str(), value);

  return Parser::OK;
}

/// \brief function handling the calib_mode_set command
/// \param cmdargs vector of the command and its arguments
/// \param output pointer (UNUSED) to buffer containing the result
/// \param obytes pointer (UNUSED) to the number of chars in the buffer
/// \param detector pointer to detector instance
/// \return status negative for error
static int calib_mode_set(const std::vector<std::string> &cmdargs,
                          __attribute__((unused)) char *output,
                          __attribute__((unused)) unsigned int *obytes,
                          std::shared_ptr<Detector> detector) {
  LOG(CMD, Sev::Debug, "CALIB_MODE_SET");

  if (cmdargs.size() != 2) {
    LOG(CMD, Sev::Warning, "CALIB_MODE_SET: wrong number of arguments");
    return -Parser::EBADARGS;
  }
  auto index = atoi(cmdargs.at(1).c_str());
  if (index != 0 and index != 1) {
    return -Parser::EBADARGS;
  }

  detector->CalibrationMode = bool(index);
  LOG(CMD, Sev::Debug, "CALIB_ {}", index);
  return Parser::OK;
}

/// \brief function handling the calib_mode_get command
/// \param cmdargs vector of the command and its arguments
/// \param output pointer to buffer containing the result
/// \param obytes pointer to the number of chars in the buffer
/// \param detector pointer to detector instance
/// \return status negative for error
static int calib_mode_get(const std::vector<std::string> &cmdargs, char *output,
                          unsigned int *obytes,
                          std::shared_ptr<Detector> detector) {
  LOG(CMD, Sev::Debug, "CALIB_MODE_GET");

  if (cmdargs.size() != 1) {
    LOG(CMD, Sev::Warning, "CALIB_MODE_GET: wrong number of arguments");
    return -Parser::EBADARGS;
  }

  int Mode = (int)(detector->CalibrationMode);
  printf("Calibration mode %d\n", Mode);

  LOG(CMD, Sev::Debug, "CALIB_MODE_GET {}", Mode);

  *obytes = snprintf(output, SERVER_BUFFER_SIZE, "CALIB_MODE_GET %d", Mode);

  return Parser::OK;
}

//=============================================================================
static int version_get(const std::vector<std::string> &cmdargs, char *output,
                       unsigned int *obytes) {
  auto nargs = cmdargs.size();
  LOG(CMD, Sev::Debug, "VERSION_GET");
  if (nargs != 1) {
    LOG(CMD, Sev::Warning, "VERSION_GET: wrong number of arguments");
    return -Parser::EBADARGS;
  }

  *obytes = snprintf(output, SERVER_BUFFER_SIZE, "VERSION_GET %s %s",
                     efu_version().c_str(), efu_buildstr().c_str());

  return Parser::OK;
}

//=============================================================================
static int stat_get_name(const std::vector<std::string> &cmdargs, char *output,
                         unsigned int *obytes,
                         std::shared_ptr<Detector> detector, Statistics stats) {
  auto nargs = cmdargs.size();
  LOG(CMD, Sev::Debug, "STAT_GET_NAME");
  if (nargs != 2) {
    LOG(CMD, Sev::Warning, "STAT_GET_NAME: wrong number of arguments");
    return -Parser::EBADARGS;
  }

  std::string name = cmdargs.at(1);

  int64_t value = detector->statvaluebyname(name);
  if (value == -1) {
    value = stats.valueByName(name);
  }

  LOG(CMD, Sev::Debug, "STAT_GET_NAME {} {}", name, value);

  *obytes = snprintf(output, SERVER_BUFFER_SIZE, "STAT_GET_NAME %s %" PRIi64,
                     name.c_str(), value);

  return Parser::OK;
}

//=============================================================================
static int cmd_get_count(const std::vector<std::string> &cmdargs, char *output,
                         unsigned int *obytes, int count) {
  auto nargs = cmdargs.size();
  LOG(CMD, Sev::Debug, "CMD_GET_COUNT");
  if (nargs != 1) {
    LOG(CMD, Sev::Warning, "CMD_GET_COUNT: wrong number of arguments");
    return -Parser::EBADARGS;
  }

  *obytes = snprintf(output, SERVER_BUFFER_SIZE, "CMD_GET_COUNT %d", count);

  return Parser::OK;
}

//=============================================================================
static int cmd_get(const std::vector<std::string> &cmdargs, char *output,
                   unsigned int *obytes, Parser *parser) {
  auto nargs = cmdargs.size();
  LOG(CMD, Sev::Debug, "CMD_GET");
  if (nargs != 2) {
    LOG(CMD, Sev::Warning, "CMD_GET: wrong number of arguments");
    return -Parser::EBADARGS;
  }

  size_t index = atoi(cmdargs.at(1).c_str());
  if (index < 1 || index > parser->commands.size()) {
    LOG(CMD, Sev::Warning, "CMD_GET: command index {}, out of range (1 - {})",
        index, parser->commands.size());
    return -Parser::EBADARGS;
  }

  auto cmditerator = parser->commands.begin();
  std::advance(cmditerator, index - 1);

  *obytes = snprintf(output, SERVER_BUFFER_SIZE, "CMD_GET %s",
                     cmditerator->first.c_str());

  return Parser::OK;
}

//=============================================================================
static int detector_info_get(const std::vector<std::string> &cmdargs,
                             char *output, unsigned int *obytes,
                             std::shared_ptr<Detector> detector) {
  auto nargs = cmdargs.size();
  LOG(CMD, Sev::Debug, "DETECTOR_INFO_GET");
  if (nargs != 1) {
    LOG(CMD, Sev::Warning, "DETECTOR_INFO_GET: wrong number of arguments");
    return -Parser::EBADARGS;
  }

  *obytes = snprintf(output, SERVER_BUFFER_SIZE, "DETECTOR_INFO_GET %s",
                     detector->EFUSettings.DetectorName.c_str());

  return Parser::OK;
}

//=============================================================================
static int efu_exit(const std::vector<std::string> &cmdargs,
                    __attribute__((unused)) char *output,
                    __attribute__((unused)) unsigned int *obytes,
                    int &keep_running) {
  auto nargs = cmdargs.size();
  LOG(CMD, Sev::Debug, "EXIT");
  if (nargs != 1) {
    LOG(CMD, Sev::Warning, "EXIT: wrong number of arguments");
    return -Parser::EBADARGS;
  }

  LOG(CMD, Sev::Info, "Sending TERMINATE command to EFU");
  keep_running = 0;

  return Parser::OK;
}

//=============================================================================
static int runtime_stats(const std::vector<std::string> &cmdargs, char *output,
                         unsigned int *obytes,
                         std::shared_ptr<Detector> detector) {
  auto nargs = cmdargs.size();
  LOG(CMD, Sev::Debug, "RUNTIMESTATS");
  if (nargs != 1) {
    LOG(CMD, Sev::Warning, "RUNTIMESTATS: wrong number of arguments");
    return -Parser::EBADARGS;
  }

  *obytes = snprintf(output, SERVER_BUFFER_SIZE, "RUNTIMESTATS %u",
                     detector->runtimestat());

  return Parser::OK;
}

Parser::Parser(std::shared_ptr<Detector> detector, Statistics &mainStats,
               int &keep_running) {

  registercmd("VERSION_GET", version_get);

  registercmd("CMD_GET_COUNT", [this](const std::vector<std::string> &cmd,
                                      char *resp, unsigned int *nrChars) {
    return cmd_get_count(cmd, resp, nrChars, this->commands.size());
  });

  registercmd("CMD_GET", [this](const std::vector<std::string> &cmd, char *resp,
                                unsigned int *nrChars) {
    return cmd_get(cmd, resp, nrChars, this);
  });

  registercmd("EXIT", [&keep_running](const std::vector<std::string> &cmd,
                                      char *resp, unsigned int *nrChars) {
    return efu_exit(cmd, resp, nrChars, keep_running);
  });

  if (detector == nullptr) {
    LOG(CMD, Sev::Debug, "No detector specified, no detector commands loaded");
    return;
  }

  registercmd("STAT_GET",
              [detector, mainStats](const std::vector<std::string> &cmd,
                                    char *resp, unsigned int *nrChars) {
                return stat_get(cmd, resp, nrChars, detector, mainStats);
              });

  registercmd("STAT_GET_NAME",
              [detector, mainStats](const std::vector<std::string> &cmd,
                                    char *resp, unsigned int *nrChars) {
                return stat_get_name(cmd, resp, nrChars, detector, mainStats);
              });

  registercmd("STAT_GET_COUNT",
              [detector, mainStats](const std::vector<std::string> &cmd,
                                    char *resp, unsigned int *nrChars) {
                return stat_get_count(cmd, resp, nrChars, detector, mainStats);
              });

  registercmd("DETECTOR_INFO_GET",
              [detector](const std::vector<std::string> &cmd, char *resp,
                         unsigned int *nrChars) {
                return detector_info_get(cmd, resp, nrChars, detector);
              });

  registercmd("RUNTIMESTATS", [detector](const std::vector<std::string> &cmd,
                                         char *resp, unsigned int *nrChars) {
    return runtime_stats(cmd, resp, nrChars, detector);
  });

  registercmd("CALIB_MODE_SET", [detector](const std::vector<std::string> &cmd,
                                           char *resp, unsigned int *nrChars) {
    return calib_mode_set(cmd, resp, nrChars, detector);
  });
  registercmd("CALIB_MODE_GET", [detector](const std::vector<std::string> &cmd,
                                           char *resp, unsigned int *nrChars) {
    return calib_mode_get(cmd, resp, nrChars, detector);
  });
}

int Parser::registercmd(const std::string &cmd_name, cmdFunction cmd_fn) {
  LOG(CMD, Sev::Info, "Registering command: {}", cmd_name);
  if (commands[cmd_name] != 0) {
    LOG(CMD, Sev::Warning, "Command already exist: {}", cmd_name);
    return -1;
  }
  commands[cmd_name] = cmd_fn;
  return 0;
}

int Parser::parse(char *input, unsigned int ibytes, char *output,
                  unsigned int *obytes) {
  LOG(CMD, Sev::Debug, "parse() received {} bytes", ibytes);
  *obytes = 0;
  memset(output, 0, SERVER_BUFFER_SIZE);

  if (ibytes == 0) {
    *obytes = snprintf(output, SERVER_BUFFER_SIZE, "Error: <BADSIZE>");
    return -EUSIZE;
  }
  if (ibytes > SERVER_BUFFER_SIZE) {
    *obytes = snprintf(output, SERVER_BUFFER_SIZE, "Error: <BADSIZE>");
    return -EOSIZE;
  }

  if (input[ibytes - 1] != '\0') {
    LOG(CMD, Sev::Debug, "adding null termination");
    auto end = std::min(ibytes, SERVER_BUFFER_SIZE - 1);
    input[end] = '\0';
  }

  std::vector<std::string> tokens;
  char *chars_array = strtok((char *)input, "\n ");
  while (chars_array) {
    std::string token(chars_array);
    tokens.push_back(token);
    chars_array = strtok(NULL, "\n ");
  }

  if ((int)tokens.size() < 1) {
    LOG(CMD, Sev::Warning, "No tokens");
    *obytes = snprintf(output, SERVER_BUFFER_SIZE, "Error: <BADCMD>");
    return -ENOTOKENS;
  }

  LOG(CMD, Sev::Debug, "Tokens in command: {}", (int)tokens.size());
  for (auto token : tokens) {
    LOG(CMD, Sev::Debug, "Token: {}", token);
  }

  auto command = tokens.at(0);
  int res = -EBADCMD;

  if ((commands[command] != 0) && (command.size() < max_command_size)) {
    LOG(CMD, Sev::Debug, "Calling registered command {}", command);
    res = commands[command](tokens, output, obytes);
  }

  LOG(CMD, Sev::Debug, "parse1 res: {}, obytes: {}", res, *obytes);
  if (*obytes == 0) { // no  reply specified, create one

    assert((res == OK) || (res == -ENOTOKENS) || (res == -EBADCMD) ||
           (res == -EBADARGS));

    LOG(CMD, Sev::Debug, "creating response");
    switch (res) {
    case OK:
      *obytes = snprintf(output, SERVER_BUFFER_SIZE, "<OK>");
      break;
    case -ENOTOKENS:
    case -EBADCMD:
      *obytes = snprintf(output, SERVER_BUFFER_SIZE, "Error: <BADCMD>");
      break;
    case -EBADARGS:
      *obytes = snprintf(output, SERVER_BUFFER_SIZE, "Error: <BADARGS>");
      break;
    }
  }
  LOG(CMD, Sev::Debug, "parse2 res: {}, obytes: {}", res, *obytes);
  return res;
}

void Parser::clearCommands() { commands.clear(); }

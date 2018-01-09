/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <common/EFUArgs.h>
#include <common/Trace.h>
#include <common/Version.h>
#include <cstring>
#include <efu/Parser.h>
#include <efu/Server.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#define UNUSED __attribute__((unused))

//=============================================================================
static int stat_get_count(std::vector<std::string> cmdargs, char *output,
                          unsigned int *obytes,
                          std::shared_ptr<Detector> detector) {
  auto nargs = cmdargs.size();
  XTRACE(CMD, INF, "STAT_GET_COUNT\n");
  GLOG_INF("STAT_GET_COUNT");
  if (nargs != 1) {
    XTRACE(CMD, WAR, "STAT_GET_COUNT: wrong number of arguments\n");
    return -Parser::EBADARGS;
  }

  *obytes = snprintf(output, SERVER_BUFFER_SIZE, "STAT_GET_COUNT %d",
                     detector->statsize());

  return Parser::OK;
}

//=============================================================================
static int stat_get(std::vector<std::string> cmdargs, char *output,
                    unsigned int *obytes, std::shared_ptr<Detector> detector) {
  auto nargs = cmdargs.size();
  XTRACE(CMD, INF, "STAT_GET\n");
  GLOG_INF("STAT_GET");
  if (nargs != 2) {
    XTRACE(CMD, WAR, "STAT_GET: wrong number of arguments\n");
    return -Parser::EBADARGS;
  }
  auto index = atoi(cmdargs.at(1).c_str());

  std::string name = detector->statname(index);
  int64_t value = detector->statvalue(index);
  *obytes = snprintf(output, SERVER_BUFFER_SIZE, "STAT_GET %s %" PRIi64,
                     name.c_str(), value);

  return Parser::OK;
}

//=============================================================================
static int version_get(std::vector<std::string> cmdargs, char *output,
                       unsigned int *obytes) {
  auto nargs = cmdargs.size();
  XTRACE(CMD, INF, "VERSION_GET\n");
  GLOG_INF("VERSION_GET");
  if (nargs != 1) {
    XTRACE(CMD, WAR, "VERSION_GET: wrong number of arguments\n");
    return -Parser::EBADARGS;
  }

  *obytes = snprintf(output, SERVER_BUFFER_SIZE, "VERSION_GET %s %s",
                     efu_version().c_str(), efu_buildstr().c_str());

  return Parser::OK;
}

//=============================================================================
static int detector_info_get(std::vector<std::string> cmdargs, char *output,
                             unsigned int *obytes,
                             std::shared_ptr<Detector> detector) {
  auto nargs = cmdargs.size();
  XTRACE(CMD, INF, "DETECTOR_INFO_GET\n");
  GLOG_INF("DETECTOR_INFO_GET");
  if (nargs != 1) {
    XTRACE(CMD, WAR, "DETECTOR_INFO_GET: wrong number of arguments\n");
    return -Parser::EBADARGS;
  }

  *obytes = snprintf(output, SERVER_BUFFER_SIZE, "DETECTOR_INFO_GET %s",
                     detector->detectorname());

  return Parser::OK;
}

//=============================================================================
static int efu_exit(std::vector<std::string> cmdargs, UNUSED char *output,
                    UNUSED unsigned int *obytes, int &keep_running) {
  auto nargs = cmdargs.size();
  XTRACE(CMD, INF, "EXIT\n");
  GLOG_INF("EXIT");
  if (nargs != 1) {
    XTRACE(CMD, WAR, "EXIT: wrong number of arguments\n");
    return -Parser::EBADARGS;
  }

  XTRACE(CMD, INF, "Sending TERMINATE command to EFU\n");
  keep_running = 0;

  return Parser::OK;
}

/******************************************************************************/
/******************************************************************************/
Parser::Parser(std::shared_ptr<Detector> detector, int &keep_running) {

  registercmd("VERSION_GET", version_get);

  registercmd("EXIT", [&keep_running](std::vector<std::string> cmd, char *resp,
                                      unsigned int *nrChars) {
    return efu_exit(cmd, resp, nrChars, keep_running);
  });

  if (detector == nullptr) {
    XTRACE(CMD, ALW, "No detector specified, no detector commands loaded\n");
    return;
  }

  registercmd("STAT_GET", [detector](std::vector<std::string> cmd, char *resp,
                                     unsigned int *nrChars) {
    return stat_get(cmd, resp, nrChars, detector);
  });
  registercmd("STAT_GET_COUNT", [detector](std::vector<std::string> cmd,
                                           char *resp, unsigned int *nrChars) {
    return stat_get_count(cmd, resp, nrChars, detector);
  });

  registercmd("DETECTOR_INFO_GET",
              [detector](std::vector<std::string> cmd, char *resp,
                         unsigned int *nrChars) {
                return detector_info_get(cmd, resp, nrChars, detector);
              });

  auto DetCmdFuncsMap = detector->GetDetectorCommandFunctions();
  for (auto &FuncObj : DetCmdFuncsMap) {
    registercmd(FuncObj.first, FuncObj.second);
  }
}

int Parser::registercmd(std::string cmd_name, cmdFunction cmd_fn) {
  XTRACE(CMD, INF, "Registering command: %s\n", cmd_name.c_str());
  GLOG_INF("Registering command: " + cmd_name);
  if (commands[cmd_name] != 0) {
    XTRACE(CMD, WAR, "Command already exist: %s\n", cmd_name.c_str());
    GLOG_WAR("Command already exist: " + cmd_name);
    return -1;
  }
  commands[cmd_name] = cmd_fn;
  return 0;
}

int Parser::parse(char *input, unsigned int ibytes, char *output,
                  unsigned int *obytes) {
  XTRACE(CMD, DEB, "parse() received %u bytes\n", ibytes);
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
    XTRACE(CMD, DEB, "adding null termination\n");
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
    XTRACE(CMD, WAR, "No tokens\n");
    *obytes = snprintf(output, SERVER_BUFFER_SIZE, "Error: <BADCMD>");
    return -ENOTOKENS;
  }

  XTRACE(CMD, DEB, "Tokens in command: %d\n", (int)tokens.size());
  for (auto token : tokens) {
    XTRACE(CMD, INF, "Token: %s\n", token.c_str());
  }

  auto command = tokens.at(0);
  int res = -EBADCMD;

  if ((commands[command] != 0) && (command.size() < max_command_size)) {
    XTRACE(CMD, INF, "Calling registered command %s\n", command.c_str());
    res = commands[command](tokens, output, obytes);
  }

  XTRACE(CMD, DEB, "parse1 res: %d, obytes: %d\n", res, *obytes);
  if (*obytes == 0) { // no  reply specified, create one

    assert((res == OK) || (res == -ENOTOKENS) || (res == -EBADCMD) ||
           (res == -EBADARGS));

    XTRACE(CMD, INF, "creating response\n");
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
  XTRACE(CMD, DEB, "parse2 res: %d, obytes: %d\n", res, *obytes);
  return res;
}
/******************************************************************************/

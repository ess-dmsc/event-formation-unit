/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <common/EFUArgs.h>
#include <common/Trace.h>
#include <cspec/CalibrationFile.h>
#include <cstring>
#include <efu/Parser.h>
#include <efu/Server.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB

#define UNUSED __attribute__((unused))

//=============================================================================
static int cspec_load_calib(std::vector<std::string> cmdargs,
                            UNUSED char *output, UNUSED unsigned int *obytes) {
  XTRACE(CMD, INF, "CSPEC_LOAD_CALIB\n");
  if (cmdargs.size() != 2) {
    XTRACE(CMD, WAR, "CSPEC_LOAD_CALIB: wrong number of arguments\n");
    return -Parser::EBADARGS;
  }
  CalibrationFile calibfile;
  auto ret = calibfile.load(cmdargs.at(1), (char *)efu_args->wirecal,
                            (char *)efu_args->gridcal);
  if (ret < 0) {
    return -Parser::EBADARGS;
  }

  /** @todo some other ipc between main and threads ? */
  efu_args->proc_cmd = 1; // send load command to processing thread

  return Parser::OK;
}

//=============================================================================
static int cspec_show_calib(std::vector<std::string> cmdargs, char *output,
                            unsigned int *obytes) {
  auto nargs = cmdargs.size();
  unsigned int offset = 0;
  XTRACE(CMD, INF, "CSPEC_SHOW_CALIB\n");
  if (nargs == 1) {
    offset = 0;
  } else if (nargs == 2) {
    offset = atoi(cmdargs.at(1).c_str());
  } else {
    XTRACE(CMD, WAR, "CSPEC_SHOW_CALIB: wrong number of arguments\n");
    return -Parser::EBADARGS;
  }

  if (offset > CSPECChanConv::adcsize - 1) {
    return -Parser::EBADARGS;
  }

  *obytes = snprintf(
      output, SERVER_BUFFER_SIZE, "wire %d 0x%04x, grid %d 0x%04x", offset,
      efu_args->wirecal[offset], offset, efu_args->gridcal[offset]);

  return Parser::OK;
}

/******************************************************************************/
/******************************************************************************/
Parser::Parser() {
  // registercmd(std::string("STAT_GET_NAMEVAL"), stat_get_nameval);
  // registercmd(std::string("STAT_GET_COUNT"), stat_get_count);
  registercmd(std::string("CSPEC_LOAD_CALIB"), cspec_load_calib);
  registercmd(std::string("CSPEC_SHOW_CALIB"), cspec_show_calib);
}

int Parser::registercmd(std::string cmd_name, function_ptr cmd_fn) {
  XTRACE(CMD, INF, "Registering command: %s\n", cmd_name.c_str());
  if (commands[cmd_name] != 0) {
    XTRACE(CMD, WAR, "Command already exist: %s\n", cmd_name.c_str());
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

/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Trace.h>
#include <efu/ExitHandler.h>
#include <signal.h>
#include <stdlib.h>
#include <string>

ExitHandler::ExitHandler() {
  signal(SIGINT, &ExitHandler::signalhandler);
  signal(SIGSEGV, &ExitHandler::signalhandler);
  signal(SIGTERM, &ExitHandler::signalhandler);
  signal(SIGBUS, &ExitHandler::signalhandler);
}

void ExitHandler::signalhandler(int sig) {
  XTRACE(MAIN, ALW, "efu2 terminated with signal %d\n", sig);
  std::string message =
      "efu2 terminated with signal " + std::to_string(sig) + "\n";
  GLOG_CRI(message);
  exit(1);
}

void ExitHandler::Exit() {
  GLOG_INF("efu2 terminated normally");
  exit(0);
}

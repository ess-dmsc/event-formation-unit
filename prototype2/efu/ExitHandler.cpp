/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Trace.h>
#include <efu/ExitHandler.h>
#include <signal.h>
#include <stdlib.h>
#include <string>

extern int keep_running; /** @todo ugly global variable declared in main() */

ExitHandler::ExitHandler() {
  signal(SIGINT, &ExitHandler::noncritical);
  signal(SIGSEGV, &ExitHandler::critical);
  signal(SIGTERM, &ExitHandler::critical);
  signal(SIGBUS, &ExitHandler::critical);
}

void ExitHandler::critical(int sig) {
  XTRACE(MAIN, ALW, "efu2 terminated with critical signal %d\n", sig);
  std::string message = "efu2 terminated with critical signal " + std::to_string(sig) + "\n";
  GLOG_CRI(message);
  exit(1);
}

void ExitHandler::noncritical(int sig) {
  XTRACE(MAIN, ALW, "efu2 terminated with signal %d\n", sig);
  std::string message = "efu2 terminated with signal " + std::to_string(sig) + "\n";
  GLOG_CRI(message);
  keep_running = 0;
}

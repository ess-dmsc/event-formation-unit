/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <common/Trace.h>
#include <efu/ExitHandler.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <string>

int *ExitHandler::keep_running = nullptr;

void ExitHandler::InitExitHandler(int *runflag) {
  ExitHandler::keep_running = runflag;
  signal(SIGINT, &ExitHandler::noncritical);
  signal(SIGSEGV, &ExitHandler::critical);
  signal(SIGTERM, &ExitHandler::critical);
  signal(SIGBUS, &ExitHandler::critical);
}

void ExitHandler::critical(int sig) {
  XTRACE(MAIN, ALW, "efu terminated with critical signal %d", sig);
  print_trace();
  exit(1);
}

void ExitHandler::noncritical(int sig) {
  XTRACE(MAIN, ALW, "efu terminated with signal %d", sig);
  *ExitHandler::keep_running = 0;
}

/* Obtain a backtrace and print it to stdout. */
void ExitHandler::print_trace(void) {
  void *array[10];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace(array, 10);
  strings = backtrace_symbols(array, size);

  XTRACE(MAIN, ALW, "Obtained %zd stack frames.", size);

  for (i = 0; i < size; i++)
    XTRACE(MAIN, ALW, "%s", strings[i]);

  free(strings);
}

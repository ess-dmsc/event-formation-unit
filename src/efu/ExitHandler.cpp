// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of exit handler and back trace
///
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <efu/ExitHandler.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <string>

static ExitHandler::Exit DoExit{ExitHandler::Exit::NoExit};
static int LastSignal{0};

void ExitHandler::InitExitHandler() {
  signal(SIGINT, &ExitHandler::nonCritical);
  signal(SIGSEGV, &ExitHandler::critical);
  signal(SIGTERM, &ExitHandler::critical);
  signal(SIGBUS, &ExitHandler::critical);
}

void ExitHandler::critical(int sig) {
  // Calling printf is undefined behaviour but we accept that here as we call
  // exit immediately anyways.
  printf("efu terminated with critical signal %d\n", sig);
  printTrace();
  exit(1);
}

void ExitHandler::nonCritical(int sig) {
  DoExit = Exit::Exit;
  LastSignal = sig;
}

ExitHandler::Exit ExitHandler::HandleLastSignal() {
  if (Exit::Exit == DoExit) {
    XTRACE(MAIN, ALW, "efu terminated with signal %d", LastSignal);
  }
  return DoExit;
}

/* Obtain a backtrace and print it to stdout. */
void ExitHandler::printTrace(void) {
  void *array[10];
  size_t size;
  char **strings;
  size_t i;

  size = backtrace(array, 10);
  strings = backtrace_symbols(array, size);

  printf("Obtained %zu stack frames.\n", size);

  for (i = 0; i < size; i++)
    printf("%s\n", strings[i]);

  free(strings);
}

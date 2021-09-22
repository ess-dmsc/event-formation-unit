/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Trace macros with masks and levels
///
//===----------------------------------------------------------------------===//

#pragma once

#include "TraceGroups.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libgen.h>
#include <stdarg.h>

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>


const unsigned int TRC_L_ALW  = 1; //Should not be used
const unsigned int TRC_L_CRI  = 2;
const unsigned int TRC_L_ERR  = 3;
const unsigned int TRC_L_WAR  = 4;
const unsigned int TRC_L_NOTE = 5;
const unsigned int TRC_L_INF  = 6;
const unsigned int TRC_L_DEB  = 7;
// clang-format on

/// \brief get rid of annoying warning
/// \todo See if there is a better solution than pragma
#pragma GCC system_header

//#define TRC_LEVEL TRC_L_DEB

#ifndef TRC_LEVEL
const unsigned int USED_TRC_LEVEL = TRC_L_ERR;
#else
const unsigned int USED_TRC_LEVEL = TRC_LEVEL;
#endif

#define TRC_LEVEL USED_TRC_LEVEL

#ifndef TRC_ADD_FUNCTIONS_AND_INDENT_NEWLINES
#define TRC_ADD_FUNCTIONS_AND_INDENT_NEWLINES 0
#endif

static void printTime() {
  char buffer[26];
  int millisec;
  struct tm* tm_info;
  struct timeval tv;

  gettimeofday(&tv, NULL);

  millisec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
  if (millisec>=1000) { // Allow for rounding up to nearest second
    millisec -=1000;
    tv.tv_sec++;
  }

  tm_info = localtime(&tv.tv_sec);

  strftime(buffer, 26, "%H:%M:%S", tm_info);
  printf("%s.%03d ", buffer, millisec);
}

inline int Trace(int const LineNumber, char const *File, __attribute__((unused)) const char* Function, const char* GroupName,  const char* SeverityName, const char *Format, ...) {
  char *MessageBuffer = nullptr;

  va_list args;
  va_start (args, Format);
  __attribute__((unused)) int Characters = vasprintf(&MessageBuffer, Format, args);
  va_end (args);

  printTime();
  if (TRC_ADD_FUNCTIONS_AND_INDENT_NEWLINES) {
    char FunctionPretty[1024];
    snprintf (FunctionPretty, 1024, "%s()", Function);
    FunctionPretty[1023] = 0;

    // turn newlines into separate lines
    for (int i=0; i<Characters; ++i) {
      if (MessageBuffer[i] == '\n') {
        MessageBuffer[i] = '\0';
      }
    }

    char* MsgLine = MessageBuffer;
    while (MsgLine < MessageBuffer + Characters) {
      printf("%-4s %-20s %5d %-20s %-7s - %s\n", SeverityName, basename((char *)File), LineNumber, FunctionPretty, GroupName, MsgLine);
      MsgLine = strchr (MsgLine, '\0') + 1;
    }
  }
  else {
    printf("%-4s %-20s %5d %-7s - %s\n", SeverityName, basename((char *)File), LineNumber, GroupName, MessageBuffer);
  }

  free(MessageBuffer);
  return 0;
}

inline int DebugTrace(char const *Format, ...) {
  char *MessageBuffer = nullptr;
  va_list args;
  va_start (args, Format);
  __attribute__((unused)) int Characters = vasprintf(&MessageBuffer, Format, args);
  va_end (args);
  printf("%s\n", MessageBuffer);
  free(MessageBuffer);
  return 0;
}

#define XTRACE(Group, Level, Format, ...)                                   \
(void)(((TRC_L_##Level <= TRC_LEVEL) && (TRC_MASK & TRC_G_##Group))          \
? Trace(__LINE__, __FILE__, __func__, #Group, #Level, Format,\
##__VA_ARGS__) \
: 0)

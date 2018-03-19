/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Trace macros with masks and levels
 */

// Regular "#ifndef" guards due to "#pragma once" failure
#ifndef __TRACE_H__
#define __TRACE_H__

#include <cstdio>
#include <cstdarg>
#include <libgen.h>
#ifdef GRAYLOG
#include <graylog_logger/GraylogInterface.hpp>
#include <graylog_logger/Log.hpp>
#endif

/** Add trace groups below - must be powers of two */
// clang-format off
const unsigned int TRC_G_INPUT   = 0x00000001U;
const unsigned int TRC_G_OUTPUT  = 0x00000002U;
const unsigned int TRC_G_PROCESS = 0x00000004U;
const unsigned int TRC_G_MAIN    = 0x00000008U;
const unsigned int TRC_G_INIT    = 0x00000010U;
const unsigned int TRC_G_IPC     = 0x00000020U;
const unsigned int TRC_G_CMD     = 0x00000040U;
const unsigned int TRC_G_DATA    = 0x00000080U;

/** Add trace masks below, bitwise or of grouops */
const unsigned int TRC_M_ALL = (TRC_G_INPUT | TRC_G_OUTPUT | TRC_G_PROCESS | TRC_G_MAIN | TRC_G_INIT | TRC_G_IPC   | TRC_G_CMD | TRC_G_DATA);

/** Do not edit below */
const unsigned int TRC_M_NONE = 0;

const unsigned int TRC_L_ALW  = 1; //Should not be used
const unsigned int TRC_L_CRI  = 2;
const unsigned int TRC_L_ERR  = 3;
const unsigned int TRC_L_WAR  = 4;
const unsigned int TRC_L_NOTE = 5;
const unsigned int TRC_L_INF  = 6;
const unsigned int TRC_L_DEB  = 7;
// clang-format on

/** @brief get rid of annoying warning
 * @todo See if there is a better solution than pragma
 */
#pragma GCC system_header

#ifndef TRC_MASK
const unsigned int USED_TRC_MASK = TRC_M_ALL;
#else
const unsigned int USED_TRC_MASK = TRC_MASK;
#endif

#ifndef TRC_LEVEL
const unsigned int USED_TRC_LEVEL = TRC_L_ERR;
#else
const unsigned int USED_TRC_LEVEL = TRC_LEVEL;
#endif

struct SeverityToString {
  int SeverityLevel;
  std::string SeverityName;
};

static SeverityToString SeverityArray[] = {
  {0, "EMERGENCY"},
  {1, "ALW"},
  {2, "CRI"},
  {3, "ERR"},
  {4, "WAR"},
  {5, "NOTE"},
  {6, "ERR"},
  {7, "DEB"},
};

struct GroupToString {
  int GroupId;
  std::string GroupName;
};

static GroupToString GroupArray[] = {
  {0, "INPUT"},
  {1, "OUTPUT"},
  {2, "PROCESS"},
  {3, "MAIN"},
  {4, "INIT"},
  {5, "IPC"},
  {6, "CMD"},
  {7, "DATA"},
};

inline void Trace(int LineNumber, std::string File, const int Group, unsigned int SeverityLevel, const char *Format, ...) {
  char *MessageBuffer = nullptr;
  
  va_list args;
  va_start (args, Format);
  __attribute__((unused)) int Characters = vasprintf(&MessageBuffer, Format, args);
  va_end (args);
  
  int TempGroupValue = Group;
  int GroupBitPosition = 0;
  TempGroupValue >>= 1;
  for (GroupBitPosition = 0; TempGroupValue != 0; GroupBitPosition++) {
    TempGroupValue >>= 1;
  }
  if (SeverityLevel <= USED_TRC_LEVEL and USED_TRC_MASK & Group) {
    printf("%-3s %-80s %5d %-s - %s\n", SeverityArray[SeverityLevel].SeverityName.c_str(), File.c_str(), LineNumber, GroupArray[GroupBitPosition].GroupName.c_str(), MessageBuffer);
  }
#ifdef GRAYLOG
  Log::Msg(Severity(SeverityLevel), std::string(MessageBuffer, Characters), {{"file", File}, {"line", static_cast<std::int64_t>(LineNumber)}, {"group", GroupArray[GroupBitPosition].GroupName}});
#endif
  
  free(MessageBuffer);
}

#define XTRACE(Group, Level, Format, ...) Trace(__LINE__, __FILE__, TRC_G_##Group, TRC_L_##Level, Format, ##__VA_ARGS__)

inline void DebugTrace(unsigned int SeverityLevel, const char *Format, ...) {
  char *MessageBuffer = nullptr;
  va_list args;
  va_start (args, Format);
  __attribute__((unused)) int Characters = vasprintf(&MessageBuffer, Format, args);
  va_end (args);
#ifdef GRAYLOG
  Log::Msg(Severity(SeverityLevel), std::string(MessageBuffer, Characters));
#endif
  if (SeverityLevel <= USED_TRC_LEVEL) {
    printf("%s\n", MessageBuffer);
  }
  free(MessageBuffer);
}

#define DTRACE(Level, Format, ...) DebugTrace(TRC_L_##Level, Format, ##__VA_ARGS__)

#endif

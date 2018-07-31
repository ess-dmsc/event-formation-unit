/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Trace macros with masks and levels
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdio>
#include <cstdlib>
#include <libgen.h>
#include <stdarg.h>

#ifdef GRAYLOG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <graylog_logger/GraylogInterface.hpp>
#include <graylog_logger/Log.hpp>
#pragma GCC diagnostic pop
#define GLOG_DEB(x) Log::Msg(Severity::Debug, x)
#define GLOG_INF(x) Log::Msg(Severity::Informational, x)
#define GLOG_WAR(x) Log::Msg(Severity::Warning, x)
#define GLOG_ERR(x) Log::Msg(Severity::Error, x)
#define GLOG_CRI(x) Log::Msg(Severity::Critical, x)
#else
#define GLOG_DEB(x)
#define GLOG_INF(x)
#define GLOG_WAR(x)
#define GLOG_ERR(x)
#define GLOG_CRI(x)
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
constexpr unsigned int TRC_M_ALL = (TRC_G_INPUT | TRC_G_OUTPUT | TRC_G_PROCESS | TRC_G_MAIN | TRC_G_INIT | TRC_G_IPC   | TRC_G_CMD | TRC_G_DATA);

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

#define TRC_LEVEL 7

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

#define TRC_MASK USED_TRC_MASK
#define TRC_LEVEL USED_TRC_LEVEL

inline int Trace(int const LineNumber, char const *File, const int Group, unsigned int const SeverityLevel, const char* GroupName,  const char* SeverityName, const char *Format, ...) {
  char *MessageBuffer = nullptr;
  
  va_list args;
  va_start (args, Format);
  __attribute__((unused)) int Characters = vasprintf(&MessageBuffer, Format, args);
  va_end (args);
  
  printf("%-3s %-20s %5d %-7s - %s\n", SeverityName, basename((char *)File), LineNumber, GroupName, MessageBuffer);
  
  free(MessageBuffer);
  return 0;
}

inline int DebugTrace(unsigned int const SeverityLevel, char const *Format, ...) {
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
(void)(((TRC_L_##Level >= TRC_LEVEL) && (TRC_MASK & TRC_G_##Group))          \
? Trace(__LINE__, __FILE__, TRC_G_##Group, TRC_L_##Level, #Group, #Level, Format,\
##__VA_ARGS__) \
: 0)


/// \brief Raw trace
#define DTRACE(Level, Format, ...)                                                \
(void)((TRC_L_##Level >= TRC_LEVEL) ? DebugTrace(TRC_L_##Level, Format, ##__VA_ARGS__) : 0)

/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Trace macros with masks and levels
 */

#include <cstdio>
#ifdef GRAYLOG
#include <graylog_logger/GraylogInterface.hpp>
#include <graylog_logger/Log.hpp>
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
#define TRC_G_INPUT   0x00000001U
#define TRC_G_OUTPUT  0x00000002U
#define TRC_G_PROCESS 0x00000004U
#define TRC_G_MAIN    0x00000008U
#define TRC_G_INIT    0x00000010U
#define TRC_G_IPC     0x00000020U
#define TRC_G_CMD     0x00000040U

/** Add trace masks below, bitwise or of grouops */
#define TRC_M_ALL                                                              \
  (TRC_G_INPUT | TRC_G_OUTPUT | TRC_G_PROCESS | TRC_G_MAIN | TRC_G_INIT |      \
   TRC_G_IPC   | TRC_G_CMD )

/** Do not edit below */
#define TRC_M_NONE 0

#define TRC_L_ALW 12
#define TRC_L_CRI 10
#define TRC_L_ERR 8
#define TRC_L_WAR 6
#define TRC_L_INF 4
#define TRC_L_DEB 2
// clang-format on

/** @brief get rid of annoying warning
 * @todo See if there is a better solution than pragma
 */
#pragma GCC system_header

#ifndef TRC_MASK
#define TRC_MASK TRC_M_ALL
#endif

#ifndef TRC_LEVEL
#define TRC_LEVEL TRC_L_ERR
#endif

#if 0
#define XTRACE(group, level, fmt, ...)                                         \
  (void)(((TRC_L_##level >= TRC_LEVEL) && (TRC_MASK & TRC_G_##group))          \
             ? printf("%-3s %80s %5d %-s - " fmt, #level, __FILE__, __LINE__,  \
                      #group, ##__VA_ARGS__)                                   \
             : 0)
#endif

#define XTRACE(group, level, fmt, ...)                                         \
  (void)(((TRC_L_##level >= TRC_LEVEL) && (TRC_MASK & TRC_G_##group))          \
             ? printf("%-3s %-8s" fmt, #level,\
                      #group, ##__VA_ARGS__)                                   \
             : 0)


/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Trace macros with masks and levels
 */

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
#define TRC_LEVEL TRC_L_CRI
#endif

#define XTRACE(group, level, fmt, ...)                                         \
  (void)(((TRC_L_##level >= TRC_LEVEL) && (TRC_MASK & TRC_G_##group))          \
             ? printf("%-3s %18s %5d %-s - " fmt, #level, __FILE__, __LINE__,  \
                      #group, ##__VA_ARGS__)                                   \
             : 0)

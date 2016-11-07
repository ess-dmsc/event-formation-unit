/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Command line argument parser for EFU
 */

 #pragma once

 #define TRC_G_INPUT   0x00000001
 #define TRC_G_OUTPUT  0x00000002
 #define TRC_G_PROCESS 0x00000004
 #define TRC_G_MAIN    0x00000008

 #define TRC_M_ALL (TRC_G_INPUT | TRC_G_OUTPUT | TRC_G_PROCESS | TRC_G_MAIN)
 #define TRC_M_NONE 0

 #define TRC_L_CRI   10
 #define TRC_L_ERR   8
 #define TRC_L_WAR   6
 #define TRC_L_INF   4
 #define TRC_L_VER   2


#define TRC_MASK TRC_M_ALL
#define TRC_LEVEL TRC_L_VER

#ifdef TRACE
 #define XTRACE(group, level, format, ...)  do {\
   if ((group && TRC_MASK) && (level >= TRC_LEVEL )) \
   printf("%s %s - " format, __FILE__, __FUNCTION__, __VA_ARGS__ );\
 } while (1);
 #else
  #define XTRACE(group, level, format, ...)
 #endif

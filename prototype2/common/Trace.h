/** Copyright (C) 2016 European Spallation Source ERIC */

/** @file
 *
 *  @brief Command line argument parser for EFU
 */

 /** Add trace groups below - must be powers of two */
 #define TRC_G_INPUT   0x00000001
 #define TRC_G_OUTPUT  0x00000002
 #define TRC_G_PROCESS 0x00000004
 #define TRC_G_MAIN    0x00000008
 #define TRC_G_INIT    0x00000010

/** Add trace masks below, bitwise or of grouops */
 #define TRC_M_ALL (TRC_G_INPUT | TRC_G_OUTPUT | TRC_G_PROCESS | TRC_G_MAIN | \
                    TRC_G_INIT)

/** Do not edit below */
 #define TRC_M_NONE 0

 #define TRC_L_CRI   10
 #define TRC_L_ERR   8
 #define TRC_L_WAR   6
 #define TRC_L_INF   4
 #define TRC_L_DEB   2


#define TRC_MASK TRC_M_ALL
#define TRC_LEVEL TRC_L_WAR

#define XTRACE(group, level, fmt, ...) \
    (void)(((TRC_L_ ## level >= TRC_LEVEL) && (TRC_MASK & group)) ? \
    printf("%18s %5d %-s - " fmt, __FILE__, __LINE__, #group, ##__VA_ARGS__) \
     : \
    0)

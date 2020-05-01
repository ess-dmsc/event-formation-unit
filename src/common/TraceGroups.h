/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Trace macros with masks and levels
///
//===----------------------------------------------------------------------===//

#pragma once

/// Add trace groups below - must be powers of two
// clang-format off
const unsigned int TRC_G_INPUT   = 0x00000001U;
const unsigned int TRC_G_OUTPUT  = 0x00000002U;
const unsigned int TRC_G_PROCESS = 0x00000004U;
const unsigned int TRC_G_MAIN    = 0x00000008U;
const unsigned int TRC_G_INIT    = 0x00000010U;
const unsigned int TRC_G_IPC     = 0x00000020U;
const unsigned int TRC_G_CMD     = 0x00000040U;
const unsigned int TRC_G_DATA    = 0x00000080U;
const unsigned int TRC_G_KAFKA   = 0x00000100U;
const unsigned int TRC_G_UTILS   = 0x00000200U;
const unsigned int TRC_G_CLUSTER = 0x00000400U;
const unsigned int TRC_G_EVENT   = 0x00000800U;
const unsigned int TRC_G_BUILDER = 0x00001000U;

/// Add trace masks below, bitwise or of grouops

// Do not edit below
const unsigned int TRC_M_NONE = 0;
const unsigned int TRC_M_ALL  = 0xffffffffU;

// clang-format on

#ifndef TRC_MASK
const unsigned int USED_TRC_MASK = TRC_M_ALL;
#else
const unsigned int USED_TRC_MASK = TRC_MASK;
#endif

#define TRC_MASK USED_TRC_MASK
//#define TRC_MASK (TRC_G_IPC | TRC_G_CMD)

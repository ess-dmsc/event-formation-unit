// Copyright (C) 2017 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Dataset for running unit tests - do not edit if unsure of what
/// they do
///
//===----------------------------------------------------------------------===//

#include <cinttypes>
#include <vector>

// clang-format off

/// \todo add link to ESS Readout and LoKI data formats eventually

// Wrong cookie, good version
std::vector<uint8_t> ErrCookie
{
    0x00, 0x00, 0x00, 0x00, // pad, pad, pad, v0
    0x45, 0x53, 0x52        // 'E', 'S', 'R'
};

// wrong version, good cookie
std::vector<uint8_t> ErrVersion
{
    0x00, 0x12,             // pad, v10
    0x45, 0x53, 0x53        // 'E', 'S', 'S'
};

// wrong padding
std::vector<uint8_t> ErrPad
{

    0x01, 0x00,             // pad, v0
    0x45, 0x53, 0x53        // 'E', 'S', 'S'
};

// An OK packet must be at least sizeof(Readout::PacketHeaderV0)
std::vector<uint8_t> ErrMaxOutputQueue
{
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x1e, 0x00, 0x0C, 0x00, // len(0x001e), OQ12, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x07, 0x00, 0x00, 0x00  // Seq number 7
};

// An OK packet must be at least sizeof(Readout::PacketHeaderV0)
// or sizeof(Readout::PacketHeaderV1)
// Next two packets must belong to same output queue because we're
// also testing sequence numbers
std::vector<uint8_t> OkVersionV0
{
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x1e, 0x00, 0x0B, 0x00, // len(0x001e), OQ11, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x07, 0x00, 0x00, 0x00  // Seq number 7
};

// An OK packet must be at least sizeof(Readout::PacketHeaderV0)
// or sizeof(Readout::PacketHeaderV1)
// Next two packets must belong to same output queue because we're
// also testing sequence numbers
std::vector<uint8_t> OkVersionV1
{
    0x00, 0x01, 0x45, 0x53, // Padding, V1, 'E', 'S'
    0x53, 0x30, 0x20, 0x00, // 'S', type 0x30, len(0x0020),
    0x0B, 0x00, 0x00, 0x00, //  OQ11, TSrc0, PT HI,
    0x00, 0x00, 0x00, 0x00, // PT HI, PT Lo
    0x00, 0x00, 0x00, 0x00, // PT Lo, PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT HI, PPT Lo
    0x00, 0x00, 0x07, 0x00, // PPT Lo, Seq number 7
    0x00, 0x00, 0x00, 0x00  // Seq number 7, CMAC Padding
};

// must be at least sizeof(Readout::PacketHeaderV0)
std::vector<uint8_t> OkVersionNextSeq
{
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x1e, 0x00, 0x0B, 0x00, // len(0x001e), OQ11, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x08, 0x00, 0x00, 0x00  // Seq number 8
};


// Length 0x005e = 30 + 4 + 3 * 20 = 94
std::vector<uint8_t> OkThreeLokiReadoutsV0
{
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x5e, 0x00, 0x0B, 0x00, // len(0x005e), OQ11, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x08, 0x00, 0x00, 0x00, // Seq number 8

    0x00, 0x00, 0x40, 0x00, // Data Header: ring 0, fen 0, size 64

    0x00, 0x00, 0x00, 0x00, // Readout 1
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, // Readout 2
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, // Readout 3
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

// Length 0x0060 = 32 + 4 + 3 * 20 = 96
std::vector<uint8_t> OkThreeLokiReadoutsV1
{
                0x00, 0x01, // pad, v1
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x60, 0x00, 0x0B, 0x00, // len(0x005e), OQ11, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x08, 0x00, 0x00, 0x00, // Seq number 8
    0x00, 0x00,                       // CMAC Padding

    0x00, 0x00, 0x40, 0x00, // Data Header: ring 0, fen 0, size 64

    0x00, 0x00, 0x00, 0x00, // Readout 1
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, // Readout 2
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    0x00, 0x00, 0x00, 0x00, // Readout 3
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

// Bad PulseTime fractional time
std::vector<uint8_t> ErrPulseTimeFracV0
{
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x1e, 0x00, 0x0B, 0x00, // len(0x001e), OQ11, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x14, 0x93, 0x3f, 0x05, // PT LO  0x053F9313 + 1 (invalid)
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x07, 0x00, 0x00, 0x00 // Seq number 7
};

// Bad PulseTime fractional time
std::vector<uint8_t> ErrPulseTimeFracV1
{
                0x00, 0x01, // pad, v0
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x20, 0x00, 0x0B, 0x00, // len(0x0020), OQ11, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x14, 0x93, 0x3f, 0x05, // PT LO  0x053F9313 + 1 (invalid)
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x07, 0x00, 0x00, 0x00, // Seq number 7
    0x00, 0x00                        // CMAC Padding
};

// Bad PrevPulseTime fractional time
std::vector<uint8_t> ErrPrevPulseTimeFracV0
{
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x1e, 0x00, 0x0B, 0x00, // len(0x001e), OQ11, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x14, 0x93, 0x3f, 0x05, // PPT Lo 0x053F9313 + 1 (invalid)
    0x07, 0x00, 0x00, 0x00  // Seq number 7
};

// Bad PrevPulseTime fractional time
std::vector<uint8_t> ErrPrevPulseTimeFracV1
{
                0x00, 0x00, // pad, v1
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x20, 0x00, 0x0B, 0x00, // len(0x0020), OQ11, TSrc0
    0x00, 0x00, 0x00, 0x00, // PT HI
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x00, 0x00, 0x00, 0x00, // PPT HI
    0x14, 0x93, 0x3f, 0x05, // PPT Lo 0x053F9313 + 1 (invalid)
    0x07, 0x00, 0x00, 0x00, // Seq number 7
    0x00, 0x00                        // CMAC Padding
};

// Too large time difference between Pulse and PrevPulse
std::vector<uint8_t> ErrMaxPulseTimeV0
{
                0x00, 0x00, // pad, v0
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x1e, 0x00, 0x0B, 0x00, // len(0x001e), OQ11, TSrc0
    0x0a, 0x00, 0x00, 0x00, // PT HI  10 s
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x08, 0x00, 0x00, 0x00, // PPT HI 8 s
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x07, 0x00, 0x00, 0x00  // Seq number 7
};

// Too large time difference between Pulse and PrevPulse
std::vector<uint8_t> ErrMaxPulseTimeV1
{
                        0x00, 0x01, // pad, v1
    0x45, 0x53, 0x53, 0x30, // 'E', 'S', 'S', type 0x30
    0x20, 0x00, 0x0B, 0x00, // len(0x0020), OQ11, TSrc0
    0x0a, 0x00, 0x00, 0x00, // PT HI  10 s
    0x00, 0x00, 0x00, 0x00, // PT LO
    0x08, 0x00, 0x00, 0x00, // PPT HI 8 s
    0x00, 0x00, 0x00, 0x00, // PPT Lo
    0x07, 0x00, 0x00, 0x00, // Seq number 7
    0x00, 0x00                        // CMAC Padding
};

// clang-format on

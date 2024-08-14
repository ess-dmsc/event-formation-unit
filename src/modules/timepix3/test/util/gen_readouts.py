# Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
# ===----------------------------------------------------------------------===#
#
# Brief: Helper program to generate test readouts file for the Timepix3 module.
# ===----------------------------------------------------------------------===#

import bitstruct
"""
This module provides functions for generating test readouts for the Timepix3 module.

Functions:
- gen_pixel(): Generates a test readout for a pixel.
- gen_tdc(): Generates a test readout for a TDC (Time-to-Digital Converter).

Usage:
 - Run with python3 to generate a test readout into a file named "tpx_readouts".
 - Use hexdump or similar to inspect the generated file.
"""


def gen_pixel():
    # Define the pixel format of the payload
    format = "u4u8u7u1u14u10u4u16"

    pixel_header = 0xB

    return bitstruct.pack(format, pixel_header, 16, 12, 1, 7937, 205, 3, 146)[::-1]


def gen_tdc():
    format = "u4u4u12u35u4u5"

    tdc_header = 0x6
    tdc_type = 10

    return bitstruct.pack(format, tdc_header, tdc_type, 4095, 34359738367, 15, 0)[::-1]


if __name__ == "__main__":
    with open("tpx_readouts", "wb") as file:
        file.write(gen_tdc())

    with open("tpx_readouts", "ab") as file:
        file.write(gen_pixel())

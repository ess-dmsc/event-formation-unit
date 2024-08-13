# Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
# ===----------------------------------------------------------------------===#
#
# Brief: This code is generating a tpx file which can be used with the tpx
#        generator. The tpx generator is a tool that generates udp packets
#        from tpx data and sends it to the EFU.
#
#        This code is generating tpx file with a minimal data to initialize
#        the timepix efu and send some pixel packets with exact delays to
#        test TOF calculation.
# ===----------------------------------------------------------------------===#

import bitstruct
import struct
import numpy as np
import sys
import argparse

PIXEL_MAX_TIMESTAMP_NS = 26843545600
UDP_MAX_SIZE = 8952

def print_pixel_readout(data):
    format = "u4u16u14u10u4u16"
    packed = struct.pack(">Q", data)
    unpacked = struct.unpack("<Q", packed)[0]
    swaped_bytes = struct.pack("<Q", unpacked)
    pix_header, pix_addr, toa, tot, ftoa, spdr = bitstruct.unpack(format, swaped_bytes)
    print(pix_header, pix_addr, toa, tot, ftoa, spdr, sep="\t")


def print_tdc_readout(data):
    format = "u4u4u12u35u4u5"
    packed = struct.pack(">Q", data)
    unpacked = struct.unpack("<Q", packed)[0]
    swaped_bytes = struct.pack("<Q", unpacked)
    tdc_header, tdc_type, counter, timestamp, stamp, _ = bitstruct.unpack(
        format, swaped_bytes
    )
    print(tdc_header, tdc_type, counter, timestamp, stamp, sep="\t")

def gen_tdc(counter, tdc_ns):
    # Define the tdc format of the payload
    format = "u4u4u12u35u4u5"

    tdc_header = 0x6
    tdc_type = 15

    timestamp = int(tdc_ns / 3.125)
    stamp = 1

    tdc_packet = bitstruct.pack(
        format, tdc_header, tdc_type, counter, timestamp, stamp, 0
    )

    tdc_packet = tdc_packet[::-1]

    return tdc_packet


def gen_pixel(tdc_ns, delay_to_tdc):
    # Define the pixel format of the payload
    format = "u4u16u14u10u4u16"

    pixel_header = 0xb

    pixel_time_ns = (tdc_ns + delay_to_tdc) - (
        PIXEL_MAX_TIMESTAMP_NS * int(tdc_ns / PIXEL_MAX_TIMESTAMP_NS)
    )

    spdr = int(pixel_time_ns / 409600)
    spdr_remainder = pixel_time_ns % 409600
    spdr_remainder2 = pixel_time_ns - (spdr * 409600)
    toa = int(spdr_remainder / 25)
    toa_remainder = spdr_remainder % 25
    ftoa = int(toa_remainder / 1.5625)

    # for clusters in range(0, pixel_cluster_in_packet):
    pix_addr = np.random.randint(100, 12000)
    tot = np.random.randint(1, 255)

    return bitstruct.pack(format, pixel_header, pix_addr, toa, tot, ftoa, spdr)[::-1]


def fill_payload(payload):

    missing_packets = (UDP_MAX_SIZE - len(payload)) // 8

    for i in range(missing_packets):
        format = "u4u60"

        rubish = bitstruct.pack(format, 0x01, 0)[::-1]
        payload += rubish

def us_to_ns(us):
    return us * 1000


def parse_arguments():
    parser = argparse.ArgumentParser(description="TPX Test Generator")
    parser.add_argument("-d", "--delay", type=float, help="Delay in microseconds")
    parser.add_argument("-q", "--freq", type=float, help="Frequency in Hz")
    return parser.parse_args()


def calculate_frequency_period(frequency):
    period = 1 / frequency
    period_ns = period * 1e9
    return period_ns

def flush_data_to_file(data):
    with open("tpx/file", "ab") as file:
        file.write(data)
        
    data = bytearray()

def main():

    args = parse_arguments()
    delay = args.delay
    frequncy = args.freq

    if delay is None:
        print(
            "Error: Delay parameter is required. Use -h or --help for more information."
        )
        sys.exit(1)

    if frequncy is None:
        print(
            "Error: Frequency parameter is required. Use -h or --help for more information."
        )
        sys.exit(1)

    counter = 1
    tdc_ns = 0

    data = bytearray()

    # Generate an empty tpx file
    with open("tpx/file", "wb") as file:
        file.write(data)

    # Generate a tdc packet to initialize the timepix efu
    data += gen_tdc(counter, tdc_ns)
    fill_payload(data)

    flush_data_to_file(data)
    
    counter += 1
    
    for i in range(1):
        data += gen_tdc(counter + i, tdc_ns)
        fill_payload(data)

        flush_data_to_file(data)
        
        data += gen_pixel(tdc_ns, us_to_ns(delay))
        fill_payload(data)

        flush_data_to_file(data)

        tdc_ns += calculate_frequency_period(frequncy)
        if tdc_ns > 107374182400:
            tdc_ns = 0

    # with open('file', 'rb') as file:
    #     raw_data = file.read()

    # data_array = np.frombuffer(raw_data, dtype=np.uint64)

    # print("p_head\tp_addr\ttoa\ttot\tftoa\tspdr")

    # for i in range(len(data_array)):
    #     if (data_array[i] >> 60) & 0xF == 0x6:
    #         print_tdc_readout(data_array[i])
    #     else:
    #         print_pixel_readout(data_array[i])


if __name__ == "__main__":
    main()

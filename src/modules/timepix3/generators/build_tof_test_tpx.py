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


def gen_tdc_readout(counter, tdc_ns):
    # Define the tdc format of the payload
    format = "u4u4u12u35u4u5"

    tdc_header = 0x6
    tdc_type = 15

    timestamp = int(tdc_ns / 3.125)
    stamp = 12

    tdc_packet = bitstruct.pack(
        format, tdc_header, tdc_type, counter, timestamp, stamp, 0
    )

    tdc_packet = tdc_packet[::-1]

    return tdc_packet


def gen_pixel_cluster(tdc_ns, delay_to_tdc, cluster_size):
    # Define the pixel format of the payload
    format = "u4u16u14u10u4u16"

    pixel_header = 11

    pixel_time_ns = (tdc_ns + delay_to_tdc) - (
        PIXEL_MAX_TIMESTAMP_NS * int(tdc_ns / PIXEL_MAX_TIMESTAMP_NS)
    )

    spdr = int(pixel_time_ns / 409600)
    spdr_remainder = pixel_time_ns % 409600
    toa = int(spdr_remainder / 25)
    toa_remainder = spdr_remainder % 25
    ftoa = int(toa_remainder / 1.5625)

    payload = b""

    pixel_cluster_in_packet = UDP_MAX_SIZE // (cluster_size * 8)

    for clusters in range(0, pixel_cluster_in_packet):
        pix_addr = np.random.randint(100, 12000)

        for i in range(cluster_size):

            tot = np.random.randint(1, 255)

            pixel_readout = bitstruct.pack(
                format, pixel_header, pix_addr + i, toa, tot, 0, spdr
            )[::-1]

            payload += pixel_readout

    return payload


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


def us_to_ns(us):
    return us * 1000


def parse_arguments():
    parser = argparse.ArgumentParser(description="TPX Test Generator")
    parser.add_argument("-d", "--delay", type=float, help="Delay in microseconds")
    return parser.parse_args()


def main():
    
    args = parse_arguments()
    delay = args.delay
    
    if delay is None:
        print("Error: Delay parameter is required. Use -h or --help for more information.")
        sys.exit(1)

    counter = 1
    tdc_ns = 10
    cluster_size = 1

    data = b""

    # Generate an empty tpx file
    with open("tpx/file", "wb") as file:
        file.write(data)

    for i in range(1):
        data += gen_tdc_readout(counter + i, tdc_ns)
        data += gen_pixel_cluster(tdc_ns, us_to_ns(delay), cluster_size)

        tdc_ns += int(1e9 / 10)
        if tdc_ns > 107374182400:
            tdc_ns = 0

        with open("tpx/file", "ab") as file:
            file.write(data)

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

# Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
# ===----------------------------------------------------------------------===#
#
# Brief: Test code.
# ===----------------------------------------------------------------------===#

import bitstruct
import struct
import numpy as np

def gen_tdc_readout(counter, tdc_ns):
    # Define the tdc format of the payload
    format = 'u4u4u12u35u4u5'

    tdc_header = 0x6
    tdc_type = 15

    timestamp = int(tdc_ns / 3.125)
    stamp = 12

    tdc_packet = bitstruct.pack(format, tdc_header, tdc_type, counter, timestamp, stamp, 0)

    tdc_packet = tdc_packet[::-1]

    return tdc_packet

def gen_pixel_cluster(tdc_ns, delay_to_tdc, cluster_size):
    # Define the pixel format of the payload
    format = 'u4u16u14u10u4u16'

    pixel_header = 11

    pixel_time_ns = tdc_ns + delay_to_tdc

    spdr = int(pixel_time_ns / 409600)
    spdr_remainder = pixel_time_ns % 409600
    toa = int(spdr_remainder / 25)
    toa_remainder = spdr_remainder % 25
    ftoa = int(toa_remainder / 1.5625)

    payload = b''
    
    for i in range(cluster_size):

        tot = 50

        pix_addr = 100 + i

        payload = bitstruct.pack(format, pixel_header, pix_addr, toa + i, tot, ftoa, spdr)[::-1]

    return payload

def print_pixel_readout(data):
    format = 'u4u16u14u10u4u16'
    packed = struct.pack('>Q', data)
    unpacked = struct.unpack('<Q', packed)[0]
    swaped_bytes = struct.pack('<Q', unpacked)
    pix_header, pix_addr, toa, tot, ftoa, spdr = bitstruct.unpack(format, swaped_bytes)
    print(pix_header, pix_addr, toa, tot, ftoa, spdr, sep='\t')

def print_tdc_readout(data):
    format = 'u4u4u12u35u4u5'
    packed = struct.pack('>Q', data)
    unpacked = struct.unpack('<Q', packed)[0]
    swaped_bytes = struct.pack('<Q', unpacked)
    tdc_header, tdc_type, counter, timestamp, stamp, _ = bitstruct.unpack(format, swaped_bytes)
    print(tdc_header, tdc_type, counter, timestamp, stamp, sep='\t')

def main():
    # Generate TDC readout payload
    counter = 1
    tdc_ns = 0
    delay_to_tdc = 50000
    cluster_size = 8

    data = b""

    for i in range(1000):
        data += gen_tdc_readout(counter, tdc_ns)
        data += gen_pixel_cluster(tdc_ns, delay_to_tdc, cluster_size)

        tdc_ns += int(1e9 / 14)
        if tdc_ns > 107374182400:
            tdc_ns = 0

    with open('file', 'wb') as file:
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

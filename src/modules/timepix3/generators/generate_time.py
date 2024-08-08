# Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
# ===----------------------------------------------------------------------===#
#
# Brief: Test code.
# ===----------------------------------------------------------------------===#

import bitstruct
import struct

def gen_tdc_readout(counter, tdc_ns):
    # Define the tdc format of the payload
    # format = 'u4u4u12u35u4u5'
    # format = 'u5u4u35u12u4u4'

    # tdc_header = 0x6
    # tdc_type = 0xb

    # timestamp = int(tdc_ns / 3.125)
    # stamp = 12

    # # Pack the values into a byte structure
    # payload = bitstruct.pack(format, tdc_header, tdc_type, counter, timestamp, stamp, 0)
    # payload = bitstruct.pack(format, 0, stamp, timestamp, counter, tdc_type, tdc_header)

    format = 't16u8u8u8u16'

    payload = bitstruct.pack(format, 'TP', ord('X'), 3, 1, 1, 0)

    # payload = b''

    # payload += bitstruct.pack('u5', 0)
    # payload += bitstruct.pack('u4', stamp)
    # payload += bitstruct.pack('u35', timestamp)
    # payload += bitstruct.pack('u12', counter)
    # payload += bitstruct.pack('u4', 0x6)
    # payload += bitstruct.pack('u4', 0xb)

    return payload

def gen_pixel_cluster(tdc_ns, delay_to_tdc, cluster_size):
    # Define the pixel format of the payload
    format = '<u4u16u14u10u4u16'

    pixel_header = 0xb

    pixel_time_ns = tdc_ns + delay_to_tdc

    spdr = int(pixel_time_ns / 409600)
    spdr_remainder = pixel_time_ns % 409600
    toa = int(spdr_remainder / 25)
    toa_remainder = spdr_remainder % 25
    ftoa = int(toa_remainder / 1.5625)

    payload = b''
    
    for i in range(cluster_size):

        tot = 50

        toa += i

        pix_addr = 1000 + i

        # Pack the values into a byte structure
        # payload += bitstruct.pack(format, pixel_header, pix_addr, toa, tot, ftoa, spdr)

    return payload

def main():
    # Generate TDC readout payload
    counter = 1
    tdc_ns = 500
    data = b""
    data += gen_tdc_readout(counter, tdc_ns)

    delay_to_tdc = 2000
    cluster_size = 8

    data += gen_pixel_cluster(tdc_ns, delay_to_tdc, cluster_size)

    with open('file', 'wb') as file:
        file.write(data)

if __name__ == "__main__":
    main()

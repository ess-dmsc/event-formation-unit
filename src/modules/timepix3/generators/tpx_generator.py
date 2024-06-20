# Copyright (C) 2023-2024 European Spallation Source, ERIC. See LICENSE file
# ===----------------------------------------------------------------------===#
#
# Brief: This script reads TPX files format and adds EVR timestamps next the the
#        TDC data. The script sends the data over UDP to a destination EFU.
# ===----------------------------------------------------------------------===#

import argparse
import multiprocessing
import numpy as np
from scapy.all import IP, UDP
import socket
import time
import logging
import os
import struct

# Constants based on the C++ code's definitions
# fmt: off
TYPE_MASK         = 0xF000000000000000
TYPE_OFFSET       = 60

# Pixel data constants
PIXEL_DCOL_MASK   = 0x0FE0000000000000
PIXEL_SPIX_MASK   = 0x001F800000000000
PIXEL_PIX_MASK    = 0x0000700000000000
PIXEL_TOA_MASK    = 0x00000FFFC0000000
PIXEL_TOT_MASK    = 0x000000003FF00000
PIXEL_FTOA_MASK   = 0x00000000000F0000
PIXEL_SPTIME_MASK = 0x000000000000FFFF
PIXEL_DCOL_OFFSET = 52
PIXEL_SPIX_OFFSET = 45
PIXEL_PIX_OFFSET  = 44
PIXEL_TOA_OFFSET  = 28
PIXEL_TOT_OFFSET  = 20
PIXEL_FTOA_OFFSET = 16

# TDC data constants
TDC_TYPE_MASK           = 0x0F00000000000000
TDC_TRIGGERCOUNTER_MASK = 0x00FFF00000000000
TDC_TIMESTAMP_MASK      = 0x00000FFFFFFFFE00
TDC_STAMP_MASK          = 0x00000000000001E0
TDC_TYPE_OFFSET         = 56
TDC_TRIGGERCOUNTER_OFFSET = 44
TDC_TIMESTAMP_OFFSET    = 9
TDC_STAMP_OFFSET        = 5
# fmt: on


def build_evr(
    counter,
    pulseTimeSeconds,
    pulseTimeNanoSeconds,
    prevPulseTimeSeconds,
    prevPulseTimeNanoSeconds,
):
    """
    Builds an EVR payload using the given parameters.

    Args:
        counter (int): The counter value.
        pulseTimeSeconds (int): The pulse time in seconds.
        pulseTimeNanoSeconds (int): The pulse time in nanoseconds.
        prevPulseTimeSeconds (int): The previous pulse time in seconds.
        prevPulseTimeNanoSeconds (int): The previous pulse time in nanoseconds.

    Returns:
        bytes: The EVR payload as a byte string.
    """
    payload = struct.pack(
        "<BBHIIIII",
        0x1,
        0,
        0,
        counter,
        pulseTimeSeconds,
        pulseTimeNanoSeconds,
        prevPulseTimeSeconds,
        prevPulseTimeNanoSeconds,
    )

    return payload

def send_packets(packets):
        """
        Sends a list of packets over UDP using standard Python sockets.

        Args:
            packets (list): The list of Scapy packets to be sent.
        """
        # Create a standard UDP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
        for packet in packets:
            # Extract payload to send. Here, we convert the entire Scapy packet to bytes.
            # If you only need the payload of a specific layer, adjust accordingly.
            payload = bytes(packet)
            
            # Extract destination IP and port from the Scapy packet
            dest_ip = packet[IP].dst
            dest_port = packet[UDP].dport
            
            # Send the payload
            sock.sendto(payload, (dest_ip, dest_port))

        # Close the socket
        sock.close()

class TPXConverter:
    """
    Converts a file to TPX format and start a thread to send out sends the
    packets over UDP.

    Args:
        ip_address (str): The destination IP address.
        port (int): The destination port number.
    """

    def __init__(self, ip_address, port):
        self.dtype = np.dtype(np.uint64)
        self.udp_max_size = 8952
        self.chunk_size = self.udp_max_size // self.dtype.itemsize
        self.evr_counter = 1
        self.dtype = np.dtype(np.uint64)
        self.udp_max_size = 8952
        self.chunk_size = self.udp_max_size // self.dtype.itemsize
        self.previous_time_ns = self.current_time_ns = time.time_ns()
        self.frequency_hz = 10
        self.period_ns = int(1 / self.frequency_hz * 1e9)
        self.packet_buffer = []
        self.send_process = None
        self.packet_time = time.time()
        self.UDP_HEADER = UDP(sport=4096, dport=port)
        self.IP_HEADER = IP(dst=ip_address)

    def convert_file(self, filename):
        """
        Converts the specified file to TPX format and sends the packets over UDP.

        Args:
            filename (str): The path to the file to be converted.
        """
        logger = logging.getLogger()

        # Reinitialize EVR time to prevent time drift for the FW
        self.previous_time_ns = self.current_time_ns = time.time_ns()
        self.packet_time = time.time()

        with open(filename, "rb") as f:
            logger.debug(f"Processing {filename} file")
            while True:
                chunk = np.frombuffer(
                    f.read(self.chunk_size * self.dtype.itemsize), dtype=self.dtype
                )
                if chunk.size == 0:
                    break  # End of file

                types = (chunk & TYPE_MASK) >> TYPE_OFFSET
                tdc_mask = types == 0x6
                tdc_data = chunk[tdc_mask]

                tpx_mask = (types == 0x6) | (types == 0xB)

                data = chunk[tpx_mask]

                packet = self.IP_HEADER / self.UDP_HEADER / data.tobytes()
                self.packet_time += 0.0005
                packet.time = self.packet_time
                self.packet_buffer.append(packet)

                if tdc_data.size > 0:
                    self.previous_time_ns = self.current_time_ns
                    self.current_time_ns += self.period_ns

                    pulseTimeSeconds = int(self.current_time_ns // 10**9)
                    pulseTimeNanoSeconds = int(self.current_time_ns % 10**9)
                    previousPulseTimeSeconds = int(self.previous_time_ns // 10**9)
                    previousPulseTimeNanoSeconds = int(self.previous_time_ns % 10**9)

                    evr_data = build_evr(
                        self.evr_counter,
                        pulseTimeSeconds,
                        pulseTimeNanoSeconds,
                        previousPulseTimeSeconds,
                        previousPulseTimeNanoSeconds,
                    )
                    extra_packet = self.IP_HEADER / self.UDP_HEADER / evr_data
                    self.packet_time += 0.0002  # 200 us
                    extra_packet.time = self.packet_time
                    self.packet_buffer.append(extra_packet)

                    self.evr_counter += 1

            # Create a new process and start sending packets
            if self.send_process and self.send_process.is_alive():
                self.send_process.join()
                logging.debug(f"Sending finished for {filename} file")

            logger.debug(
                f"Sending {len(self.packet_buffer)} packets for {filename} file"
            )
            self.send_process = multiprocessing.Process(
                target=send_packets,
                args=(self.packet_buffer,),
            )
            self.send_process.start()
            # Clear the packet buffer
            self.packet_buffer = []


def main(folder, ip, port, debug=False):
    """
    Converts files in a folder to a specific format using TPXConverter.

    Args:
        folder (str): The path to the folder containing the files to be converted.
        ip (str): The IP address of the TPXConverter.
        port (int): The port number of the TPXConverter.
        debug (bool, optional): Whether to enable debug logging. Defaults to False.
    """
    logging.basicConfig(
        level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s"
    )

    if debug:
        logging.getLogger().setLevel(logging.DEBUG)
    else:
        logging.getLogger().setLevel(logging.INFO)

    # Set logging to write to stdout
    # Open the folder
    files = os.listdir(folder)
    files = sorted(files)
    # Create list of file paths
    file_paths = [os.path.join(folder, file) for file in files]

    # Create an instance of the TPXConverter class
    converter = TPXConverter(ip, port)
    # Execute convert_file on every file in the folder

    for file in file_paths:
        print(f"Converting {file}")
        converter.convert_file(file)


if __name__ == "__main__":
    # Create an argument parser
    parser = argparse.ArgumentParser(description="TPX Converter")

    # Add an argument for the folder path
    parser.add_argument(
        "-f", "--folder", type=str, help="Path to the folder containing TPX files"
    )
    parser.add_argument("-d", "--debug", action="store_true", help="Turn on debug mode")
    parser.add_argument(
        "-p",
        "--port",
        type=int,
        help="Destination port for the UDP packages",
        default="9888",
    )
    # Add an argument for the folder path
    parser.add_argument(
        "-i",
        "--ip",
        type=str,
        help="Ip address of the destination",
        default="10.103.2.33",
    )

    # Parse the command line arguments
    args = parser.parse_args()

    main(args.folder, args.ip, args.port, args.debug)

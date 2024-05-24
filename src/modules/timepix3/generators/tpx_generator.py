import multiprocessing
import numpy as np
from scapy.all import Ether, IP, UDP, sendp
import time
import pandas as pd
import os
import struct
import argparse
import logging

# Constants based on the C++ code's definitions
TYPE_MASK = 0xF000000000000000
TYPE_OFFSET = 60

# Pixel data constants
PIXEL_DCOL_MASK = 0x0FE0000000000000
PIXEL_SPIX_MASK = 0x001F800000000000
PIXEL_PIX_MASK = 0x0000700000000000
PIXEL_TOA_MASK = 0x00000FFFC0000000
PIXEL_TOT_MASK = 0x000000003FF00000
PIXEL_FTOA_MASK = 0x00000000000F0000
PIXEL_SPTIME_MASK = 0x000000000000FFFF
PIXEL_DCOL_OFFSET = 52
PIXEL_SPIX_OFFSET = 45
PIXEL_PIX_OFFSET = 44
PIXEL_TOA_OFFSET = 28
PIXEL_TOT_OFFSET = 20
PIXEL_FTOA_OFFSET = 16

# TDC data constants
TDC_TYPE_MASK = 0x0F00000000000000
TDC_TRIGGERCOUNTER_MASK = 0x00FFF00000000000
TDC_TIMESTAMP_MASK = 0x00000FFFFFFFFE00
TDC_STAMP_MASK = 0x00000000000001E0
TDC_TYPE_OFFSET = 56
TDC_TRIGGERCOUNTER_OFFSET = 44
TDC_TIMESTAMP_OFFSET = 9
TDC_STAMP_OFFSET = 5


def build_evr(
    counter,
    pulseTimeSeconds,
    pulseTimeNanoSeconds,
    prevPulseTimeSeconds,
    prevPulseTimeNanoSeconds,
):

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


class TPXConverter:
    def __init__(self, ip_address):
        self.evr_counter = 1
        self.dtype = np.dtype(np.uint64)
        self.udp_max_size = 8952
        self.chunk_size = self.udp_max_size // self.dtype.itemsize
        self.previous_time_ns = self.current_time_ns = time.time_ns()
        self.frequency_hz = 10
        self.period_ns = int(1 / self.frequency_hz * 1e9)
        self.send_process = None
        self.packet_time = time.time()
        self.udp_dport = 9888
        self.udp_sport = 4096
        self.ip_address = ip_address

    def send_packets(self, packet_buffer, filename):
        logger = logging.getLogger(__name__)
        logger.info(f"Sending out {len(packet_buffer)} packets of {filename} file")
        #  for index, packet in enumerate(packet_buffer):
        #       if len(packet) == 66:
        #         evr_message = struct.unpack('<BBHIIIII', bytes(packet[UDP].payload))
        #         print(f"pkt index: {index}, counter: {evr_message[3]}, pulseTimeSeconds: {evr_message[4]}, pulseTimeNanoSeconds: {evr_message[5]}, prevPulseTimeSeconds: {evr_message[6]}, prevPulseTimeNanoSeconds: {evr_message[7]}")
        sendp(packet_buffer, iface="eth2", realtime=True, verbose=False)

    def tpx_to_packet(self, filepath):
        logger = logging.getLogger(__name__)
        logger.debug(f"Processing file: {filepath}")
        try:
            packet_buffer = []
            is_tdc_map = []
            with open(filepath, "rb") as f:
                while True:
                    packet_time = time.time()
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

                    packet = Ether() / IP(dst="10.103.2.33") / UDP(sport=4096, dport=9888) / data.tobytes()
                    packet_time += 0.00005
                    packet.time = packet_time
                    packet_buffer.append(packet)

                    is_tdc_map.append(tdc_data.size > 0)

        except Exception as e:
            logger.error(f"Error processing file {filepath}: {e}")
            return None, None
        
        return filepath, packet_buffer, is_tdc_map

    def insert_evr(self, packet_buffer, is_tdc_map):
        logger = logging.getLogger()
        logger.debug("Adding EVR to packets...")
        # packets are proccessed per tpx files, adjust the pusle time source
        # before each tpx file is processed
        self.previous_time_ns = self.current_time_ns = time.time_ns()
        evr_packets = []

        for index, is_tdc in enumerate(is_tdc_map):
            if is_tdc:
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
                extra_packet = Ether() / IP(dst=self.ip_address) / UDP(sport=self.udp_sport, dport=self.udp_dport) / evr_data
                extra_packet.time = packet_buffer[index].time + 0.00002
                evr_packets.append(extra_packet)

                self.evr_counter += 1
            else:
                evr_packets.append(None)

        # Convert your lists to pandas Series
        evr_data_series = pd.Series(evr_packets)
        packet_buffer_series = pd.Series(packet_buffer)

        # Drop the None values from the evr_data_series
        evr_data_series = evr_data_series.dropna()

        # Add a small offset to the index of the evr_data_series
        evr_data_series.index = evr_data_series.index + 0.5

        # Concatenate the two Series and sort by index
        merged_series = pd.concat([evr_data_series, packet_buffer_series]).sort_index()

        # Reset the index and convert the merged Series back to a list
        return merged_series.reset_index(drop=True).tolist()

def main(folder, process, ip, debug=False):
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
    
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
    sender = None

    # Create an instance of the TPXConverter class
    converter = TPXConverter(ip)
    # Execute convert_file on every file in the folder
    with multiprocessing.Pool(process) as p:
        logger = logging.getLogger(__name__)
        logger.debug(f"Processing files with {process} process ...")
        for filepath, packet_buffer, is_tdc_map in p.imap(converter.tpx_to_packet, file_paths):
            filename = os.path.basename(filepath)
            logger.debug(f"Adding EVR's to {len(packet_buffer)} packets from {filename}")
            evr_enriched = converter.insert_evr(packet_buffer, is_tdc_map)

            if sender and sender.is_alive():
                sender.join()

            sender = multiprocessing.Process(
                target=converter.send_packets, args=(evr_enriched, filename)
            )

            sender.start()

if __name__ == "__main__":
    # Create an argument parser
    parser = argparse.ArgumentParser(description="TPX Converter")

    # Add an argument for the folder path
    parser.add_argument(
        "-f", "--folder", type=str, help="Path to the folder containing TPX files"
    )
    parser.add_argument(
        "-d", "--debug", action="store_true", help="Turn on debug mode"
    )
    parser.add_argument(
        "-p",
        "--process",
        type=int,
        help="Number of processes to use for conversion",
        default="2",
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

    main(args.folder, args.process, args.ip, args.debug)
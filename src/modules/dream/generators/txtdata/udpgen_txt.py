
import argparse
import csv
import socket
import sys
import time

seqnum = [0,0]
sock = None


# Very specific readout generator for some data collected at SOurce Testing Facility
# in Lund. Data is ascii (tab separated) with cathode, anode, sector and sumo
# this generator tries to remap these value into ring, fen and uid which goes into
# the DREAM readout data
# for each readout a ESS Readout UDP kacket is sent.

# Version 1 header constructed according to Readout ICD
def makeheader(length, output_queue, seqno) -> []:
    header =  [0 for x in range(32)]
    header[1] = 1
    header[2] = ord('E')
    header[3] = ord('S')
    header[4] = ord('S')
    header[5] = 96 # DREAM
    header[6] = length % 256
    header[7] = length//256
    header[8] = output_queue
    header[26] = (seqno >>  0) & 0xff
    header[27] = (seqno >>  8) & 0xff
    header[28] = (seqno >> 16) & 0xff
    header[29] = (seqno >> 24)
    return header


# map readout (mainly sector) to Fiber value
def readout2fiber(readout: {}) -> int:
    sector = readout['sector']
    if sector <= 5:
        return 0 # 1 would also work
    elif sector <= 11:
        return 2 # 3 would also work
    else:
        return 255


# map readout (sector and sumo) to FEN
def readout2fen(readout : {}) -> int:
    sector = readout['sector']
    sumo = readout['sumo']
    fen = sector * 2  # works for ring 0, else we need to subtract an offset

    if sumo in [4, 3]: # sumos 6/5 are followed by 4/3 on the ring
        fen += 1

    if sector >= 6: # when we get into ring 1 fens srart at 0 again
        fen -= 12
    return fen


# Map sector to output queue. Not sure why I added this as it only makes the generator
# more complicated.
def sector2oq(sector: int) -> int:
    if sector <=5:
        return 0
    elif sector <= 10:
        return 1
    else:
        print('invalid sector')
        return 255


def send_packet(data):
    global sock
    UDP_IP = "127.0.0.1"
    UDP_PORT = 9000
    bdata = bytearray(data)
    sock.sendto(bdata, (UDP_IP, UDP_PORT))


def add_readout(readout : {}) -> None:
    global seqnum
    data = [0 for z in range(16)]
    fiber = readout2fiber(readout)
    fen = readout2fen(readout)
    data[0] = fiber
    data[1] = fen
    data[2] = 16 # length
    data[3] = 0  # length
    data[12] = 0  # OM
    data[13] = readout['sumo']
    data[14] = readout['cathode']
    data[15] = readout['anode']

    oq = sector2oq(readout['sector'])
    header = makeheader(48, oq, seqnum[oq])
    send_packet(header + data)
    seqnum[oq] += 1


def main(args) -> {}:
    count = 0
    global sock
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    with open(args.file) as csvfile:
        reader = csv.reader(csvfile, delimiter='\t')
        for row in reader:
            res = {
                "cathode" : int(row[0]),
                "anode" : int(row[1]),
                "sector" : int(row[2]),
                "sumo" : int(row[3])
            }
            # if res['sumo'] != 3:
            #     continue
            add_readout(res)
            count += 1
            if count == 10000:
                time.sleep(1)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--file", help = "csv text file (CDT specific)", type = str)
    args = parser.parse_args()

    main(args)
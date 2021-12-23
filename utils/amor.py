
# Utility to visualise the digital geometry for AMOR
#
# This script is nonessential and considered a 'helper' function
# it should reflect the AMOR geometry but not be used as a ground truth

from graphics import *
import sys, argparse

MaxX = 63
MaxY = 14 * 32 -1


# From ICD and instrument configuration
def getcassette(ring, fen, hybrid):
    cass = [1, 5, 7, 9, 11, 13]
    return cass[ring] + (fen - 1) *2 + hybrid


# From ICD
def iswire(vmm):
    return (vmm == 1)

def isstrip(vmm):
    return (vmm == 0)

def wire(channel):
    return channel - 15

def strip(channel):
    return 64 - channel

def xcoord(channel):
    return strip(channel) - 1

def ycoord(cassette, channel):
    y = (cassette - 1) * 32 + 32 - wire(channel)
    return y

# Drawing helpers

# convert ESS Logical Geometry to geometry.py coordinates
# ESS has y positive downward which is opposite to geometry.py
def ly(oldy):
    return MaxY - oldy


def draw_cassette(win, cassette, colour):
    y = (cassette - 1) * 32
    y0 = ly(y)
    y1 = ly(y + 31)
    cass = Rectangle(Point(0, y0), Point(MaxX, y1))
    cass.setFill(colour)
    cass.draw(win)
    text = Text(Point(32, (y0 + y1)/2), str(cassette))
    text.setOutline("white")
    text.draw(win)


def drawcass(win, ring, fen, vmm):
    c = getcassette(ring, fen, vmm >> 1)
    for cas in range(1, 15, 1):
        color = "grey"
        if cas == c:
            color = "blue"
        draw_cassette(win, cas, color)


def drawhit(win, ring, fen, vmm, channel):
    cassette = getcassette(ring, fen, vmm >> 1)
    if isstrip(vmm & 0x1):
        line = Line(Point(xcoord(channel), 0), Point(xcoord(channel), MaxY))
    else:
        line = Line(Point(0, ly(ycoord(cassette, channel))), Point(MaxX, ly(ycoord(cassette, channel))))
    line.setOutline("yellow")
    line.setWidth(2)
    line.draw(win)

#
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--ring", metavar='ring', help = "ring (0 - 11)", type = int)
    parser.add_argument("--fen", metavar='fen', help = "1 or 2 on Ring 0, 1 on all others", type = int)
    parser.add_argument("--vmm1", metavar='vmm1', help = "0, 1, 2, 3", type = int)
    parser.add_argument("--ch1", metavar='ch1', help = "0 - 63", type = int)
    parser.add_argument("--vmm2", metavar='vmm2', help = "0, 1, 2, 3", type = int, default = 1000)
    parser.add_argument("--ch2", metavar='ch2', help = "0 - 63", type = int, default = 1000)
    args = parser.parse_args()

    ring = args.ring
    fen = args.fen

    secondch = (args.vmm2 != 1000) and args.ch2 != 1000

    if secondch:
        if (args.vmm1 >> 1) != (args.vmm2 >> 1):
            print("error: different hybrids!")
            sys.exit(1)

    print("ring {}, fen {}, hybrid {}, vmm {} ch {}".format(ring, fen, args.vmm1 >> 1, args.vmm1 & 0x1, args.ch1))
    win = GraphWin(width = 400, height = 800) # create a window
    win.setCoords(0, 0, MaxX, MaxY) # set the coordinates (0,0) bottom left!

    drawcass(win, ring, fen, args.vmm1)
    drawhit(win, ring, fen, args.vmm1, args.ch1)
    if secondch:
        print("                         vmm {} ch {}".format(args.vmm2 >> 1, args.vmm2 & 0x1, args.ch2))
        drawhit(win, ring, fen, args.vmm2, args.ch2)

    win.getMouse() # pause before closing

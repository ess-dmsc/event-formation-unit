#!/usr/bin/python

# Copyright (C) 2016, 2017 European Spallation Source ERIC

from __future__ import print_function, absolute_import

# Initial geometry guess
def asch2xy(asic, channel):
    x = channel % 4
    y = channel / 4

    if asic == 1:
        x += 4
    if (asic == 2):
        y += 4
    if (asic == 3):
        x += 4
        y += 4
    print("asic: %2d, channel: %2d -> (%2d, %2d)" % (asic, channel, x,y))
    return [x,y]

# New geometry after time in Utgård
def asch2xynew(asic, channel):
    x = channel % 4
    y = channel / 4
    if asic == 0:
        x = 7 - x
    if (asic == 3):
        y = y + 4
    if (asic == 2):
        x = 7 - x
        y = y + 4
    print("asic: %2d, channel: %2d -> (%2d, %2d)" % (asic, channel, x,y))
    return [x,y]


asch2xy(0,0)
asch2xy(0,15)
asch2xy(1,0)
asch2xy(1,15)
asch2xy(2,0)
asch2xy(2,15)
asch2xy(3,0)
asch2xy(3,15)

print("")

asch2xynew(0,0)
asch2xynew(0,15)
asch2xynew(1,0)
asch2xynew(1,15)
asch2xynew(2,0)
asch2xynew(2,15)
asch2xynew(3,0)
asch2xynew(3,15)

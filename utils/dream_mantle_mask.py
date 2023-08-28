# Copyright (C) 2023 European Spallation Source ERIC, see LICENSE file

# pixel value generator based on masks for Mantle detector
# formulaes derived from the DREAM ICD

debug = False

allwires = list(range(32))
allstrips = list(range(256))
pixel_offset = 229376

#

# ICD 2 draft 1 section 3.2.3
def mantle_pixel(mu, cas, ctr, wir, strp):
    x = strp
    y = 60 * wir + 12 * mu + 2 * cas + ctr
    return 256*y + x + 1 + pixel_offset


# If the current mounting unit, cassette, ... is in its mask set() then
# we deem this a candidate for masking
def filter(msk_mu, msk_cas, msk_ctr, msk_wir, msk_str,
               mu,     cas,     ctr,     wir,     strp, pixel):
    if mu in msk_mu and cas in msk_cas and ctr in msk_ctr and wir in msk_wir and strp in msk_str:
        return True
    return False


# each parameter is a list of mask values
def mantle(mounting_unit, cassette, counter,  wire, strip):
    msk_mu = set(mounting_unit)
    msk_cas = set(cassette)
    msk_ctr = set(counter)
    msk_wir = set(wire)
    msk_str = set(strip)

    if debug:
        print('mnt. unit cassette counter wire strip pixel')

    i = pixel_offset
    mask = []
    for wir in range(32): # number of nwires
        for mu in range(5): # number of mounting units
            for cas in range(6): # number of cassettes
                for ctr in range(2): #number of counters
                    for strp in range(256): #number of strips
                        i += 1
                        pixel = mantle_pixel(mu, cas, ctr, wir, strp)
                        assert pixel == i
                        if filter(msk_mu, msk_cas, msk_ctr, msk_wir, msk_str, mu, cas, ctr, wir, strp, pixel):
                            if debug:
                                print(f'{mu:9}{cas:9}{ctr:8}{wir:5}{strp:6}{pixel:7}')
                            mask.append(pixel)
    return mask


def test(mu, cas, ctr, w, s):
    print(f'M.U. {mu}, cassette {cas} counter {ctr}\nstrips {s}, wires {w}\n{"-"*60}')
    res = mantle(mu, cas, ctr, w, s)
    print(f'pixels in mask {len(res)}')
    print(res[:5], '...', res[-5:])

debug = True
test([0], [0], [0], [0]     , allstrips)
test([0], [0], [0], allwires, [3])
test([4], [5], [1], [31]    , allstrips)

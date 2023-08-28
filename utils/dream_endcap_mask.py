# Copyright (C) 2023 European Spallation Source ERIC, see LICENSE file

# pixel value generator based on masks for forwardandbackward endcaps

debug = False

ncass = {6:10, 5:8, 4:6, 3:4}
sumo_offsets = {6:0, 5:20, 4:36, 3:48}
allwires = list(range(16))
allstrips = list(range(16))

#

# The logical geometry between the two endcaps is different due to the
# different numbers of sectors. Also pixels are globally unique so
# different detector elements have different pixel offsets
def endcap_pixel(sec, sum, cas, ctr, wire, strip, width, pixel_offset):
    x = 56 * sec + sumo_offsets[sum] + 2 * cas + ctr
    y = 16 * strip + 15 - wire
    return width*y + x + 1 + pixel_offset


# If the current sector, sumo, cassette, ... is in its mask set() then
# we deem this a candidate for masking
def filter(msk_sec, msk_sum, msk_cas, msk_ctr, msk_wir, msk_str,
               sec,     sum,     cas,     ctr,     wir, strp, pixel):
    if sec in msk_sec and sum in msk_sum and cas in msk_cas and ctr in msk_ctr and wir in msk_wir and strp in msk_str:
        return True
    return False


# each parameter is a list of mask values
def endcap(sector, sumo, cassette, counter,  wire, strip, nsectors, width, pixel_offset):
    msk_sec = set(sector)
    msk_sum = set(sumo)
    msk_cas = set(cassette)
    msk_ctr = set(counter)
    msk_wir = set(wire)
    msk_str = set(strip)

    if debug:
        print('sector sumo cassette counter wire strip pixel')

    i = pixel_offset
    mask = []
    for strp in range(16):
        for wir in range(15, -1, -1):
            for sec in range(nsectors):
                for sum in range(6, 2, -1):
                    for cas in range(ncass[sum]):
                        for ctr in range(2):
                            i += 1
                            pixel = endcap_pixel(sec, sum, cas, ctr, wir, strp, width, pixel_offset)
                            assert pixel == i
                            if filter(msk_sec, msk_sum, msk_cas, msk_ctr, msk_wir, msk_str, sec, sum, cas, ctr, wir, strp, pixel):
                                if debug:
                                    print(f'{sec:6}{sum:5}{cas:8}{ctr:8}{wir:5}{strp:6}{pixel:7}')
                                mask.append(pixel)
    return mask


def test(det, sec, sum, cas, ctr, w, s):
    print(f'{"-"*60}\n{det}\nsector {sec}, sumo {sum}, cassette {cas} counter {ctr}\nstrips {s}, wires {w}\n{"-"*60}')
    if det == 'fwendcap':
        res = endcap(sec, sum, cas, ctr, w, s, 5, 280, 0)
    elif det == 'bwendcap':
        res = endcap(sec, sum, cas, ctr, w, s, 11, 616, 71680)
    else:
        assert 1==0
    print(f'pixels in mask {len(res)}')
    print(res[:5], '...', res[-5:])

debug = True
test('bwendcap', [0], [6], [4], [1], allwires, [5])
test('fwendcap', [0], [6], [0], [0], allwires, [0])
test('fwendcap', [0], [6], [0], [0], allwires, [0,1,2])

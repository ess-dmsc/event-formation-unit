# Copyright (C) 2023 European Spallation Source ERIC, see LICENSE file

# pixel value generator based on masks for Mantle detector
# formulaes derived from the DREAM ICD

debug = False

allwires = list(range(16))
allstrips = list(range(32))
hr_pixel_offset = 1122304
sans_pixel_offset = 720896

hr_offsets = {          0:(32, 0),  1:(48, 0),  2:(64, 0),
            3:(16,16),  4:(32,16),  5:(48,16),  6:(64,16),  7:(80,16),
 8:(0,32),  9:(16,32), 10:(32,32), 11:(48,32), 12:(64,32), 13:(80,32), 14:(96,32),
15:(0,48), 16:(16,48), 17:(32,48),             18:(64,48), 19:(80,48), 20:(96,48),
21:(0,64), 22:(16,64), 23:(32,64),             24:(64,64), 25:(80,64), 26:(96,64),
           27:(16,80), 28:(32,80),             29:(64,80), 30:(80,80),
                       31:(32,96),             32:(64,96)}

hr_rot = [0,0,1,0,0,0,1,1,0,0,0,0,1,1,1,3,3,3,1,1,1,3,3,3,2,2,2,3,3,2,2,3,2]
assert len(hr_rot) == len(hr_offsets)

sans_offsets = {        0:(32, 0),  1:(48, 0),  2:(64, 0),
            3:(16,16),  4:(32,16),  5:(48,16),  6:(64,16),  7:(80,16),
 8:(0,32),  9:(16,32), 10:(32,32), 11:(48,32), 12:(64,32), 13:(80,32), 14:(96,32),
15:(0,48), 16:(16,48), 17:(32,48),             18:(64,48), 19:(80,48), 20:(96,48),
21:(0,64), 22:(16,64), 23:(32,64), 24:(48,64), 25:(64,64), 26:(80,64), 27:(96,64),
           28:(16,80), 29:(32,80), 30:(48,80), 31:(64,80), 32:(80,80),
                       33:(32,96), 34:(48,96), 35:(64,96)}

sans_rot = [0,0,1,0,0,0,1,1,0,0,0,0,1,1,1,3,3,3,1,1,1,3,3,3,2,2,2,2,3,3,2,2,2,3,2,2]
assert len(sans_rot) == len(sans_offsets)

detector = {'hr':   (hr_pixel_offset, hr_offsets, hr_rot),
            'sans': (sans_pixel_offset, sans_offsets, sans_rot)}
#
#

def rotate(x,y, r):
    assert r in [0,1,2,3]
    if r == 0:
        return x,y
    elif r == 1:
        return 15 - y, x
    elif r == 2:
        return 15 - x, 15 - y
    else:
        return y, 15 - x



# ICD 2 draft 1 section 3.4.2
def cuboid_pixel(cub, cas, ctr, wir, strp, offsets, rotations, pixel_offset):
    glb_x_offset, grid_y_offset = offsets[cub]
    glb_y_offset = 112 * strp
    loc_x, loc_y = rotate(2 * cas + ctr, 15 - wir, rotations[cub])

    x = glb_x_offset + loc_x
    y = glb_y_offset + grid_y_offset + loc_y
    return 112 * y + x + 1 + pixel_offset


# If the current mounting unit, cassette, ... is in its mask set() then
# we deem this a candidate for masking
def filter(msk_cub, msk_cas, msk_ctr, msk_wir, msk_str,
               cub,     cas,     ctr,     wir,     strp, pixel):
    if cub in msk_cub and cas in msk_cas and ctr in msk_ctr and wir in msk_wir and strp in msk_str:
        return True
    return False


# each parameter is a list of mask values
def cuboid(det, cuboid, cassette, counter,  wire, strip):
    msk_cub = set(cuboid)
    msk_cas = set(cassette)
    msk_ctr = set(counter)
    msk_wir = set(wire)
    msk_str = set(strip)

    pixel_offset, offsets, rotations = detector[det]

    if debug:
        print('cuboid cassette counter wire strip    pixel')

    i = pixel_offset
    mask = []
    for strp in range(32): # number of strips
        for cub in range(33): # number of cuboids
            for cas in range(8): # number of cassettes
                for ctr in range(2): #number of counters
                    for wir in range(16): #number of strips
                        i += 1
                        pixel = cuboid_pixel(cub, cas, ctr, wir, strp, offsets, rotations, pixel_offset)
                        #assert pixel == i
                        if filter(msk_cub, msk_cas, msk_ctr, msk_wir, msk_str, cub, cas, ctr, wir, strp, pixel):
                            if debug:
                                print(f'{cub:6}{cas:9}{ctr:8}{wir:5}{strp:6}{pixel:9}')
                            mask.append(pixel)
    return mask


def test(det, cub, cas, ctr, w, s):
    assert det in ['hr', 'sans']
    print(f'cuboid {cub}, cassette {cas} counter {ctr}\nstrips {s}, wires {w}\n{"-"*60}')
    res = cuboid(det, cub, cas, ctr, w, s)
    print(f'pixels in mask {len(res)}')
    print(res[:5], '...', res[-5:])

debug = True
test('hr', [0], [0], [0], [0]     , allstrips)
test('hr', [24], [0], [0], allwires, [0])
test('sans', [24], [0], [0], allwires, [0])

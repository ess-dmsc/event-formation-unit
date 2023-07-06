import json, argparse, datetime

# Script's main purpose is to
# 1) Convert the old calibration file format for loki into the new one
# 2) Transform the old calibration parameters to fit the new scheme

# Helper function to print the polynomials
def printpoly(poly, done):
    polystr = '['
    for index, val in enumerate(poly):
        polystr += f'{val:.7f}'
        if index != len(poly) -1:
            polystr += ', '

    if done:
        polystr += ']]'
    else:
        polystr += '],'
    print(f'            {polystr}')


# Main conversion routine
# 1) reads the old calibration file as json
# 2) perform some sanity checks
# 3) print the new json which can then be redirected to a file
def convert(args):
    f = open(args.f)
    data = json.load(f)

    calib = data['LokiCalibration']

    res = calib['resolution']
    ntubes = calib['ntubes']
    nstraws = calib['nstraws']
    polys = calib['polynomials']
    assert res == args.r
    assert nstraws == 7

    entries = len(polys)
    assert entries == nstraws * ntubes
    ngroups = entries // 7

    info = f'convertloki.py: {args.f} autoconverted {entries} polynomials into {ngroups} groups'
    now = datetime.datetime.now()

    print(f'{{')
    print(f'  "Calibration" : {{')
    print(f'    "version" : 0,')
    print(f'    "date" : "{now.isoformat()}",')
    print(f'    "info" : "{info}",')
    print('')
    print(f'    "instrument" : "loki",')
    print(f'    "groups" : {ngroups},')
    print(f'    "groupsize" : {nstraws},')
    print('')
    print(f'    "Parameters" : [')
    print(f'      {{')

    for group in range(ngroups):
        print(f'        "groupindex" : {group}, ')
        print(f'        "intervals" : [[  0.0,0.143], [0.144,0.286], [0.287,0.429], [ 0.43,0.571], [0.572,0.714], [0.715,0.857], [0.858,  1.0]],')
        print(f'        "polynomials" : [')

        for straw in range(nstraws):
            index = group * nstraws + straw
            poly = polys[index]
            assert index == poly[0]

            oldpoly = poly[1:]
            assert len(oldpoly) == 4

            # this is the key feature discussed in the Charge Calibration document
            # (a,b,c,d) -> (a/N, b, cN, ,dN^2)
            newpoly = [1.0*oldpoly[i]*(res**(i-1)) for i in range(len(oldpoly))]

            if straw == nstraws - 1:
                printpoly(newpoly, True)
            else:
                printpoly(newpoly, False)

        if group == ngroups - 1:
            print(f'      }}')
        else:
            print(f'      }},{{')

    print(f'    ]')
    print(f'  }}')
    print(f'}}')
    f.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", help="loki calibration file (old format)",
        type=str, default="")
    parser.add_argument("-r", help="resolution (along straw)",
        type=int, default=512)
    args = parser.parse_args()

    convert(args)

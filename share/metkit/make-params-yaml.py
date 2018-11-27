#!/usr/bin/env python3

import subprocess
import os
import yaml

dates = "20181101/to/20181130"
dates = "20181101"

target = "streams.list"
if not os.path.exists(target):
    with open('tmp', "w") as f:
        print(""" list,
       stream=all,
       type=all,
       output=tree,
       date=%s,
       target=%s,
       hide=cubes/count/year/month/time/date/refdate/refdatemonth/refdateyear/branch/origin/system/method/expver/class/product/section/number/obsgroup""" % (dates, target,),
              file=f)

    subprocess.call(["mars", "tmp"])

reqs = []
with open(target) as f:
    for n in set(n.strip() for n in f):
        r = dict(x.split('=') for x in n.split(','))
        reqs.append(r)

P = {}

for req in reqs:
    type = req['type']
    levtype = req.get('levtype', 'sfc')
    stream = req['stream']
    target = "%s-%s-%s.list" % (type, levtype, stream)
    if not os.path.exists(target):
        with open('tmp', "w") as f:
            r = {}
            r['stream'] = stream
            r['type'] = type
            r['levtype'] = levtype
            r['time'] = "all"
            r['date'] = dates
            r['hide'] = """channel/ident/instrument/branch/
                           frequency/direction/method/system/interval/
                           origin/quantile/domain/number/
                           fcmonth/type/class/expver/stream/
                           hdate/levtype/month/year/obsgroup/
                           date/levelist/time/step/anoffset/reportype/subtype/
                           files/missing/offset/length/fcperiod/product/
                           stattotalcount/stattotallength/stattotallines/
                           statcount/statdate/statlength/statsubtype/stattime/
                           iteration/grid/section/
                           grand-total/cost/file/id/refdate/
                           refdatemonth/refdateyear"""
            r['target'] = target
            rr = 'list,' + ",".join("\n%s=%s" % (a, b) for a, b in r.items())
            print(rr, file=f)

        subprocess.call(["mars", "tmp"])

    params = set()
    print(target)
    with open(target) as f:
        for n in f:
            n = n.strip()
            if n == '':
                continue
            if n == 'param':
                continue
            if n.startswith('param = '):
                n = n.split(' ')[-1]
            m = n.split('.')
            if len(m) == 2:
                p, t = int(m[0]), int(m[1])
                if t == 128:
                    t = 0
                m = t * 1000 + p
            elif len(m) == 1:
                m = int(m[0])
            else:
                print(m[0])
                exit(1)
            params.add(m)

    for table in (0, 129000, 171000, 200000):
        if table + 129 in params:
            params.add(table + 156)

        if table + 138 in params and table + 155 in params:
            params.add(table + 131)
            params.add(table + 132)

    params = sorted(params)
    if params:
        levtype = req.get('levtype', '')
        P[(stream, type, levtype)] = params
        print(params)
    else:
        print("No params for stream=%s, type=%s, levtype=%s" % (stream, type, levtype))


def add(where, *params):
    global P
    P[where] = sorted(set(P[where]) | set(params))


"""
Add WP
"""
P[('oper', 'wp', 'sfc')] = P[('oper', 'fc', 'sfc')]
P[('oper', 'wp', 'pl')] = P[('oper', 'fc', 'pl')]

P[('enfo', 'wp', 'sfc')] = P[('enfo', 'cf', 'sfc')]
P[('enfo', 'wp', 'pl')] = P[('enfo', 'cf', 'pl')]

# Z (orog) on ML is not archived in MARS
add(('oper', 'fc', 'ml'), 129)
add(('scda', 'fc', 'ml'), 129)
add(('lwda', 'fc', 'ml'), 129)

# Tropfical cyclones

P[('enfo', 'tf', '')] = (129, 999)
P[('oper', 'tf', '')] = (129, 999)

# ML for enfo is not in mars
P[('enfo', 'cf', 'ml')] = sorted(set(P[('oper', 'fc', 'ml')]) | set(P[('oper', 'an', 'ml')]))
P[('enfo', 'pf', 'ml')] = P[('enfo', 'cf', 'ml')]


# fsr = 244

add(('enfo', 'cf', 'sfc'), 244)
add(('enfo', 'pf', 'sfc'), 244)

# SCDA pv, pt not in MARS
P[('scda', 'an', 'pt')] = P[('oper', 'an', 'pt')]
P[('scda', 'an', 'pv')] = P[('oper', 'an', 'pv')]
P[('scda', 'fc', 'pt')] = P[('oper', 'fc', 'pt')]
P[('scda', 'fc', 'pv')] = P[('oper', 'fc', 'pv')]

# VPOT=2 and STRF=1 not in mars
add(('enfh', 'cf', 'pl'), 1, 2)
P[('enfh', 'pf', 'pl')] = P[('enfh', 'cf', 'pl')]

# ssro=9, sro=8, ro=205, tisr=212
add(('msmm', 'em', 'sfc'), 8, 9, 205, 212)
add(('msmm', 'fcmean', 'sfc'), 8, 9, 205, 212)

# Still in FR/FRA/req/curr/FX
P[('enfo', 'fp', 'sfc')] = sorted([131165, 131228])
P[('enfo', 'fp', 'pl')] = sorted([131130])
P[('waef', 'fp', '')] = sorted([131229, 131232])

P[('enfo', 'cs', 'pl')] = sorted([129])


# PRES = 54
add(('enfo', 'pf', 'pv'), 54)


add(('enfo', 'pf', 'pv'), 129)  # Z
add(('enfo', 'cm', 'pl'), 130)  # T
add(('enfo', 'cs', 'pl'), 130)  # T


Y = []
for k, v in sorted(P.items()):
    if k[2]:
        Y.append([dict(stream=k[0], type=k[1], levtype=k[2]), v])
    else:
        Y.append([dict(stream=k[0], type=k[1]), v])

with open("params.yaml", "w") as f:
    f.write('# File automatically generated by %s\n# Do not edit\n\n' % (os.path.basename(__file__)))
    f.write(yaml.safe_dump(Y, default_flow_style=False))

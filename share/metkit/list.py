#!/usr/bin/env python3

import subprocess
import os
import yaml

dates = "20181101/to/20181130"
dates = "20181101"

target = "streams.list"
if not os.path.exists(target):
    with open("tmp", "w") as f:
        print(
            """ list, 
       stream=all, 
       type=all, 
       output=tree, 
       date=%s,
       target=%s, 
       hide=cubes/count/year/month/time/date/refdate/refdatemonth/refdateyear/branch/origin/system/method/expver/class/product/section/number/obsgroup"""
            % (
                dates,
                target,
            ),
            file=f,
        )

    subprocess.call(["mars", "tmp"])

reqs = []
with open(target) as f:
    for n in set(n.strip() for n in f):
        r = dict(x.split("=") for x in n.split(","))
        reqs.append(r)

P = {}

for req in reqs:
    type = req["type"]
    levtype = req.get("levtype", "sfc")
    stream = req["stream"]
    target = "%s-%s-%s.list" % (type, levtype, stream)
    if not os.path.exists(target):
        with open("tmp", "w") as f:
            r = {}
            r["stream"] = stream
            r["type"] = type
            r["levtype"] = levtype
            r["time"] = "all"
            r["date"] = dates
            r[
                "hide"
            ] = """channel/ident/instrument/branch/
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
            r["target"] = target
            rr = "list," + ",".join("\n%s=%s" % (a, b) for a, b in r.items())
            print(rr, file=f)

        subprocess.call(["mars", "tmp"])

    params = set()
    print(target)
    with open(target) as f:
        for n in f:
            n = n.strip()
            if n == "":
                continue
            if n == "param":
                continue
            if n.startswith("param = "):
                n = n.split(" ")[-1]
            m = n.split(".")
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
        levtype = req.get("levtype", "")
        P[(stream, type, levtype)] = params
    else:
        print("No params for stream=%s, type=%s, levtype=%s" % (stream, type, levtype))

Q = {}
for k, v in sorted(P.items()):
    v = tuple(v)
    Q.setdefault(v, [])
    Q[v].append(k)

Y = []


def merge(kinds):
    streams = set()
    types = set()
    levtypes = set()

    for k in sorted(kinds):
        streams.add(k[0])
        types.add(k[1])
        levtypes.add(k[2])

    streams = sorted(streams)
    types = sorted(types)
    levtypes = sorted(levtypes)

    d = {}
    if len(streams) == 1:
        d["stream"] = streams[0]
    else:
        d["stream"] = streams

    if len(types) == 1:
        d["type"] = types[0]
    else:
        d["type"] = types

    if len(levtypes) == 1:
        d["levtype"] = levtypes[0]
    else:
        d["levtype"] = levtypes

    if d["levtype"] == "":
        del d["levtype"]

    return d


for v, k in sorted(Q.items()):
    k = merge(k)
    Y.append([k, v])

with open("params.yaml", "w") as f:
    f.write(
        "# File automatically generated by %s\n# Do not edit\n\n"
        % (os.path.basename(__file__))
    )
    f.write(yaml.safe_dump(Y, default_flow_style=False))

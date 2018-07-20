#!/usr/bin/env python3
import yaml
import re
import os
import itertools
import subprocess

PARAMS = {}
if os.path.exists("params.yaml"):
    with open("params.yaml") as f:
        PARAMS = yaml.load(f.read())

OUT = []
for entry in PARAMS:
    when, parms = entry

    if 'file' in when:
        continue

    if not when:
        continue

    for k, v in list(when.items()):
        if not isinstance(v, list):
            when[k] = [v]

    with open('tmp', "w") as f:

        names = list(when.keys())
        values = list(when.values())

        print(when)

        for n in itertools.product(*values):
            r = dict((a, b) for a, b in zip(names, n))
            if r.get('type') == 'wp':
                continue

            print(r)

            for levtype in ('sfc', 'pl', 'ml', 'pt', 'pv'):
                r['levtype'] = levtype
                r['param'] = 'all'
                r['step'] = "0/12/0-168"
                r['time'] = "0/12"
                r['levelist'] = "500/1/10/2000/300/850"
                r['date'] = "20180701/to/20180707"
                r['expect'] = 'any'
                r['target'] = 'data'
                r['number'] = '1'

                rr = 'retrieve,' + ",".join("%s=%s" % (a, b) for a, b in r.items())
                print(rr, file=f)

    subprocess.call(["mars", "tmp"])
    if os.path.getsize('data') == 0:
        exit(1)
    out = subprocess.check_output(["grib_ls", "-p", "paramId", "data"]).decode().split('\n')
    out = [x.strip() for x in out]
    params = set()
    for p in out:
        try:
            params.add(int(p))
        except:
            pass
    if 155 in params and 138 in params:
        params.add(131)
        params.add(132)
    params = sorted(params)
    print(params)

    OUT.append([when, params])

    with open("paramids2.yaml", "w") as f:
        f.write(yaml.safe_dump(OUT, default_flow_style=False))


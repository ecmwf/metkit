#!/usr/bin/env python3
import yaml
import re
import os
import json
import itertools
import subprocess

target = "mars.list"

PARAMS = {}
if os.path.exists("params.yaml"):
    with open("params.yaml") as f:
        PARAMS = yaml.load(f.read())

OUT = []
for entry in PARAMS:
    when, parms = entry
    orig = dict(**when)

    if 'file' in when:
        OUT.append([orig, parms])
        continue

    if not when:
        continue

    done = "%s.done" % (json.dumps(when,sort_keys=True),)
    if os.path.exists(done):
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
            if r.get('type') in ('wp', 'cv', 'tf' ):
                continue

            if r.get('stream') == 'efov':
                continue

            print(r)

            for levtype in ('sfc', 'pl', 'ml', 'pt', 'pv'):
                r['levtype'] = levtype
                r['time'] = "0/12"
                r['date'] = "20180701/to/20180707"
                r['hide'] = 'channel/ident/instrument/branch/frequency/direction/method/system/origin/quantile/domain/number/fcmonth/type/class/expver/stream/hdate/levtype/month/year/date/levelist/time/step/files/missing/offset/length/grand-total/cost/file/id'
                r['target'] = '"%s"' % (target,)

                rr = 'list,' + ",".join("%s=%s" % (a, b) for a, b in r.items())
                print(rr, file=f)

    subprocess.call(["mars", "tmp"])

    params = set()
    with open(target) as f:
        lines = [x.strip() for x in f.readlines()]
        for n in lines:
            if n == '':
               continue
            if n == 'param':
               continue
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

    if 155 in params and 138 in params:
        params.add(131)
        params.add(132)

    if 129 in params:
        params.add(156)

    params = sorted(params)
    print(params)

    if not params:
        params = parms

    OUT.append([orig, params])

    with open(done, "w") as f:
        f.write(yaml.safe_dump([orig, params], default_flow_style=False))

with open("params2.yaml", "w") as f:
	f.write(yaml.safe_dump(OUT, default_flow_style=False))

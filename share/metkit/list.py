import subprocess
import os

dates = "20181101/to/20181130"
dates = "20181101"

target="streams.list"
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

O = []

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
                           frequency/direction/method/system/
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

    params = sorted(params)
    O.append([dict(stream=stream,type=type,levtype=levtype), params])

f = open("list.yaml", "w")
f.write(yaml.safe_dump(O, default_flow_style=False))

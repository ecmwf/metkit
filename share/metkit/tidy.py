import yaml
import sys


P = {}
with open(sys.argv[1]) as f:
   for e in yaml.safe_load(f):
       rule, params = tuple(e)
       streams = rule['stream'] 
       types = rule['type'] 
       levtypes = rule.get('levtype', '')
       if not isinstance(streams, list):
           streams = [streams]
       if not isinstance(types, list):
           types = [types]
       if not isinstance(levtypes, list):
           levtypes = [levtypes]
       for s in streams:
           for t in types:
               for l in levtypes:
                   key = (s, t, l)
                   P[key] = tuple(sorted(params))

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
        d['stream'] = streams[0]
    else:
        d['stream'] = streams

    if len(types) == 1:
        d['type'] = types[0]
    else:
        d['type'] = types

    if len(levtypes) == 1:
        d['levtype'] = levtypes[0]
    else:
        d['levtype'] = levtypes

    if d['levtype'] == '':
        del d['levtype']

    return d


for v, k in sorted(Q.items()):
    k = merge(k)
    Y.append([k, v])

print(yaml.safe_dump(Y, default_flow_style=False))

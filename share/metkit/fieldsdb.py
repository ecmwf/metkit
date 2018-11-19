from sqlalchemy import create_engine, text
import yaml

db = create_engine("postgres://ecmwf_ro:ecmwf_ro@db-products-dev-00.ecmwf.int/products")
TYPES = text("select distinct stream, type, levtype from fields")
PARAMS = text("select distinct param from fields where stream=:stream and type=:type and levtype=:levtype")

P = {}

for stream, type, levtype in db.execute(TYPES):
    kind = dict(stream=stream, type=type, levtype=levtype)
    key = (stream, type, levtype)
    print('-'.join([stream, type, levtype]))
    params = list(db.execute(PARAMS, kind))

    try:
        P[key] = tuple(sorted([int(x[0]) for x in params]))
    except:
        print("  %s" % params,)


Q = {}
for k, v in sorted(P.items()):
    d = dict(stream=k[0], type=k[1], levtype=k[2])
    v = tuple(v)
    Q.setdefault(v, [])
    Q[v].append(d)

Y = []


def merge(kinds):
    streams = set()
    types = set()
    levtypes = set()

    for k in kinds:
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

    return d


for v, k in Q.items():
    k = merge(k)
    Y.append([k, v])

print(yaml.safe_dump(Y, default_flow_style=False))

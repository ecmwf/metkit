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
    if k[2] == '':
        d = dict(stream=k[0], type=k[1])
    else:
        d = dict(stream=k[0], type=k[1], levtype=k[2])

    v = tuple(v)
    Q.setdefault(v, [])
    Q[v].push_back(d)

Y = []

for v, k in Q.items():
    Y.append([k, v])

print(yaml.safe_dump(Y, default_flow_style=False))

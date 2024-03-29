from sqlalchemy import create_engine, text
import yaml
import os

db = create_engine("postgresql://products:products@k8s-bol-webapps-test-worker-016.ecmwf.int:30544/products")
TYPES = text("select distinct stream, type, levtype from fields")
PARAMS = text(
    "select distinct param from fields where stream=:stream and type=:type and levtype=:levtype"
)

P = {}

for stream, type, levtype in db.execute(TYPES):
    kind = dict(stream=stream, type=type, levtype=levtype)
    key = (stream, type, levtype)
    # print('-'.join([stream, type, levtype]))
    params = list(db.execute(PARAMS, kind))

    try:
        P[key] = tuple(sorted([int(x[0]) for x in params]))
    except:
        pass
        # print("  %s" % params,)
        # P[key] = (129, 999)


Y = []
for k, v in sorted(P.items()):
    if k[2]:
        Y.append([dict(stream=k[0], type=k[1], levtype=k[2]), v])
    else:
        Y.append([dict(stream=k[0], type=k[1]), v])

with open("fieldsdb.yaml", "w") as f:
    f.write(
        "# File automatically generated by %s\n# Do not edit\n\n"
        % (os.path.basename(__file__))
    )
    f.write(yaml.safe_dump(Y, default_flow_style=False))

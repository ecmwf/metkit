from collections import OrderedDict
import psycopg2
import yaml

DSN = 'host=db-products-preprod-00 user=ecmwf_ro password=ecmwf_ro dbname=products'
with psycopg2.connect(DSN) as conn:
    with conn.cursor() as cur:
        cur.execute("SELECT DISTINCT stream, type, levtype, param::INTEGER FROM fields WHERE param != '' ORDER BY stream, type, levtype, param::INTEGER")
        rows = cur.fetchall()

index = OrderedDict()
for row in rows:
    stream, type_, levtype, param = row
    key = (stream, type_, levtype)
    if key not in index:
        index[key] = []
    index[key].append(param)

# Add the same Weather Parameters that were there before, until they are
# fixed in the Fields Catalogue.
index[('oper', 'wp', 'sfc')] = sorted(set([123, 228, 129, 164, 167, 165, 166,
                                      168, 144, 121, 122, 172, 142, 143, 151, 189,
                                      59, 134, 141, 169, 139, 170, 183, 236,
                                      39, 40, 41, 42, 121, 122, 186, 187, 188]))
index[('oper', 'wp', 'pl')] = sorted(set([130, 129, 157, 131, 132]))

index[('enfo', 'wp', 'sfc')] = index[('oper', 'wp', 'sfc')]
index[('enfo', 'wp', 'pl')] = index[('oper', 'wp', 'pl')]
    
# Manually add type=tf parameters for PGEN
index[('oper', 'tf', '')] = [129, 999]
index[('enfo', 'tf', '')] = [129, 999]

yaml_dump_data = []
for key, vals in sorted(index.items()):
    if key[2]:
        yaml_dump_data.append([dict(stream=key[0], type=key[1], levtype=key[2]), vals])
    else:
        yaml_dump_data.append([dict(stream=key[0], type=key[1]), vals])

with open('params.yaml', 'w') as f:
    f.write(yaml.dump(yaml_dump_data, default_flow_style=False))

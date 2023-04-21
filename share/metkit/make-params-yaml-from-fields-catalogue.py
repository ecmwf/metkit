import os
import sys
from collections import OrderedDict
import psycopg2
import yaml

DEFAULT_PORT = 5432

def main():
    try:
        HOST = os.environ['PRODUCT_CATALOGUE_HOST']
        DB = os.environ['PRODUCT_CATALOGUE_DB']
        USER = os.environ['PRODUCT_CATALOGUE_USER']
        PASSWORD = os.environ['PRODUCT_CATALOGUE_PASSWORD']
        PORT = (
            int(os.environ['PRODUCT_CATALOGUE_PORT'])
            if 'PRODUCT_CATALOGUE_PORT' in os.environ
            else DEFAULT_PORT
        )
    except KeyError as e:
        print("ERROR: Environment variable not found: {}".format(e))
        sys.exit(1)

    with psycopg2.connect(host=HOST, dbname=DB, user=USER, password=PASSWORD, port=PORT) as conn:
        with conn.cursor() as cur:
            cur.execute(
                "SELECT DISTINCT stream, type, levtype, param::INTEGER FROM fields WHERE param != '' ORDER BY stream, type, levtype, param::INTEGER"
            )
            rows = cur.fetchall()

    index = OrderedDict()
    for row in rows:
        stream, type_, levtype, param = row
        key = (stream, type_, levtype)
        if key not in index:
            index[key] = []
        index[key].append(param)

    # Manually add type=tf parameters for PGEN
    index[("oper", "tf", "")] = [129, 142, 143, 156, 172, 188, 999]
    index[("scda", "tf", "")] = [129, 142, 143, 156, 172, 188, 999]
    index[("enfo", "tf", "")] = [129, 142, 143, 156, 172, 188, 999]
    
    # type=wp
    index[("oper", "wp", "")] = [129, 142, 143, 156, 172, 188, 999]
    index[("scda", "wp", "")] = [129, 142, 143, 156, 172, 188, 999]
    index[("enfo", "wp", "")] = [129, 142, 143, 156, 172, 188, 999]
    
    yaml_dump_data = []
    for key, vals in sorted(index.items()):
        if key[2]:
            yaml_dump_data.append(
                [dict(stream=key[0], type=key[1], levtype=key[2]), vals]
            )
        else:
            yaml_dump_data.append([dict(stream=key[0], type=key[1]), vals])

    with open("params.yaml", "w") as f:
        f.write(
            "# File automatically generated by %s\n# Do not edit\n\n"
            % (os.path.basename(__file__))
        )
        f.write(yaml.safe_dump(yaml_dump_data, default_flow_style=False))


if __name__ == "__main__":
    main()

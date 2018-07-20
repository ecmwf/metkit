#!/usr/bin/env python
import MySQLdb
import yaml


db = MySQLdb.connect("grib-param-db-prod.ecmwf.int",
                      "ecmwf_ro",
                      "ecmwf_ro",
                      "param")

prodgen = {}

with open("prodgen.yaml") as f:
    PRODGEN = yaml.load(f.read())

with open("paramids.yaml") as f:
    PARAMSIDS = yaml.load(f.read())

cursor = db.cursor()

cursor.execute("select * from param")

for data in cursor.fetchall():
    paramid, abbr, longname = int(data[0]), data[1].lower(), data[2].lower()

    if abbr == '~':
        continue

    entry = [abbr, longname]

    if paramid in PRODGEN:
      pass
  
    entry = tuple(entry)

    if paramid in PARAMSIDS:
       before = tuple(PARAMSIDS[paramid]) 
       if before != entry:
           print("WARNING! updated paramid: {},  {} => {}".format(paramid, before, entry))
           PARAMSIDS[paramid] = list(entry)
    else:
        print("new paramid: {} ".format(paramid, entry))
        PARAMSIDS[paramid] = list(entry)

cursor.close()
db.close()

with open("prodgen.yaml", "w") as f:
    f.write(yaml.safe_dump(PRODGEN, default_flow_style=False))

with open("paramids.yaml", "w") as f:
    f.write(yaml.safe_dump(PARAMSIDS, default_flow_style=False))

#!/usr/bin/env python
import MySQLdb
import yaml
import re


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

    abbr = re.sub(r'\W', '_', abbr)
    abbr = re.sub(r'_+', '_', abbr)
    abbr = re.sub(r'^_', '', abbr)
    abbr = re.sub(r'_$', '', abbr)

    entry = [abbr, longname]

    if False and paramid in PRODGEN:
       pgen = PRODGEN[paramid]
       if len(pgen) > 2:
           prodgen[paramid] = pgen[2:]
           for n in prodgen[paramid]:
               entry.append(n)
          
  
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

#with open("prodgen.yaml", "w") as f:
    #f.write(yaml.safe_dump(PRODGEN, default_flow_style=False))

with open("paramids.yaml", "w") as f:
    f.write(yaml.safe_dump(PARAMSIDS, default_flow_style=False))

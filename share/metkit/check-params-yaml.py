#!/usr/bin/env python
import MySQLdb
import yaml
import re
import os


db = MySQLdb.connect("grib-param-db-prod.ecmwf.int", "ecmwf_ro", "ecmwf_ro", "param")


cursor = db.cursor()


with open("params.yaml") as f:
    PARAMS = yaml.load(f.read())

for entry in PARAMS:
    when, parms = entry
    for p in parms:
        cursor.execute("select count(*) from param where id=%s" % p)
        for data in cursor.fetchall():
            if data[0] == 0:
                print("%s: %s is not in param DB" % (when, p))

#!/usr/bin/env python

# import psycopg2
import yaml
import mysql.connector
import sys
import re

gdb = mysql.connector.connect(host="grib-param-db-prod.ecmwf.int",
                     user="ecmwf_ro",
                     password="ecmwf_ro",
                     database="param")


params = sys.argv[1:]
if not params:
   params = ["130"]


what = "id"
values = []

for p in params:
    try:
      a, b = p.split(".")
      a, b = int(a), int(b)
      if b == 128:
        b = 0
      p = str(b*1000 + a)
    except:
      pass

    try:
      int(p)
    except:
      what="shortName"
      p="'%s'" % (p,)


    values.append(p)




gcursor = gdb.cursor()
gcursor.execute("""
SELECT param.id, param.shortName, param.name, units.name
FROM param, units
WHERE param.%s IN (%s) AND units.id = units_id
""" % (what, ",".join(values)))


for n in gcursor.fetchall():
    print("{},{},{},{}".format(*tuple(n)))

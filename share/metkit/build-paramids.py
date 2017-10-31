import MySQLdb
db = MySQLdb.connect("grib-param-db-prod.ecmwf.int","ecmwf_ro","ecmwf_ro","param" )


cursor = db.cursor()

cursor.execute("select * from param")

for data in cursor.fetchall():
	paramid, abbr, longname = data[0], data[1], data[2]
	print(paramid, abbr, longname)

# disconnect from server
db.close()

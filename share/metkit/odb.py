#!/usr/bin/env python3
# (c) Copyright 1996-2012 ECMWF.
# 
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
# In applying this licence, ECMWF does not waive the privileges and immunities 
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


import re


T = """- context:\n    type: [ofb,mfb,oai,sfb,fsoifb,fcdfb,ofa]\n    obsgroup: [%(OBSGROUP)s]\n  defaults:\n    reportype: [%(REPORTYPE_LIST)s]\n"""

connectionDetails = dict(host = "webapps-db-prod", user = "ecmwf_ro", passwd = "ecmwf_ro")

def execute(fun):
	import mysql.connector
	connection = mysql.connector.connect(**connectionDetails)
	cursor = connection.cursor()
	cursor.execute("USE odbgov")
	r = fun(cursor)
	cursor.close()
	connection.close()
	return r

def getGroups(cursor):
	cursor.execute("SELECT id,marsname FROM odbadmin_group")
	return dict([(r[0],str(r[1]).lower()) for r in cursor.fetchall()])

GROUP = execute(getGroups)

def getGroups(cursor):
	cursor.execute("SELECT id,name FROM odbadmin_group")
	return dict([(r[0],r[1]) for r in cursor.fetchall()])

GROUPNAME = execute(getGroups)

def getListOfRT(group_id):
	def sorted(cursor):
		cursor.execute("SELECT code FROM odbadmin_reporttype WHERE group_id = " + str(group_id))
		r = [r[0] for r in cursor.fetchall()]
		r.sort()
		return r
	return sorted

################################################################################################
def rt_by_obsgroup(cursor):
	cursor.execute("SELECT DISTINCT group_id FROM odbadmin_reporttype ORDER BY group_id")
	groups = [r[0] for r in cursor.fetchall()]
	def expand(g):
		return T % dict(OBSGROUP = GROUP[g], REPORTYPE_LIST = "/".join([str(r) for r in execute(getListOfRT(g))]))
	return "\n".join([expand(group_id) for group_id in groups])

def obsgroupstxt(cursor):
	cursor.execute("SELECT id,marsname FROM odbadmin_group ORDER BY id")
	return "\n".join(["%s ; %s" % (r[0], r[1]) for r in cursor.fetchall()])

def reportypescodes(cursor): 
	print("     code&uarr;    ; group    ; description    ;    ")
	cursor.execute("SELECT code,group_id,description FROM odbadmin_reporttype ORDER BY code")
	return "\n".join(["%s  ; %s  ; %s  ;    " % (r[0], GROUPNAME[r[1]], r[2]) for r in cursor.fetchall()])

options = 'obsgroups reportypes rt_by_obsgroup webmars obsgroupstxt reportypescodes groupcodes'.split()
################################################################################################

def main():
	import sys
	opts = sys.argv[1:]
	if len(opts) != 1 or opts[0].lstrip('-') not in options:
		print(sys.argv[0] + ": Possible options: --" + ", --".join(options))
		sys.exit(1)
	o = opts[0].lstrip('-')
	print(execute(globals()[o]))

if __name__ == "__main__":
	"""
	with open("rt_by_obsgroup.chk", "w") as f: f.write(execute(rt_by_obsgroup))
	with open("reportypes.def", "w") as f: f.write(execute(reportypes))
	with open("obsgroups.def", "w") as f: f.write(execute(obsgroups))
	"""
	main()


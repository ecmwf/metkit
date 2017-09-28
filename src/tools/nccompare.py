#!/usr/bin/env python

# (C) Copyright 1996-2015 ECMWF.

# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.

# Baudouin Raoult - ECMWF Jan 2015


# See http://unidata.github.io/netcdf4-python/netCDF4.Dataset-class.html

from netCDF4 import Dataset
import sys
import numpy

ERRORS = 0


def error(text):
    global ERRORS
    ERRORS += 1
    print "+    ERROR:", text


def mismatch(title, v1, v2):
    global ERRORS
    ERRORS += 1
    print "+ MISMATCH:", title
    print "      FILE:", sys.argv[1]
    print "     VALUE:", v1
    print "      FILE:", sys.argv[2]
    print "     VALUE:", v2


def compare_lists(what, l1, l2):
    s1 = set(l1)
    s2 = set(l2)

    if len(s1 - s2):
        error("%s [%s] only in %s" % (what, ",".join([str(x) for x in list(s1-s2)]), sys.argv[1]))
        return False

    if len(s2 - s1):
        error("%s [%s] only in %s" % (what, ",".join([str(x) for x in list(s2-s1)]), sys.argv[2]))
        return False

    return True


def compare_attributes(ns, name, a1, a2):

    if type(a1) != type(a2):
        mismatch("Type of attributes '%s:%s'" % (ns, name,), type(a1), type(a2))

    if a1 != a2:
        a1 = str(a1)
        a2 = str(a2)
        if len(a1) > 80:
            a1 = a1[:79] + '...'
        if len(a2) > 80:
            a2 = a2[:79] + '...'
        mismatch("Value of attributes '%s:%s'" % (ns, name,), a1, a2)


def compare_variables(name, v1, v2):
    compare_lists("Dimensions for '%s'" % (name,), v1.dimensions, v2.dimensions)

    if v1.datatype != v2.datatype:
        mismatch("Type of variable '%s'" % (name,), v1.datatype, v2.datatype)

    if compare_lists("Variable '%s' attributes" % (name,), v1.ncattrs(), v2.ncattrs()):
        for a in v1.ncattrs():
            compare_attributes(name, a, v1.getncattr(a), v2.getncattr(a))

    vals1 = v1[:]
    vals2 = v2[:]

    if vals1.size != vals2.size:
        mismatch("Size of variable '%s'" % (name,), vals1.size, vals2.size)

    if not (vals1 == vals2).all():
        vals1 = vals1.flatten()
        vals2 = vals2.flatten()
        idx = numpy.nonzero(vals1 - vals2)[0][0]
        if idx < len(vals1) and idx < len(vals2):
            mismatch("Value for variable '%s', first difference at index %s" % (name, idx), vals1[idx], vals2[idx])


def compare_dimensions(name, d1, d2):
    if len(d1) != len(d2):
        mismatch("Length of dimension '%s'" % (name,), len(d1), len(d2))


def check_file(nc, path):

    if len(nc.groups):
        error("Groups not supported (file %s)" % (path,))

    if len(nc.cmptypes):
        error("Compound types not supported (file %s)" % (path,))

    if len(nc.vltypes):
        error("Variable-length types not supported (file %s)" % (path,))


def compare_files(nc1, nc2):

    check_file(nc1, sys.argv[1])
    check_file(nc2, sys.argv[2])

    dims1 = nc1.dimensions
    dims2 = nc2.dimensions

    if compare_lists("Dimensions", dims1.keys(), dims2.keys()):
        for k in dims2.keys():
            compare_dimensions(k, dims1[k], dims2[k])

    vars1 = nc1.variables
    vars2 = nc2.variables

    if compare_lists("Variables", vars1.keys(), vars2.keys()):
        for k in vars1.keys():
            compare_variables(k, vars1[k], vars2[k])

    if compare_lists("Global attributes", nc1.ncattrs(), nc2.ncattrs()):
        for a in nc1.ncattrs():
            compare_attributes("", a, nc1.getncattr(a), nc2.getncattr(a))

    # Other things to checks
    # print nc1.data_model, nc1.disk_format
    # print nc1.parent, nc1.vltypes


nc1 = Dataset(sys.argv[1], 'r')
nc2 = Dataset(sys.argv[2], 'r')

compare_files(nc1, nc2)

if ERRORS:
    sys.exit(1)

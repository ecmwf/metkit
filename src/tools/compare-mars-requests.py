#!/usr/bin/env python

# (C) Copyright 1996- ECMWF.

# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.

# Emanuele Danovaro - ECMWF May 2021

import sys
import os.path
import json
import datetime
from dateutil.parser import parse

def check_one(name, old, new):
    assert len(old) == 1 and len(new) == 1, f"{name.upper()}: too many values {len(old)} and {len(new)}"
    return old[0], new[0]

def compare_int(name, old, new):
    for o, n in zip(old, new):
        assert int(o) == int(n), f"{name.upper()}: value mismatch '{old}' and '{new}'"

def compare_str(name, old, new):
    o, n = check_one(name, old, new)
    assert o.replace('"', '').lower() == n.replace('"', '').lower(), f"{name.upper()}: string mismatch '{old[0]}' and '{new[0]}'"

def compare_strList(name, old, new):
    for o, n in zip(old, new):
        compare_str(name, [o], [n])

def compare_strCaseSensitive(name, old, new):
    o, n = check_one(name, old, new)
    assert o.replace('"', '') == n.replace('"', ''), f"{name.upper()}: string mismatch '{old[0]}' and '{new[0]}'"

def compare_time(name, old, new):
    o, n = check_one(name, old, new)

    if isinstance(o, str):
        o = int(o.replace(':', ''))
    if o>9999:
        o = o/100

    if isinstance(n, str):
        n = int(n.replace(':', ''))
    if n > 9999:
        n = n / 100

    assert o == n, f"{name.upper()}: time mismatch '{old[0]}' and '{new[0]}'"

def compare_date(name, old, new):
    o, n = check_one(name, old, new)
    oStart = parse(str(o), default=datetime.datetime(1901, 1, 1))
    oEnd = parse(str(o), default=datetime.datetime(2001, 12, 31))
    nStart = parse(str(n), default=datetime.datetime(1901, 1, 1))
    nEnd = parse(str(n), default=datetime.datetime(2001, 12, 31))
    assert oStart.date() == nStart.date() and oEnd.date() == nEnd.date(), f"{name.upper()}: date mismatch '{old[0]}' and '{new[0]}'"

def compare_expver(name, old, new):
    o, n = check_one(name, old, new)
    if o == n:
        return
    if isinstance(o, int) or len(o)<4:
        o = str(o).zfill(4)
    if isinstance(n, int) or len(n)<4:
        n = str(n).zfill(4)
    assert o == n, f"{name.upper()}: expver mismatch '{old[0]}' and '{new[0]}'"

def ignore(name, old, new):
    pass

def ignore_fixYaml(name, old, new):
    pass

comparators = {
    "class": compare_str,
    "stream": compare_str,
    "type": compare_str,
    "domain": compare_str,
    "date": compare_date,
    "expver": compare_expver,
    "levtype": compare_str,
    "levelist": compare_int,
    "param": compare_strList,
    "step": compare_int,
    "time": compare_time,
    "target": compare_strCaseSensitive,
    "accuracy": ignore_fixYaml,
    "repres": ignore,
    "area": ignore_fixYaml
}

def compare(name, old, new):
    if not isinstance(old, list):
        old = [old]
    if not isinstance(new, list):
        new = [new]

    assert len(old) == len(new), f"{name}: list mismatch {len(old)} and {len(new)}"
    assert name in comparators.keys(), f"{name}: not supported"
    comparators[name](name, old, new)

# compare_date('date', '20210501', '2021-05-01')
# compare_date('date', '20210501', '2021-May-01')
# compare_date('date', '20210501', '2021-may-01')
# compare_date('date', 'May-1', 'may-01')
# compare_date('date', 'May', 'may')
# compare_date('date', 'May', '20210521')

# check command line parameters
if len(sys.argv) != 3 or not os.path.isfile(sys.argv[1]) or not os.path.isfile(sys.argv[2]):
    print('Please specify the json files to be compared.')
    print('Usage: ',sys.argv[0], '<first.json> <second.json>')
    if len(sys.argv)>1 and not os.path.isfile(sys.argv[1]):
        print(sys.argv[1], 'is not a file')
    if len(sys.argv)>2 and not os.path.isfile(sys.argv[2]):
        print(sys.argv[2], 'is not a file')
    exit(1)

#open json files and build case insensitive dictionaries
j1 = {}
with open(sys.argv[1]) as json_file1:
    for key, value in json.load(json_file1).items():
        j1[key.lower()] = value;

j2 = {}
with open(sys.argv[2]) as json_file2:
    for key, value in json.load(json_file2).items():
        j2[key.lower()] = value;

for key in set(j1.keys()) | set(j2.keys()):
    compare(key, j1.get(key), j2.get(key))

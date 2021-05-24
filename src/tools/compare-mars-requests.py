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

def compare_int(name, old, new):
    assert int(old) == int(new), f"{name.upper()}: value mismatch '{old}' and '{new}'"

def compare_str(name, old, new):
    assert old.replace('"', '').lower() == new.replace('"', '').lower(), f"{name.upper()}: string mismatch '{old}' and '{new}'"

def compare_strCaseSensitive(name, old, new):
    assert old.replace('"', '') == new.replace('"', ''), f"{name.upper()}: string mismatch '{old}' and '{new}'"

def compare_time(name, old, new):
    if isinstance(old, str):
        old = int(old.replace(':', ''))
    if old>9999:
        old = old/100

    if isinstance(new, str):
        new = int(new.replace(':', ''))
    if new > 9999:
        new = new / 100

    assert old == new, f"{name.upper()}: time mismatch '{old}' and '{new}'"

def compare_date(name, old, new):
    oStart = parse(str(old), default=datetime.datetime(1901, 1, 1))
    oEnd = parse(str(old), default=datetime.datetime(2001, 12, 31))
    nStart = parse(str(new), default=datetime.datetime(1901, 1, 1))
    nEnd = parse(str(new), default=datetime.datetime(2001, 12, 31))
    # print(old, oStart.date(), oEnd.date(), new, nStart.date(), nEnd.date(), oStart.date() == nStart.date() and oEnd.date() == nEnd.date())
    assert oStart.date() == nStart.date() and oEnd.date() == nEnd.date(), f"{name.upper()}: date mismatch '{old}' and '{new}'"

def compare_expver(name, old, new):
    if (old == new):
        return
    if isinstance(old, int) or len(old)<4:
        old = str(old).zfill(4)
    if isinstance(new, int) or len(new)<4:
        new = str(new).zfill(4)
    assert old == new, f"{name.upper()}: expver mismatch '{old}' and '{new}'"

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
    "param": compare_str,
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
    all(comparators[name](name, o, n) for o, n in zip(old, new))

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

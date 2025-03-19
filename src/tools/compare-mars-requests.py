#!/usr/bin/env python3

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

def check_atMostOne(name, old, new):
    assert len(old) < 2 and len(new) < 2, f"{name.upper()}: too many values {len(old)} and {len(new)}"
    return old[0] if 0 < len(old) else None, new[0] if 0 < len(new) else None

def compare_int(name, old, new):
    for o, n in zip(old, new):
        assert str(o).isnumeric() and str(n).isnumeric() and int(o) == int(n), f"{name.upper()}: value mismatch '{old}' and '{new}'"

def compare_float(name, old, new):
    for o, n in zip(old, new):
        try:
            o = float(o)
        except ValueError:
            o = o.lower()

        try:
            n = float(n)
        except ValueError:
            n = n.lower()

        assert o == n, f"{name.upper()}: value mismatch '{old}' and '{new}'"

def compare_intAndKeywords(name, old, new, keywords):
    o, n = check_atMostOne(name, old, new)
    if o in keywords or str(o).lower() in keywords:
        o = str(o).lower() if isinstance(o, str) else None
    if n in keywords or str(n).lower() in keywords:
        n = str(n).lower() if isinstance(n, str) else None
    assert o == n or (str(o).isnumeric() and str(n).isnumeric() and int(o) == int(n)), f"{name.upper()}: value mismatch '{old}' and '{new}'"

def compare_grid(name, old, new):
    for o, n in zip(old, new):
        try:
            o = float(o)
        except ValueError:
            o = o.lower()

        try:
            n = float(n)
        except ValueError:
            n = n.lower()
        assert o == n, f"{name.upper()}: value mismatch '{old}' and '{new}'"

def compare_expect(name, old, new):
    # 'any' or a positive integer value
    compare_intAndKeywords(name, old, new, ["any"])

def compare_accuracy(name, old, new):
    # 'off', 'av' or a positive integer value
    compare_intAndKeywords(name, old, new, [None, "off", "av", "n"])

def compare_frame(name, old, new):
    # 'off' or a positive integer value
    compare_intAndKeywords(name, old, new, ["off"])

def compare_truncation(name, old, new):
    # 'none', 'auto', 'off' or a positive integer value
    compare_intAndKeywords(name, old, new, ["off", "none", "auto"])

def compare_step(name, old, new):
    for o, n in zip(old, new):
        oParts = str(o).split('-')
        nParts = str(n).split('-')
        assert len(oParts) == len(nParts) and all(float(oi) == float(ni) for oi, ni in zip(oParts, nParts)), f"{name.upper()}: value mismatch '{old}' and '{new}'"

def compare_str(name, old, new):
    o, n = check_one(name, old, new)
    assert o.replace('"', '').lower() == n.replace('"', '').lower(), f"{name.upper()}: string mismatch '{old[0]}' and '{new[0]}'"

def compare_strList(name, old, new):
    for o, n in zip(old, new):
        compare_str(name, [o], [n])

def compare_strCaseSensitive(name, old, new):
    for o, n in zip(old, new):
        assert o.replace('"', '') == n.replace('"', ''), f"{name.upper()}: string mismatch '{old}' and '{new}'"

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

def compare_dates(name, old, new):
    for o, n in zip(old, new):
        oStart = parse(str(o), default=datetime.datetime(1901, 1, 1))
        oEnd = parse(str(o), default=datetime.datetime(2001, 12, 31))
        nStart = parse(str(n), default=datetime.datetime(1901, 1, 1))
        nEnd = parse(str(n), default=datetime.datetime(2001, 12, 31))
        assert oStart.date() == nStart.date() and oEnd.date() == nEnd.date(), f"{name.upper()}: date mismatch '{o}' and '{n}'"

def compare_expver(name, old, new):
    o, n = check_one(name, old, new)
    if isinstance(o, str):
        try:
            o = int(o)
        except ValueError:
            o = o.lower()

    if isinstance(n, str):
        try:
            n = int(n)
        except ValueError:
            n = n.lower()

    assert o == n, f"{name.upper()}: mismatch '{old[0]}' and '{new[0]}'"

def ignore(name, old, new):
    pass

def ignore_fixYaml(name, old, new):
    pass

comparators = {
    "accuracy": ignore_fixYaml, # ?????
    # "accuracy": compare_accuracy,
    "anoffset": compare_int, # ?????
    "area": ignore_fixYaml,
    "bitmap": compare_strCaseSensitive,
    # block
    # channel
    "class": compare_str,
    "database": compare_str, # ?????
    "date": compare_date,
    "diagnostic": compare_int, # ??????
    "direction": compare_int, # should we handle case "all"
    "domain": compare_str,
    "duplicates": compare_str, # can we ignore ?
    "expect": compare_expect,
    "expver": compare_expver,
    "fcmonth": compare_int,
    "fcperiod": compare_step, # ??????
    "fieldset": compare_strCaseSensitive,
    "filter": compare_str,
    "format": compare_str, # ?????
    "frame": compare_frame,
    "frequency": compare_int, # ??????
    "grid": compare_grid,
    "hdate": compare_dates,
    "ident": compare_int,
    "interpolation": compare_str,
    # "intgrid":
    "iteration": compare_int, # ??????
    "latitude": compare_float,
    "levelist": compare_float,
    "levtype": compare_str,
    "longitude": compare_float,
    "lsm": compare_str, # ?????
    "method": compare_int,
    "number": compare_int,
    "obsgroup": compare_str, # ?????
    "obstype": compare_int,
    "origin": compare_expver, # ?????
    "packing": compare_str,
    "padding": compare_int, # ??????
    "param": compare_float, # <-- to check that short names have been replaced, otherwise we can use compare_strList
    "priority": compare_int,
    "product": compare_str, # ?????
    "range": compare_int,
    "refdate": compare_date,
    "reference": compare_int,
    "reportype": compare_int,
    "repres": ignore,
    # resol
    "rotation": compare_grid,
    "section": compare_str,
    "source": compare_strCaseSensitive,
    "step": compare_step,
    "stream": compare_str,
    "system": compare_int,
    "target": compare_strCaseSensitive,
    "time": compare_time,
    "truncation": compare_truncation,
    "type": compare_str,
    "use": compare_str
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
json_lines_1 = []
with open(sys.argv[1]) as json_file:
    for line in json_file:
        j = {}
        for key, value in json.loads(line).items():
            j[key.lower()] = value
        json_lines_1.append(j)

json_lines_2 = []
with open(sys.argv[2]) as json_file:
    for line in json_file:
        j = {}
        for key, value in json.loads(line).items():
            j[key.lower()] = value
        json_lines_2.append(j)

assert len(json_lines_1) == len(json_lines_2), f"Json mismatch: {sys.argv[1]} contains {len(json_lines_1)} while {sys.argv[2]} contains {len(json_lines_2)}"

for j1, j2 in zip(json_lines_1, json_lines_2):
    for key in set(j1.keys()) | set(j2.keys()):
        compare(key, j1.get(key), j2.get(key))

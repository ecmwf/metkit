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
from dateutil.parser import parse

ignore = ['repres', 'accuracy', 'area']

class Compare:
    __V1 = None
    __V2 = None

    def compare(self, key, v1, v2):
        if type(v1) == str:
            self.__V1 = v1.lower()
        else:
            self.__V1 = v1
        if type(v2) == str:
            self.__V2 = v2.lower()
        else:
            self.__V2 = v2
        default = self.__V1 == self.__V2
        # class, type, stream, levtype, domain, [repres]
        return getattr(self, key, lambda: default)()

    def target(self):
        return self.__V1.replace('"', '') == self.__V2.replace('"', '')

    def toInt(self):
        return int(self.__V1) == int(self.__V2)
    def expver(self):
        return self.toInt()
    def levelist(self):
        return self.toInt()
    def step(self):
        return self.toInt()

    def date(self):
        d1 = parse(str(self.__V1))
        d2 = parse(str(self.__V2))
        return d1.date() == d2.date()

    def time(self):
        v1 = self.__V1
        if type(v1) == str:
            v1 = int(v1.replace(':', ''))
        if v1>9999:
            v1 = v1/100

        v2 = self.__V2
        if type(v2) == str:
            v2 = int(v2.replace(':', ''))
        if v2 > 9999:
            v2 = v2 / 100

        return v1 == v2



def compare(key, v1, v2):
    if type(v1) != list:
        v1 = [v1]
    if type(v2) != list:
        v2 = [v2]

    if len(v1) != len(v2):
        return False

    c = Compare()
    for i, j in zip(v1, v2):
        if not c.compare(key, i, j):
            return False
    return True

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

ok = True
#iterate over all keys and compare their values
for key in set(j1.keys()) | set(j2.keys()):
    if not key in ignore:
        if not (key in j1 and key in j2 and compare(key, j1[key], j2[key])):
            print('Error with key', '{:10}'.format(key.upper()), end=' ', flush=False)
            ok = False
            if not key in j1:
                print('missing in:', sys.argv[1])
            else:
                if not key in j2:
                    print('missing in:', sys.argv[2])
                else:
                    print('differs:', j1[key], ' and ', j2[key])

if ok:
    exit(0)
else:
    exit(1)
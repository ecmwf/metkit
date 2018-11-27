import yaml
import sys


def read(path):
    r = {}
    with open(path) as f:
        for k, v in yaml.safe_load(f):
            u = (k.get('stream'), k.get('type'), k.get('levtype', 'sfc'))
            r[u] = set(v)
    return r


r = read('fieldsdb.yaml')
s = read('params.yaml')


for k, v in sorted(r.items()):
    if k not in s:
        print("Only in db", k)

print()
for k, v in sorted(s.items()):
    if k not in r:
        print("Only in params", k)

print()
for k in sorted(set(r.keys()) & set(s.keys())):
    print(k)
    a = r[k] - s[k]
    b = s[k] - r[k]
    if a:
        print("    In db and not in params", a)
    if b:
        print("    In params and not in db", b)

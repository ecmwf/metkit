import json
import itertools

y = {}
f = open("params.txt")
g = open("/Users/baudouin/Dropbox/params.txt")

EXTRA= [
    "xxx tf oper xxx 999",
    "xxx tf oper xxx 129",
    "xxx tf enfo xxx 999",
    "xxx tf enfo xxx 129",
    "xxx cs enfo xxx 129",
    "xxx cs enfo xxx 130",
    "xxx fp enfo xxx 131130",
    "xxx fp waef xxx 131229",
    "xxx fp waef xxx 131232",
    "xxx fc mnfc xxx 151",
    "xxx fc mnfc xxx 228",
    "xxx fc mnfc xxx 129",
    "xxx fc mnfc xxx 2",
    "xxx fc oper xxx 3041",
]

for n in itertools.chain(EXTRA, f.readlines(), g.readlines()):
        n = [x for x in n.strip().split(" ") if x]
        if len(n) != 5:
            continue

        try:
            int(n[4])
        except:
            continue

        if int(n[4]) == 0:
            continue
        
        if n[1] == "not_found":
            if int(n[4]) / 1000 == 140:
                n[1] = 'an'
                n[2] = 'wave'
            else:
                n[1] = 'an'
                n[2] = 'oper'

        if n[1] in ('pf', 'cf'):
            n[1] = 'cf'

        if n[1] in ('fc', 'an'):
            n[1] = 'an'

        if n[2] in ('oper', 'scda'):
            n[2] = 'oper'

        if n[2] in ('wave', 'scwv'):
            n[2] = 'wave'

        k = "-".join((n[2], n[1]))
        y.setdefault(k, set()).add(n[4])

P = []
for k, v in sorted(y.items()):
    s, t = k.split("-")

    if t == 'cf':
        t = ['cf', 'pf', 'wp', 'cv']

    if t == 'an':
        t = ['an', 'fc', 'wp']

    if s == 'oper':
        s = ['oper', 'scda']

    if s == 'wave':
        s = ['wave', 'scwv']

    P.append([{"type":t, "stream":s}, sorted(v)])


print json.dumps(P, sort_keys=True, indent=4)

#!/usr/bin/env python3

import yaml

# import oyaml as yaml

with open("language.yaml") as f:
    language = yaml.load(f.read(), Loader=yaml.SafeLoader)

with open("paramids.yaml") as f:
    paramids = yaml.load(f.read(), Loader=yaml.SafeLoader)

params = []
for k, v in sorted(paramids.items(), key=lambda a: a[1][0]):
    params.append([k] + v)

paramids = params

paramids.append("@")
paramids.append("*")


def get_enum(name, attributes):
    return attributes["values"]


def get_expver(name, attributes):
    return ("@", "*", '"')


def get_any(name, attributes):
    return ("@", "*", '"')


def get_to_by_list(name, attributes):
    return ("TO", "BY", "*")


def get_param(name, attributes):
    return paramids


def get_date(name, attributes):
    return "@"


def get_time(name, attributes):
    return "@"


def get_range(name, attributes):
    return "@"


def get_integer(name, attributes):
    return "*"


def get_float(name, attributes):
    return "*"


def get_string(name, attributes):
    return '"'


def get_regex(name, attributes):
    return "! regex: {}".format(attributes["regex"])


def as_client(x):
    if x is True:
        return "ON"
    if x is False:
        return "OFF"
    x = str(x)

    return x.upper()


G = globals()

EXTRAS = {}

for verb, params in sorted(language.items()):
    if verb.startswith("_"):
        continue
    print()
    print("{} ; ignore ; MARS".format(as_client(verb)))
    print("{")
    for param, attributes in params.items():
        extra = EXTRAS.get((verb, param), EXTRAS.get(param))
        if extra:
            print("   {} [{}]".format(as_client(param), extra))
        else:
            print("   {}".format(as_client(param)))
        print("   {")
        kind = attributes.get("type")
        if kind is None:
            vals = get_string(param, attributes)
        elif isinstance(kind, list):
            vals = []
            for k in kind:
                v = G["get_{}".format(k.replace("-", "_"))](param, attributes)
                if isinstance(v, (list, tuple)):
                    vals += v
                else:
                    vals.append(v)
        else:
            vals = G["get_{}".format(kind.replace("-", "_"))](param, attributes)
        if not isinstance(vals, (list, tuple)):
            vals = [vals]
        if attributes.get("multiple", False):
            vals = list(vals)
            vals.append("/")
        for v in vals:
            if isinstance(v, (tuple, list)):
                v = " ; ".join(as_client(x) for x in reversed(v))
            else:
                v = as_client(v)

            print(f"     {v}")
        default = attributes.get("default")
        if default is None:
            print("   }")
        else:
            if isinstance(default, (tuple, list)):
                default = "/".join(as_client(x) for x in default)
            else:
                default = as_client(default)
            print("   {} = {}".format("}", default))
        print()
    print("}")

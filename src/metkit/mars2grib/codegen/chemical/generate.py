#!/bin/env python3

import csv
import os
import sys
from collections import defaultdict
from subprocess import run


HERE = os.path.join(os.path.dirname(os.path.abspath(__file__)))
MAPPINGS_PATH = os.path.join(HERE, 'mappings.csv')

M2M_TEMPLATE_PATH = os.path.join(HERE, 'm2m-chemical.h')
M2M_DESTINATION_PATH = os.path.abspath(os.path.join(HERE, '../../../mars2mars/mappings/rules/chemical.h'))

M2G_TEMPLATE_PATH = os.path.join(HERE, 'm2g-compositionMatcher.h')
M2G_DESTINATION_PATH = os.path.abspath(os.path.join(HERE, '../../backend/concepts/composition/compositionMatcher.h'))


def write_code(destination_path, template_path, code):
    with open(template_path, 'r') as f:
        template = f.read()
    final = template.replace('#codegen replace', code)
    with open(destination_path, 'w') as f:
        f.write(final)
    run(f'clang-format --style=file -i {destination_path}'.split(' '))


###############################################################################
#                         mars2mars Conversion Rules                          #
###############################################################################

def set_param_chem(mapping, new_param=None):
    """Format the return statement that sets the output param, chem (and wavelength) for one mapping.

    new_param overrides the param with a C++ expression (see ssaParam in the m2m template)."""
    new_paramId = new_param or mapping['new_paramId']
    chemId = mapping['chemId']
    wavelength = mapping['wavelength']
    if wavelength:
        return f"return setParamChemWavelength(out, {new_paramId}, {chemId}, {wavelength:.1f});"
    return f"return setParamChem(out, {new_paramId}, {chemId});"


def levtype_dependent_param(stream, old_paramId, by_levtype):
    """Return the C++ expression selecting the new param from the levtype of the request.

    The only levtype-dependent mapping supported is the single-scattering albedo, which is cssa (482000) at levtype
    sfc and ssa (458000) otherwise, see ssaParam in the m2m template."""
    sfc, default = by_levtype.get('sfc'), by_levtype.get('')
    same_chem = sfc and default and all(sfc[k] == default[k] for k in ('chemId', 'wavelength'))
    if set(by_levtype) != {'sfc', ''} or not same_chem or (sfc['new_paramId'], default['new_paramId']) != (482000, 458000):
        print(f"Unsupported levtype-dependent mapping for stream={stream},old_paramId={old_paramId}: "
              f"only (sfc: 482000, default: 458000) with identical chemId and wavelength is supported", file=sys.stderr)
        exit(255)
    return 'ssaParam(in)'


def generateM2M():
    # {(stream, old_paramId): {levtype: mapping}}, where an empty levtype is the default for any other levtype
    mappings = defaultdict(dict)
    with open(MAPPINGS_PATH, 'r') as f:
        reader = csv.DictReader(f, delimiter=';')
        for row in reader:
            stream = row['stream']
            levtype = row['levtype']
            old_paramId = int(row['old_paramId'])
            if levtype in mappings[(stream, old_paramId)]:
                print(f"More than one mapping exists from stream={stream},levtype={levtype},old_paramId={old_paramId}", file=sys.stderr)
                exit(255)
            new_paramId = int(row['new_paramId'])
            chemId = int(row['chemId'])
            wavelength = None if row['wavelength'] == '' else int(row['wavelength'])
            mappings[(stream, old_paramId)][levtype] = {'new_paramId': new_paramId, 'chemId': chemId, 'wavelength': wavelength}

    rules = {'oper': [], 'gfas': []}
    for stream, old_paramId in sorted(mappings.keys()):
        by_levtype = mappings[(stream, old_paramId)]
        new_param = None if set(by_levtype) == {''} else levtype_dependent_param(stream, old_paramId, by_levtype)
        rules[stream].append(f"case {old_paramId}:\n    {set_param_chem(by_levtype[''], new_param)}")

    code = f"""if (stream == "oper") {{ switch (param) {{
    {''.join(rules['oper'])}
    default: break; }} }} else if (stream == "gfas") {{ switch (param) {{
    {''.join(rules['gfas'])}
    default: break; }} }}"""

    write_code(M2M_DESTINATION_PATH, M2M_TEMPLATE_PATH, code)


###############################################################################
#                    mars2grib Composition Concept Matcher                    #
###############################################################################

def format_match_any(variable, values):
    """Format a matchAny() call with range() for 3+ consecutive values."""
    sorted_vals = sorted(values)
    args = []
    i = 0
    while i < len(sorted_vals):
        # Find consecutive run starting at i
        run_end = i
        while run_end + 1 < len(sorted_vals) and sorted_vals[run_end + 1] == sorted_vals[run_end] + 1:
            run_end += 1
        run_len = run_end - i + 1
        if run_len >= 3:
            args.append(f"range({sorted_vals[i]}, {sorted_vals[run_end]})")
        else:
            for j in range(i, run_end + 1):
                args.append(str(sorted_vals[j]))
        i = run_end + 1
    return f"matchAny({variable}, {', '.join(args)})"

# Group params with identical {type: chemIds} signatures so they can be merged
def signature(type_to_chems):
    return tuple(sorted((t, tuple(sorted(c))) for t, c in type_to_chems.items()))

def generateM2G():
    # Collect data into nested structure:
    # {hasWavelength: {paramId: {compositionType: set(chemIds)}}}
    data = defaultdict(lambda: defaultdict(lambda: defaultdict(set)))

    with open(MAPPINGS_PATH, 'r') as f:
        reader = csv.DictReader(f, delimiter=';')
        for row in reader:
            compositionConceptType = row['compositionConceptType']
            paramId = int(row['new_paramId'])
            chemId = int(row['chemId'])
            hasWavelength = row['wavelength'] != ''
            paramId = paramId - (paramId % 10)
            data[hasWavelength][paramId][compositionConceptType].add(chemId)

    lines = []
    indent = "    "

    for wl_idx, has_wl in enumerate([True, False]):
        wl_keyword = "if" if wl_idx == 0 else "} else"
        lines.append(f"{wl_keyword} (hasWavelength) {{" if has_wl else f"{wl_keyword} {{")

        # Group params by identical signature
        sig_to_params = defaultdict(list)
        for param_id, type_to_chems in data[has_wl].items():
            sig_to_params[signature(type_to_chems)].append(param_id)

        first_param_group = True
        for sig in sorted(sig_to_params.keys()):
            param_ids = sorted(sig_to_params[sig])
            param_cond = format_match_any("paramInstant", param_ids)
            keyword = "if" if first_param_group else "} else if"
            lines.append(f"{indent}{keyword} ({param_cond}) {{")
            first_param_group = False

            for comp_type, chem_ids in sorted(sig):
                chem_cond = format_match_any("chem", chem_ids)
                lines.append(f"{indent}{indent}if ({chem_cond}) {{")
                lines.append(f"{indent}{indent}{indent}return static_cast<std::size_t>(CompositionType::{comp_type});")
                lines.append(f"{indent}{indent}}}")

        lines.append(f"{indent}}}")

    lines.append("}")

    write_code(M2G_DESTINATION_PATH, M2G_TEMPLATE_PATH, '\n'.join(lines))


if __name__ == '__main__':
    generateM2M()
    generateM2G()

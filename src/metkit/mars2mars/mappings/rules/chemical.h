/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file.h
/// @brief Conversion rules used by the mars2mars mapper.
#pragma once

#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2mars/mappings/Mars2MarsReturnValue.h"
#include "metkit/mars2mars/mappings/rules/common.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"
#include "metkit/mars2mars/utils/paramMatcher.h"

namespace metkit::mars2mars::rules::impl {


/// @brief Assign `param`, and `chem` together.
template <class OutDict_t>
inline void setParamChem(OutDict_t& out, long param, long chem) {

    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        set_or_throw<long>(out, "param", param);
        set_or_throw<long>(out, "chem", chem);
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2marsGenericException("Failed to set param, and/or chem and/or in output dictionaries", Here()));
    }
}


/// @brief Assign `param`, `chem`, and `wavelength` together.
template <class OutDict_t>
inline void setParamChemWavelength(OutDict_t& out, long param, long chem, double wavelength) {

    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        set_or_throw<long>(out, "param", param);
        set_or_throw<long>(out, "chem", chem);
        set_or_throw<double>(out, "wavelength", wavelength);
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2marsGenericException("Failed to set param, chem and/or, wavelength in output dictionaries", Here()));
    }
}


/// @brief Convert surface-like legacy requests into sol layer output.
template <class InDict_t, class OutDict_t>
inline void convertChemical(const InDict_t& in, OutDict_t& out, eckit::LocalConfiguration& misc) {

    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        const auto param = get_or_throw<long>(in, "param");

        switch (param) {

            case 210001:
                return setParamChem(out, 402000, 901);
            case 210002:
                return setParamChem(out, 402000, 902);
            case 210003:
                return setParamChem(out, 402000, 903);
            case 210004:
                return setParamChem(out, 402000, 904);
            case 210005:
                return setParamChem(out, 402000, 905);
            case 210006:
                return setParamChem(out, 402000, 906);
            case 210007:
                return setParamChem(out, 402000, 907);
            case 210008:
                return setParamChem(out, 402000, 908);
            case 210009:
                return setParamChem(out, 402000, 909);
            case 210010:
                return setParamChem(out, 402000, 910);
            case 210011:
                return setParamChem(out, 402000, 911);
            case 210022:
                return setParamChem(out, 453000, 922);
            case 210025:
                return setParamChem(out, 453000, 922);
            case 210048:
                return setParamChem(out, 402000, 924);
            case 210071:
                return setParamChem(out, 479000, 404);
            case 210072:
                return setParamChem(out, 400000, 929);
            case 210073:
                return setParamChem(out, 400000, 930);
            case 210074:
                return setParamChem(out, 400000, 931);
            case 210081:
                return setParamChem(out, 469000, 2);
            case 210085:
                return setParamChem(out, 469000, 129);
            case 210090:
                return setParamChem(out, 469000, 934);
            case 210091:
                return setParamChem(out, 469000, 933);
            case 210102:
                return setParamChem(out, 469000, 233);
            case 210103:
                return setParamChem(out, 469000, 42);
            case 210104:
                return setParamChem(out, 469000, 46);
            case 210105:
                return setParamChem(out, 469000, 47);
            case 210106:
                return setParamChem(out, 469000, 10);
            case 210107:
                return setParamChem(out, 469000, 48);
            case 210108:
                return setParamChem(out, 469000, 16);
            case 210113:
                return setParamChem(out, 469000, 5);
            case 210115:
                return setParamChem(out, 469000, 52);
            case 210116:
                return setParamChem(out, 469000, 19);
            case 210117:
                return setParamChem(out, 469000, 18);
            case 210118:
                return setParamChem(out, 469000, 45);
            case 210121:
                return setParamChem(out, 402000, 17);
            case 210122:
                return setParamChem(out, 402000, 233);
            case 210123:
                return setParamChem(out, 402000, 2);
            case 210124:
                return setParamChem(out, 402000, 5);
            case 210125:
                return setParamChem(out, 401000, 17);
            case 210126:
                return setParamChem(out, 401000, 233);
            case 210127:
                return setParamChem(out, 401000, 2);
            case 210128:
                return setParamChem(out, 401000, 5);
            case 210170:
                return setParamChem(out, 402000, 8);
            case 210181:
                return setParamChem(out, 402000, 11);
            case 210183:
                return setParamChem(out, 401000, 11);
            case 210203:
                return setParamChem(out, 402000, 236);
            case 210206:
                return setParamChem(out, 401000, 231);
            case 210207:
                return setParamChemWavelength(out, 457000, 922, 550.0);
            case 210208:
                return setParamChemWavelength(out, 457000, 936, 550.0);
            case 210209:
                return setParamChemWavelength(out, 457000, 935, 550.0);
            case 210210:
                return setParamChemWavelength(out, 457000, 934, 550.0);
            case 210211:
                return setParamChemWavelength(out, 457000, 933, 550.0);
            case 210212:
                return setParamChemWavelength(out, 457000, 911, 550.0);
            case 210213:
                return setParamChemWavelength(out, 457000, 922, 469.0);
            case 210214:
                return setParamChemWavelength(out, 457000, 922, 670.0);
            case 210215:
                return setParamChemWavelength(out, 457000, 922, 865.0);
            case 210216:
                return setParamChemWavelength(out, 457000, 922, 1240.0);
            case 210217:
                return setParamChemWavelength(out, 457000, 922, 340.0);
            case 210218:
                return setParamChemWavelength(out, 457000, 922, 355.0);
            case 210219:
                return setParamChemWavelength(out, 457000, 922, 380.0);
            case 210220:
                return setParamChemWavelength(out, 457000, 922, 400.0);
            case 210221:
                return setParamChemWavelength(out, 457000, 922, 440.0);
            case 210222:
                return setParamChemWavelength(out, 457000, 922, 500.0);
            case 210223:
                return setParamChemWavelength(out, 457000, 922, 532.0);
            case 210224:
                return setParamChemWavelength(out, 457000, 922, 645.0);
            case 210225:
                return setParamChemWavelength(out, 457000, 922, 800.0);
            case 210226:
                return setParamChemWavelength(out, 457000, 922, 858.0);
            case 210227:
                return setParamChemWavelength(out, 457000, 922, 1020.0);
            case 210228:
                return setParamChemWavelength(out, 457000, 922, 1064.0);
            case 210229:
                return setParamChemWavelength(out, 457000, 922, 1640.0);
            case 210230:
                return setParamChemWavelength(out, 457000, 922, 2130.0);
            case 210247:
                return setParamChem(out, 402000, 912);
            case 210248:
                return setParamChem(out, 402000, 913);
            case 210249:
                return setParamChem(out, 402000, 21);
            case 210250:
                return setParamChemWavelength(out, 457000, 900, 550.0);
            case 210251:
                return setParamChemWavelength(out, 457000, 914, 550.0);
            case 210252:
                return setParamChem(out, 402000, 915);
            case 210253:
                return setParamChem(out, 402000, 916);
            case 211001:
                return setParamChem(out, 402000, 901);
            case 211002:
                return setParamChem(out, 402000, 902);
            case 211003:
                return setParamChem(out, 402000, 903);
            case 211004:
                return setParamChem(out, 402000, 904);
            case 211005:
                return setParamChem(out, 402000, 905);
            case 211006:
                return setParamChem(out, 402000, 906);
            case 211007:
                return setParamChem(out, 402000, 907);
            case 211008:
                return setParamChem(out, 402000, 908);
            case 211009:
                return setParamChem(out, 402000, 909);
            case 211010:
                return setParamChem(out, 402000, 910);
            case 211011:
                return setParamChem(out, 402000, 911);
            case 211048:
                return setParamChem(out, 402000, 924);
            case 211121:
                return setParamChem(out, 402000, 17);
            case 211122:
                return setParamChem(out, 402000, 233);
            case 211123:
                return setParamChem(out, 402000, 2);
            case 211124:
                return setParamChem(out, 402000, 5);
            case 211170:
                return setParamChem(out, 402000, 8);
            case 211203:
                return setParamChem(out, 402000, 236);
            case 211247:
                return setParamChem(out, 402000, 900);
            case 211248:
                return setParamChem(out, 402000, 903);
            case 211249:
                return setParamChem(out, 402000, 914);
            case 211252:
                return setParamChem(out, 402000, 915);
            case 211253:
                return setParamChem(out, 402000, 916);
            case 215001:
                return setParamChem(out, 453000, 901);
            case 215002:
                return setParamChem(out, 453000, 902);
            case 215003:
                return setParamChem(out, 453000, 903);
            case 215004:
                return setParamChem(out, 406000, 901);
            case 215005:
                return setParamChem(out, 406000, 902);
            case 215006:
                return setParamChem(out, 406000, 903);
            case 215007:
                return setParamChem(out, 407000, 901);
            case 215008:
                return setParamChem(out, 407000, 902);
            case 215009:
                return setParamChem(out, 407000, 903);
            case 215010:
                return setParamChem(out, 410000, 901);
            case 215011:
                return setParamChem(out, 410000, 902);
            case 215012:
                return setParamChem(out, 410000, 903);
            case 215013:
                return setParamChem(out, 411000, 901);
            case 215014:
                return setParamChem(out, 411000, 902);
            case 215015:
                return setParamChem(out, 411000, 903);
            case 215016:
                return setParamChem(out, 451000, 901);
            case 215017:
                return setParamChem(out, 451000, 902);
            case 215018:
                return setParamChem(out, 451000, 903);
            case 215019:
                return setParamChem(out, 401000, 901);
            case 215020:
                return setParamChem(out, 401000, 902);
            case 215021:
                return setParamChem(out, 401000, 903);
            case 215022:
                return setParamChemWavelength(out, 457000, 901, 550.0);
            case 215023:
                return setParamChemWavelength(out, 457000, 902, 550.0);
            case 215024:
                return setParamChemWavelength(out, 457000, 903, 550.0);
            case 215025:
                return setParamChem(out, 453000, 904);
            case 215026:
                return setParamChem(out, 453000, 905);
            case 215027:
                return setParamChem(out, 453000, 906);
            case 215028:
                return setParamChem(out, 406000, 904);
            case 215029:
                return setParamChem(out, 406000, 905);
            case 215030:
                return setParamChem(out, 406000, 906);
            case 215031:
                return setParamChem(out, 407000, 904);
            case 215032:
                return setParamChem(out, 407000, 905);
            case 215033:
                return setParamChem(out, 407000, 906);
            case 215034:
                return setParamChem(out, 410000, 904);
            case 215035:
                return setParamChem(out, 410000, 905);
            case 215036:
                return setParamChem(out, 410000, 906);
            case 215037:
                return setParamChem(out, 411000, 904);
            case 215038:
                return setParamChem(out, 411000, 905);
            case 215039:
                return setParamChem(out, 411000, 906);
            case 215040:
                return setParamChem(out, 451000, 904);
            case 215041:
                return setParamChem(out, 451000, 905);
            case 215042:
                return setParamChem(out, 451000, 906);
            case 215043:
                return setParamChem(out, 401000, 904);
            case 215044:
                return setParamChem(out, 401000, 905);
            case 215045:
                return setParamChem(out, 401000, 906);
            case 215046:
                return setParamChemWavelength(out, 457000, 904, 550.0);
            case 215047:
                return setParamChemWavelength(out, 457000, 905, 550.0);
            case 215048:
                return setParamChemWavelength(out, 457000, 906, 550.0);
            case 215049:
                return setParamChem(out, 453000, 908);
            case 215050:
                return setParamChem(out, 453000, 907);
            case 215051:
                return setParamChem(out, 406000, 908);
            case 215052:
                return setParamChem(out, 406000, 907);
            case 215053:
                return setParamChem(out, 407000, 908);
            case 215054:
                return setParamChem(out, 407000, 907);
            case 215055:
                return setParamChem(out, 410000, 908);
            case 215056:
                return setParamChem(out, 410000, 907);
            case 215057:
                return setParamChem(out, 411000, 908);
            case 215058:
                return setParamChem(out, 411000, 907);
            case 215059:
                return setParamChem(out, 451000, 908);
            case 215060:
                return setParamChem(out, 451000, 907);
            case 215061:
                return setParamChem(out, 401000, 908);
            case 215062:
                return setParamChem(out, 401000, 907);
            case 215063:
                return setParamChemWavelength(out, 457000, 908, 550.0);
            case 215064:
                return setParamChemWavelength(out, 457000, 907, 550.0);
            case 215065:
                return setParamChem(out, 453000, 910);
            case 215066:
                return setParamChem(out, 453000, 909);
            case 215067:
                return setParamChem(out, 406000, 910);
            case 215068:
                return setParamChem(out, 406000, 909);
            case 215069:
                return setParamChem(out, 407000, 910);
            case 215070:
                return setParamChem(out, 407000, 909);
            case 215071:
                return setParamChem(out, 410000, 910);
            case 215072:
                return setParamChem(out, 410000, 909);
            case 215073:
                return setParamChem(out, 411000, 910);
            case 215074:
                return setParamChem(out, 411000, 909);
            case 215075:
                return setParamChem(out, 451000, 910);
            case 215076:
                return setParamChem(out, 451000, 909);
            case 215077:
                return setParamChem(out, 401000, 910);
            case 215078:
                return setParamChem(out, 401000, 909);
            case 215079:
                return setParamChemWavelength(out, 457000, 910, 550.0);
            case 215080:
                return setParamChemWavelength(out, 457000, 909, 550.0);
            case 215081:
                return setParamChem(out, 453000, 911);
            case 215082:
                return setParamChem(out, 406000, 911);
            case 215083:
                return setParamChem(out, 407000, 911);
            case 215084:
                return setParamChem(out, 410000, 911);
            case 215085:
                return setParamChem(out, 411000, 911);
            case 215086:
                return setParamChem(out, 451000, 911);
            case 215087:
                return setParamChem(out, 401000, 911);
            case 215088:
                return setParamChemWavelength(out, 457000, 911, 550.0);
            case 215093:
                return setParamChemWavelength(out, 457000, 922, 532.0);
            case 215094:
                return setParamChemWavelength(out, 457000, 904, 532.0);
            case 215095:
                return setParamChemWavelength(out, 457000, 910, 532.0);
            case 215096:
                return setParamChemWavelength(out, 472000, 922, 340.0);
            case 215097:
                return setParamChemWavelength(out, 472000, 922, 355.0);
            case 215098:
                return setParamChemWavelength(out, 472000, 922, 380.0);
            case 215099:
                return setParamChemWavelength(out, 472000, 922, 400.0);
            case 215100:
                return setParamChemWavelength(out, 472000, 922, 440.0);
            case 215101:
                return setParamChemWavelength(out, 472000, 922, 469.0);
            case 215102:
                return setParamChemWavelength(out, 472000, 922, 500.0);
            case 215103:
                return setParamChemWavelength(out, 472000, 922, 532.0);
            case 215104:
                return setParamChemWavelength(out, 472000, 922, 550.0);
            case 215105:
                return setParamChemWavelength(out, 472000, 922, 645.0);
            case 215106:
                return setParamChemWavelength(out, 472000, 922, 670.0);
            case 215107:
                return setParamChemWavelength(out, 472000, 922, 800.0);
            case 215108:
                return setParamChemWavelength(out, 472000, 922, 858.0);
            case 215109:
                return setParamChemWavelength(out, 472000, 922, 865.0);
            case 215110:
                return setParamChemWavelength(out, 472000, 922, 1020.0);
            case 215111:
                return setParamChemWavelength(out, 472000, 922, 1064.0);
            case 215112:
                return setParamChemWavelength(out, 472000, 922, 1240.0);
            case 215113:
                return setParamChemWavelength(out, 472000, 922, 1640.0);
            case 215114:
                return setParamChemWavelength(out, 457000, 923, 340.0);
            case 215115:
                return setParamChemWavelength(out, 457000, 923, 355.0);
            case 215116:
                return setParamChemWavelength(out, 457000, 923, 380.0);
            case 215117:
                return setParamChemWavelength(out, 457000, 923, 400.0);
            case 215118:
                return setParamChemWavelength(out, 457000, 923, 440.0);
            case 215119:
                return setParamChemWavelength(out, 457000, 923, 469.0);
            case 215120:
                return setParamChemWavelength(out, 457000, 923, 500.0);
            case 215121:
                return setParamChemWavelength(out, 457000, 923, 532.0);
            case 215122:
                return setParamChemWavelength(out, 457000, 923, 550.0);
            case 215123:
                return setParamChemWavelength(out, 457000, 923, 645.0);
            case 215124:
                return setParamChemWavelength(out, 457000, 923, 670.0);
            case 215125:
                return setParamChemWavelength(out, 457000, 923, 800.0);
            case 215126:
                return setParamChemWavelength(out, 457000, 923, 858.0);
            case 215127:
                return setParamChemWavelength(out, 457000, 923, 865.0);
            case 215128:
                return setParamChemWavelength(out, 457000, 923, 1020.0);
            case 215129:
                return setParamChemWavelength(out, 457000, 923, 1064.0);
            case 215130:
                return setParamChemWavelength(out, 457000, 923, 1240.0);
            case 215131:
                return setParamChemWavelength(out, 457000, 923, 1640.0);
            case 215132:
                return setParamChemWavelength(out, 458000, 922, 340.0);
            case 215133:
                return setParamChemWavelength(out, 458000, 922, 355.0);
            case 215134:
                return setParamChemWavelength(out, 458000, 922, 380.0);
            case 215135:
                return setParamChemWavelength(out, 458000, 922, 400.0);
            case 215136:
                return setParamChemWavelength(out, 458000, 922, 440.0);
            case 215137:
                return setParamChemWavelength(out, 458000, 922, 469.0);
            case 215138:
                return setParamChemWavelength(out, 458000, 922, 500.0);
            case 215139:
                return setParamChemWavelength(out, 458000, 922, 532.0);
            case 215140:
                return setParamChemWavelength(out, 458000, 922, 550.0);
            case 215141:
                return setParamChemWavelength(out, 458000, 922, 645.0);
            case 215142:
                return setParamChemWavelength(out, 458000, 922, 670.0);
            case 215143:
                return setParamChemWavelength(out, 458000, 922, 800.0);
            case 215144:
                return setParamChemWavelength(out, 458000, 922, 858.0);
            case 215145:
                return setParamChemWavelength(out, 458000, 922, 865.0);
            case 215146:
                return setParamChemWavelength(out, 458000, 922, 1020.0);
            case 215147:
                return setParamChemWavelength(out, 458000, 922, 1064.0);
            case 215148:
                return setParamChemWavelength(out, 458000, 922, 1240.0);
            case 215149:
                return setParamChemWavelength(out, 458000, 922, 1640.0);
            case 215150:
                return setParamChemWavelength(out, 459000, 922, 340.0);
            case 215151:
                return setParamChemWavelength(out, 459000, 922, 355.0);
            case 215152:
                return setParamChemWavelength(out, 459000, 922, 380.0);
            case 215153:
                return setParamChemWavelength(out, 459000, 922, 400.0);
            case 215154:
                return setParamChemWavelength(out, 459000, 922, 440.0);
            case 215155:
                return setParamChemWavelength(out, 459000, 922, 469.0);
            case 215156:
                return setParamChemWavelength(out, 459000, 922, 500.0);
            case 215157:
                return setParamChemWavelength(out, 459000, 922, 532.0);
            case 215158:
                return setParamChemWavelength(out, 459000, 922, 550.0);
            case 215159:
                return setParamChemWavelength(out, 459000, 922, 645.0);
            case 215160:
                return setParamChemWavelength(out, 459000, 922, 670.0);
            case 215161:
                return setParamChemWavelength(out, 459000, 922, 800.0);
            case 215162:
                return setParamChemWavelength(out, 459000, 922, 858.0);
            case 215163:
                return setParamChemWavelength(out, 459000, 922, 865.0);
            case 215164:
                return setParamChemWavelength(out, 459000, 922, 1020.0);
            case 215165:
                return setParamChemWavelength(out, 459000, 922, 1064.0);
            case 215166:
                return setParamChemWavelength(out, 459000, 922, 1240.0);
            case 215167:
                return setParamChemWavelength(out, 459000, 922, 1640.0);
            case 215176:
                return setParamChemWavelength(out, 472000, 922, 2130.0);
            case 215177:
                return setParamChemWavelength(out, 457000, 923, 2130.0);
            case 215178:
                return setParamChemWavelength(out, 458000, 922, 2130.0);
            case 215179:
                return setParamChemWavelength(out, 459000, 922, 2130.0);
            case 215180:
                return setParamChemWavelength(out, 462000, 922, 355.0);
            case 215181:
                return setParamChemWavelength(out, 462000, 922, 532.0);
            case 215182:
                return setParamChemWavelength(out, 462000, 922, 1064.0);
            case 215183:
                return setParamChemWavelength(out, 460000, 922, 355.0);
            case 215184:
                return setParamChemWavelength(out, 460000, 922, 532.0);
            case 215185:
                return setParamChemWavelength(out, 460000, 922, 1064.0);
            case 215186:
                return setParamChemWavelength(out, 461000, 922, 355.0);
            case 215187:
                return setParamChemWavelength(out, 461000, 922, 532.0);
            case 215188:
                return setParamChemWavelength(out, 461000, 922, 1064.0);
            case 215189:
                return setParamChem(out, 453000, 912);
            case 215190:
                return setParamChem(out, 453000, 913);
            case 215191:
                return setParamChem(out, 406000, 912);
            case 215192:
                return setParamChem(out, 406000, 913);
            case 215193:
                return setParamChem(out, 407000, 912);
            case 215194:
                return setParamChem(out, 407000, 913);
            case 215195:
                return setParamChem(out, 410000, 912);
            case 215196:
                return setParamChem(out, 410000, 913);
            case 215197:
                return setParamChem(out, 411000, 912);
            case 215198:
                return setParamChem(out, 411000, 913);
            case 215199:
                return setParamChem(out, 451000, 912);
            case 215200:
                return setParamChem(out, 451000, 913);
            case 215201:
                return setParamChem(out, 401000, 912);
            case 215202:
                return setParamChem(out, 401000, 913);
            case 215203:
                return setParamChemWavelength(out, 457000, 912, 550.0);
            case 215204:
                return setParamChemWavelength(out, 457000, 913, 550.0);
            case 215205:
                return setParamChem(out, 453000, 914);
            case 215206:
                return setParamChem(out, 406000, 914);
            case 215207:
                return setParamChem(out, 407000, 914);
            case 215208:
                return setParamChem(out, 410000, 914);
            case 215209:
                return setParamChem(out, 411000, 914);
            case 215210:
                return setParamChem(out, 451000, 914);
            case 215211:
                return setParamChem(out, 401000, 914);
            case 215212:
                return setParamChem(out, 453000, 915);
            case 215213:
                return setParamChem(out, 406000, 915);
            case 215214:
                return setParamChem(out, 407000, 915);
            case 215215:
                return setParamChem(out, 410000, 915);
            case 215216:
                return setParamChem(out, 411000, 915);
            case 215217:
                return setParamChem(out, 451000, 915);
            case 215218:
                return setParamChem(out, 401000, 915);
            case 215219:
                return setParamChem(out, 453000, 916);
            case 215220:
                return setParamChem(out, 406000, 916);
            case 215221:
                return setParamChem(out, 407000, 916);
            case 215222:
                return setParamChem(out, 410000, 916);
            case 215223:
                return setParamChem(out, 411000, 916);
            case 215224:
                return setParamChem(out, 451000, 916);
            case 215225:
                return setParamChem(out, 401000, 916);
            case 215226:
                return setParamChemWavelength(out, 457000, 918, 550.0);
            case 217003:
                return setParamChem(out, 402000, 3);
            case 217004:
                return setParamChem(out, 402000, 4);
            case 217006:
                return setParamChem(out, 402000, 6);
            case 217007:
                return setParamChem(out, 402000, 7);
            case 217009:
                return setParamChem(out, 402000, 9);
            case 217010:
                return setParamChem(out, 402000, 10);
            case 217011:
                return setParamChem(out, 402000, 311);
            case 217012:
                return setParamChem(out, 402000, 12);
            case 217013:
                return setParamChem(out, 402000, 13);
            case 217014:
                return setParamChem(out, 402000, 14);
            case 217015:
                return setParamChem(out, 402000, 15);
            case 217016:
                return setParamChem(out, 402000, 16);
            case 217018:
                return setParamChem(out, 402000, 18);
            case 217019:
                return setParamChem(out, 402000, 19);
            case 217020:
                return setParamChem(out, 402000, 20);
            case 217021:
                return setParamChem(out, 402000, 21);
            case 217022:
                return setParamChem(out, 402000, 22);
            case 217023:
                return setParamChem(out, 402000, 23);
            case 217024:
                return setParamChem(out, 402000, 24);
            case 217026:
                return setParamChem(out, 402000, 26);
            case 217027:
                return setParamChem(out, 402000, 27);
            case 217028:
                return setParamChem(out, 402000, 28);
            case 217029:
                return setParamChem(out, 402000, 29);
            case 217030:
                return setParamChem(out, 402000, 30);
            case 217032:
                return setParamChem(out, 402000, 32);
            case 217033:
                return setParamChem(out, 402000, 33);
            case 217034:
                return setParamChem(out, 402000, 34);
            case 217035:
                return setParamChem(out, 402000, 35);
            case 217036:
                return setParamChem(out, 402000, 36);
            case 217037:
                return setParamChem(out, 402000, 37);
            case 217038:
                return setParamChem(out, 402000, 38);
            case 217039:
                return setParamChem(out, 402000, 39);
            case 217040:
                return setParamChem(out, 402000, 40);
            case 217041:
                return setParamChem(out, 402000, 41);
            case 217042:
                return setParamChem(out, 402000, 42);
            case 217043:
                return setParamChem(out, 402000, 43);
            case 217044:
                return setParamChem(out, 402000, 44);
            case 217045:
                return setParamChem(out, 402000, 45);
            case 217046:
                return setParamChem(out, 402000, 46);
            case 217047:
                return setParamChem(out, 402000, 47);
            case 217048:
                return setParamChem(out, 402000, 48);
            case 217049:
                return setParamChem(out, 402000, 49);
            case 217050:
                return setParamChem(out, 402000, 50);
            case 217051:
                return setParamChem(out, 402000, 900);
            case 217052:
                return setParamChem(out, 402000, 52);
            case 217053:
                return setParamChem(out, 402000, 53);
            case 217054:
                return setParamChem(out, 402000, 112);
            case 217055:
                return setParamChem(out, 402000, 55);
            case 217056:
                return setParamChem(out, 402000, 56);
            case 217057:
                return setParamChem(out, 402000, 57);
            case 217058:
                return setParamChem(out, 402000, 58);
            case 217059:
                return setParamChem(out, 402000, 59);
            case 217063:
                return setParamChem(out, 402000, 63);
            case 217064:
                return setParamChem(out, 402000, 64);
            case 217065:
                return setParamChem(out, 402000, 65);
            case 217066:
                return setParamChem(out, 402000, 66);
            case 217067:
                return setParamChem(out, 402000, 67);
            case 217068:
                return setParamChem(out, 402000, 68);
            case 217069:
                return setParamChem(out, 402000, 69);
            case 217070:
                return setParamChem(out, 402000, 70);
            case 217071:
                return setParamChem(out, 402000, 71);
            case 217072:
                return setParamChem(out, 402000, 72);
            case 217073:
                return setParamChem(out, 402000, 73);
            case 217074:
                return setParamChem(out, 402000, 74);
            case 217075:
                return setParamChem(out, 402000, 75);
            case 217076:
                return setParamChem(out, 402000, 76);
            case 217077:
                return setParamChem(out, 402000, 77);
            case 217078:
                return setParamChem(out, 402000, 78);
            case 217079:
                return setParamChem(out, 402000, 79);
            case 217080:
                return setParamChem(out, 402000, 80);
            case 217082:
                return setParamChem(out, 402000, 82);
            case 217083:
                return setParamChem(out, 402000, 83);
            case 217085:
                return setParamChem(out, 402000, 85);
            case 217086:
                return setParamChem(out, 402000, 86);
            case 217099:
                return setParamChem(out, 402000, 99);
            case 217100:
                return setParamChem(out, 402000, 100);
            case 217101:
                return setParamChem(out, 402000, 101);
            case 217107:
                return setParamChem(out, 402000, 107);
            case 217118:
                return setParamChem(out, 402000, 118);
            case 217159:
                return setParamChem(out, 402000, 159);
            case 217161:
                return setParamChem(out, 402000, 161);
            case 217169:
                return setParamChem(out, 402000, 169);
            case 217173:
                return setParamChem(out, 402000, 173);
            case 217174:
                return setParamChem(out, 402000, 174);
            case 217175:
                return setParamChem(out, 402000, 175);
            case 217176:
                return setParamChem(out, 402000, 176);
            case 217177:
                return setParamChem(out, 402000, 177);
            case 217178:
                return setParamChem(out, 402000, 178);
            case 217186:
                return setParamChem(out, 402000, 186);
            case 217187:
                return setParamChem(out, 402000, 187);
            case 217188:
                return setParamChem(out, 402000, 188);
            case 217189:
                return setParamChem(out, 402000, 189);
            case 217190:
                return setParamChem(out, 402000, 190);
            case 217191:
                return setParamChem(out, 402000, 191);
            case 217192:
                return setParamChem(out, 402000, 192);
            case 217193:
                return setParamChem(out, 402000, 193);
            case 217194:
                return setParamChem(out, 402000, 194);
            case 217195:
                return setParamChem(out, 402000, 195);
            case 217196:
                return setParamChem(out, 402000, 196);
            case 217197:
                return setParamChem(out, 402000, 197);
            case 217198:
                return setParamChem(out, 402000, 198);
            case 217199:
                return setParamChem(out, 402000, 199);
            case 217200:
                return setParamChem(out, 402000, 200);
            case 217201:
                return setParamChem(out, 402000, 201);
            case 217202:
                return setParamChem(out, 402000, 202);
            case 217203:
                return setParamChem(out, 402000, 203);
            case 217204:
                return setParamChem(out, 402000, 204);
            case 217206:
                return setParamChem(out, 402000, 917);
            case 217222:
                return setParamChem(out, 402000, 222);
            case 217224:
                return setParamChem(out, 402000, 224);
            case 217225:
                return setParamChem(out, 402000, 225);
            case 217226:
                return setParamChem(out, 402000, 226);
            case 217227:
                return setParamChem(out, 402000, 227);
            case 217228:
                return setParamChem(out, 402000, 228);
            case 217229:
                return setParamChem(out, 402000, 229);
            case 217230:
                return setParamChem(out, 402000, 230);
            case 218003:
                return setParamChem(out, 401000, 3);
            case 218004:
                return setParamChem(out, 401000, 404);
            case 218006:
                return setParamChem(out, 401000, 6);
            case 218007:
                return setParamChem(out, 401000, 7);
            case 218009:
                return setParamChem(out, 401000, 9);
            case 218010:
                return setParamChem(out, 401000, 10);
            case 218011:
                return setParamChem(out, 401000, 311);
            case 218012:
                return setParamChem(out, 401000, 12);
            case 218013:
                return setParamChem(out, 401000, 13);
            case 218014:
                return setParamChem(out, 401000, 14);
            case 218015:
                return setParamChem(out, 401000, 15);
            case 218016:
                return setParamChem(out, 401000, 16);
            case 218018:
                return setParamChem(out, 401000, 18);
            case 218019:
                return setParamChem(out, 401000, 19);
            case 218020:
                return setParamChem(out, 401000, 20);
            case 218021:
                return setParamChem(out, 401000, 21);
            case 218022:
                return setParamChem(out, 401000, 22);
            case 218023:
                return setParamChem(out, 401000, 23);
            case 218024:
                return setParamChem(out, 401000, 24);
            case 218026:
                return setParamChem(out, 401000, 26);
            case 218027:
                return setParamChem(out, 401000, 27);
            case 218028:
                return setParamChem(out, 401000, 28);
            case 218029:
                return setParamChem(out, 401000, 29);
            case 218030:
                return setParamChem(out, 401000, 30);
            case 218032:
                return setParamChem(out, 401000, 32);
            case 218033:
                return setParamChem(out, 401000, 33);
            case 218034:
                return setParamChem(out, 401000, 34);
            case 218035:
                return setParamChem(out, 401000, 35);
            case 218036:
                return setParamChem(out, 401000, 36);
            case 218037:
                return setParamChem(out, 401000, 37);
            case 218038:
                return setParamChem(out, 401000, 38);
            case 218039:
                return setParamChem(out, 401000, 39);
            case 218040:
                return setParamChem(out, 401000, 40);
            case 218041:
                return setParamChem(out, 401000, 41);
            case 218042:
                return setParamChem(out, 401000, 42);
            case 218043:
                return setParamChem(out, 401000, 43);
            case 218044:
                return setParamChem(out, 401000, 44);
            case 218045:
                return setParamChem(out, 401000, 45);
            case 218046:
                return setParamChem(out, 401000, 46);
            case 218047:
                return setParamChem(out, 401000, 47);
            case 218048:
                return setParamChem(out, 401000, 48);
            case 218049:
                return setParamChem(out, 401000, 49);
            case 218050:
                return setParamChem(out, 401000, 50);
            case 218051:
                return setParamChem(out, 401000, 900);
            case 218052:
                return setParamChem(out, 401000, 52);
            case 218053:
                return setParamChem(out, 401000, 53);
            case 218054:
                return setParamChem(out, 401000, 112);
            case 218055:
                return setParamChem(out, 401000, 55);
            case 218056:
                return setParamChem(out, 401000, 56);
            case 218057:
                return setParamChem(out, 401000, 57);
            case 218058:
                return setParamChem(out, 401000, 58);
            case 218059:
                return setParamChem(out, 401000, 359);
            case 218063:
                return setParamChem(out, 401000, 63);
            case 218064:
                return setParamChem(out, 401000, 64);
            case 218065:
                return setParamChem(out, 401000, 65);
            case 218066:
                return setParamChem(out, 401000, 66);
            case 218067:
                return setParamChem(out, 401000, 67);
            case 218068:
                return setParamChem(out, 401000, 68);
            case 218069:
                return setParamChem(out, 401000, 69);
            case 218070:
                return setParamChem(out, 401000, 70);
            case 218071:
                return setParamChem(out, 401000, 71);
            case 218072:
                return setParamChem(out, 401000, 72);
            case 218073:
                return setParamChem(out, 401000, 73);
            case 218074:
                return setParamChem(out, 401000, 74);
            case 218075:
                return setParamChem(out, 401000, 75);
            case 218076:
                return setParamChem(out, 401000, 76);
            case 218077:
                return setParamChem(out, 401000, 77);
            case 218078:
                return setParamChem(out, 401000, 78);
            case 218079:
                return setParamChem(out, 401000, 79);
            case 218080:
                return setParamChem(out, 401000, 80);
            case 218082:
                return setParamChem(out, 401000, 82);
            case 218083:
                return setParamChem(out, 401000, 83);
            case 218085:
                return setParamChem(out, 401000, 85);
            case 218086:
                return setParamChem(out, 401000, 86);
            case 218099:
                return setParamChem(out, 401000, 99);
            case 218100:
                return setParamChem(out, 401000, 100);
            case 218101:
                return setParamChem(out, 401000, 101);
            case 218107:
                return setParamChem(out, 401000, 107);
            case 218117:
                return setParamChem(out, 401000, 14);
            case 218159:
                return setParamChem(out, 401000, 159);
            case 218161:
                return setParamChem(out, 401000, 161);
            case 218169:
                return setParamChem(out, 401000, 169);
            case 218173:
                return setParamChem(out, 401000, 173);
            case 218174:
                return setParamChem(out, 401000, 174);
            case 218175:
                return setParamChem(out, 401000, 175);
            case 218176:
                return setParamChem(out, 401000, 176);
            case 218177:
                return setParamChem(out, 401000, 177);
            case 218178:
                return setParamChem(out, 401000, 178);
            case 218186:
                return setParamChem(out, 401000, 186);
            case 218187:
                return setParamChem(out, 401000, 187);
            case 218188:
                return setParamChem(out, 401000, 188);
            case 218189:
                return setParamChem(out, 401000, 189);
            case 218190:
                return setParamChem(out, 401000, 190);
            case 218191:
                return setParamChem(out, 401000, 191);
            case 218192:
                return setParamChem(out, 401000, 192);
            case 218193:
                return setParamChem(out, 401000, 193);
            case 218194:
                return setParamChem(out, 401000, 194);
            case 218195:
                return setParamChem(out, 401000, 195);
            case 218196:
                return setParamChem(out, 401000, 196);
            case 218197:
                return setParamChem(out, 401000, 197);
            case 218198:
                return setParamChem(out, 401000, 198);
            case 218199:
                return setParamChem(out, 401000, 199);
            case 218200:
                return setParamChem(out, 401000, 200);
            case 218201:
                return setParamChem(out, 401000, 201);
            case 218202:
                return setParamChem(out, 401000, 202);
            case 218203:
                return setParamChem(out, 401000, 203);
            case 218204:
                return setParamChem(out, 401000, 204);
            case 218206:
                return setParamChem(out, 401000, 917);
            case 218221:
                return setParamChem(out, 401000, 8);
            case 218222:
                return setParamChem(out, 401000, 222);
            case 218224:
                return setParamChem(out, 401000, 224);
            case 218225:
                return setParamChem(out, 401000, 225);
            case 218226:
                return setParamChem(out, 401000, 226);
            case 218227:
                return setParamChem(out, 401000, 227);
            case 218228:
                return setParamChem(out, 401000, 228);
            case 218229:
                return setParamChem(out, 401000, 229);
            case 218230:
                return setParamChem(out, 401000, 230);
            case 219207:
                return setParamChem(out, 469000, 9);
            case 219208:
                return setParamChem(out, 469000, 311);
            case 219209:
                return setParamChem(out, 469000, 12);
            case 219212:
                return setParamChem(out, 469000, 99);
            case 219213:
                return setParamChem(out, 469000, 100);
            case 219219:
                return setParamChem(out, 469000, 226);
            case 219220:
                return setParamChem(out, 469000, 224);
            case 222001:
                return setParamChem(out, 445000, 236);
            case 222006:
                return setParamChem(out, 445000, 6);
            case 222013:
                return setParamChem(out, 445000, 13);
            case 222015:
                return setParamChem(out, 445000, 15);
            case 222017:
                return setParamChem(out, 445000, 8);
            case 222019:
                return setParamChem(out, 445000, 19);
            case 222027:
                return setParamChem(out, 445000, 27);
            case 222031:
                return setParamChem(out, 445000, 17);
            case 222033:
                return setParamChem(out, 445000, 33);
            case 223006:
                return setParamChem(out, 444000, 6);
            case 223013:
                return setParamChem(out, 444000, 13);
            case 223015:
                return setParamChem(out, 444000, 15);
            case 223017:
                return setParamChem(out, 444000, 8);
            case 223019:
                return setParamChem(out, 444000, 19);
            case 223026:
                return setParamChem(out, 444000, 26);
            case 223027:
                return setParamChem(out, 444000, 27);
            case 223031:
                return setParamChem(out, 444000, 17);
            case 223033:
                return setParamChem(out, 444000, 33);

            default:
                break;
        }
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2marsGenericException("Failed to convert input dictionary in convertChemical", Here()));
    }
}

}  // namespace metkit::mars2mars::rules::impl

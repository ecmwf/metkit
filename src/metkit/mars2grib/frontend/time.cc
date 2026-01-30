/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "time.h"

#include "eckit/config/LocalConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

#include "common.h"

using metkit::mars2grib::utils::dict_traits::get_or_throw;

namespace metkit::mars2grib::frontend {

void setTime(const eckit::LocalConfiguration& mars, eckit::LocalConfiguration& sections) {
    const auto param = get_or_throw<long>(mars, "param");
    if (matchAny(param, range(1, 3), 10, range(15, 18), range(21, 23), range(26, 43), 53, 54, 59, 60, 66, 67,
                 range(74, 79), range(129, 139), 141, 148, 151, 152, range(155, 157), range(159, 168), 170,
                 range(172, 174), 183, range(186, 188), 198, 203, 206, 207, range(229, 232), range(234, 236), 238,
                 range(243, 248), 3020, 3031, 3067, range(3073, 3075), 129172, range(140098, 140105), 140112, 140113,
                 range(140121, 140129), range(140131, 140134), range(140207, 140209), 140211, 140212,
                 range(140214, 140239), range(140244, 140249), range(140252, 140254), 160198, range(162059, 162063),
                 162071, 162072, 162093, 174096, 200199, range(210186, 210191), range(210200, 210202),
                 range(210262, 210264), range(213101, 213160), 228001, 228003, range(228007, 228020), 228023, 228024,
                 228029, 228032, 228037, 228038, range(228044, 228048), 228050, 228052, range(228088, 228090), 228131,
                 228132, 228141, 228164, range(228217, 228221), range(228231, 228237), 260004, 260005, 260015, 260038,
                 260048, 260109, 260121, 260123, 260132, 260199, 260242, 260255, 260260, 260289, 260290, 260292, 260293,
                 260360, 260509, 260688, 261001, 261002, range(261014, 261016), 261018, 262000, 262024, 262100, 262104,
                 262118, 262124, 262139, 262140, 262144)) {
        setPointInTime(sections);
    }
    else if (matchAny(param, 8, 9, 20, 44, 45, 47, 49, 50, 57, 58, range(142, 147), 169, range(175, 182), 189,
                      range(195, 197), 201, 202, 205, range(208, 213), 228, 239, 240, 3062, 3099, range(141101, 141105),
                      141208, 141209, 141215, 141216, 141220, 141229, 141232, 141233, 141245, range(143101, 143105),
                      143208, 143209, 143215, 143216, 143220, 143229, 143232, 143233, 143245, range(144101, 144105),
                      144208, 144209, 144215, 144216, 144220, 144229, 144232, 144233, 144245, range(145101, 145105),
                      145208, 145209, 145215, 145216, 145220, 145229, 145232, 145233, 145245, range(162100, 162113),
                      range(222001, 222256), 228004, 228005, 228021, 228022, 228129, 228130, 228143, 228144, 228216,
                      range(228226, 228228), 228251, range(231001, 231003), 231005, 231010, 231012, 231057, 231058,
                      range(233000, 233031), 235020, 235021, range(235029, 235031), range(235033, 235043),
                      range(235048, 235053), 235055, 235058, range(235077, 235080), 235083, 235084, 235087, 235088,
                      235090, 235091, 235093, 235094, 235097, 235098, 235100, 235108, range(235129, 235138), 235151,
                      235152, 235155, 235157, 235159, 235165, 235166, 235168, 235189, 235203, 235246, 235263, 235269,
                      235283, 235287, 235288, 235290, 235305, 235309, 235322, 235326, 235339, 235383, 237013, 237041,
                      237042, 237055, 237077, 237078, 237080, 237083, 237084, 237087, 237088, 237090, 237091, 237093,
                      237094, 237097, 237108, 237117, 237131, 237132, 237134, 237137, 237151, 237159,
                      range(237165, 237168), 237203, 237207, 237263, 237287, 237288, 237290, 237305, 237309, 237318,
                      237321, 237322, 237326, 238013, 238041, 238042, 238055, 238077, 238078, 238080, 238083, 238084,
                      238087, 238088, 238090, 238091, 238093, 238094, 238097, 238108, 238131, 238132, 238134, 238137,
                      238151, 238159, range(238165, 238168), 238203, 238207, 238263, 238287, 238288, 238290, 238305,
                      238309, 238322, 238326, 239041, 239042, 239077, 239078, 239080, 239083, 239084, 239087, 239088,
                      239090, 239091, 239093, 239094, 239097, 239108, 239131, 239132, 239134, 239137, 239151, 239159,
                      range(239165, 239168), 239203, 239207, 239263, 239287, 239288, 239290, 239305, 239309, 239322,
                      239326, 260259, 260682, 260683, 263024, 263107, 265024, 266024, 267024)) {
        setSinceLastPostProcessingStep(sections);
    }
    else if (matchAny(param, 228051, 228053, 260318, 260320)) {
        setFixedTimeRange(sections, "1h");
    }
    else if (matchAny(param, range(228026, 228028), 228057, 228059, 228222, 228223, 260319, 260321)) {
        setFixedTimeRange(sections, "3h");
    }
    else if (matchAny(param, range(121, 123), 228035, 228036, 228058, 228060, 228224, 228225, 260338, 260339)) {
        setFixedTimeRange(sections, "6h");
    }
    else {
        throw eckit::Exception{"No mapping exists for param \"" + std::to_string(param) + "\"", Here()};
    }
}

}  // namespace metkit::mars2grib::frontend

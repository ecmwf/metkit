#pragma once

// System include
#include <cstddef>
#include <string>

// Utils
#include "metkit/mars2grib/backend/concepts/statistics/statisticsEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/paramMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t statisticsMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    using metkit::mars2grib::util::param_matcher::matchAny;
    using metkit::mars2grib::util::param_matcher::range;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    const auto param = get_or_throw<long>(mars, "param");

    if (matchAny(param, 8, 9, 20, 44, 45, 47, 50, 57, 58, range(142, 147), 169, range(175, 182), 189, range(195, 197),
                 205, range(208, 213), 228, 239, 240, 3062, 3099, range(162100, 162113), range(222001, 222256), 228021,
                 228022, 228129, 228130, 228143, 228144, 228216, 228228, 228251, range(231001, 231003), 231005, 231010,
                 231012, 231057, 231058, range(233000, 233031), 260259)) {
        return static_cast<std::size_t>(StatisticsType::Accumulation);
    }
    if (matchAny(param, range(141101, 141105), 141208, 141209, 141215, 141216, 141220, 141229, 141232, 141233, 141245,
                 228004, 228005, 228051, 228053, range(228057, 228060), 235020, 235021, range(235029, 235031),
                 range(235033, 235043), range(235048, 235053), 235055, 235058, range(235077, 235080), 235083, 235084,
                 235087, 235088, 235090, 235091, 235093, 235094, 235097, 235098, 235100, 235108, range(235129, 235138),
                 235151, 235152, 235155, 235157, 235159, 235165, 235166, 235168, 235189, 235203, 235246, 235263, 235269,
                 235283, 235287, 235288, 235290, 235305, 235309, 235322, 235326, 235339, 235383, 263024, 263107)) {
        return static_cast<std::size_t>(StatisticsType::Average);
    }
    if (matchAny(param, 49, 121, 123, 201, range(143101, 143105), 143208, 143209, 143215, 143216, 143220, 143229,
                 143232, 143233, 143245, 228026, 228028, 228035, 228036, 228222, 228224, 228226, 237013, 237041, 237042,
                 237055, 237077, 237078, 237080, 237083, 237084, 237087, 237088, 237090, 237091, 237093, 237094, 237097,
                 237108, 237117, 237131, 237132, 237134, 237137, 237151, 237159, range(237165, 237168), 237203, 237207,
                 237263, 237287, 237288, 237290, 237305, 237309, 237318, 237321, 237322, 237326, 265024)) {
        return static_cast<std::size_t>(StatisticsType::Maximum);
    }
    if (matchAny(param, 122, 202, range(144101, 144105), 144208, 144209, 144215, 144216, 144220, 144229, 144232, 144233,
                 144245, 228027, 228223, 228225, 228227, 238013, 238041, 238042, 238055, 238077, 238078, 238080, 238083,
                 238084, 238087, 238088, 238090, 238091, 238093, 238094, 238097, 238108, 238131, 238132, 238134, 238137,
                 238151, 238159, range(238165, 238168), 238203, 238207, 238263, 238287, 238288, 238290, 238305, 238309,
                 238322, 238326, 266024)) {
        return static_cast<std::size_t>(StatisticsType::Minimum);
    }
    if (matchAny(param, 260320, 260321, 260339, 260683)) {
        return static_cast<std::size_t>(StatisticsType::Mode);
    }
    if (matchAny(param, 260318, 260319, 260338, 260682)) {
        return static_cast<std::size_t>(StatisticsType::Severity);
    }
    if (matchAny(param, range(145101, 145105), 145208, 145209, 145215, 145216, 145220, 145229, 145232, 145233, 145245,
                 239041, 239042, 239077, 239078, 239080, 239083, 239084, 239087, 239088, 239090, 239091, 239093, 239094,
                 239097, 239108, 239131, 239132, 239134, 239137, 239151, 239159, range(239165, 239168), 239203, 239207,
                 239263, 239287, 239288, 239290, 239305, 239309, 239322, 239326, 267024)) {
        return static_cast<std::size_t>(StatisticsType::StandardDeviation);
    }

    // Chemical products
    if (matchAny(param, range(228080, 228082), range(233032, 233035), range(235062, 235064))) {
        return static_cast<std::size_t>(StatisticsType::Accumulation);
    }

    // TODO: Don't handle products with timespan as non-statistical if they are not handled above!
    // if (has(mars, "timespan")) {
    //     throw utils::exceptions::Mars2GribMatcherException("MARS contains `timespan` but typeOfStatisticalProcessing
    //     is defined for param " + std::to_string(param), Here());
    // }

    return compile_time_registry_engine::MISSING;
}

}  // namespace metkit::mars2grib::backend::concepts_

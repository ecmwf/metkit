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

    if (matchAny(param, 8, 9, 20, 44, 45, 47, 48, 50, 57, 58, range(142, 147), 149, 150, 169, range(175, 182), 189,
                 range(195, 197), 205, range(208, 213), 228, 239, 240, 3062, 3099, range(162100, 162113),
                 range(222001, 222256), 228021, 228022, range(228080, 228082), 228109, 228129, 228130, 228143, 228144,
                 228216, 228228, 228251, range(231001, 231003), 231005, 231010, 231012, 231033, 231036, 231039, 231041,
                 231057, 231058, range(233000, 233035), range(235062, 235064), 235075, 236386, 236387, 260138, 260259,
                 260427, range(260652, 260655), 260693, 264900, 264904, 264905)) {
        return static_cast<std::size_t>(StatisticsType::Accumulation);
    }
    if (matchAny(param, range(141098, 141129), range(141131, 141150), range(141207, 141209), 141211, 141212,
                 range(141214, 141218), range(141220, 141239), 141244, 141245, 141249, range(141252, 141254),
                 range(228004, 228006), 228051, 228053, range(228057, 228060), range(235001, 235014),
                 range(235020, 235043), range(235045, 235053), range(235055, 235060), range(235062, 235064),
                 range(235068, 235070), 235074, range(235076, 235094), range(235097, 235103), range(235105, 235111),
                 range(235113, 235120), range(235129, 235138), 235141, range(235149, 235152), 235155, 235157, 235159,
                 235165, 235166, 235168, range(235186, 235189), 235203, 235238, range(235243, 235248), 235257, 235258,
                 range(235261, 235263), range(235269, 235383), range(235386, 235411), range(263000, 263025),
                 range(263100, 263149), range(263500, 263522), range(263900, 263909))) {
        return static_cast<std::size_t>(StatisticsType::Average);
    }
    if (matchAny(param, 49, 121, 123, 201, range(143098, 143129), range(143131, 143150), range(143207, 143209), 143211,
                 143212, range(143214, 143218), range(143220, 143239), 143244, 143245, 143249, range(143252, 143254),
                 228026, 228028, 228035, 228036, 228222, 228224, 228226, range(237001, 237014), range(237022, 237043),
                 range(237045, 237059), range(237062, 237064), range(237068, 237070), range(237077, 237080),
                 range(237083, 237094), range(237097, 237103), range(237105, 237111), 237117, 237120,
                 range(237129, 237138), 237151, 237152, 237155, 237157, 237159, range(237165, 237168), 237189, 237203,
                 237207, 237238, range(237244, 237248), 237257, 237258, range(237261, 237263), 237269,
                 range(237281, 237383), range(237386, 237397), range(237404, 237411), range(265000, 265025),
                 range(265100, 265149), range(265500, 265522), range(265900, 265909))) {
        return static_cast<std::size_t>(StatisticsType::Maximum);
    }
    if (matchAny(param, 122, 202, range(144098, 144129), range(144131, 144150), range(144207, 144209), 144211, 144212,
                 range(144214, 144218), range(144220, 144239), 144244, 144245, 144249, range(144252, 144254), 228027,
                 228223, 228225, 228227, range(238001, 238014), range(238022, 238043), range(238045, 238059),
                 range(238062, 238064), range(238068, 238070), range(238077, 238080), range(238083, 238094),
                 range(238097, 238103), range(238105, 238111), 238117, 238120, range(238129, 238138), 238151, 238152,
                 238155, 238157, 238159, range(238165, 238168), 238189, 238203, 238207, 238238, range(238244, 238248),
                 238257, 238258, range(238261, 238263), 238269, range(238281, 238383), range(238386, 238397),
                 range(238404, 238411), range(266000, 266025), range(266100, 266149), range(266500, 266522),
                 range(266900, 266909))) {
        return static_cast<std::size_t>(StatisticsType::Minimum);
    }
    if (matchAny(param, 260320, 260321, 260339, 260683)) {
        return static_cast<std::size_t>(StatisticsType::Mode);
    }
    if (matchAny(param, 260318, 260319, 260338, 260682)) {
        return static_cast<std::size_t>(StatisticsType::Severity);
    }
    if (matchAny(param, range(145098, 145129), range(145131, 145150), range(145207, 145209), 145211, 145212,
                 range(145214, 145218), range(145220, 145239), 145244, 145245, 145249, range(145252, 145254),
                 range(239001, 239014), range(239022, 239043), range(239045, 239059), range(239062, 239064),
                 range(239068, 239070), range(239077, 239080), range(239083, 239091), 239093, 239094,
                 range(239097, 239103), range(239105, 239111), 239117, 239120, range(239129, 239138), 239151, 239152,
                 239155, 239157, 239159, range(239165, 239168), 239189, 239203, 239207, 239238, range(239244, 239248),
                 239257, 239258, range(239261, 239263), 239269, range(239281, 239381), 239383, range(239386, 239397),
                 range(239404, 239411), range(267000, 267025), range(267100, 267149), range(267500, 267522),
                 range(267900, 267909))) {
        return static_cast<std::size_t>(StatisticsType::StandardDeviation);
    }

    // TODO: Don't handle products with timespan as non-statistical if they are not handled above!
    // if (has(mars, "timespan")) {
    //     throw utils::exceptions::Mars2GribMatcherException("MARS contains `timespan` but typeOfStatisticalProcessing
    //     is defined for param " + std::to_string(param), Here());
    // }

    return compile_time_registry_engine::MISSING;
}

}  // namespace metkit::mars2grib::backend::concepts_

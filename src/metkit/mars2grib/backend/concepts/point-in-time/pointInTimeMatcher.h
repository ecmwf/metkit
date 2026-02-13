#pragma once

// System include
#include <cstddef>

// Utils
#include "metkit/mars2grib/backend/concepts/point-in-time/pointInTimeEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/paramMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

template <class MarsDict_t, class OptDict_t>
std::size_t pointInTimeMatcher(const MarsDict_t& mars, const OptDict_t& opt) {

    using metkit::mars2grib::util::param_matcher::matchAny;
    using metkit::mars2grib::util::param_matcher::range;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;

    const auto param = get_or_throw<long>(mars, "param");
    if (matchAny(
            param, range(1, 3), 10, range(15, 18), range(21, 23), range(26, 43), 53, 54, 59, 60, 66, 67, range(74, 79),
            range(129, 139), 141, 148, 151, 152, range(155, 157), range(159, 168), 170, range(172, 174), 183,
            range(186, 188), 198, 203, 206, 207, range(229, 232), range(234, 236), 238, range(243, 248), 3020, 3031,
            3067, range(3073, 3075), 129172, range(131074, 131077), range(140098, 140105), 140112, 140113,
            range(140121, 140129), range(140131, 140134), range(140207, 140209), 140211, 140212, range(140214, 140239),
            range(140244, 140249), range(140252, 140254), 160198, range(162059, 162063), 162071, 162072, 162093, 174096,
            200199, range(210186, 210191), range(210200, 210202), range(210262, 210264), range(213101, 213160), 228001,
            228003, range(228007, 228020), 228023, 228024, 228029, 228032, 228037, 228038, range(228044, 228048),
            228050, 228052, range(228088, 228090), 228131, 228132, 228141, 228164, range(228217, 228221),
            range(228231, 228237), 260004, 260005, 260015, 260038, 260048, 260109, 260121, 260123, 260132, 260199,
            260242, 260255, 260260, 260289, 260290, 260292, 260293, 260360, 260509, 260688, 261001, 261002,
            range(261014, 261016), 261018, 262000, 262024, 262100, 262104, 262118, 262124, 262139, 262140, 262144)) {
        return static_cast<std::size_t>(PointInTimeType::Default);
    }

    // Wave products
    if (matchAny(param, range(140114, 140120), 140251)) {
        return static_cast<std::size_t>(PointInTimeType::Default);
    }

    // Satellite products
    if (matchAny(param, 194, range(260510, 260512))) {
        return static_cast<std::size_t>(PointInTimeType::Default);
    }

    // Chemical products
    if (matchAny(param, range(228083, 228085))) {
        return static_cast<std::size_t>(PointInTimeType::Default);
    }

    return compile_time_registry_engine::MISSING;
}

}  // namespace metkit::mars2grib::backend::concepts_

#pragma once

#include <string>
#include <unordered_map>

#include "eckit/config/LocalConfiguration.h"

#include "metkit/codes/api/CodesAPI.h"

#include "metkit/grib2mars/utils/grib2marsExceptions.h"

// One extractor header per MARS keyword
#include "metkit/grib2mars/mappings/rules/activity.h"
#include "metkit/grib2mars/mappings/rules/anoffset.h"
#include "metkit/grib2mars/mappings/rules/channel.h"
#include "metkit/grib2mars/mappings/rules/class.h"
#include "metkit/grib2mars/mappings/rules/coeffindex.h"
#include "metkit/grib2mars/mappings/rules/dataset.h"
#include "metkit/grib2mars/mappings/rules/date.h"
#include "metkit/grib2mars/mappings/rules/direction.h"
#include "metkit/grib2mars/mappings/rules/domain.h"
#include "metkit/grib2mars/mappings/rules/experiment.h"
#include "metkit/grib2mars/mappings/rules/expver.h"
#include "metkit/grib2mars/mappings/rules/frequency.h"
#include "metkit/grib2mars/mappings/rules/grid.h"
#include "metkit/grib2mars/mappings/rules/hdate.h"
#include "metkit/grib2mars/mappings/rules/ident.h"
#include "metkit/grib2mars/mappings/rules/instrument.h"
#include "metkit/grib2mars/mappings/rules/iteration.h"
#include "metkit/grib2mars/mappings/rules/leg_number.h"
#include "metkit/grib2mars/mappings/rules/levelist.h"
#include "metkit/grib2mars/mappings/rules/levtype.h"
#include "metkit/grib2mars/mappings/rules/method.h"
#include "metkit/grib2mars/mappings/rules/model.h"
#include "metkit/grib2mars/mappings/rules/number.h"
#include "metkit/grib2mars/mappings/rules/origin.h"
#include "metkit/grib2mars/mappings/rules/packing.h"
#include "metkit/grib2mars/mappings/rules/param.h"
#include "metkit/grib2mars/mappings/rules/resolution.h"
#include "metkit/grib2mars/mappings/rules/stattype.h"
#include "metkit/grib2mars/mappings/rules/step.h"
#include "metkit/grib2mars/mappings/rules/stream.h"
#include "metkit/grib2mars/mappings/rules/system.h"
#include "metkit/grib2mars/mappings/rules/time.h"
#include "metkit/grib2mars/mappings/rules/timespan.h"
#include "metkit/grib2mars/mappings/rules/truncation.h"
#include "metkit/grib2mars/mappings/rules/type.h"
#include "metkit/grib2mars/mappings/rules/wavelength.h"

namespace metkit::grib2mars::rules::impl {

namespace detail {

template <class MarsDict, class MiscDict, class OptDict_t>
using MarsExtractor = void (*)(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars,
                               MiscDict& misc, const OptDict_t& opts);

template <class MarsDict, class MiscDict, class OptDict_t>
const std::unordered_map<std::string, MarsExtractor<MarsDict, MiscDict, OptDict_t>>& extractorRegistry() {
    static const std::unordered_map<std::string, MarsExtractor<MarsDict, MiscDict, OptDict_t>> registry = {
        {"class", extractClass<MarsDict, MiscDict, OptDict_t>},
        {"stream", extractStream<MarsDict, MiscDict, OptDict_t>},
        {"type", extractType<MarsDict, MiscDict, OptDict_t>},
        {"expver", extractExpver<MarsDict, MiscDict, OptDict_t>},
        {"param", extractParam<MarsDict, MiscDict, OptDict_t>},
        {"levtype", extractLevtype<MarsDict, MiscDict, OptDict_t>},
        {"levelist", extractLevelist<MarsDict, MiscDict, OptDict_t>},
        {"frequency", extractFrequency<MarsDict, MiscDict, OptDict_t>},
        {"direction", extractDirection<MarsDict, MiscDict, OptDict_t>},
        {"ident", extractIdent<MarsDict, MiscDict, OptDict_t>},
        {"channel", extractChannel<MarsDict, MiscDict, OptDict_t>},
        {"instrument", extractInstrument<MarsDict, MiscDict, OptDict_t>},
        {"_leg_number", extractLegNumber<MarsDict, MiscDict, OptDict_t>},
        {"anoffset", extractAnoffset<MarsDict, MiscDict, OptDict_t>},
        {"number", extractNumber<MarsDict, MiscDict, OptDict_t>},
        {"grid", extractGrid<MarsDict, MiscDict, OptDict_t>},
        {"truncation", extractTruncation<MarsDict, MiscDict, OptDict_t>},
        {"packing", extractPacking<MarsDict, MiscDict, OptDict_t>},
        {"date", extractDate<MarsDict, MiscDict, OptDict_t>},
        {"hdate", extractHdate<MarsDict, MiscDict, OptDict_t>},
        {"time", extractTime<MarsDict, MiscDict, OptDict_t>},
        {"step", extractStep<MarsDict, MiscDict, OptDict_t>},
        {"system", extractSystem<MarsDict, MiscDict, OptDict_t>},
        {"method", extractMethod<MarsDict, MiscDict, OptDict_t>},
        {"origin", extractOrigin<MarsDict, MiscDict, OptDict_t>},
        {"timespan", extractTimespan<MarsDict, MiscDict, OptDict_t>},
        {"stattype", extractStatType<MarsDict, MiscDict, OptDict_t>},
        {"domain", extractDomain<MarsDict, MiscDict, OptDict_t>},
        {"activity", extractActivity<MarsDict, MiscDict, OptDict_t>},
        {"dataset", extractDataset<MarsDict, MiscDict, OptDict_t>},
        {"experiment", extractExperiment<MarsDict, MiscDict, OptDict_t>},
        {"resolution", extractResolution<MarsDict, MiscDict, OptDict_t>},
        {"model", extractModel<MarsDict, MiscDict, OptDict_t>},
        {"wavelength", extractWavelength<MarsDict, MiscDict, OptDict_t>},
        {"iteration", extractIteration<MarsDict, MiscDict, OptDict_t>},
        {"coeffindex", extractCoeffindex<MarsDict, MiscDict, OptDict_t>},
    };

    return registry;
}

template <class MarsDict, class MiscDict, class OptDict_t>
MarsExtractor<MarsDict, MiscDict, OptDict_t> resolveExtractor(const std::string& keyword) {
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        const std::unordered_map<std::string, MarsExtractor<MarsDict, MiscDict, OptDict_t>>& registry =
            extractorRegistry<MarsDict, MiscDict, OptDict_t>();

        const typename std::unordered_map<std::string, MarsExtractor<MarsDict, MiscDict, OptDict_t>>::const_iterator
            it = registry.find(keyword);

        if (it == registry.end()) {
            throw Grib2MarsGenericException("No grib2mars extractor registered for MARS keyword `" + keyword + "`",
                                            Here());
        }

        const MarsExtractor<MarsDict, MiscDict, OptDict_t> extractor = it->second;
        return extractor;
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException(
            "Failed to resolve grib2mars extractor for MARS keyword `" + keyword + "`", Here()));
    }
}

}  // namespace detail

template <class MarsDict, class MiscDict, class OptDict_t>
void extract(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars, MiscDict& misc,
             const OptDict_t& opts) {
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        const detail::MarsExtractor<MarsDict, MiscDict, OptDict_t> extractor =
            detail::resolveExtractor<MarsDict, MiscDict, OptDict_t>(keyword);

        extractor(keyword, grib, mars, misc, opts);
    }
    catch (...) {
        std::throw_with_nested(
            Grib2MarsGenericException("Failed to extract MARS keyword `" + keyword + "` from GRIB message", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl

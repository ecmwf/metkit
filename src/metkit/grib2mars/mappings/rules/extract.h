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
#include "metkit/grib2mars/mappings/rules/chem.h"
#include "metkit/grib2mars/mappings/rules/class.h"
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

template <class MarsDict, class MiscDict>
using MarsExtractor = void (*)(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars,
                               MiscDict& misc);

template <class MarsDict, class MiscDict>
const std::unordered_map<std::string, MarsExtractor<MarsDict, MiscDict>>& extractorRegistry() {
    static const std::unordered_map<std::string, MarsExtractor<MarsDict, MiscDict>> registry = {
        {"class", extractClass<MarsDict, MiscDict>},
        {"stream", extractStream<MarsDict, MiscDict>},
        {"type", extractType<MarsDict, MiscDict>},
        {"expver", extractExpver<MarsDict, MiscDict>},
        {"param", extractParam<MarsDict, MiscDict>},
        {"chem", extractChem<MarsDict, MiscDict>},
        {"levtype", extractLevtype<MarsDict, MiscDict>},
        {"levelist", extractLevelist<MarsDict, MiscDict>},
        {"frequency", extractFrequency<MarsDict, MiscDict>},
        {"direction", extractDirection<MarsDict, MiscDict>},
        {"ident", extractIdent<MarsDict, MiscDict>},
        {"channel", extractChannel<MarsDict, MiscDict>},
        {"instrument", extractInstrument<MarsDict, MiscDict>},
        {"anoffset", extractAnoffset<MarsDict, MiscDict>},
        {"number", extractNumber<MarsDict, MiscDict>},
        {"grid", extractGrid<MarsDict, MiscDict>},
        {"truncation", extractTruncation<MarsDict, MiscDict>},
        {"packing", extractPacking<MarsDict, MiscDict>},
        {"date", extractDate<MarsDict, MiscDict>},
        {"hdate", extractHdate<MarsDict, MiscDict>},
        {"time", extractTime<MarsDict, MiscDict>},
        {"step", extractStep<MarsDict, MiscDict>},
        {"system", extractSystem<MarsDict, MiscDict>},
        {"method", extractMethod<MarsDict, MiscDict>},
        {"origin", extractOrigin<MarsDict, MiscDict>},
        {"timespan", extractTimespan<MarsDict, MiscDict>},
        {"stattype", extractStatType<MarsDict, MiscDict>},
        {"domain", extractDomain<MarsDict, MiscDict>},
        {"activity", extractActivity<MarsDict, MiscDict>},
        {"dataset", extractDataset<MarsDict, MiscDict>},
        {"experiment", extractExperiment<MarsDict, MiscDict>},
        {"resolution", extractResolution<MarsDict, MiscDict>},
        {"model", extractModel<MarsDict, MiscDict>},
        {"wavelength", extractWavelength<MarsDict, MiscDict>},
    };

    return registry;
}

template <class MarsDict, class MiscDict>
MarsExtractor<MarsDict, MiscDict> resolveExtractor(const std::string& keyword) {
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        const std::unordered_map<std::string, MarsExtractor<MarsDict, MiscDict>>& registry =
            extractorRegistry<MarsDict, MiscDict>();

        const typename std::unordered_map<std::string, MarsExtractor<MarsDict, MiscDict>>::const_iterator it =
            registry.find(keyword);

        if (it == registry.end()) {
            throw Grib2MarsGenericException("No grib2mars extractor registered for MARS keyword `" + keyword + "`",
                                            Here());
        }

        const MarsExtractor<MarsDict, MiscDict> extractor = it->second;
        return extractor;
    }
    catch (...) {
        std::throw_with_nested(Grib2MarsGenericException(
            "Failed to resolve grib2mars extractor for MARS keyword `" + keyword + "`", Here()));
    }
}

}  // namespace detail

template <class MarsDict, class MiscDict>
void extract(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars, MiscDict& misc) {
    using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

    try {
        const detail::MarsExtractor<MarsDict, MiscDict> extractor =
            detail::resolveExtractor<MarsDict, MiscDict>(keyword);

        extractor(keyword, grib, mars, misc);
    }
    catch (...) {
        std::throw_with_nested(
            Grib2MarsGenericException("Failed to extract MARS keyword `" + keyword + "` from GRIB message", Here()));
    }
}

}  // namespace metkit::grib2mars::rules::impl
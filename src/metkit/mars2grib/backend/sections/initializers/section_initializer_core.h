#pragma once

#include <cstddef>
#include <utility>

namespace metkit::mars2grib::backend::sections::initializers {

// ======================================================
// Function pointer type
// ======================================================
template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
using Fn = void (*)(const MarsDict_t&, const GeoDict_t&, const ParDict_t&, const OptDict_t&, OutDict_t&);

// ======================================================
// Registry entry: (TemplateNumber, Fn)
// ======================================================
template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
using Entry = std::pair<std::size_t, Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>>;

}  // namespace metkit::mars2grib::backend::sections::initializers

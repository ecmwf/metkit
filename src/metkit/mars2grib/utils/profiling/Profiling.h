/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <cstddef>
#include "eckit/exception/Exceptions.h"

namespace metkit::mars2grib::utils::profiling {

struct NoProfileContext {
    static constexpr bool hasProfile = false;
};

template <class Cntx_t>
inline Cntx_t& callSite(Cntx_t& cntx, const eckit::CodeLocation& location) noexcept {
    if constexpr (Cntx_t::hasProfile) {
        static_assert(noexcept(cntx.callSite(location)), "Profiling hooks must be noexcept");
        cntx.callSite(location);
    }
    return cntx;
}

template <std::size_t Stage, std::size_t Section, auto Variant, class Cntx_t>
inline Cntx_t& conceptCallSite(Cntx_t& cntx, const eckit::CodeLocation& location) noexcept {
    if constexpr (Cntx_t::hasProfile) {
        static_assert(noexcept(cntx.template conceptCallSite<Stage, Section, Variant>(location)),
                      "Profiling hooks must be noexcept");
        cntx.template conceptCallSite<Stage, Section, Variant>(location);
    }
    return cntx;
}

template <std::size_t SectionNumber, std::size_t TemplateNumber, class Cntx_t>
inline Cntx_t& sectionInitializerCallSite(Cntx_t& cntx, const eckit::CodeLocation& location) noexcept {
    if constexpr (Cntx_t::hasProfile) {
        static_assert(noexcept(cntx.template sectionInitializerCallSite<SectionNumber, TemplateNumber>(location)),
                      "Profiling hooks must be noexcept");
        cntx.template sectionInitializerCallSite<SectionNumber, TemplateNumber>(location);
    }
    return cntx;
}

template <class Cntx_t>
inline void profileEnterFunction(Cntx_t& cntx, const eckit::CodeLocation& location) noexcept {
    if constexpr (Cntx_t::hasProfile) {
        static_assert(noexcept(cntx.profileEnterFunction(location)), "Profiling hooks must be noexcept");
        cntx.profileEnterFunction(location);
    }
}

template <class Cntx_t>
inline void profileExitFunction(Cntx_t& cntx, const eckit::CodeLocation& location) noexcept {
    if constexpr (Cntx_t::hasProfile) {
        static_assert(noexcept(cntx.profileExitFunction(location)), "Profiling hooks must be noexcept");
        cntx.profileExitFunction(location);
    }
}

template <std::size_t Stage, std::size_t Section, auto Variant, class Cntx_t>
inline void profileEnterConcept(Cntx_t& cntx, const eckit::CodeLocation& location) noexcept {
    if constexpr (Cntx_t::hasProfile) {
        static_assert(noexcept(cntx.template profileEnterConcept<Stage, Section, Variant>(location)),
                      "Profiling hooks must be noexcept");
        cntx.template profileEnterConcept<Stage, Section, Variant>(location);
    }
}

template <std::size_t Stage, std::size_t Section, auto Variant, class Cntx_t>
inline void profileExitConcept(Cntx_t& cntx, const eckit::CodeLocation& location) noexcept {
    if constexpr (Cntx_t::hasProfile) {
        static_assert(noexcept(cntx.template profileExitConcept<Stage, Section, Variant>(location)),
                      "Profiling hooks must be noexcept");
        cntx.template profileExitConcept<Stage, Section, Variant>(location);
    }
}

template <std::size_t SectionNumber, std::size_t TemplateNumber, class Cntx_t>
inline void profileEnterSectionInitializer(Cntx_t& cntx, const eckit::CodeLocation& location) noexcept {
    if constexpr (Cntx_t::hasProfile) {
        static_assert(noexcept(cntx.template profileEnterSectionInitializer<SectionNumber, TemplateNumber>(location)),
                      "Profiling hooks must be noexcept");
        cntx.template profileEnterSectionInitializer<SectionNumber, TemplateNumber>(location);
    }
}

template <std::size_t SectionNumber, std::size_t TemplateNumber, class Cntx_t>
inline void profileExitSectionInitializer(Cntx_t& cntx, const eckit::CodeLocation& location) noexcept {
    if constexpr (Cntx_t::hasProfile) {
        static_assert(noexcept(cntx.template profileExitSectionInitializer<SectionNumber, TemplateNumber>(location)),
                      "Profiling hooks must be noexcept");
        cntx.template profileExitSectionInitializer<SectionNumber, TemplateNumber>(location);
    }
}

}  // namespace metkit::mars2grib::utils::profiling

/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Philipp Geier
/// @date   Nov 2022

#include "Decoder.h"

namespace metkit {
namespace codes {
namespace decoder {

template<>
HandleDeleter<codes_handle>::~HandleDeleter() noexcept(true) {
    if (h_) {
        codes_handle_delete(h_);
    }
}

template<>
HandleDeleter<codes_keys_iterator>::~HandleDeleter() noexcept(true) {
    if (h_) {
        codes_keys_iterator_delete(h_);
    }
}

template<>
HandleDeleter<codes_bufr_keys_iterator>::~HandleDeleter() noexcept(true) {
    if (h_) {
        codes_bufr_keys_iterator_delete(h_);
    }
}


// Maps specific flags to Eccodes flag.
// TODO make constexpr in C++14
unsigned long metadataFilterFlagToEccodes(eckit::message::MetadataFilter f) noexcept {
    using eckit::message::MetadataFilter;
    // Perform a switch. The compiler should be able to generate an efficient table lookup.
    switch (f) {
        case MetadataFilter::AllKeys:
            return CODES_KEYS_ITERATOR_ALL_KEYS;
        case MetadataFilter::SkipReadOnly:
            return CODES_KEYS_ITERATOR_SKIP_READ_ONLY;
        case MetadataFilter::SkipOptional:
            return CODES_KEYS_ITERATOR_SKIP_OPTIONAL;
        case MetadataFilter::SkipEditionSpecific:
            return CODES_KEYS_ITERATOR_SKIP_EDITION_SPECIFIC;
        case MetadataFilter::SkipCoded:
            return CODES_KEYS_ITERATOR_SKIP_CODED;
        case MetadataFilter::SkipComputed:
            return CODES_KEYS_ITERATOR_SKIP_COMPUTED;
        case MetadataFilter::SkipDuplicates:
            return CODES_KEYS_ITERATOR_SKIP_DUPLICATES;
        case MetadataFilter::SkipFunction:
            return CODES_KEYS_ITERATOR_SKIP_FUNCTION;
        case MetadataFilter::DumpOnly:
            return CODES_KEYS_ITERATOR_DUMP_ONLY;
        default:
            return 0;
    }
}

// Maps a combined flags to an eccodes
// TODO make constexpr in C++14
unsigned long metadataFilterToEccodes(eckit::message::MetadataFilter f) noexcept {
    using eckit::message::MetadataFilter;
    return (metadataFilterFlagToEccodes(f & MetadataFilter::AllKeys))
           | (metadataFilterFlagToEccodes(f & MetadataFilter::SkipReadOnly))
           | (metadataFilterFlagToEccodes(f & MetadataFilter::SkipOptional))
           | (metadataFilterFlagToEccodes(f & MetadataFilter::SkipEditionSpecific))
           | (metadataFilterFlagToEccodes(f & MetadataFilter::SkipCoded))
           | (metadataFilterFlagToEccodes(f & MetadataFilter::SkipComputed))
           | (metadataFilterFlagToEccodes(f & MetadataFilter::SkipDuplicates))
           | (metadataFilterFlagToEccodes(f & MetadataFilter::SkipFunction))
           | (metadataFilterFlagToEccodes(f & MetadataFilter::DumpOnly));
}

}  // namespace decoder
}  // namespace codes
}  // namespace metkit


/*
 * (C) Copyright 2022- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Philipp Geier
/// @author Simon Smart
/// @date   Nov 2022

#pragma once

#include <eccodes.h>
#include <memory>

//----------------------------------------------------------------------------------------------------------------------

namespace std {

template <>
struct default_delete<codes_handle> {
    void operator()(codes_handle* h) { ::codes_handle_delete(h); }
};

template <>
struct default_delete<codes_keys_iterator> {
    void operator()(codes_keys_iterator* it) { ::codes_keys_iterator_delete(it); }
};

template <>
struct default_delete<codes_bufr_keys_iterator> {
    void operator()(codes_bufr_keys_iterator* it) { ::codes_bufr_keys_iterator_delete(it); }
};

}  // namespace std

//----------------------------------------------------------------------------------------------------------------------

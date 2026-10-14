// SPDX-FileCopyrightText: 2022- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

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

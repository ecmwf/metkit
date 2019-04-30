/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef metkit_bufr_BufrHandle_H
#define metkit_bufr_BufrHandle_H

#include <map>
#include <string>

#include "eccodes.h"

#include "eckit/filesystem/PathName.h"
#include "eckit/io/Buffer.h"
#include "eckit/log/CodeLocation.h"

namespace eckit {
class DataHandle;
}

namespace metkit {
namespace bufr {

//----------------------------------------------------------------------------------------------------------------------

void bufr_call(int code, const char* msg, const eckit::CodeLocation& where);

#define BUFR_CALL(a) metkit::bufr::bufr_call(a, #a, Here())

//----------------------------------------------------------------------------------------------------------------------

class BufrHandle : eckit::NonCopyable {
public: // methods

    using keys_t = std::map<std::string, std::string>;

    /// constructor from file path, creates codes_handle and takes ownership
    /// @note currently this only handles local paths
    explicit BufrHandle(const eckit::PathName&);

    /// constructor taking ownership of a codes_handle pointer
    BufrHandle(codes_handle*);

    /// constructor not taking ownership
    BufrHandle(codes_handle&);

    /// constructor creating a codes_handle from a buffer
    explicit BufrHandle(const eckit::Buffer&, bool copy = true);

    /// constructor creating a codes_handle from a buffer
    explicit BufrHandle(const void* buffer, size_t length, bool copy = true);

    /// destructor
    /// @pre deletes the internal codes_handle if has ownership
    ~BufrHandle() noexcept(false);

    bool hasKey(const char*) const;

    long edition() const;

    keys_t keys(const char* namespc = nullptr) const;

    void getLong(const std::string& k, long& v) const;
    void getString(const std::string& k, std::string& v) const;

private:  // methods

    void init(const char* buff, size_t len, bool copy);

private:  // members
    codes_handle* handle_ = nullptr; ///< @pre handle is alwasy valid
    codes_context* ctxt_  = nullptr; ///< we dont use this ATM but makes more readable calls
    bool owned_ = false;             ///< do we own the handle?
};

//------------------------------------------------------------------------------------------------------

class MarsBufrHandle : eckit::NonCopyable {
public: // methods

    MarsBufrHandle();

    long reportType() const;
    long reportSubType() const;
    long year() const;
    long month() const;
    long day() const;
    long hour() const;
    long minute() const;
    long second() const;

};

//------------------------------------------------------------------------------------------------------

}  // namespace bufr
}  // namespace metkit

#endif

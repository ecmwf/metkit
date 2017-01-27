/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Manuel Fuentes
/// @author Tiago Quintino

/// @date Dec 2015

#ifndef grib_GribMetaData_H
#define grib_GribMetaData_H

#include "eckit/io/Buffer.h"
#include "eckit/memory/NonCopyable.h"
#include "eckit/serialisation/Stream.h"
#include "eckit/types/Metadata.h"

namespace metkit {
namespace grib {

    class GribHandle;

//----------------------------------------------------------------------------------------------------------------------

/// Previously existed in mars-server code as metkit/GribHandle

class GribMetaData : public eckit::Metadata {

public: // methods

    GribMetaData(eckit::Stream&);
    GribMetaData(const void* buffer, size_t length);

	virtual ~GribMetaData();

// -- from Metadata

    virtual std::vector<std::string> keywords() const;

    virtual bool has(const std::string& key) const;

    virtual void get(const std::string& name, std::string& value) const;
    virtual void get(const std::string& name, long& value) const;
    virtual void get(const std::string& name, double& value) const;

    void getValue(const std::string& name, std::string& value) const;
    void getValue(const std::string& name, long& value) const;
    void getValue(const std::string& name, double& value) const;

	std::string substitute(const std::string& pattern) const;

    size_t length() const;

protected: // members

    virtual void print(std::ostream&) const;

private: // members

	typedef std::map<std::string, std::string> string_store;

	string_store stringValues_;

	std::map<std::string, long>        longValues_;
	std::map<std::string, double>      doubleValues_;

    long length_;
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace grib
} // namespace metkit

#endif

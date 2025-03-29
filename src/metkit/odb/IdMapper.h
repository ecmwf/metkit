/*
 * (C) Copyright 1996-2012 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Piotr Kuchta
/// @date April 2011
/// @author Simon Smart
/// @date April 2019

#ifndef metkit_odb_IdMapper_H
#define metkit_odb_IdMapper_H

#include <cstddef>
#include <map>
#include <set>
#include <string>

namespace eckit {
class PathName;
}


namespace metkit {
namespace odb {

//----------------------------------------------------------------------------------------------------------------------

class IdMap {

public:  // methods

    IdMap(const std::string& configFile, const std::string& fieldDelimiter = " \t", size_t numericIndex = 0,
          size_t alphanumericIndex = 1);
    ~IdMap();

    std::string alphanumeric(long numeric);

private:  // methods

    std::map<long, std::string> numeric2alpha_;
};


//----------------------------------------------------------------------------------------------------------------------

class IdMapper {

public:  // methods

    static IdMapper& instance();

    /// Returns true if modified
    bool alphanumeric(const std::string& keyword, long numeric, std::string& output);
    bool alphanumeric(const std::string& keyword, const std::set<long>& numeric, std::set<std::string>& output);

private:  // methods

    IdMapper();
    ~IdMapper();

private:  // members

    std::map<std::string, IdMap> maps_;
};

//----------------------------------------------------------------------------------------------------------------------


}  // namespace odb
}  // namespace metkit

#endif

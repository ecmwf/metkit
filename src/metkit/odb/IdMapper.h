// SPDX-FileCopyrightText: 1996-2012 European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Piotr Kuchta
/// @date April 2011
/// @author Simon Smart
/// @date April 2019

#pragma once

#include <cstddef>
#include <map>
#include <set>
#include <string>

namespace eckit {
class PathName;
}


namespace metkit::odb {

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


}  // namespace metkit::odb

/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Manuel Fuentes
/// @author Baudouin Raoult
/// @author Tiago Quintino

/// @date Sep 96

#pragma once

#include "metkit/mars/Dictionary.h"
#include "metkit/mars/MarsRequest.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class MarsParsedRequest : public MarsRequest {
public:

    MarsParsedRequest(const std::string& verb, size_t line);
    ~MarsParsedRequest() = default;

    const std::string& verb() const override;
    void getParams(std::vector<std::string>& keys) const override;
    const std::vector<std::string>& values(const std::string&, bool emptyOk = false) const override;

    void verb(const std::string& v) override;

    void values(const std::string&, const std::vector<std::string>&) override;
    void erase(const std::string& key) override;

    const std::list<StringParameter>& params() const;

    void info(std::ostream&) const;
    void dump(std::ostream&, const char* cr = "\n", const char* tab = "\t", bool verb = true) const override;

    friend std::ostream& operator<<(std::ostream& s, const MarsParsedRequest& r) {
        r.info(s);
        return s;
    }

private:

    std::optional<std::reference_wrapper<const Parameter>> find(const std::string& name) const override;
    std::optional<std::reference_wrapper<Parameter>> find(const std::string& name) override;

    std::string verb_;
    std::list<StringParameter> params_;
    std::size_t line_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars

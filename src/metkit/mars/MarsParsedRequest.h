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

#include <unordered_map>

#include "eckit/serialisation/Stream.h"

#include "metkit/mars/Dictionary.h"
#include "metkit/mars/MarsRequest.h"

namespace metkit::mars {

//----------------------------------------------------------------------------------------------------------------------

class MarsParsedRequest : public MarsRequest {
public:

    MarsParsedRequest() = default;
    MarsParsedRequest(const std::string& verb, size_t line);
    MarsParsedRequest(const MarsRequest& request);
    explicit MarsParsedRequest(eckit::Stream& s, bool lowercase = false);

    ~MarsParsedRequest() override = default;

    Verb verbId() const override;
    const std::string& verb() const override { return verb_; }

    size_t countValues(Keyword) const override;
    size_t countValues(const std::string&) const override;

    bool has(Keyword) const override;
    bool has(const std::string& name) const override;

    const std::vector<std::string>& values(Keyword, bool emptyOk = false) const override;
    const std::vector<std::string>& values(const std::string&, bool emptyOk = false) const override;

    void values(Keyword, const std::vector<std::string>&) override;
    void values(const std::string&, const std::vector<std::string>&) override;

    void setValuesTyped(const Type*, const std::vector<std::string>&) override;

    void info(std::ostream& out) const;

    const Parameter* find(Keyword) const override;
    const Parameter* find(const std::string& name) const override;

protected:

    void erase(Keyword) override;
    void erase(const std::string&) override;

private:  // members

    std::string verb_;
    std::unordered_map<std::string, size_t> paramMap_;

    std::size_t line_;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars

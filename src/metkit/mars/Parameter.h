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

#include "eckit/types/Date.h"
#include "eckit/types/Double.h"
#include "eckit/types/Time.h"
#include "eckit/utils/Translator.h"
#include "eckit/value/Value.h"

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "metkit/mars/Dictionary.h"

namespace eckit {
class JSON;
class MD5;
}  // namespace eckit

namespace metkit::mars {

class Type;

//----------------------------------------------------------------------------------------------------------------------

/// A keyword of a request, with its values.
///
/// A parameter is either:
///  - *untyped*: it only knows the name it was created with and its values. Its keyword does not need to be
///    registered in the language (e.g. custom keys), and nothing is looked up when it is created. It behaves like a
///    generic keyword: all the values are used, and several values are accepted.
///  - *typed*: it refers to the Type of the keyword in a language, which defines how the values are counted, filtered
///    and matched. Requests produced by the expansion hold typed parameters.
///
/// This is a plain value class: copying and moving are cheap and never allocate beyond the values themselves.
class Parameter {
public:  // methods

    Parameter() = default;

    /// Untyped parameter
    Parameter(const std::string& name, const std::vector<std::string>& values);
    Parameter(const std::string& name, std::vector<std::string>&& values);

    /// Typed parameter
    Parameter(std::shared_ptr<const Type> type, const std::vector<std::string>& values);
    Parameter(std::shared_ptr<const Type> type, std::vector<std::string>&& values);

    /// Parameters are ordered by name (and then by values): the ids of the keywords depend on the order in which the
    /// languages are loaded, so they cannot be used to order parameters in a way that is stable across processes.
    bool operator<(const Parameter&) const;

    /// @return the id of the keyword, or 0 if the name is not a registered keyword. It does not register anything.
    Keyword id() const;
    const std::string& name() const;

    /// @return true if the id of the keyword is @p key (the name is compared if the parameter is untyped)
    bool is(Keyword key) const;

    const std::vector<std::string>& values() const { return values_; }
    void values(const std::vector<std::string>& values) { values_ = values; }

    bool typed() const { return type_ != nullptr; }
    /// @throws SeriousBug if the parameter is untyped
    const Type& type() const;
    const std::shared_ptr<const Type>& typePtr() const { return type_; }

    bool multiple() const;
    size_t count() const;

    /// Keeps only the values that are in @p filter. @return true if some value is left
    bool filter(const std::vector<std::string>& filter);
    /// Filters by another keyword, e.g. a date by day. @return false if this keyword does not support it
    bool filter(Keyword keyword, const std::vector<std::string>& filter);
    bool filter(const std::string& name, const std::vector<std::string>& filter);

    /// @return true if at least one of the values is in @p matches
    bool matches(const std::vector<std::string>& matches) const;

    /// Adds the values of @p p that are not already there (the order is respected)
    void merge(const Parameter& p);

protected:

    void print(std::ostream& s) const;

    friend std::ostream& operator<<(std::ostream& s, const Parameter& p) {
        p.print(s);
        return s;
    }

private:  // members

    std::string name_;  // the name of an untyped parameter. Empty if the parameter is typed, the type knows its name
    std::vector<std::string> values_;
    std::shared_ptr<const Type> type_;  // null if the parameter is untyped
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars

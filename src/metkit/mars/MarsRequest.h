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

#include <cstddef>  // for size_t
#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "eckit/utils/Translator.h"
#include "eckit/value/Value.h"

#include "metkit/mars/Dictionary.h"
#include "metkit/mars/Parameter.h"

namespace eckit {
class MD5;
class Stream;
namespace message {
class Message;
}
}  // namespace eckit

struct metkit_marsrequest_t;

namespace metkit::mars {

class Type;

//----------------------------------------------------------------------------------------------------------------------

class MarsID {
public:

    const std::string& get(Keyword key) const;
    void set(Keyword key, const std::string& value);
    void unset(Keyword key);
    bool has(Keyword key) const;
    const std::unordered_map<Keyword, std::string>& values() const;

private:

    std::unordered_map<Keyword, std::string> values_;
};

//----------------------------------------------------------------------------------------------------------------------

class MarsDataCube {
public:  // methods

    const std::vector<std::string>& get(Keyword key) const;
    void set(Keyword key, const std::vector<std::string>& value);
    void unset(Keyword key);
    bool has(Keyword key) const;
    void add(const MarsID& id);
    void merge(const MarsDataCube& other);
    const std::unordered_map<Keyword, std::vector<std::string>>& values() const;

private:

    std::unordered_map<Keyword, std::vector<std::string>> values_;
};


//----------------------------------------------------------------------------------------------------------------------

/// A MARS request: a verb and a list of parameters (keywords and their values), in the order they were added.
///
/// The parameters of a request that was just parsed or created are untyped. The expansion (see MarsLanguage and
/// MarsExpansion) produces requests whose parameters are typed.
///
/// Keywords can be given by name or by id (see MarsLanguage::keyword()). The id is faster for typed parameters, which
/// know theirs. Names of registered keywords are case insensitive, custom names are compared as they are.
///
/// @note Pointers and references returned by find() and values() (and the ones to the parameters) are valid until the
/// request is modified (a value set, a parameter added or removed), or destroyed.
class MarsRequest {
public:  // methods

    MarsRequest() = default;
    explicit MarsRequest(Verb verb);
    explicit MarsRequest(const std::string& verb);
    MarsRequest(const std::string& verb, const std::map<std::string, std::string>& vals);
    MarsRequest(const std::string& verb, const eckit::Value& vals);

    /// @param validate if true, every parameter is checked against the language of the verb and typed
    /// @param lowercase if true, the verb and the names of the parameters are converted to lowercase
    explicit MarsRequest(eckit::Stream& s, bool validate = false, bool lowercase = false);

    MarsRequest(const MarsRequest& request)              = default;
    MarsRequest(MarsRequest&& other) noexcept            = default;
    MarsRequest& operator=(const MarsRequest& other)     = default;
    MarsRequest& operator=(MarsRequest&& other) noexcept = default;

    ~MarsRequest() = default;

    const std::string& operator[](const std::string&) const;

    /// @throws UserError if the verb is not a verb of the language
    Verb verbId() const;
    const std::string& verb() const { return verb_; }

    void verb(Verb id);
    void verb(const std::string& name) { verb_ = name; }

    size_t countValues(Keyword key) const;
    size_t countValues(const std::string& name) const;

    bool has(Keyword key) const;
    bool has(const std::string& name) const;

    template <class T>
    size_t getValues(Keyword, std::vector<T>& v, bool emptyOk = false) const;
    template <class T>
    size_t getValues(const std::string& name, std::vector<T>& v, bool emptyOk = false) const;

    // getters
    // Returns reference to values or nullopt if not found
    std::optional<std::reference_wrapper<const std::vector<std::string>>> get(const std::string& keyword) const;

    /// @throws UserError if there is no such parameter, unless @p emptyOk
    const std::vector<std::string>& values(Keyword key, bool emptyOk = false) const;
    const std::vector<std::string>& values(const std::string& name, bool emptyOk = false) const;

    std::vector<std::string> params() const;

    /// @deprecated use params()
    void getParams(std::vector<std::string>& p) const { p = params(); }

    const std::vector<Parameter>& parameters() const { return params_; }

    template <class T>
    void setValue(Keyword key, const T& value);
    template <class T>
    void setValue(const std::string& name, const T& value);

    // setters
    // They set the values of the parameter, which is created (untyped) if needed
    void values(Keyword key, const std::vector<std::string>& v);
    void values(const std::string& name, const std::vector<std::string>& v);

    void unsetValues(const std::string& name) { erase(name); }

    void erase(Keyword key);
    void erase(const std::string& name);

    /// Splits a MARS request into multiple requests along the indicated keys
    std::vector<MarsRequest> split(const std::vector<std::string>& keys) const;

    /// Merges one MarsRequest into another
    // parameters existing in the other request but not present in the current request will be ignored
    void merge(const MarsRequest& other);

    /// Create a new MarsRequest from this one with only the given set of keys
    MarsRequest subset(const std::set<std::string>&) const;

    void json(eckit::JSON&, bool array = false) const;

    void md5(eckit::MD5&) const;

    void dump(std::ostream& s, const char* cr = "\n", const char* tab = "\t", bool verb = true) const;

    /// Sets the values of the parameter of the type, which becomes typed
    /// @note @p type must be owned by a std::shared_ptr, or allocated with new: the request takes the ownership in
    /// the latter case
    void setValuesTyped(const Type* type, const std::vector<std::string>& values);
    void setValuesTyped(std::shared_ptr<const Type> type, const std::vector<std::string>& values);

    bool filter(const MarsRequest& filter);
    bool matches(const MarsRequest& other) const;
    bool empty() const { return params_.empty(); }

    bool operator<(const MarsRequest& other) const;

    /// The number of fields the request refers to
    size_t count() const;

    std::string asString() const;

    const Parameter* find(Keyword key) const;
    const Parameter* find(const std::string& name) const;

public:  // static methods

    static MarsRequest parse(const std::string& s, bool strict = false);
    static std::vector<MarsRequest> parse(std::istream&, bool strict = false);

    /// Implementation in api/metkit_c.cc
    static const MarsRequest& fromOpaque(const metkit_marsrequest_t* request);

private:  // methods

    void print(std::ostream& s) const;
    void encode(eckit::Stream& s) const;

    /// @return the position of the parameter, or params_.size() if there is no such parameter
    size_t locate(Keyword key) const;
    size_t locate(const std::string& name) const;

    // -- Class members

    static eckit::ClassSpec classSpec_;
    static eckit::Reanimator<MarsRequest> reanimator_;

    friend std::ostream& operator<<(std::ostream& s, const MarsRequest& r) {
        r.print(s);
        return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& s, const MarsRequest& r) {
        r.json(s);
        return s;
    }

    friend eckit::Stream& operator<<(eckit::Stream& s, const MarsRequest& r) {
        r.encode(s);
        return s;
    }

private:  // members

    std::string verb_;
    std::vector<Parameter> params_;
};

template <class T>
size_t MarsRequest::getValues(Keyword key, std::vector<T>& values, bool emptyOk) const {

    eckit::Translator<std::string, T> t;

    const auto& vv = this->values(key, emptyOk);

    values.clear();
    values.reserve(vv.size());

    for (const auto& v : vv) {
        values.push_back(t(v));
    }

    return values.size();
}
template <class T>
size_t MarsRequest::getValues(const std::string& name, std::vector<T>& val, bool emptyOk) const {

    eckit::Translator<std::string, T> t;

    const auto& vv = values(name, emptyOk);

    val.clear();
    val.reserve(vv.size());

    for (const auto& v : vv) {
        val.push_back(t(v));
    }

    return val.size();
}

template <class T>
void MarsRequest::setValue(Keyword key, const T& value) {
    eckit::Translator<T, std::string> t;
    values(key, std::vector<std::string>{t(value)});
}
template <class T>
void MarsRequest::setValue(const std::string& name, const T& value) {
    eckit::Translator<T, std::string> t;
    values(name, std::vector<std::string>{t(value)});
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars

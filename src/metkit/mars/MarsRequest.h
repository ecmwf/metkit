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

#include <optional>

#include "eckit/value/Value.h"

#include "metkit/mars/Dictionary.h"
#include "metkit/mars/Parameter.h"

namespace eckit {
class MD5;
namespace message {
class Message;
}
}  // namespace eckit

struct metkit_marsrequest_t;

namespace metkit::mars {

class Type;

class MarsRequest {
public:  // methods

    MarsRequest();

    explicit MarsRequest(Verb);
    explicit MarsRequest(const std::string&);
    explicit MarsRequest(eckit::Stream&, bool lowercase = false);

    MarsRequest(const std::string&, const std::map<std::string, std::string>&);
    MarsRequest(const std::string&, const eckit::Value&);

    explicit MarsRequest(const eckit::message::Message&);

    // MarsRequest(const MarsParsedRequest& parsed);

    virtual ~MarsRequest() = default;

    bool operator<(const MarsRequest& other) const;

    // eckit::Value&        operator[](const std::string&);
    const std::string& operator[](Keyword) const;
    const std::string& operator[](const std::string&) const;

    operator eckit::Value() const;

    virtual const std::string& verb() const;

    size_t countValues(Keyword) const;
    size_t countValues(const std::string&) const;
    bool has(Keyword) const;
    bool has(const std::string&) const;

    bool is(Keyword param, const std::string& value) const;
    bool is(const std::string& param, const std::string& value) const;

    const std::vector<std::string>& values(Keyword, bool emptyOk = false) const;
    virtual const std::vector<std::string>& values(const std::string&, bool emptyOk = false) const;

    // Returns reference to values or nullopt if not found
    std::optional<std::reference_wrapper<const std::vector<std::string>>> get(Keyword id) const;
    std::optional<std::reference_wrapper<const std::vector<std::string>>> get(const std::string& keyword) const;

    template <class T>
    size_t getValues(Keyword, std::vector<T>& v, bool emptyOk = false) const;
    template <class T>
    size_t getValues(const std::string& name, std::vector<T>& v, bool emptyOk = false) const;

    virtual void getParams(std::vector<std::string>&) const;
    std::vector<std::string> params() const;

    std::list<TypeParameter>& parameters() { return params_; }

    const std::list<TypeParameter>& parameters() const { return params_; }

    virtual void verb(const std::string&);

    void values(Keyword, const std::vector<std::string>&);
    virtual void values(const std::string&, const std::vector<std::string>&);

    template <class T>
    void setValue(const std::string& name, const T& value);
    // virtual void setValue(const std::string& name, const std::vector<std::string>& value);


    void unsetValues(Keyword);
    virtual void unsetValues(const std::string&);

    /// Splits a MARS request into multiple requests along the provided key
    std::vector<MarsRequest> split(const std::string& keys) const;

    /// Splits a MARS request into multiple requests along the indicated keys
    std::vector<MarsRequest> split(const std::vector<std::string>& keys) const;

    /// Merges one MarsRequest into another
    /// @todo Improve performance -- uses O(N^2) search / merge in std::list's
    void merge(const MarsRequest& other);

    /// Create a new MarsRequest from this one with only the given set of keys
    MarsRequest subset(const std::set<std::string>&) const;

    void json(eckit::JSON&, bool array = false) const;

    void md5(eckit::MD5&) const;

    virtual void dump(std::ostream&, const char* cr = "\n", const char* tab = "\t", bool verb = true) const;

    void setValuesTyped(const Type*, const std::vector<std::string>&);

    bool filter(const MarsRequest& filter);
    bool matches(const MarsRequest& filter) const;
    bool empty() const;

    size_t count() const;

    // MarsRequest extract(const std::string& category) const;

    void erase(Keyword);
    virtual void erase(const std::string& param);

    virtual std::string asString() const;

public:  // static methods

    static MarsRequest parse(const std::string& s, bool strict = false);
    static std::vector<MarsRequest> parse(std::istream&, bool strict = false);

    /// Implementation in api/metkit_c.cc
    static const MarsRequest& fromOpaque(const metkit_marsrequest_t* request);

protected:  // methods
    std::optional<std::reference_wrapper<const Parameter>> find(Keyword) const;
    std::optional<std::reference_wrapper<Parameter>> find(Keyword);

    virtual std::optional<std::reference_wrapper<const Parameter>> find(const std::string& name) const;
    virtual std::optional<std::reference_wrapper<Parameter>> find(const std::string& name);

private:  // methods

    void print(std::ostream&) const;
    void encode(eckit::Stream&) const;

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

    Verb verb_;
    std::list<TypeParameter> params_;
};

template <class T>
size_t MarsRequest::getValues(Keyword key, std::vector<T>& values, bool emptyOk) const {

    eckit::Translator<std::string, T> t;

    const auto& vv = values(key, emptyOk);

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
void MarsRequest::setValue(const std::string& name, const T& value) {
    eckit::Translator<T, std::string> t;
    values(name, std::vector<std::string>{t(value)});
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars

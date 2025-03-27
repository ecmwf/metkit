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

#ifndef metkit_MarsRequest_H
#define metkit_MarsRequest_H

#include "eckit/value/Value.h"
#include "metkit/mars/Parameter.h"

namespace eckit {
class MD5;
namespace message {
class Message;
}
}  // namespace eckit

struct metkit_marsrequest_t;

namespace metkit {
namespace mars {

class Type;
class MarsRequest;


//----------------------------------------------------------------------------------------------------------------------

class MarsRequest {
public:  // methods

    MarsRequest();

    explicit MarsRequest(const std::string&);
    explicit MarsRequest(eckit::Stream&, bool lowercase = false);

    MarsRequest(const std::string&, const std::map<std::string, std::string>&);
    MarsRequest(const std::string&, const eckit::Value&);

    explicit MarsRequest(const eckit::message::Message&);

    ~MarsRequest() = default;

    bool operator<(const MarsRequest& other) const;

    // eckit::Value&        operator[](const std::string&);
    const std::string& operator[](const std::string&) const;

    operator eckit::Value() const;

    const std::string& verb() const;

    size_t countValues(const std::string&) const;
    bool has(const std::string&) const;

    bool is(const std::string& param, const std::string& value) const;

    const std::vector<std::string>& values(const std::string&, bool emptyOk = false) const;

    template <class T>
    size_t getValues(const std::string& name, std::vector<T>& v, bool emptyOk = false) const;

    void getParams(std::vector<std::string>&) const;
    std::vector<std::string> params() const;

    std::list<Parameter>& parameters() { return params_; }

    const std::list<Parameter>& parameters() const { return params_; }

    void verb(const std::string&);

    void values(const std::string&, const std::vector<std::string>&);

    template <class T>
    void setValue(const std::string& name, const T& value);

    void setValue(const std::string& name, const char* value);

    void unsetValues(const std::string&);

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

    void dump(std::ostream&, const char* cr = "\n", const char* tab = "\t", bool verb = true) const;

    void setValuesTyped(Type*, const std::vector<std::string>&);

    bool filter(const MarsRequest& filter);
    bool matches(const MarsRequest& filter) const;
    bool empty() const;

    size_t count() const;

    MarsRequest extract(const std::string& category) const;

    void erase(const std::string& param);

    std::string asString() const;

public:  // static methods

    static MarsRequest parse(const std::string& s, bool strict = false);
    static std::vector<MarsRequest> parse(std::istream&, bool strict = false);

    /// Implementation in api/metkit_c.cc
    static const MarsRequest& fromOpaque(const metkit_marsrequest_t* request);

private:  // members

    std::string verb_;
    std::list<Parameter> params_;

private:  // methods

    void print(std::ostream&) const;
    void encode(eckit::Stream&) const;

    std::list<Parameter>::const_iterator find(const std::string&) const;
    std::list<Parameter>::iterator find(const std::string&);

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
};


template <class T>
size_t MarsRequest::getValues(const std::string& name, std::vector<T>& v, bool emptyOk) const {
    const std::vector<std::string>& s = values(name, emptyOk);

    eckit::Translator<std::string, T> t;

    v.clear();

    for (std::vector<std::string>::const_iterator j = s.begin(); j != s.end(); ++j) {
        v.push_back(t(*j));
    }

    return v.size();
}


template <class T>
void MarsRequest::setValue(const std::string& name, const T& value) {
    eckit::Translator<T, std::string> t;
    std::vector<std::string> v(1, t(value));
    values(name, v);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mars
}  // namespace metkit

#endif

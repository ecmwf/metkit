/*
 * (C) Copyright 1996-2017 ECMWF.
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

#include "eckit/types/Date.h"
#include "eckit/types/Double.h"
#include "eckit/types/Time.h"
#include "eckit/value/Value.h"
#include "eckit/utils/Translator.h"

namespace eckit {
class JSON;
class MD5;
}

namespace metkit {

class Type;
class MarsRequest;

class Parameter {
    Type* type_;
    std::vector<std::string> values_;
public:
    Parameter();
    ~Parameter();

    Parameter(const std::vector<std::string>& values, Type* = 0);
    Parameter(const Parameter&);
    Parameter& operator=(const Parameter&);
    bool operator<(const Parameter&) const;

    const std::vector< std::string >& values() const { return values_; }
    void values(const std::vector< std::string >& values);

    bool filter(const std::vector< std::string >& filter);
    bool matches(const std::vector< std::string >& matches) const;

    Type& type() const { return *type_; }
    const std::string& name() const;

    size_t count() const;
};

//----------------------------------------------------------------------------------------------------------------------

class MarsRequest {

public: // methods

// -- Contructors

    MarsRequest();
    MarsRequest(const std::string&);
    MarsRequest(eckit::Stream&);

    MarsRequest(const eckit::ValueMap&);

// -- Copy

    // MarsRequest(const MarsRequest&);
    // MarsRequest& operator=(const MarsRequest&);

// -- Destructor

    ~MarsRequest();

// -- Operators

    bool operator<(const MarsRequest& other) const;

    // eckit::Value&        operator[](const std::string&);
    // const eckit::Value&  operator[](const std::string&) const;

    operator eckit::Value() const;

    /* DEPRECATED METHODS */

    // These methods are required for ODB_API 0.15.x
    // to maintain temporary compatibility with metkit 0.3.0

    void name(const std::string& v) { verb(v); }
    void setValues(const std::string& name, const std::vector<std::string>& v) { values(name, v); }

    /* END DEPRECATED METHODS */

// -- Methods

    const std::string& verb() const { return verb_; }

    size_t countValues(const std::string&) const;

    bool is(const std::string& param, const std::string& value) const;

    const std::vector<std::string> &values(const std::string&, bool emptyOk = false) const;


    template<class T>
    void getValues(const std::string& name, std::vector<T>& v) const {
        const std::vector< std::string >& s = values(name);

        eckit::Translator<std::string, T> t;

        v.clear();

        for (std::vector<std::string>::const_iterator j = s.begin(); j != s.end(); ++j) {
            v.push_back(t(*j));
        }
    }


    void getParams(std::vector<std::string>&) const;
    std::vector<std::string> params() const;

    void verb(const std::string&);

    void values(const std::string&, const std::vector<std::string>&);

    template<class T>
    void setValue(const std::string& name, const T& value)
    { std::vector<T> v(1, value); values(name, v); }

    void setValue(const std::string& name, const char* value)
    { std::string v(value); setValue(name, v); }

    void unsetValues(const std::string&);

    /// Merges one MarsRequest into another
    /// @todo Improve performance -- uses O(N^2) search / merge in std::list's
    void merge(const MarsRequest& other);

    /// Create a new MarsRequest from this one with only the given set of keys
    MarsRequest subset(const std::set<std::string>&);

    void json(eckit::JSON&) const;

    void md5(eckit::MD5&) const;

    void dump(std::ostream&, const char* cr = "\n", const char* tab = "\t") const;

    void setValuesTyped(Type*, const std::vector<std::string>&);

    bool filter(const MarsRequest& filter);
    bool matches(const MarsRequest& filter) const;
    bool empty() const;

    size_t count() const;


private: // members

    std::string         verb_;
    std::list<Parameter> params_;

private: // methods

    void print(std::ostream&) const;
    void encode(eckit::Stream&) const;

    std::list<Parameter>::const_iterator find(const std::string&) const;
    std::list<Parameter>::iterator find(const std::string&);

// -- Class members

    static eckit::ClassSpec                 classSpec_;
    static eckit::Reanimator<MarsRequest>   reanimator_;

    friend std::ostream& operator<<(std::ostream& s, const MarsRequest& r) {
        r.print(s); return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& s, const MarsRequest& r) {
        r.json(s); return s;
    }

    friend eckit::Stream& operator<<(eckit::Stream& s, const MarsRequest& r) {
        r.encode(s); return s;
    }

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

#endif

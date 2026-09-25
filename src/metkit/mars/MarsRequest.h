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
class MarsValidatedRequest;


// //----------------------------------------------------------------------------------------------------------------------

// class MarsID {
// public:
//     const std::string& get(Keyword key) const;
//     void set(Keyword key, const std::string& value);
//     void unset(Keyword key);
//     bool has(Keyword key) const;
//     const std::unordered_map<Keyword, std::string>& values() const;

// private:
//     std::unordered_map<Keyword, std::string> values_;
// };

// //----------------------------------------------------------------------------------------------------------------------


// class MarsDataCube {
// public:  // methods

//     const std::vector<std::string>& get(Keyword key) const;
//     void set(Keyword key, const std::vector<std::string>& value);
//     void unset(Keyword key);
//     bool has(Keyword key) const;
//     void add(const MarsID& id);
//     void merge(const MarsDataCube& other);
//     const std::unordered_map<Keyword, std::vector<std::string>>& values() const;

// private:
//     std::unordered_map<Keyword, std::vector<std::string>> values_;
// };


//----------------------------------------------------------------------------------------------------------------------

class MarsRequest {
public:  // methods

    MarsRequest() = default;
    // MarsRequest(const MarsRequest&);

    // explicit MarsRequest(Verb);
    // explicit MarsRequest(const std::string&);
    // explicit MarsRequest(eckit::Stream&, bool lowercase = false);

    // MarsRequest(const std::string&, const std::map<std::string, std::string>&);
    // MarsRequest(const std::string&, const eckit::Value&);

    // explicit MarsRequest(const eckit::message::Message&);

    // // MarsRequest(const MarsParsedRequest& parsed);

    virtual ~MarsRequest() = default;

    // bool operator<(const MarsRequest& other) const;

    // // eckit::Value&        operator[](const std::string&);
    // const std::string& operator[](Keyword) const;
    // const std::string& operator[](const std::string&) const;

    // operator eckit::Value() const;

    virtual Verb verbId() const = 0;
    virtual const std::string& verb() const = 0;

    virtual size_t countValues(Keyword) const = 0;
    virtual size_t countValues(const std::string&) const = 0;

    virtual bool has(Keyword) const = 0;
    virtual bool has(const std::string&) const = 0;

    virtual const std::vector<std::string>& values(Keyword, bool emptyOk = false) const = 0;
    virtual const std::vector<std::string>& values(const std::string&, bool emptyOk = false) const = 0;

    // // Returns reference to values or nullopt if not found
    // std::optional<std::reference_wrapper<const std::vector<std::string>>> get(Keyword id) const;
    // std::optional<std::reference_wrapper<const std::vector<std::string>>> get(const std::string& keyword) const;

    template <class T>
    size_t getValues(Keyword, std::vector<T>& v, bool emptyOk = false) const;
    template <class T>
    size_t getValues(const std::string& name, std::vector<T>& v, bool emptyOk = false) const;

    // virtual void getParams(std::vector<std::string>&) const;
    // std::vector<std::string> params() const;

    const std::vector<std::unique_ptr<Parameter>>& parameters() const { return params_; }

    // virtual void verb(const std::string&);

    virtual void values(Keyword, const std::vector<std::string>&) = 0;
    virtual void values(const std::string&, const std::vector<std::string>&) = 0;

    template <class T>
    void setValue(const std::string& name, const T& value);
    // virtual void setValue(const std::string& name, const std::vector<std::string>& value);

    void unsetValues(const std::string&);

    virtual void erase(Keyword);
    virtual void erase(const std::string&);

    // /// Splits a MARS request into multiple requests along the provided key
    // std::vector<MarsRequest> split(const std::string& keys) const;
    // /// Splits a MARS request into multiple requests along the indicated keys
    // std::vector<MarsRequest> split(const std::vector<std::string>& keys) const;

    /// Merges one MarsRequest into another
    // parameters existing in the other request but not present in the current request will be ignored
    virtual void merge(const MarsRequest& other);

    // /// Create a new MarsRequest from this one with only the given set of keys
    // MarsRequest subset(const std::set<std::string>&) const;

    void json(eckit::JSON&, bool array = false) const;

    // void md5(eckit::MD5&) const;

    void dump(std::ostream&, const char* cr = "\n", const char* tab = "\t", bool verb = true) const;

    virtual void setValuesTyped(const Type*, const std::vector<std::string>&) = 0;

    bool filter(const MarsRequest& filter);
    bool matches(const MarsRequest& filter) const;
    bool empty() const;

    size_t count() const;

    // MarsRequest extract(const std::string& category) const;

    virtual std::string asString() const;

    virtual const Parameter* find(Keyword) const = 0;
    // Parameter* find(Keyword);
    virtual const Parameter* find(const std::string& name) const = 0;
    // Parameter* find(const std::string& name);

public:  // static methods

    /// Implementation in api/metkit_c.cc
    static const MarsRequest& fromOpaque(const metkit_marsrequest_t* request);

private:  // methods

    friend class Type;
    friend class MarsLanguage;

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

protected:  // members

    std::vector<std::unique_ptr<Parameter>> params_;
};

//----------------------------------------------------------------------------------------------------------------------


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

class MarsValidatedRequest : public MarsRequest {
public:
    MarsValidatedRequest() = default;
    MarsValidatedRequest(Verb verb);
    MarsValidatedRequest(const MarsRequest& request);
    explicit MarsValidatedRequest(eckit::Stream& s, bool lowercase = false);

    ~MarsValidatedRequest() override = default;

    Verb verbId() const override { return verb_; }
    const std::string& verb() const override;

    size_t countValues(Keyword) const override;
    size_t countValues(const std::string&) const override;

    bool has(Keyword) const override;
    bool has(const std::string&) const override;

    const std::vector<std::string>& values(Keyword, bool emptyOk = false) const override;
    const std::vector<std::string>& values(const std::string&, bool emptyOk = false) const override;

    void values(Keyword, const std::vector<std::string>&) override;
    void values(const std::string&, const std::vector<std::string>&) override;

    void erase(Keyword) override;
    void erase(const std::string&) override;

    void merge(const MarsRequest& other) override;

    void setValuesTyped(const Type*, const std::vector<std::string>&) override;

    const Parameter* find(Keyword) const override;
    const Parameter* find(const std::string& name) const override;

private:  // members

    Verb verb_;
    std::unordered_map<Keyword, size_t> paramMap_;

};

MarsValidatedRequest parse(const std::string& s, bool strict = false);
std::vector<MarsValidatedRequest> parse(std::istream&, bool strict = false);

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars

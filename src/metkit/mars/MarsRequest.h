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

class MarsBaseRequest {
public:  // methods

    MarsBaseRequest() = default;
    virtual ~MarsBaseRequest() = default;

    virtual std::unique_ptr<MarsBaseRequest> clone() const = 0;

    virtual Verb verbId() const = 0;
    virtual const std::string& verb() const = 0;

    virtual void verb(Verb id) = 0;
    virtual void verb(const std::string&) = 0;

    virtual size_t countValues(Keyword) const = 0;
    virtual size_t countValues(const std::string&) const = 0;

    virtual bool has(Keyword) const = 0;
    virtual bool has(const std::string&) const = 0;

    virtual const std::vector<std::string>& values(Keyword, bool emptyOk = false) const = 0;
    virtual const std::vector<std::string>& values(const std::string&, bool emptyOk = false) const = 0;

    std::vector<Parameter>& parameters() { return params_; }
    const std::vector<Parameter>& parameters() const { return params_; }

    virtual void values(Keyword, const std::vector<std::string>&) = 0;
    virtual void values(const std::string&, const std::vector<std::string>&) = 0;

    virtual void erase(Keyword) = 0;
    virtual void erase(const std::string&) = 0;


    /// Merges one MarsRequest into another
    // parameters existing in the other request but not present in the current request will be ignored
    virtual void merge(const MarsRequest& other) = 0;

    virtual void setValuesTyped(std::shared_ptr<const Type>, const std::vector<std::string>&) = 0;

    virtual bool filter(const MarsRequest& filter) = 0;
    virtual bool matches(const MarsRequest& filter) const = 0;
    bool empty() const { return params_.empty(); }

    virtual size_t count() const = 0;

    virtual const Parameter* find(Keyword) const = 0;
    virtual const Parameter* find(const std::string& name) const = 0;


protected:  // members

    std::vector<Parameter> params_;
};

class MarsRequest {
public:  // methods

    MarsRequest();
    MarsRequest(Verb verb);
    MarsRequest(const std::string& verb);
    MarsRequest(const std::string&, const std::map<std::string, std::string>&);
    MarsRequest(const std::string&, const eckit::Value&);
    MarsRequest(const MarsRequest& request);
    MarsRequest(MarsRequest&& other);
    explicit MarsRequest(eckit::Stream& s, bool validate = false, bool lowercase = false);

    MarsRequest& operator=(const MarsRequest& other);
    MarsRequest& operator=(MarsRequest&& other);
    const std::string& operator[](const std::string&) const;

    Verb verbId() const { return req_->verbId(); }
    const std::string& verb() const { return req_->verb(); }

    void verb(Verb id) { return req_->verb(id); }
    void verb(const std::string& name) { return req_->verb(name); }

    size_t countValues(Keyword key) const { return req_->countValues(key); }
    size_t countValues(const std::string& name) const { return req_->countValues(name); }

    bool has(Keyword key) const { return req_->has(key); }
    bool has(const std::string& name) const { return req_->has(name); }

    template <class T>
    size_t getValues(Keyword, std::vector<T>& v, bool emptyOk = false) const;
    template <class T>
    size_t getValues(const std::string& name, std::vector<T>& v, bool emptyOk = false) const;

    // getters
    // Returns reference to values or nullopt if not found
    std::optional<std::reference_wrapper<const std::vector<std::string>>> get(const std::string& keyword) const;

    const std::vector<std::string>& values(Keyword key, bool emptyOk = false) const { return req_->values(key, emptyOk); }
    const std::vector<std::string>& values(const std::string& name, bool emptyOk = false) const { return req_->values(name, emptyOk); }

    std::vector<std::string> params() const;

    std::vector<Parameter>& parameters() { return req_->parameters(); }
    const std::vector<Parameter>& parameters() const { return req_->parameters(); }

    template <class T>
    void setValue(Keyword key, const T& value);
    template <class T>
    void setValue(const std::string& name, const T& value);

    // setters
    void values(Keyword key, const std::vector<std::string>& v) { req_->values(key, v); }
    void values(const std::string& name, const std::vector<std::string>& v) { req_->values(name, v); }

    void unsetValues(const std::string& name) { req_->erase(name); }

    void erase(Keyword key) { req_->erase(key); }
    void erase(const std::string& name) { req_->erase(name); }

    /// Splits a MARS request into multiple requests along the indicated keys
    std::vector<MarsRequest> split(const std::vector<std::string>& keys) const;

    /// Merges one MarsRequest into another
    // parameters existing in the other request but not present in the current request will be ignored
    void merge(const MarsRequest& other) { req_->merge(other); }

    /// Create a new MarsRequest from this one with only the given set of keys
    MarsRequest subset(const std::set<std::string>&) const;

    void json(eckit::JSON&, bool array = false) const;

    void md5(eckit::MD5&) const;

    void dump(std::ostream& s, const char* cr = "\n", const char* tab = "\t", bool verb = true) const;

    void setValuesTyped(const Type* type, const std::vector<std::string>& values);
    void setValuesTyped(std::shared_ptr<const Type> type, const std::vector<std::string>& values) { req_->setValuesTyped(type, values); }

    bool filter(const MarsRequest& filter) { return req_->filter(filter); }
    bool matches(const MarsRequest& other) const { return req_->matches(other); }
    bool empty() const { return req_->empty(); }

    bool operator<(const MarsRequest& other) const;

    size_t count() const { return req_->count(); }

    std::string asString() const;

    const Parameter* find(Keyword key) const { return req_->find(key); }
    const Parameter* find(const std::string& name) const { return req_->find(name); }

public:  // static methods

    static MarsRequest parse(const std::string& s, bool strict = false);
    static std::vector<MarsRequest> parse(std::istream&, bool strict = false);

    /// Implementation in api/metkit_c.cc
    static const MarsRequest& fromOpaque(const metkit_marsrequest_t* request);

private:  // methods

    friend class Type;
    friend class MarsLanguage;

    void print(std::ostream& s) const;
    void encode(eckit::Stream& s) const;

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

    std::unique_ptr<MarsBaseRequest> req_;
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

class MarsRawRequest : public MarsBaseRequest {
public:

    MarsRawRequest() = default;
    MarsRawRequest(const std::string& verb);
    MarsRawRequest(const MarsRawRequest& request);
    explicit MarsRawRequest(eckit::Stream& s, bool lowercase = false);

    ~MarsRawRequest() override = default;

    std::unique_ptr<MarsBaseRequest> clone() const override { return std::make_unique<MarsRawRequest>(*this); }

    Verb verbId() const override;
    const std::string& verb() const override { return verb_; }

    void verb(Verb id) override;
    void verb(const std::string&) override;

    size_t countValues(Keyword) const override;
    size_t countValues(const std::string&) const override;

    bool has(Keyword) const override;
    bool has(const std::string& name) const override;

    const std::vector<std::string>& values(Keyword, bool emptyOk = false) const override;
    const std::vector<std::string>& values(const std::string&, bool emptyOk = false) const override;

    void values(Keyword, const std::vector<std::string>&) override;
    void values(const std::string&, const std::vector<std::string>&) override;

    void merge(const MarsRequest& other) override;
    
    void setValuesTyped(std::shared_ptr<const Type>, const std::vector<std::string>&) override;
    
    bool filter(const MarsRequest& filter) override;
    bool matches(const MarsRequest& filter) const override;

    const Parameter* find(Keyword) const override;
    const Parameter* find(const std::string& name) const override;

    size_t count() const override;

protected:

    void erase(Keyword) override;
    void erase(const std::string&) override;

private:  // members

    std::string verb_;
    std::unordered_map<std::string, size_t> paramMap_;

};

//----------------------------------------------------------------------------------------------------------------------

class MarsValidatedRequest : public MarsBaseRequest {
public:
    MarsValidatedRequest() = default;
    MarsValidatedRequest(Verb verb);
    MarsValidatedRequest(const MarsValidatedRequest& request);
    explicit MarsValidatedRequest(eckit::Stream& s);

    ~MarsValidatedRequest() override = default;

    std::unique_ptr<MarsBaseRequest> clone() const override { return std::make_unique<MarsValidatedRequest>(*this); }

    Verb verbId() const override { return verb_; }
    const std::string& verb() const override;

    void verb(Verb id) override;
    void verb(const std::string&) override;

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

    void setValuesTyped(std::shared_ptr<const Type>, const std::vector<std::string>&) override;

    bool filter(const MarsRequest& filter) override;
    bool matches(const MarsRequest& filter) const override;

    const Parameter* find(Keyword) const override;
    const Parameter* find(const std::string& name) const override;

    size_t count() const override;

private:  // members

    friend class MarsLanguage;

    Verb verb_;
    std::map<Keyword, size_t> paramMap_;

};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars

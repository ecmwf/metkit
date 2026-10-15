// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Manuel Fuentes
/// @author Baudouin Raoult
/// @author Tiago Quintino

/// @date Sep 96

#pragma once

#include <array>
#include <cstddef>  // for size_t
#include <functional>
#include <iosfwd>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <string>
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

class MarsLanguage;
class Type;

//----------------------------------------------------------------------------------------------------------------------

class MarsID {
public:

    const std::string& get(Keyword key) const;
    void set(Keyword key, const std::string& value);
    void unset(Keyword key);
    bool has(Keyword key) const;
    const std::map<Keyword, std::string>& values() const;

private:

    std::map<Keyword, std::string> values_;
};

//----------------------------------------------------------------------------------------------------------------------

class MarsDataCube {
public:  // methods

    MarsDataCube(Verb verb);
    virtual ~MarsDataCube() = default;

    virtual Verb verbId() const;
    virtual void verb(Verb id);

    using iterator       = std::vector<Parameter>::iterator;
    using const_iterator = std::vector<Parameter>::const_iterator;

    std::vector<Parameter>::const_iterator begin() const { return data_.begin(); }
    std::vector<Parameter>::const_iterator end() const { return data_.end(); }

    size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }

    /// The number of fields the datacube refers to
    size_t count() const;

    const std::vector<std::string>& get(Keyword key) const;
    void set(Keyword key, const std::vector<std::string>& value);
    void set(Keyword key, std::vector<std::string>&& value);
    void unset(Keyword key);
    bool has(Keyword key) const;
    void merge(const MarsDataCube& other);

protected:

    Verb verb_;
    const MarsLanguage* lang_ = nullptr;
    std::vector<Parameter> data_;
};


//----------------------------------------------------------------------------------------------------------------------

/// Random access iterator over the parameters of a request: it goes through the data parameters, then the postproc
/// ones, and then the others (each one in the order they were added)
class ParameterIterator {
public:  // types

    using iterator_category = std::random_access_iterator_tag;
    using value_type        = Parameter;
    using difference_type   = std::ptrdiff_t;
    using pointer           = const value_type*;
    using reference         = const value_type&;

public:  // methods

    ParameterIterator() = default;

    /// @param end if true, the iterator is the one past the last parameter
    ParameterIterator(const std::vector<Parameter>& data, const std::vector<Parameter>& postproc,
                      const std::vector<Parameter>& other, bool end = false) :
        containers_{&data, &postproc, &other}, index_(end ? data.size() + postproc.size() + other.size() : 0) {}

    reference operator*() const { return at(index_); }
    pointer operator->() const { return &at(index_); }
    reference operator[](difference_type n) const { return at(index_ + n); }

    ParameterIterator& operator++() {
        ++index_;
        return *this;
    }
    ParameterIterator operator++(int) {
        ParameterIterator tmp = *this;
        ++index_;
        return tmp;
    }
    ParameterIterator& operator--() {
        --index_;
        return *this;
    }
    ParameterIterator operator--(int) {
        ParameterIterator tmp = *this;
        --index_;
        return tmp;
    }

    ParameterIterator& operator+=(difference_type n) {
        index_ += n;
        return *this;
    }
    ParameterIterator& operator-=(difference_type n) {
        index_ -= n;
        return *this;
    }

    friend ParameterIterator operator+(ParameterIterator it, difference_type n) { return it += n; }
    friend ParameterIterator operator+(difference_type n, ParameterIterator it) { return it += n; }
    friend ParameterIterator operator-(ParameterIterator it, difference_type n) { return it -= n; }
    friend difference_type operator-(const ParameterIterator& a, const ParameterIterator& b) {
        return static_cast<difference_type>(a.index_) - static_cast<difference_type>(b.index_);
    }

    bool operator==(const ParameterIterator& other) const { return index_ == other.index_; }
    bool operator!=(const ParameterIterator& other) const { return index_ != other.index_; }
    bool operator<(const ParameterIterator& other) const { return index_ < other.index_; }
    bool operator>(const ParameterIterator& other) const { return index_ > other.index_; }
    bool operator<=(const ParameterIterator& other) const { return index_ <= other.index_; }
    bool operator>=(const ParameterIterator& other) const { return index_ >= other.index_; }

private:  // methods

    /// @param i position in the concatenation of the three containers
    const Parameter& at(size_t i) const {
        for (const auto* container : containers_) {
            if (i < container->size()) {
                return (*container)[i];
            }
            i -= container->size();
        }
        __builtin_unreachable();
    }

private:  // members

    std::array<const std::vector<Parameter>*, 3> containers_{};
    size_t index_ = 0;
};

//----------------------------------------------------------------------------------------------------------------------

/// A MARS request: a verb and a list of parameters (keywords and their values).
///
/// The parameters are held in three containers: the ones whose type is of category Data, the ones of category
/// PostProc, and the others (including the untyped ones). The axes (the keywords from 1 to
/// MarsLanguage::maxDataKeyword() - 1) are always held with the data ones, typed or not. parameters() iterates over
/// the containers in that order, and in the order the parameters were added within each one.
///
/// The parameters of a request that was just parsed or created are untyped. The expansion (see MarsLanguage and
/// MarsExpansion) produces requests whose parameters are typed.
///
/// Keywords can be given by name or by id (see MarsLanguage::keyword()). The id is faster for typed parameters, which
/// know theirs. Names of registered keywords are case insensitive, custom names are compared as they are.
///
/// @note Pointers and references returned by find() and values() (and the ones to the parameters) are valid until the
/// request is modified (a value set, a parameter added or removed), or destroyed.
class MarsRequest : public MarsDataCube {
public:  // methods

    using iterator       = ParameterIterator;
    using const_iterator = ParameterIterator;

    ParameterIterator begin() const { return ParameterIterator(data_, postproc_, other_); }
    ParameterIterator end() const { return ParameterIterator(data_, postproc_, other_, true); }

    size_t size() const { return data_.size() + postproc_.size() + other_.size(); }

    MarsRequest();
    explicit MarsRequest(Verb verb);
    explicit MarsRequest(const std::string& verb);
    MarsRequest(const std::string& verb, const std::map<std::string, std::string>& vals);
    MarsRequest(const std::string&, const std::map<std::string, std::vector<std::string>>&);
    MarsRequest(const std::string& verb, const eckit::Value& vals);
    MarsRequest(const eckit::message::Message& message);

    /// @param validate if true, every parameter is checked against the language of the verb and typed
    /// @param lowercase if true, the verb and the names of the parameters are converted to lowercase
    explicit MarsRequest(eckit::Stream& s, bool validate = false);

    MarsRequest(const MarsRequest& request)              = default;
    MarsRequest(MarsRequest&& other) noexcept            = default;
    MarsRequest& operator=(const MarsRequest& other)     = default;
    MarsRequest& operator=(MarsRequest&& other) noexcept = default;

    ~MarsRequest() noexcept override = default;

    const std::string& operator[](const std::string&) const;

    Verb verbId() const override;
    const std::string& verb() const { return verbName_; }

    void verb(Verb id) override;
    void verb(const std::string& name);

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

    [[deprecated("iterate directly over the MarsRequest")]]
    MarsRequest& parameters() const {
        return *const_cast<MarsRequest*>(this);
    }

    template <class T>
    void setValue(Keyword key, const T& value);
    template <class T>
    void setValue(const std::string& name, const T& value);

    // setters
    // They set the values of the parameter, which is created (untyped) if needed
    void values(Keyword key, const std::vector<std::string>& v);
    void values(const std::string& name, const std::vector<std::string>& v);
    void values(Keyword key, std::vector<std::string>&& v);
    void values(const std::string& name, std::vector<std::string>&& v);

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
    bool empty() const { return data_.empty() && postproc_.empty() && other_.empty(); }

    bool operator<(const MarsRequest& other) const;

    MarsDataCube data() const;
    const std::vector<Parameter>& postproc() const { return postproc_; }
    [[deprecated("extract is deprecated, use data() or postproc() instead")]]
    MarsRequest extract(const std::string& category) const;

    // void erase(const std::string& param);

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

    /// Where a parameter is held: the container (0 data, 1 postproc, 2 other) and the index in it
    struct Position {
        size_t container = 3;
        size_t index     = 0;
        explicit operator bool() const { return container < 3; }
    };

    /// @return the position of the parameter, or a false Position if there is no such parameter
    Position locate(Keyword key) const;
    Position locate(const std::string& name) const;

    const std::vector<Parameter>& container(size_t c) const;
    std::vector<Parameter>& container(size_t c);

    /// Adds the parameter to the container that corresponds to its keyword @p key (0 if not registered) and type
    void add(Parameter&& p);

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

    std::string verbName_;
    std::vector<Parameter> postproc_;  // typed parameters of category PostProc
    std::vector<Parameter> other_;     // the rest: other categories, and untyped parameters
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

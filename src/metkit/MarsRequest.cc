/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <algorithm>
#include <set>
#include <list>

#include "eckit/types/Types.h"
#include "eckit/parser/JSON.h"
#include "eckit/log/Log.h"
#include "eckit/config/Resource.h"
#include "eckit/utils/Translator.h"
#include "eckit/utils/MD5.h"
#include "eckit/parser/StringTools.h"

#include "metkit/MarsRequest.h"
#include "metkit/types/TypeAny.h"
#include "metkit/MarsParser.h"

using namespace eckit;

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

class UndefinedType : public Type {
    virtual void print( std::ostream &out ) const  {
        out << "<undefined type>";
    }

    virtual bool filter(const std::vector< std::string >& filter, std::vector<std::string>& values) {
        NOTIMP;
    }


public:
    UndefinedType() : Type("<undefined>", eckit::Value()) { attach(); }
};


static UndefinedType undefined;


Parameter::Parameter():
    type_(&undefined) {
    type_->attach();
}

Parameter::~Parameter() {
    type_->detach();
}

Parameter::Parameter(const std::vector<std::string>& values, Type* type):
    type_(type),
    values_(values) {
    if (!type) {
        type_ = &undefined;
    }
    type_->attach();
    type_->check(values);
}


Parameter::Parameter(const Parameter& other):
    type_(other.type_),
    values_(other.values_) {
    type_->attach();
}

Parameter& Parameter::operator=(const Parameter& other) {
    Type *old = type_;
    type_ = other.type_;
    type_->attach();
    old->detach();

    values_ = other.values_;
    return *this;
}

void Parameter::values(const std::vector<std::string>& values) {
    values_ = values;
}

bool Parameter::filter(const std::vector<std::string> &filter)  {
    return type_->filter(filter, values_);
}


bool Parameter::matches(const std::vector<std::string> &match) const {
    return type_->matches(match, values_);
}


const std::string& Parameter::name() const {
    return type_->name();
}

size_t Parameter::count() const {
    return type_->count(values_);
}

bool Parameter::operator<(const Parameter& other) const {
    if (name() != other.name()) {
        return name() < other.name();
    }
    return values_ < other.values_;
}

//----------------------------------------------------------------------------------------------------------------------


MarsRequest::MarsRequest()
{
}


MarsRequest::MarsRequest(const std::string& s):
    verb_(s)
{
}

MarsRequest::MarsRequest(eckit::Stream& s)
{
    int size;


    s >> verb_;
    s >> size;

    for (int i = 0; i < size; i++)
    {
        std::string param;
        int    count;

        s >> param;
        s >> count;

        std::vector<std::string> v;
        v.reserve(count);

        for (int k = 0; k < count; k++)
        {
            std::string value;
            s >> value;
            v.push_back(value);
        }

        params_.push_back(Parameter(v, new TypeAny(param)));
    }
}

void MarsRequest::encode(eckit::Stream& s) const
{
    s << verb_;
    int size = params_.size();
    s << size;


    for (std::list<Parameter>::const_iterator i = params_.begin(); i != params_.end(); ++i)
    {
        s << (*i).name();

        const std::vector<std::string>& v = (*i).values();

        int size = v.size(); // For backward compatibility
        s << size;

        for (std::vector<std::string>::const_iterator k = v.begin(); k != v.end(); ++k) {
            s << *k;
        }
    }
}

MarsRequest::MarsRequest(const eckit::ValueMap&) {
    NOTIMP;
}

bool MarsRequest::empty() const {
    return params_.empty();
}


void MarsRequest::print(std::ostream& s) const
{
    dump(s, "", "");
}

void MarsRequest::dump(std::ostream& s, const char* cr, const char* tab) const {

    std::list<Parameter>::const_iterator begin = params_.begin();
    std::list<Parameter>::const_iterator end = params_.end();


    s << verb_ ;

    if (begin != end) {
        s << ',' << cr << tab;

        int a = 0;
        for (std::list<Parameter>::const_iterator i = begin; i != end; ++i)
        {
            if (a++) {
                s << ',';
                s << cr << tab;
            }

            int b = 0;
            s  << (*i).name()
//               << "." << (*i).second.type()
               << "=";
            const std::vector<std::string>& v = (*i).values();

            for (std::vector<std::string>::const_iterator k = v.begin();
                    k != v.end(); ++k)
            {
                if (b++) {
                    s << '/';
                }
                MarsParser::quoted(s, *k);
            }
        }
    }

    s << cr << cr;
}

void MarsRequest::json(eckit::JSON& s) const
{
    s.startObject();
    s << "verb" << verb_;
    std::list<Parameter>::const_iterator begin = params_.begin();
    std::list<Parameter>::const_iterator end = params_.end();

    for (std::list<Parameter>::const_iterator i = begin; i != end; ++i)
    {
        s << (*i).name();
        const std::vector<std::string>& v = (*i).values();

        if (v.size() != 1) {
            s.startList();
        }

        for (std::vector<std::string>::const_iterator k = v.begin(); k != v.end(); ++k) {
            s << (*k) ;
        }

        if (v.size() != 1) {s.endList();}
    }

    s.endObject();
}

void MarsRequest::md5(eckit::MD5& md5) const
{
    std::ostringstream oss;
    oss << *this;
    md5.add(oss.str());
}

MarsRequest::~MarsRequest()
{
}

void MarsRequest::unsetValues(const std::string& name)
{
    std::list<Parameter>::iterator i = find(name);
    if (i != params_.end()) {
        params_.erase(i);
    }
}

void MarsRequest::setValuesTyped(Type* type, const std::vector<std::string>& values) {
    std::list<Parameter>::iterator i = find(type->name());
    if (i != params_.end()) {
        (*i) = Parameter(values, type);
    }
    else {
        params_.push_back(Parameter(values, type));
    }
}

bool MarsRequest::filter(const MarsRequest &filter) {
    for (std::list<Parameter>::iterator i = params_.begin(); i != params_.end(); ++i) {

        std::list<Parameter>::const_iterator j = filter.find((*i).name());
        if (j == filter.params_.end()) {
            continue;
        }

        if (!(*i).filter((*j).values())) {
            return false;
        }
    }
    return true;
}

bool MarsRequest::matches(const MarsRequest &matches) const {

    std::vector<std::string> params = matches.params();
    for (std::vector<std::string>::const_iterator j = params.begin(); j != params.end(); ++j) {
        std::list<Parameter>::const_iterator k = find(*j);
        if (k == params_.end()) {
            return false;
        }

        if (!(*k).matches(matches.values(*j))) {
            return false;
        }
    }

    return true;
}

void MarsRequest::values(const std::string& name, const std::vector<std::string>& v)
{
    std::list<Parameter>::iterator i = find(name);
    if (i != params_.end()) {
        (*i).values(v);
    }
    else {
        params_.push_back(Parameter(v, new TypeAny(name)));
    }
}


size_t MarsRequest::countValues(const std::string& name) const
{
    std::list<Parameter>::const_iterator i = find(name);
    if (i != params_.end()) {
        return (*i).values().size();
    }
    return 0;
}

bool MarsRequest::is(const std::string& name, const std::string& value) const
{
    std::list<Parameter>::const_iterator i = find(name);
    if (i != params_.end()) {
        const std::vector<std::string>& v = (*i).values();
        return v.size() == 1 && v[0] == value;
    }
    return false;
}


const std::vector<std::string>& MarsRequest::values(const std::string& name, bool emptyOk) const
{
    std::list<Parameter>::const_iterator i = find(name);
    if (i == params_.end()) {

        if (emptyOk) {
            static std::vector<std::string> empty;
            return empty;
        }

        std::ostringstream oss;
        oss << "No parameter called '" << name << "' in request " << *this;
        throw eckit::UserError(oss.str());
    }
    return (*i).values();
}


void MarsRequest::getParams(std::vector<std::string>& p) const
{
    p.clear();
    for (std::list<Parameter>::const_iterator i = params_.begin(); i != params_.end(); ++i) {
        p.push_back((*i).name());
    }

}

size_t MarsRequest::count() const
{
    size_t result = 1;
    for (std::list<Parameter>::const_iterator i = params_.begin(); i != params_.end(); ++i) {
        result *= (*i).count();
    }
    return result;
}


std::vector<std::string> MarsRequest::params() const
{
    std::vector<std::string> p;
    getParams(p);
    return p;
}

MarsRequest::operator eckit::Value() const {
    NOTIMP;
}

void MarsRequest::merge(const MarsRequest &other) {
    NOTIMP;
}


void MarsRequest::verb(const std::string &verb) {
    verb_ = verb;
}

bool MarsRequest::operator<(const MarsRequest& other) const {
    if (verb_ != other.verb_) {
        return verb_ < other.verb_;
    }
    return params_ < other.params_;
}


std::list<Parameter>::const_iterator MarsRequest::find(const std::string& name) const {
    for (std::list<Parameter>::const_iterator i = params_.begin(); i != params_.end(); ++i) {
        if ((*i).name() == name) {
            return i;
        }
    }
    return params_.end();
}

std::list<Parameter>::iterator MarsRequest::find(const std::string & name) {
    for (std::list<Parameter>::iterator i = params_.begin(); i != params_.end(); ++i) {
        if ((*i).name() == name) {
            return i;
        }
    }
    return params_.end();
}
//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

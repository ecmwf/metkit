/*
 * (C) Copyright 1996-2013 ECMWF.
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

bool Parameter::filter(const std::vector<std::string> &filter) {
    return type_->filter(filter, values_);
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

        params_[param] = Parameter(v, new TypeAny(param));
    }
}

void MarsRequest::encode(eckit::Stream& s) const
{
    s << verb_;
    int size = params_.size();
    s << size;


    for (std::map<std::string, Parameter>::const_iterator i = params_.begin(); i != params_.end(); ++i)
    {
        s << (*i).first;

        const std::vector<std::string>& v = (*i).second.values();

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


void MarsRequest::print(std::ostream& s) const
{
    dump(s, "", "");
}

void MarsRequest::dump(std::ostream& s, const char* cr, const char* tab) const {

    std::map<std::string, Parameter>::const_iterator begin = params_.begin();
    std::map<std::string, Parameter>::const_iterator end = params_.end();


    s << verb_ ;

    if (begin != end) {
        s << ',' << cr << tab;

        int a = 0;
        for (std::map<std::string, Parameter>::const_iterator i = begin; i != end; ++i)
        {
            if (a++) {
                s << ',';
                s << cr << tab;
            }

            int b = 0;
            s  << (*i).first
//               << "." << (*i).second.type()
                 << "=";
            const std::vector<std::string>& v = (*i).second.values();

            for (std::vector<std::string>::const_iterator k = v.begin();
                    k != v.end(); ++k)
            {
                if (b++) {
                    s << '/';
                }
                s << *k;
            }
        }
    }

    s << cr << cr;
}

void MarsRequest::json(eckit::JSON& s) const
{
    s.startObject();
    s << "verb" << verb_;
    std::map<std::string, Parameter>::const_iterator begin = params_.begin();
    std::map<std::string, Parameter>::const_iterator end = params_.end();

    for (std::map<std::string, Parameter>::const_iterator i = begin; i != end; ++i)
    {
        s << (*i).first;
        const std::vector<std::string>& v = (*i).second.values();

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
    std::map<std::string, Parameter>::iterator i = params_.find(name);
    if (i != params_.end()) {
        params_.erase(i);
    }
}

void MarsRequest::setValuesTyped(Type* type, const std::vector<std::string>& values) {
    params_[type->name()] = Parameter(values, type);
}

bool MarsRequest::filter(const MarsRequest &filter) {
    for (std::map<std::string, Parameter>::iterator i = params_.begin(); i != params_.end(); ++i) {

        std::map<std::string, Parameter>::const_iterator j = filter.params_.find((*i).first);
        if(j == filter.params_.end()) {
            continue;
        }

        if(!(*i).second.filter((*j).second.values())) {
            return false;
        }
    }
    return true;
}

void MarsRequest::values(const std::string& name, const std::vector<std::string>& v)
{
    params_[name].values(v);
}


size_t MarsRequest::countValues(const std::string& name) const
{
    std::map<std::string, Parameter>::const_iterator i = params_.find(name);
    if (i != params_.end()) {
        return (*i).second.values().size();
    }
    return 0;
}

bool MarsRequest::is(const std::string& name, const std::string& value) const
{
    std::map<std::string, Parameter>::const_iterator i = params_.find(name);
    if (i != params_.end()) {
        const std::vector<std::string>& v = (*i).second.values();
        return v.size() == 1 && v[0] == value;
    }
    return false;
}


const std::vector<std::string>& MarsRequest::values(const std::string& name, bool emptyOk) const
{
    std::map<std::string, Parameter>::const_iterator i = params_.find(name);
    if (i == params_.end()) {

        if(emptyOk) {
            static std::vector<std::string> empty;
            return empty;
        }

        std::ostringstream oss;
        oss << "No parameter called '" << name << "' in request " << *this;
        throw eckit::UserError(oss.str());
    }
    return (*i).second.values();
}


void MarsRequest::getParams(std::vector<std::string>& p) const
{
    p.clear();
    for (std::map<std::string, Parameter>::const_iterator i = params_.begin(); i != params_.end(); ++i) {
        p.push_back((*i).first);
    }

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
//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

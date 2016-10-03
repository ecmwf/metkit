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

using namespace eckit;

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

template<class T>
static void setV(const std::string& name, MarsRequest::Params& r, const std::vector<T>& v, bool append)
{
	MarsRequest::Values& values = r[name];

	if(!append)
		values.clear();

	for(size_t i = 0; i < v.size(); i++)
	{
		std::string s = Translator<T,std::string>()(v[i]);
		values.push_back(s);
	}
}

static bool has(const MarsRequest::Values& v, const std::string& s) {
	return std::find(v.begin(), v.end(), s) != v.end();
}

static void appendValues(const std::string& name, const MarsRequest::Values& v, MarsRequest::Params& r)
{
	MarsRequest::Values& values = r[name];

	for(MarsRequest::Values::const_iterator i = v.begin(); i != v.end(); ++i) {

		if(!has(values,*i))
			values.push_back(*i);
	}
}

template<class T>
static long copyValues(const std::string& name, const MarsRequest::Params& r, std::vector<T>& v,bool append)
{

	if(!append) v.clear();

	MarsRequest::Params::const_iterator i = r.find(name);
	if(i == r.end())
		return 0;

	if((*i).second.size() == 0)
		throw eckit::SeriousBug("No values found for parameter '" + name  + "'");

	v.reserve(v.size() + (*i).second.size());

	for(MarsRequest::Values::const_iterator k = (*i).second.begin();
		k != (*i).second.end(); ++k)
			v.push_back(Translator<std::string,T>()(*k));

	return (*i).second.size();
}

//----------------------------------------------------------------------------------------------------------------------


MarsRequest::MarsRequest()
{
}


MarsRequest::MarsRequest(const std::string& s):
	name_(s)
{
}

MarsRequest::MarsRequest(eckit::Stream& s)
{
	int size;


	s >> name_;

//    Log::info() << "MarsRequest name : " << name_ << std::endl;

    s >> size;

//    Log::info() << "MarsRequest size : " << size << std::endl;

	for(int i=0; i<size; i++)
	{
		std::string param;
		int    count;

		s >> param;
//        Log::info() << "MarsRequest param : " << param << std::endl;
        s >> count;
//        Log::info() << "MarsRequest count : " << count << std::endl;

		Values& v = params_[param];

		for(int k=0; k<count; k++)
		{
			std::string value;
            s >> value;
//            Log::info() << "MarsRequest value : " << value << std::endl;
            v.push_back(value);
		}
    }
}

void MarsRequest::encode(eckit::Stream& s) const
{
	s << name_;
	int size = params_.size();
	s << size;

	Params::const_iterator begin = params_.begin();
    Params::const_iterator end   = params_.end();

	for(Params::const_iterator i = begin; i != end; ++i)
	{
		s << (*i).first;

		int size = (*i).second.size();
		s << size;

		for(Values::const_iterator k = (*i).second.begin();
			k != (*i).second.end(); ++k)
				s << *k;
	}
}

MarsRequest::MarsRequest(const ValueMap& v)
{
    ValueMap::const_iterator iverb = v.find(Value("verb"));
    if(iverb == v.end())
        throw BadParameter("ValueMap does not represent a MarsRequest", Here());

    name_ = std::string( (*iverb).second );

//    Log::info() << "MarsRequest name : " << name_ << std::endl;

    ValueMap::const_iterator iparm = v.find(Value("params"));
    if(iparm == v.end())
        throw BadParameter("ValueMap does not represent a MarsRequest", Here());

    ValueMap params = (*iparm).second;

    for(ValueMap::iterator i = params.begin(); i != params.end(); ++i) {
        std::string pname = (*i).first;
        ValueList   list  = (*i).second;
        Values& mp = params_[pname];
        for(ValueList::iterator j = list.begin(); j != list.end(); ++j) {
            mp.push_back(*j);
        }

//        Log::info() << "MarsRequest param : " << pname << " = ";
//        std::ostream_iterator<std::string> outitr (Log::info(),"/");
//        std::copy(mp.begin(), mp.end(), outitr );
//        Log::info() << std::endl;
    }
}

metkit::MarsRequest::operator Value() const
{
    Value dict = Value::makeMap();

    dict["verb"] = name_;

    dict["params"] = Value::makeMap();

    Value& params = dict["params"];

    Params::const_iterator begin = params_.begin();
    Params::const_iterator end   = params_.end();

    for(Params::const_iterator i = begin; i != end; ++i) {
        params[ (*i).first ] = eckit::makeVectorValue( (*i).second );
    }

    return dict;
}

void MarsRequest::merge(const MarsRequest& other) {

	Params::const_iterator begin = other.params_.begin();
	Params::const_iterator end   = other.params_.end();

	for(Params::const_iterator i = begin; i != end; ++i) {

		appendValues((*i).first, (*i).second, params_);
	}
}

void MarsRequest::print(std::ostream& s) const
{
	s << name_ << ',' << std::endl;

	Params::const_iterator begin = params_.begin();
	Params::const_iterator end   =  params_.end();

	int a = 0;
	for(Params::const_iterator i = begin; i != end; ++i)
	{
		if(a++) s << ',' << std::endl;

		int b = 0;
		s << '\t' << (*i).first << " = ";
		for(Values::const_iterator k = (*i).second.begin();
			k != (*i).second.end(); ++k)
			{
				if(b++) s << '/';
				s << (*k);
			}
	}
}

void MarsRequest::json(eckit::JSON& s) const
{
    s.startObject();
    s << "verb" << name_;

	Params::const_iterator begin = params_.begin();
	Params::const_iterator end   =  params_.end();

	for(Params::const_iterator i = begin; i != end; ++i)
	{

		s << (*i).first;

		if((*i).second.size() != 1) s.startList();

		for(Values::const_iterator k = (*i).second.begin(); k != (*i).second.end(); ++k)
        {
            s << (*k) ;
        }

		if((*i).second.size() != 1) s.endList();
	}

    s.endObject();
}

void MarsRequest::md5(eckit::MD5& md5) const
{
    md5.add( StringTools::lower(name_));

    Params::const_iterator begin = params_.begin();
    Params::const_iterator end   = params_.end();

    for(Params::const_iterator i = begin; i != end; ++i) {

        md5.add( StringTools::lower((*i).first) );

        std::set<std::string> s; // ensures order is same and unique

        for(std::list<std::string>::const_iterator j = (*i).second.begin(); j != (*i).second.end(); ++j) {
            s.insert(*j);
        }

//        std::copy((*i).second.begin(), (*i).second.end(), s.begin());

        for(std::set<std::string>::const_iterator i = s.begin(); i != s.end(); ++i) {
            md5.add( StringTools::lower(*i) );
        }
    }
}

MarsRequest::~MarsRequest()
{
}

void MarsRequest::unsetValues(const std::string& name)
{
	Params::iterator i = params_.find(name);
	if(i != params_.end()) params_.erase(i);
}

long MarsRequest::getValues(const std::string& name,std::vector<double>& v,bool append) const
{
	return metkit::copyValues(name,params_,v,append);
}

long MarsRequest::getValues(const std::string& name,std::vector<Double>& v,bool append) const
{
	return metkit::copyValues(name,params_,v,append);
}

long MarsRequest::getValues(const std::string& name,std::vector<std::string>& v,bool append) const
{
	return metkit::copyValues(name,params_,v,append);
}

long MarsRequest::getValues(const std::string& name,std::vector<long>& v,bool append) const
{
	return metkit::copyValues(name,params_,v,append);
}

long MarsRequest::getValues(const std::string& name,std::vector<unsigned long>& v,bool append) const
{
	return metkit::copyValues(name,params_,v,append);
}

long MarsRequest::getValues(const std::string& name,std::vector<Date>& v,bool append) const
{
	return metkit::copyValues(name,params_,v,append);
}

long MarsRequest::getValues(const std::string& name,std::vector<Time>& v,bool append) const
{
	return metkit::copyValues(name,params_,v,append);
}

long MarsRequest::getValues(const std::string& name,std::vector<char>& v,bool append) const
{
	return metkit::copyValues(name,params_,v,append);
}

long MarsRequest::getValues(const std::string& name,std::vector<Value>& v,bool append) const
{
	return metkit::copyValues(name,params_,v,append);
}

MarsRequest::MarsRequest(const MarsRequest& other):
	name_(other.name_),
	params_(other.params_)
{
}

MarsRequest& MarsRequest::operator=(const MarsRequest& other)
{
	name_   = other.name_;
	params_ = other.params_;
	return *this;
}

void MarsRequest::setValues(const std::string& name,const std::vector<std::string>& v)
{
	metkit::setV(name,params_,v,false);
}

void MarsRequest::setValues(const std::string& name,const std::vector<long>& v)
{
	metkit::setV(name,params_,v,false);
}

void MarsRequest::setValues(const std::string& name,const std::vector<unsigned long>& v)
{
	metkit::setV(name,params_,v,false);
}

void MarsRequest::setValues(const std::string& name,const std::vector<Date>& v)
{
	metkit::setV(name,params_,v,false);
}

void MarsRequest::setValues(const std::string& name,const std::vector<Time>& v)
{
	metkit::setV(name,params_,v,false);
}

void MarsRequest::setValues(const std::string& name,const std::vector<char>& v)
{
	metkit::setV(name,params_,v,false);
}

long MarsRequest::getParams(std::vector<std::string>& p, bool) const
{
	for(Params::const_iterator i = params_.begin(); i != params_.end(); ++i)
		p.push_back((*i).first);

	return p.size();
}

void MarsRequest::name(const std::string& s)
{
	name_ = s;
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

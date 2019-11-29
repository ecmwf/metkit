/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// #include <limits>

#include "pointdb/FieldInfoKey.h"
#include "metkit/grib/GribAccessor.h"
#include "eckit/value/Value.h"
#include "eckit/log/JSON.h"
#include "eckit/utils/Translator.h"

using namespace eckit;
using namespace metkit::grib;


static GribAccessor<std::string>     type("type");
static GribAccessor<long>            endStep("endStep");
static GribAccessor<unsigned long>   level("level");
static GribAccessor<unsigned long>   paramId("paramId");
static GribAccessor<unsigned long>   number("number");
static GribAccessor<std::string>     levtype("levtype");

FieldInfoKey::FieldInfoKey():
    type_('a'),
    levtype_('a'),
    paramId_(0),
    endStep_(0),
    level_(0),
    number_(0)
{
}

void FieldInfoKey::fill(const Value& req)
{
    Value nil;

    if(req["param"] != nil) {
        paramId_ = req["param"];
    }

    if(req["type"] != nil) {
        type_ = std::string(req["type"])[0];
    }

    if(req["levtype"] != nil) {
        levtype_ = std::string(req["levtype"])[0];
    }

    if(req["step"] != nil) {
        endStep_ = req["step"];
    }

    if(req["number"] != nil) {
        number_ = req["number"];
    }

    if(req["level"] != nil) {
        level_ = req["level"];
    }

}

FieldInfoKey FieldInfoKey::minimum()
{
    FieldInfoKey k;

    k.paramId_ = 0;
    k.type_    = 'a';
    k.levtype_ = 'a';
    k.level_   = 0;
    k.endStep_ = 0;
    k.number_  = 0;

    return k;
}

FieldInfoKey FieldInfoKey::maximum()
{
    FieldInfoKey k;

    k.paramId_ = std::numeric_limits<long>::max();
    k.type_    = 'z';
    k.levtype_ = 'z';
    k.level_   = std::numeric_limits<long>::max();
    k.endStep_ = std::numeric_limits<long>::max();
    k.number_  = std::numeric_limits<long>::max();

    return k;
}

//========================================================================================

#define Y(A) { if(k1.A == k2.A) { if(A != k1.A) return false; } }

bool FieldInfoKey::match(const FieldInfoKey& k1, const FieldInfoKey& k2) const
{

    Y(type_);
    Y(levtype_);
    Y(paramId_);
    Y(endStep_);
    Y(level_);
    Y(number_);

    return true;

}


#define X(A) { if  (A < other.A)  return true;  if  (A > other.A)  return false; }

bool FieldInfoKey::operator<(const FieldInfoKey& other) const
{
    X(paramId_);
    X(type_);
    X(levtype_);
    X(endStep_);
    X(level_);
    X(number_);

    return false;
}

bool FieldInfoKey::operator==(const FieldInfoKey& other) const
{
    return  !(other < *this) && !(*this < other);
}

void FieldInfoKey::print(std::ostream& s) const
{
    s << "[paramId=" << paramId_
      << ",type="    << type_
      << ",levtype=" << levtype_
      << ",endStep=" << endStep_
      << ",level="   << level_
      << ",number="  << number_ << "]";
}

void FieldInfoKey::update(const GribHandle& h)
{
    endStep_ = endStep(h);
    level_ = level(h);
    type_ = std::string(type(h))[0];
    levtype_ = std::string(levtype(h))[0];
    paramId_ = paramId(h);
    number_ = number(h, 0);
}

void FieldInfoKey::json(eckit::JSON& j) const
{
    j << "param"   << paramId_
      << "type"    << type_
      << "levtype" << levtype_
      << "step"    << endStep_
      << "level"   << level_
      << "number"  << number_ ;
}

void FieldInfoKey::fill(const std::map<std::string, std::string> & m)
{
    std::map<std::string,std::string>::const_iterator k;
    if((k = m.find("step"))!= m.end())
        endStep_ = Translator<std::string, unsigned long>()((*k).second);

    if((k = m.find("levelist"))!= m.end())
        level_ = Translator<std::string, unsigned long>()((*k).second);

    if((k = m.find("type"))!= m.end())
        type_ = ((*k).second)[0];

    if((k = m.find("levtype"))!= m.end())
        levtype_ = ((*k).second)[0];

    if((k = m.find("parameter"))!= m.end())
        paramId_ = Translator<std::string, unsigned long>()((*k).second);

    if((k = m.find("number"))!= m.end())
        number_ = Translator<std::string, unsigned long>()((*k).second);
}

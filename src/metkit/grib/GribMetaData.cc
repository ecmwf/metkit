/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "grib_api.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/parser/StringTools.h"
#include "eckit/config/Resource.h"

#include "metkit/grib/GribMetaData.h"
#include "metkit/grib/GribHandle.h"



namespace metkit {
namespace grib {

//----------------------------------------------------------------------------------------------------------------------

GribMetaData::GribMetaData( eckit::Stream& s )
{
    bool b, more;
    std::string key;
    std::string str;
    long ival;
    double dval;

    s >> more;
    while(more)
    {
        s >> key;

        s >> b;
        if(b) {
            s >> str;
            stringValues_[key] = str;
        }

        s >> b;
        if(b) {
            s >> ival;
            longValues_[key] = ival;
        }

        s >> b;
        if(b) {
            s >> dval;
            doubleValues_[key] = dval;
        }
        s >> more;
    }
}

GribMetaData::GribMetaData(const void *buffer, size_t length)
{
    size_t len;
    char val[80] = {0,};
    double d;
    long l;

    /// JIRA TICKET >> const void*
    grib_handle* h = grib_handle_new_from_message(NULL, const_cast<void*>(buffer), length);
    ASSERT(h);

    static std::string gribToRequestNamespace = eckit::Resource<std::string>("gribToRequestNamespace", "mars");

    grib_keys_iterator* ks = grib_keys_iterator_new(h, GRIB_KEYS_ITERATOR_ALL_KEYS, gribToRequestNamespace.c_str());
    ASSERT(ks);

    while(grib_keys_iterator_next(ks))
    {
        const char* name = grib_keys_iterator_get_name(ks);

        if(name[0] == '_') {
            continue;
        }

        // in method parameters() we assume that all keys have a string representation

        len = sizeof(val);
        ASSERT( grib_keys_iterator_get_string(ks,val,&len) == 0 );
        stringValues_[name] = val;

        len = 1;
        if(grib_keys_iterator_get_double(ks,&d,&len) == 0)
            doubleValues_[name] = d;

        len = 1;
        if(grib_keys_iterator_get_long(ks,&l,&len) == 0)
            longValues_[name] = l;

    }

    {
        char value[1024];
        size_t len = sizeof(value);

        if(grib_get_string(h, "paramId", value, &len) == 0) {
            stringValues_["param"] = value;
        }
    }

    ASSERT(grib_get_long(h, "totalLength", &length_) == 0);

#if 0
    const char *extra[] = {"editionNumber", "table2Version", "indicatorOfParameter", 0};
    int i  = 0;

    while(extra[i]) {
        long value;
        if(grib_get_long(h,extra[i],&value) == 0)
            longValues_[std::string(extra[i])] = value;
        i++;
    }
#endif

    grib_keys_iterator_delete(ks);

    grib_handle_delete(h);
}

GribMetaData::~GribMetaData() {
}

std::vector<std::string> GribMetaData::keywords() const {
    std::vector<std::string> res;
    res.reserve(stringValues_.size());
    for(string_store::const_iterator itr = stringValues_.begin(); itr != stringValues_.end(); ++itr) {
        res.push_back(itr->first);
    }
    return res;
}

bool GribMetaData::has(const std::string& key) const
{
    return
        stringValues_.find(key) != stringValues_.find(key) ||
        doubleValues_.find(key) != doubleValues_.end() ||
                                   longValues_.find(key)   != longValues_.end();
}

void GribMetaData::get(const std::string &name, std::string &value) const
{
    getValue(name, value);
}

void GribMetaData::get(const std::string &name, long &value) const
{
    getValue(name, value);
}

void GribMetaData::get(const std::string &name, double &value) const
{
    getValue(name, value);
}

std::string GribMetaData::substitute(const std::string& pattern) const
{
    return eckit::StringTools::substitute(pattern,stringValues_);
}

size_t GribMetaData::length() const
{
    return length_;
}

void GribMetaData::getValue(const std::string& key,double& value) const
{
    std::map<std::string,double>::const_iterator j = doubleValues_.find(key);
    if(j == doubleValues_.end()) throw eckit::UserError(std::string("GribMetaData::getDouble failed for [") + key + "]");
    value = (*j).second;
}

void GribMetaData::getValue(const std::string& key,long& value) const
{
    std::map<std::string,long>::const_iterator j = longValues_.find(key);
    if(j == longValues_.end()) throw eckit::UserError(std::string("GribMetaData::getLong failed for [") + key + "]");
    value = (*j).second;
}

void GribMetaData::getValue(const std::string& key,std::string& value) const
{
    std::map<std::string,std::string>::const_iterator j = stringValues_.find(key);
    if(j == stringValues_.end()) throw eckit::UserError(std::string("GribMetaData::getString failed for [") + key + "]");
    value = (*j).second;
}

void GribMetaData::print(std::ostream& os) const {
    os << "GribMetaData[]";
}


//----------------------------------------------------------------------------------------------------------------------

} // namespace grib
} // namespace metkit

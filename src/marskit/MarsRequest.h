/*
 * (C) Copyright 1996-2013 ECMWF.
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

#ifndef marskit_MarsRequest_H
#define marskit_MarsRequest_H

#include "eckit/types/Date.h"
#include "eckit/types/Double.h"
#include "eckit/types/Time.h"
#include "eckit/value/Value.h"

namespace eckit {
    class JSON;
    class MD5;
}

namespace marskit {

//----------------------------------------------------------------------------------------------------------------------

class MarsRequest {

public: // types

    typedef std::list< std::string > Values;
    typedef std::map< std::string, Values, std::less<std::string> >  Params;

public: // methods

// -- Contructors

	MarsRequest();
	MarsRequest(const std::string&);
    MarsRequest(eckit::Stream&);

    MarsRequest(const eckit::ValueMap&);

// -- Copy

	MarsRequest(const MarsRequest&);
	MarsRequest& operator=(const MarsRequest&);

// -- Destructor

    ~MarsRequest();

// -- Operators

	// eckit::Value&        operator[](const std::string&);
	// const eckit::Value&  operator[](const std::string&) const;

	operator eckit::Value() const;

// -- Methods


	const std::string& name() const { return name_; }

	long getValues(const std::string&,std::vector<std::string>&,bool = false) const;
	long getValues(const std::string&,std::vector<double>&,bool = false) const;
	long getValues(const std::string&,std::vector<eckit::Double>&,bool = false) const;
	long getValues(const std::string&,std::vector<long>&,bool = false) const;
	long getValues(const std::string&,std::vector<unsigned long>&,bool = false) const;
	long getValues(const std::string&,std::vector<eckit::Date>&,bool = false) const;
	long getValues(const std::string&,std::vector<eckit::Time>&,bool = false) const;
	long getValues(const std::string&,std::vector<char>&,bool = false) const;
	long getValues(const std::string&,std::vector<eckit::Value>&,bool = false) const;

	long getParams(std::vector<std::string>&, bool = false) const;

	void name(const std::string&);

	void setValues(const std::string&,const std::vector<std::string>&);
	void setValues(const std::string&,const std::vector<long>&);
	void setValues(const std::string&,const std::vector<unsigned long>&);
	void setValues(const std::string&,const std::vector<eckit::Date>&);
	void setValues(const std::string&,const std::vector<eckit::Time>&);
	void setValues(const std::string&,const std::vector<char>&);
	void setValues(const std::string&,const std::vector<double>&);
	void setValues(const std::string&,const std::vector<eckit::Double>&);

    template<class T>
    void setValue(const std::string& name,const T& value)
        { std::vector<T> v(1,value); setValues(name, v); }

    void setValue(const std::string& name,const char* value)
        { std::string v(value); setValue(name,v); }

	void unsetValues(const std::string&);

	/// Merges one MarsRequest into another
	/// @todo Improve performance -- uses O(N^2) search / merge in std::list's
	void merge(const MarsRequest& other);

	void json(eckit::JSON&) const;

    void md5(eckit::MD5&) const;

private: // members

    std::string	    name_;
	Params          params_;

private: // methods

	void print(std::ostream&) const;
    void encode(eckit::Stream&) const;

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

} // namespace marskit

#endif

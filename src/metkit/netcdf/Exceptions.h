/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#ifndef Exceptions_H
#define Exceptions_H

#include <string>
#include <sstream>


class NCError : public std::exception
{
    std::string msg_;
    virtual const char *what() const throw();
public:
    NCError(int e, const std::string &call, const std::string &path);
    virtual ~NCError() throw();
};


class NotImplemented : public std::exception
{
    std::string msg_;
    virtual const char *what() const throw() {
        return msg_.c_str();
    }
public:
    NotImplemented(const std::string &msg) : msg_(msg) {};
    virtual ~NotImplemented() throw() {};
};

class AssertionFailed : public std::exception
{
    std::string msg_;
    virtual const char *what() const throw() {
        return msg_.c_str();
    }
public:
    AssertionFailed(const std::string &msg) : msg_(msg) {};
    virtual ~AssertionFailed() throw() {};
};

class MergeError : public std::exception
{
    std::string msg_;
    virtual const char *what() const throw() {
        return msg_.c_str();
    }
public:
    MergeError(const std::string &msg) : msg_(std::string("MergeError: ") + msg) {};
    virtual ~MergeError() throw() {};
};

inline int _nc_call(int e, const char *call, const std::string &path)
{
    if (e)
    {
        throw  NCError(e, call, path);
    }
    return e;
}

inline void _assert(int e, const char *call, const char *file, int line, const char *function)
{
    if (!e)
    {
        std::stringstream s;
        s << "Assertion Failed: " << call << ", file " << file << ", line " << line << " (" << function << ")";
        throw  AssertionFailed(s.str());
    }
}

inline void _notimp(const char *file, int line, const char *function)
{
    std::stringstream s;
    s << "NotImplemented: " << file << ", line " << line << " (" << function << ")";
    throw  NotImplemented(s.str());
}


#define NC_CALL(a, path) _nc_call(a, #a, path)
#define ASSERT(a) _assert(a, #a, __FILE__, __LINE__, __FUNCTION__)
#define NOTIMP _notimp(__FILE__, __LINE__, __FUNCTION__)

#endif

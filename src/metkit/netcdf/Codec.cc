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

#include "Codec.h"

#include "Exceptions.h"

#include <iostream>


Codec::Codec()
{
}

Codec::~Codec() {
}

void Codec::decode(std::vector<double> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::decode(std::vector<float> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::decode(std::vector<long> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::decode(std::vector<short> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::decode(std::vector<unsigned char> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::encode(std::vector<double> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::encode(std::vector<float> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::encode(std::vector<long> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::encode(std::vector<short> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::encode(std::vector<unsigned char> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::addAttributes(Variable &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Codec::updateAttributes(int nc, int varid, const std::string &path) {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

bool Codec::timeAxis() const {
    return false;
}

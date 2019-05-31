/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @author Simon Smart

/// @date Febuary 2019

#ifndef metkit_ParamID_H
#define metkit_ParamID_H

#include <vector>
#include <set>
#include <algorithm>
#include <string>

#include "eckit/config/Resource.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/Log.h"
#include "eckit/types/Types.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/Param.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

class ParamID {

public: // types

    struct WindFamily {
        Param u_;
        Param v_;
        Param vo_;
        Param d_;

        WindFamily(const std::string& u, const std::string& v, const std::string& vo, const std::string& d):
            u_(u), v_(v), vo_(vo), d_(d) {}
    };

public: // methods

    template <typename REQUEST_T, typename AXIS_T>
    static void normalise(const REQUEST_T& r,
                          std::vector<Param>& req,
                          const AXIS_T& axis,
                          bool& windConversion);

private: // methods

    static const std::vector<WindFamily>& getWindFamilies();
};

//----------------------------------------------------------------------------------------------------------------------

template <typename REQUEST_T, typename AXIS_T>
void ParamID::normalise(const REQUEST_T& r,
                        std::vector<Param>& req,
                        const AXIS_T& axis,
                        bool& windConversion) {

    static const bool useGRIBParamID = eckit::Resource<bool>("useGRIBParamID", false);

    const std::vector<WindFamily>& windFamilies(getWindFamilies());

    if (useGRIBParamID) {


        std::set<Param> inAxis;
        for (typename AXIS_T::const_iterator j = axis.begin(); j != axis.end(); ++j) {
            inAxis.insert((*j));
        }

        std::vector<Param> newreq; newreq.reserve(req.size());

        for (std::vector<Param>::const_iterator k = req.begin(); k != req.end(); ++k) {
            const Param& p = (*k);

            Param alt;

            if (p.table()) {
                alt = Param(0, (p.table() == 128 ? 0 : p.table()) * 1000 + p.value()); // No '.' version
            }
            else {
                size_t t = p.value() / 1000;
                size_t v = p.value() % 1000;
                alt = Param(t == 0 ? 128 : t, v); // '.' version
            }

            if (inAxis.find(p) != inAxis.end()) {
                newreq.push_back(p);
            }
            else if (inAxis.find(alt) != inAxis.end()) {
                newreq.push_back(alt);
            } else {
                newreq.push_back(p);
            }

            eckit::Log::info() << "useGRIBParamID p=" << p
                               << ", alt=" << alt
                               << ", choice=" << newreq.back() << std::endl;

        }


        req = newreq;
    }
    else {

        std::set<long> tables;
        std::set<Param> inAxis;
        std::set<Param> inRequest;
        for (typename AXIS_T::const_iterator j = axis.begin(); j != axis.end(); ++j)
        {
            tables.insert((*j).table());
            inAxis.insert((*j));
        }

        std::vector<Param> newreq; newreq.reserve(req.size());

        // We have GRIB2 in the cube
        if (tables.find(0) != tables.end()) {

            if (tables.size() == 1) { // GRIB2 only

                eckit::Log::debug<LibMetkit>() << "Layout contains only GRIB2 fields" << std::endl;

                for (std::vector<Param>::const_iterator k = req.begin(); k != req.end(); ++k)
                {
                    eckit::Ordinal t = (*k).table();
                    eckit::Ordinal v = (*k).value();
                    // If user specifies param.value
                    if (t)
                    {
                        Param p(0, (t == 128 ? 0 : t) * 1000 + v);
                        //Log::userWarning() << "Parameter " << (*k) << " changed to " << p << std::endl;
                        newreq.push_back(p);
                        inRequest.insert(p);
                    }
                    else
                    {
                        if (inAxis.find(*k) != inAxis.end()) {
                            // Perfect match
                            newreq.push_back(*k);
                            inRequest.insert(*k);
                        }
                        else {
                            bool ok = false;
                            for (typename AXIS_T::const_iterator j = axis.begin(); j != axis.end(); ++j)
                            {
                                Param p(*j);
                                if ((p.value() % 1000) == v) {
                                    //Log::userWarning() << "Trying parameter " << p << " for " << (*k) << ", please change your request" << std::endl;
                                    newreq.push_back(p);
                                    inRequest.insert(p);
                                    ok = true;
                                }
                            }

                            // Special case for U/V
                            if (!ok) {
                                int wind = -1;

                                Param param(0, (t == 128 ? 0 : t) * 1000 + v);

                                // Try exact match
                                for (eckit::Ordinal w = 0; w < windFamilies.size() ; w++) {

                                    if (param == windFamilies[w].u_ || param == windFamilies[w].v_) {
                                        wind = w;
                                        break;
                                    }
                                }

                                if (wind == -1) {
                                    // Try partial match
                                    for (eckit::Ordinal w = 0; w < windFamilies.size() ; w++) {

                                        if (param.value() == windFamilies[w].u_.value() || param.value() == windFamilies[w].v_.value()) {
                                            wind = w;
                                            break;
                                        }
                                    }
                                }

                                if (wind != -1) {
                                    for (eckit::Ordinal w = 0; w < windFamilies.size() ; w++) {

                                        if (inAxis.find(windFamilies[w].vo_) != inAxis.end() && inAxis.find(windFamilies[w].d_) != inAxis.end()) {
                                            Param p(v == windFamilies[w].u_.value() ? windFamilies[w].u_ : windFamilies[w].v_);
                                            if (inRequest.find(p) == inRequest.end()) {
                                                //Log::userWarning() << "Trying parameter " << p << " for " << (*k) << " (wind field)" << std::endl;
                                                newreq.push_back(p);
                                                inRequest.insert(p);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

            }
            else // GRIB1 and GRIB2 mixed
            {
                eckit::Log::debug<LibMetkit>() << "Layout contains a mixture of GRIB1 and GRIB2 fields" << std::endl;

                for (std::vector<Param>::const_iterator k = req.begin(); k != req.end(); ++k)
                {
                    eckit::Ordinal t = (*k).table();
                    eckit::Ordinal v = (*k).value();

                    eckit::Log::debug<LibMetkit>() << "Trying to match " << (*k) << " t:" << t << " v:" << v << std::endl;

                    bool ok = false;

                    // Push perfect match
                    {
                        // Block for (p)
                        Param p(*k);
                        if (inAxis.find(p) != inAxis.end() && inRequest.find(p) == inRequest.end()) {
                            eckit::Log::debug<LibMetkit>() << "Trying parameter " << p << " for " << (*k) << " @ " << Here() << std::endl;
                            newreq.push_back(p);
                            inRequest.insert(p);
                            ok = true;
                        }
                    }


                    if (t) {
                        Param p(0, (t == 128 ? 0 : t) * 1000 + v);
                        // User specifies xxx.yyy
                        if (inAxis.find(p) != inAxis.end() && inRequest.find(p) == inRequest.end()) {
                            eckit::Log::debug<LibMetkit>() << "Trying parameter " << p << " for " << (*k) << " @ " << Here() << std::endl;
                            newreq.push_back(p);
                            inRequest.insert(p);
                            ok = true;
                        }
                    }

                    if (t == 0 && v >= 1000) {
                        // User specifies yyyxxx
                        Param p(v / 1000, v % 1000);
                        if (inAxis.find(p) != inAxis.end() && inRequest.find(p) == inRequest.end()) {
                            eckit::Log::debug<LibMetkit>() << "Trying parameter " << p << " for " << (*k) << " @ " << Here() << std::endl;
                            newreq.push_back(p);
                            inRequest.insert(p);
                            ok = true;
                        }
                    }

                    if (t == 0 && v < 1000) {
                        // User specifies xxx

                        // prioritise 128
                        for (typename AXIS_T::const_iterator j = axis.begin(); j != axis.end(); ++j)
                        {
                            Param p(*j);
                            if(p.table() != 128) continue;
                            if ((p.value() % 1000) == v) {
                                if (inRequest.find(p) == inRequest.end()) {
                                    eckit::Log::debug<LibMetkit>() << "Trying parameter " << p << " for " << (*k) << " @ " << Here() << std::endl;
                                    newreq.push_back(p);
                                    inRequest.insert(p);
                                    eckit::Log::debug<LibMetkit>() << "inRequest: " << inRequest << std::endl;
                                    ok = true;
                                }
                            }
                        }

                        if (!ok) {
                            for (typename AXIS_T::const_iterator j = axis.begin(); j != axis.end(); ++j)
                            {
                                Param p(*j);
                                if ((p.value() % 1000) == v) {
                                    if (inRequest.find(p) == inRequest.end()) {
                                        eckit::Log::debug<LibMetkit>() << "Trying parameter " << p << " for " << (*k) << " @ " << Here() << std::endl;
                                        newreq.push_back(p);
                                        inRequest.insert(p);
                                        eckit::Log::debug<LibMetkit>() << "inRequest: " << inRequest << std::endl;
                                        ok = true;
                                    }
                                }
                            }
                        }
                    }

                    // Special case for U/V
                    if (!ok) {
                        int wind = -1;

                        // Find the most likely match
                        for (eckit::Ordinal w = 0; w < windFamilies.size() ; w++) {
                            if (windFamilies[w].u_.table() == 0 && windFamilies[w].v_.table() == 0) {
                                if (v == windFamilies[w].u_.value() || v == windFamilies[w].v_.value() ) {
                                    wind = w;
                                    break;
                                }
                            }
                        }

                        if (wind != -1) {
                            for (eckit::Ordinal w = 0; w < windFamilies.size() ; w++) {
                                if (inAxis.find(windFamilies[w].vo_) != inAxis.end() && inAxis.find(windFamilies[w].d_) != inAxis.end()) {
                                    Param p(v == windFamilies[w].u_.value() ? windFamilies[w].u_ : windFamilies[w].v_);
                                    if (inRequest.find(p) == inRequest.end()) {
                                        eckit::Log::debug<LibMetkit>() << "Trying parameter " << p << " for " << (*k) << " (wind field)" << std::endl;
                                        newreq.push_back(p);
                                        inRequest.insert(p);
                                    }
                                }
                            }
                        }
                    }

                }
            }
        }
        else
        {
            // GRIB1 only

            eckit::Log::debug<LibMetkit>() << "Layout contains GRIB1 fields" << std::endl;

            for (std::vector<Param>::const_iterator k = req.begin(); k != req.end(); ++k)
            {
                eckit::Ordinal t = (*k).table();
                eckit::Ordinal v = (*k).value();

                if (t)
                {
                    newreq.push_back(*k);
                    inRequest.insert(*k);
                }
                else
                {
                    if (v > 1000) {
                        // Asking for param=228130, old style (not a paramId)
                        Param p(v / 1000, v % 1000);
                        newreq.push_back(p);
                        inRequest.insert(p);
                    }
                    else
                    {
                        // Asking for param=130, old style (not a paramId)
                        bool ok = false;

                        // Try 130.128

                        Param p(128, v);
                        if(std::find(axis.begin(), axis.end(), p) != axis.end()) {
                            // This is a match
                            ok = true;
                            newreq.push_back(p);
                        }
                        else {

                            for (typename AXIS_T::const_iterator j = axis.begin(); j != axis.end(); ++j)
                            {
                                Param p(*j);
                                if ((p.value() % 1000) == v) {
                                    if (inRequest.find(p) == inRequest.end()) {
                                        //Log::userWarning() << "Trying parameter " << p << " for " << (*k) << std::endl;
                                        newreq.push_back(p);
                                        inRequest.insert(p);
                                        ok = true;
                                    }
                                }
                            }
                        }

                        // Special case for U/V
                        if (!ok) {
                            int wind = -1;
                            Param param(*k);

                            // Try exact match
                            for (eckit::Ordinal w = 0; w < windFamilies.size() ; w++) {

                                if (param == windFamilies[w].u_ || param == windFamilies[w].v_) {
                                    wind = w;
                                    break;
                                }
                            }

                            if (wind == -1) {
                                // Try partial match
                                for (eckit::Ordinal w = 0; w < windFamilies.size() ; w++) {

                                    if (param.value() == windFamilies[w].u_.value() || param.value() == windFamilies[w].v_.value()) {
                                        wind = w;
                                        break;
                                    }
                                }
                            }


                            if (wind != -1) {
                                for (eckit::Ordinal w = 0; w < windFamilies.size() ; w++) {
                                    if (inAxis.find(windFamilies[w].vo_) != inAxis.end() && inAxis.find(windFamilies[w].d_) != inAxis.end()) {
                                        Param p(v == windFamilies[w].u_.value() ? windFamilies[w].u_ : windFamilies[w].v_);
                                        if (inRequest.find(p) == inRequest.end()) {
                                            //Log::userWarning() << "Trying parameter " << p << " for " << (*k) << " (wind field)" << std::endl;
                                            newreq.push_back(p);
                                            inRequest.insert(p);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        req = newreq;
    }

    for (eckit::Ordinal w = 0; w < windFamilies.size() ; w++)
    {

        const Param windU(windFamilies[w].u_);
        const Param windV(windFamilies[w].v_);
        const Param windVO(windFamilies[w].vo_);
        const Param windD(windFamilies[w].d_);

        //Log::userWarning() <<  "Trying uv " << windU << " " << windV << " " << windVO << " " << windD << std::endl;


        bool wantU  = false;
        bool wantV  = false;
        bool wantVO = false;
        bool wantD  = false;

        // Check if wind is requested

        for (eckit::Ordinal i = 0; i < req.size() ; i++)
        {
            if (req[i] == windU)  wantU  = true;
            if (req[i] == windV)  wantV  = true;
            if (req[i] == windVO) wantVO = true;
            if (req[i] == windD)  wantD  = true;
            //Log::userWarning() << "req[i] = " << req[i] << std::endl;
        }

        //Log::userWarning() <<  "wantU " << wantU << " wantV " << wantV << " wantVO " << wantVO << " wantD " << wantD << std::endl;

        // if (wantVO && wantD)  continue;
        // if (!wantU && !wantV) continue;

        // Check if we have got it, axis should be sorted

        bool gotU = false;
        bool gotV = false;

        if (wantU)
            gotU = std::binary_search(axis.begin(), axis.end(), windU);

        if (wantV)
            gotV = std::binary_search(axis.begin(), axis.end(), windV);


        if ( (wantU && !gotU) || (wantV && !gotV))
        {
            // Push VO and D if needed
            if (!wantVO) req.push_back(windVO);
            if (!wantD)  req.push_back(windD);

            eckit::Log::debug<LibMetkit>() << "U/V conversion requested U=" << windU << ", V=" << windV << ", VO=" << windVO << ", D=" << windD << std::endl;
            windConversion = true;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

#endif

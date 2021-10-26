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
#include "metkit/mars/Param.h"

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

    static const std::vector<WindFamily>& getWindFamilies();
    static const std::map<size_t, std::set<size_t>>& getParamTableExpansion();

};

//----------------------------------------------------------------------------------------------------------------------
Param ToParamID(Param p) {
    eckit::Ordinal t = (p.table() == 0 ? p.value() / 1000 : p.table());
    eckit::Ordinal v = p.value() % 1000;

    return Param(0, (t == 128 ? 0 : t) * 1000 + v);
}

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

            eckit::Log::debug<LibMetkit>() << "useGRIBParamID p=" << p
                               << ", alt=" << alt
                               << ", choice=" << newreq.back() << std::endl;

        }


        req = newreq;

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
    else {

        std::set<Param> inAxis;
        std::map<Param, Param> inAxisParamID;
        std::set<Param> wind;
        for (typename AXIS_T::const_iterator j = axis.begin(); j != axis.end(); ++j) {
            inAxis.emplace(*j);
            inAxisParamID[ToParamID(*j)] = (*j);
        }

        std::vector<Param> newreq; newreq.reserve(req.size());

        for (std::vector<Param>::const_iterator k = req.begin(); k != req.end(); ++k)
        {
            bool ok = false;
            Param paramid = ToParamID(*k);

            if (inAxis.find(*k) != inAxis.end()) { // Perfect match - not looking forward
                newreq.push_back(*k);
                ok = true;
            }
            if (!ok) {
                auto ap = inAxisParamID.find(paramid);
                if (ap != inAxisParamID.end()) { // Perfect match (beside GRIB1 or GRIB2 representation) - not looking forward
                    newreq.push_back(ap->second);
                    ok = true;
                }
                if (!ok) { // Special case for U/V - exact match
                    for (eckit::Ordinal w = 0; w < windFamilies.size() ; w++) {
                        if ((paramid == ToParamID(windFamilies[w].u_) || paramid == ToParamID(windFamilies[w].v_)) &&
                            inAxis.find(windFamilies[w].vo_) != inAxis.end() && inAxis.find(windFamilies[w].d_) != inAxis.end()) {

                            if (paramid == ToParamID(windFamilies[w].u_))
                                newreq.push_back(windFamilies[w].u_);
                            else
                                newreq.push_back(windFamilies[w].v_);

                            wind.emplace(windFamilies[w].vo_);
                            wind.emplace(windFamilies[w].d_);
                            windConversion = true;

                            ok = true;
                            break;
                        }
                    }
                    if (!ok) { // Partial match
                        size_t uv = paramid.value() % 1000;
                        size_t ut = paramid.value() / 1000;

                        for (typename AXIS_T::const_iterator j = axis.begin(); !ok && j != axis.end(); ++j)
                        {
                            Param p = ToParamID(*j);
                            size_t av = p.value() % 1000;
                            size_t at = p.value() / 1000;

                            if (av == uv) { // values are matching - check param-table expansion

                                const std::map<size_t, std::set<size_t>>& pte = ParamID::getParamTableExpansion();
                                auto pt = pte.find(uv);

                                if (pt != pte.end() &&          // param-table expansion allowed
                                    (at == 0 || pt->second.find(at) != pt->second.end()) &&
                                    (ut == 0 || pt->second.find(ut) != pt->second.end())) {

                                    //Log::userWarning() << "Trying parameter " << p << " for " << (*k) << ", please change your request" << std::endl;
                                    newreq.push_back(*j);
                                    ok = true;
                                    break;
                                }
                            }
                            if (!ok && ut == 0) { // Special case for U/V - partial match
                                for (eckit::Ordinal w = 0; !ok && w < windFamilies.size() ; w++) {
                                    if (uv == windFamilies[w].u_.value()%1000 || uv == windFamilies[w].v_.value()%1000) {
                                        auto vo = inAxisParamID.find(ToParamID(windFamilies[w].vo_));
                                        auto d = inAxisParamID.find(ToParamID(windFamilies[w].d_));
                                        if (vo != inAxisParamID.end() && d != inAxisParamID.end()) {

                                            if (uv == windFamilies[w].u_.value()%1000)
                                                newreq.push_back(windFamilies[w].u_);
                                            else
                                                newreq.push_back(windFamilies[w].v_);

                                            wind.emplace(vo->second);
                                            wind.emplace(d->second);
                                            windConversion = true;

                                            ok = true;
                                            break;
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

        for (auto w: wind) {
            bool exist = false;
            for (eckit::Ordinal i = 0; i < req.size() ; i++)
                if (req[i] == w) {
                    exist = true;
                    break;
                }
            if (!exist) {
                req.push_back(w);
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit

#endif

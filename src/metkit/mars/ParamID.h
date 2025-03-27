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

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "eckit/config/Resource.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/Log.h"
#include "eckit/runtime/Metrics.h"
#include "eckit/types/Types.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/Param.h"

namespace metkit {

//----------------------------------------------------------------------------------------------------------------------

class ParamID {

public:  // types

    struct WindFamily {
        Param u_;
        Param v_;
        Param vo_;
        Param d_;

        WindFamily(const std::string& u, const std::string& v, const std::string& vo, const std::string& d) :
            u_(u), v_(v), vo_(vo), d_(d) {}
    };

public:  // methods

    template <typename REQUEST_T, typename AXIS_T>
    static void normalise(const REQUEST_T& r, std::vector<Param>& req, const AXIS_T& axis, bool& windConversion,
                          bool fullTableDropping = ParamID::fullTableDropping());

    static const std::vector<WindFamily>& getWindFamilies();
    static const std::vector<size_t>& getDropTables();
    static bool fullTableDropping();
};

//----------------------------------------------------------------------------------------------------------------------

inline long replaceTable(size_t table, long paramid) {
    return (table * 1000 + paramid % 1000);
}

template <typename REQUEST_T, typename AXIS_T>
void ParamID::normalise(const REQUEST_T& request, std::vector<Param>& req, const AXIS_T& axis, bool& windConversion,
                        bool fullTableDropping) {

    static const bool useGRIBParamID = eckit::Resource<bool>("useGRIBParamID", false);

    const std::vector<WindFamily>& windFamilies(getWindFamilies());


    if (useGRIBParamID) {


        std::set<Param> inAxis;
        for (typename AXIS_T::const_iterator j = axis.begin(); j != axis.end(); ++j) {
            inAxis.insert((*j));
        }

        std::vector<Param> newreq;
        newreq.reserve(req.size());

        for (std::vector<Param>::const_iterator k = req.begin(); k != req.end(); ++k) {
            const Param& p = (*k);

            Param alt;

            if (p.table()) {
                alt = Param(0, (p.table() == 128 ? 0 : p.table()) * 1000 + p.value());  // No '.' version
            }
            else {
                size_t t = p.value() / 1000;
                size_t v = p.value() % 1000;
                alt      = Param(t == 0 ? 128 : t, v);  // '.' version
            }

            if (inAxis.find(p) != inAxis.end()) {
                newreq.push_back(p);
            }
            else if (inAxis.find(alt) != inAxis.end()) {
                newreq.push_back(alt);
            }
            else {
                newreq.push_back(p);
            }

            LOG_DEBUG_LIB(LibMetkit) << "useGRIBParamID p=" << p << ", alt=" << alt << ", choice=" << newreq.back()
                                     << std::endl;
        }


        req = newreq;

        for (eckit::Ordinal w = 0; w < windFamilies.size(); w++) {

            const Param windU(windFamilies[w].u_);
            const Param windV(windFamilies[w].v_);
            const Param windVO(windFamilies[w].vo_);
            const Param windD(windFamilies[w].d_);

            // Log::userWarning() <<  "Trying uv " << windU << " " << windV << " " << windVO << " " << windD <<
            // std::endl;


            bool wantU  = false;
            bool wantV  = false;
            bool wantVO = false;
            bool wantD  = false;

            // Check if wind is requested

            for (eckit::Ordinal i = 0; i < req.size(); i++) {
                if (req[i] == windU)
                    wantU = true;
                if (req[i] == windV)
                    wantV = true;
                if (req[i] == windVO)
                    wantVO = true;
                if (req[i] == windD)
                    wantD = true;
                // Log::userWarning() << "req[i] = " << req[i] << std::endl;
            }

            // Log::userWarning() <<  "wantU " << wantU << " wantV " << wantV << " wantVO " << wantVO << " wantD " <<
            // wantD << std::endl;

            // if (wantVO && wantD)  continue;
            // if (!wantU && !wantV) continue;

            // Check if we have got it, axis should be sorted

            bool gotU = false;
            bool gotV = false;

            if (wantU)
                gotU = std::binary_search(axis.begin(), axis.end(), windU);

            if (wantV)
                gotV = std::binary_search(axis.begin(), axis.end(), windV);


            if ((wantU && !gotU) || (wantV && !gotV)) {
                // Push VO and D if needed
                if (!wantVO)
                    req.push_back(windVO);
                if (!wantD)
                    req.push_back(windD);

                LOG_DEBUG_LIB(LibMetkit) << "U/V conversion requested U=" << windU << ", V=" << windV
                                         << ", VO=" << windVO << ", D=" << windD << std::endl;
                windConversion = true;
            }
        }
    }
    else {

        std::vector<std::pair<Param, Param> > tableDropped;

        std::set<Param> inAxis;
        std::map<long, Param> inAxisParamID;
        std::set<Param> wind;

        for (typename AXIS_T::const_iterator j = axis.begin(); j != axis.end(); ++j) {
            inAxis.emplace(*j);
            inAxisParamID[j->paramId()] = *j;
        }

        std::vector<Param> newreq;
        newreq.reserve(req.size());

        for (auto r : req) {
            if (inAxis.find(r) != inAxis.end()) {  // Perfect match - not looking forward
                newreq.push_back(r);
            }
            else {  // r is normalised to ParamID
                long paramid = r.paramId();
                auto ap      = inAxisParamID.find(paramid);
                if (ap != inAxisParamID.end()) {  // ParamID representation matching - not looking forward
                    newreq.push_back(ap->second);
                }
                else {  // Special case for U/V - exact match
                    bool ok = false;
                    for (eckit::Ordinal w = 0; w < windFamilies.size(); w++) {
                        if ((paramid == windFamilies[w].u_.paramId() || paramid == windFamilies[w].u_.grib1value() ||
                             paramid == windFamilies[w].v_.paramId() || paramid == windFamilies[w].v_.grib1value()) &&
                            inAxis.find(windFamilies[w].vo_) != inAxis.end() &&
                            inAxis.find(windFamilies[w].d_) != inAxis.end()) {

                            if (paramid == windFamilies[w].u_.paramId() || paramid == windFamilies[w].u_.grib1value())
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
                    if (!ok && r.table() == 0 &&
                        paramid < 1000) {  // Partial match (only it table has not been specified by user)
                        const std::vector<size_t>& dropTables = ParamID::getDropTables();
                        for (auto t : dropTables) {
                            auto ap = inAxisParamID.find(replaceTable(t, paramid));
                            if (ap != inAxisParamID.end()) {  // ParamID representation matching - not looking forward
                                newreq.push_back(ap->second);
                                ok = true;
                                break;
                            }
                        }

                        if (!ok) {  // Special case for U/V - partial match
                            for (eckit::Ordinal w = 0; !ok && w < windFamilies.size(); w++) {
                                if (paramid == windFamilies[w].u_.paramId() ||
                                    paramid == windFamilies[w].v_.paramId()) {
                                    for (auto t : dropTables) {
                                        auto vo = inAxisParamID.find(replaceTable(t, windFamilies[w].vo_.paramId()));
                                        auto d  = inAxisParamID.find(replaceTable(t, windFamilies[w].d_.paramId()));

                                        if (vo != inAxisParamID.end() && d != inAxisParamID.end()) {
                                            bool grib1 = vo->second.table() > 0;
                                            if (paramid == windFamilies[w].u_.paramId())
                                                newreq.push_back(grib1 ? Param(t, paramid)
                                                                       : Param(0, replaceTable(t, paramid)));
                                            else
                                                newreq.push_back(grib1 ? Param(t, paramid)
                                                                       : Param(0, replaceTable(t, paramid)));

                                            wind.emplace(vo->second);
                                            wind.emplace(d->second);
                                            windConversion = true;

                                            ok = true;
                                            break;
                                        }
                                    }
                                    if (ok)
                                        break;
                                }
                            }
                        }
                        if (fullTableDropping &&
                            !ok) {  // Backward compatibility - Partial match (drop completely table information)
                            for (auto ap : inAxisParamID) {
                                if (ap.first % 1000 == paramid % 1000) {
                                    newreq.push_back(ap.second);
                                    ok = true;
                                    tableDropped.push_back(std::make_pair(r, ap.second));
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        req = newreq;

        for (auto w : wind) {
            bool exist = false;
            for (eckit::Ordinal i = 0; i < req.size(); i++)
                if (req[i] == w) {
                    exist = true;
                    break;
                }
            if (!exist) {
                req.push_back(w);
            }
        }
        if (tableDropped.size() > 0) {
            eckit::MetricsPrefix prefix("paramid_normalisation");
            {
                std::ostringstream oss;
                oss << request;
                eckit::Metrics::set("user_request", oss.str());
            }
            {
                std::ostringstream oss;
                oss << tableDropped;
                eckit::Metrics::set("params", oss.str());
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit

#endif

/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/mars/rules/Cost.h"

#include "eckit/log/Bytes.h"
#include "eckit/log/JSON.h"
#include "eckit/log/Plural.h"
#include "eckit/runtime/Metrics.h"
#include "eckit/serialisation/Stream.h"
#include "eckit/types/Types.h"

using namespace eckit;

namespace metkit::mars::rules {

//----------------------------------------------------------------------------------------------------------------------

void Cost::reset() {

    layout_ = 0;

    onLine_ = 0;
    offLine_ = 0;

    tapes_ = 0;
    disks_ = 0;
    unavailable_ = 0;
    offsite_ = 0;

    onLineFields_ = 0;
    offLineFields_ = 0;

    updated_ = 0;

    media_.clear();
    nodes_.clear();
    libraries_.clear();
    damaged_.clear();
}

eckit::Stream& operator<<(eckit::Stream& s, const Cost& c) {
    s << c.layout_;
    s << c.onLine_;
    s << c.offLine_;
    s << c.tapes_;
    s << c.disks_;
    s << c.unavailable_;
    s << c.offsite_;
    s << c.onLineFields_;
    s << c.offLineFields_;
    s << c.updated_;

    s << Ordinal(c.damaged_.size());
    for (std::set<std::string>::const_iterator j = c.damaged_.begin(); j != c.damaged_.end(); ++j)
        s << *j;

    s << Ordinal(c.media_.size());
    for (std::set<std::string>::const_iterator j = c.media_.begin(); j != c.media_.end(); ++j)
        s << *j;

    s << Ordinal(c.nodes_.size());
    for (std::set<std::string>::const_iterator j = c.nodes_.begin(); j != c.nodes_.end(); ++j)
        s << *j;

    s << Ordinal(c.libraries_.size());
    for (std::set<std::string>::const_iterator j = c.libraries_.begin(); j != c.libraries_.end(); ++j)
        s << *j;

    return s;
}

eckit::Stream& operator>>(eckit::Stream& s, Cost& c) {
    s >> c.layout_;
    s >> c.onLine_;
    s >> c.offLine_;
    s >> c.tapes_;
    s >> c.disks_;
    s >> c.unavailable_;
    s >> c.offsite_;
    s >> c.onLineFields_;
    s >> c.offLineFields_;
    s >> c.updated_;

    Ordinal n;

    s >> n;
    while (n-- > 0) {
        std::string m;
        s >> m;
        c.damaged_.insert(m);
    }

    s >> n;
    while (n-- > 0) {
        std::string m;
        s >> m;
        c.media_.insert(m);
    }

    s >> n;
    while (n-- > 0) {
        std::string m;
        s >> m;
        c.nodes_.insert(m);
    }

    s >> n;
    while (n-- > 0) {
        std::string m;
        s >> m;
        c.libraries_.insert(m);
    }

    return s;
}

Cost& Cost::operator+=(const Cost& other) {
    layout_ += other.layout_;
    onLine_ += other.onLine_;
    offLine_ += other.offLine_;
    tapes_ += other.tapes_;
    disks_ += other.disks_;
    unavailable_ += other.unavailable_;
    offsite_ += other.offsite_;
    onLineFields_ += other.onLineFields_;
    offLineFields_ += other.offLineFields_;

    for (auto& c: other.damaged_) {
        damaged_.insert(c);
    }

    for (auto& c: other.media_) {
        media_.insert(c);
    }

    for (auto& c: other.nodes_) {
        nodes_.insert(c);
    }

    for (auto& c: other.libraries_) {
        libraries_.insert(c);
    }

    return *this;
}

void Cost::print(std::ostream& s) const {

    s << Plural(onLineFields_ + offLineFields_, "field");
    s << ", ";

    if (disks_)
        s << Bytes(onLine_) << " online";

    if (disks_ && tapes_)
        s << ", ";

    if (tapes_) {
        s << Bytes(offLine_) << " on " << Plural(media_.size(), "tape");
    }

    if (unavailable_)
        s << " warning: " << Plural(unavailable_, "unavailable file");

    if (offsite_)
        s << " warning: " << Plural(offsite_, "off-site file");

    if (damaged_.size()) {
        if (damaged_.size() > 1) {
            s << " warning: accessing damaged tages ";
            const char* sep = "";
            for (auto p: damaged_) {
                s << sep << p;
                sep = ", ";
            }
        } else {
            s << " warning: accessing damaged tage " << *(damaged_.begin());
        }
    }

    if (nodes_.size()) {
        s << ", nodes:";
        for (std::set<std::string>::const_iterator j = nodes_.begin(); j != nodes_.end(); ++j) {
            s << " " << (*j);
        }
    }

    if (libraries_.size()) {
        s << ", libraries:";
        for (std::set<std::string>::const_iterator j = libraries_.begin(); j != libraries_.end(); ++j) {
            s << " " << (*j);
        }
    }
}

void Cost::json(eckit::JSON& s) const {
    s.startObject();
    s << "layout" << layout_;
    s << "onLine" << onLine_;
    s << "offLine" << offLine_;
    s << "tapes" << tapes_;
    s << "disks" << disks_;
    s << "unavailable" << unavailable_;
    s << "offsite" << offsite_;
    s << "onLineFields" << onLineFields_;
    s << "offLineFields" << offLineFields_;
    s << "updated" << updated_;

    s << "damaged";
    s.startList();
    for (std::set<std::string>::const_iterator j = damaged_.begin(); j != damaged_.end(); ++j)
        s << *j;
    s.endList();

    s << "media";
    s.startList();
    for (std::set<std::string>::const_iterator j = media_.begin(); j != media_.end(); ++j)
        s << *j;
    s.endList();

    s << "nodes";
    s.startList();
    for (std::set<std::string>::const_iterator j = nodes_.begin(); j != nodes_.end(); ++j)
        s << *j;
    s.endList();

    s << "libraries";
    s.startList();
    for (std::set<std::string>::const_iterator j = libraries_.begin(); j != libraries_.end(); ++j)
        s << *j;
    s.endList();

    s.endObject();
}

void Cost::collectMetrics() const {

    if (layout_ > 0) {
        eckit::MetricsPrefix cost("cost");
        eckit::Metrics::set("layouts", layout_);
        eckit::Metrics::set("bytes_online", onLine_);
        eckit::Metrics::set("bytes_offline", offLine_);
        eckit::Metrics::set("tapes_files", tapes_);
        eckit::Metrics::set("disks_files", disks_);
        eckit::Metrics::set("files_unavailable", unavailable_);
        eckit::Metrics::set("files_offsite", offsite_);
        eckit::Metrics::set("damaged_tapes", damaged_);
        eckit::Metrics::set("online_fields", onLineFields_);
        eckit::Metrics::set("offline_fields", offLineFields_);
        eckit::Metrics::set("nodes", nodes_);
        eckit::Metrics::set("media", media_);
        eckit::Metrics::set("libraries", libraries_);
    }
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit::mars::rules

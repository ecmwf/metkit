/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/mars/rules/MarsExpression.h"

#include "metkit/mars/rules/AccessRule.h"
#include "metkit/mars/rules/QueuePermission.h"

#include "metkit/mars/StepRange.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars/rules/Cost.h"

#include "eckit/config/Resource.h"
#include "eckit/filesystem/TmpFile.h"
#include "eckit/io/Buffer.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/value/Value.h"

#include <algorithm>
#include <fstream>


using namespace eckit;

namespace metkit::mars::rules {

//----------------------------------------------------------------------------------------------------------------------

static Mutex local_mutex;


static QueuePermission* marsrules_permissions = nullptr;
static AccessRule* marsrules_accesses         = nullptr;

static std::string marsrules_path;

/* #define _CPP_IOSTREAMS 1 */

#define YYDEBUG 0

namespace MarsRulesYacc {

#if (defined __linux__) || (defined __APPLE__ && defined __MACH__)
void marsrules_error(char* msg) {
    std::ostringstream os;
    extern int marsrules_lineno;
    os << msg << " line " << marsrules_lineno << " of " << marsrules_path;
    throw eckit::SeriousBug(os.str());
}
#endif

#if YYDEBUG
extern int marsrules_debug;
#endif

extern int marsrules_lineno;

#ifdef VISUAL_AGE
extern "C" {
void marsrules_error(char* msg)
#else
void marsrules_error(const char* msg)
#endif
{
    // This documentation is here in case analysts come looking for what causes 'SeriousBug' exception
    // If ever chkrules or mars aborts with an error like:
    // Serious Bug:memory exhausted line 5039 of /users/max/mars/buildRules [marsdev-core]
    // This might be due to a not small initial parser stack size
    // which is controlled where 'rulesy.y' defines YYINITDEPTH -- try to increase this size

    std::ostringstream os;
    os << msg << " line " << marsrules_lineno << " of " << marsrules_path;
    throw eckit::SeriousBug(os.str());
}

#ifdef VISUAL_AGE
}
#endif

#include "metkit/mars/rules/rulesy.c"

}  //  end namespace MarsRulesYacc


struct Include {

    int lineno;
    std::string path;
    FILE* in;

    Include() : lineno(MarsRulesYacc::marsrules_lineno), path(marsrules_path), in(MarsRulesYacc::marsrules_in) {}
    ~Include() {}
};

static std::vector<Include> includeStack;

extern "C" int marsrules_wrap() {
    if (includeStack.size() == 0)
        return 1;

    Log::info() << "End of " << marsrules_path << " at " << MarsRulesYacc::marsrules_lineno << ", back to "
                << includeStack.back().path << " at " << includeStack.back().lineno << std::endl;

    fclose(MarsRulesYacc::marsrules_in);

    MarsRulesYacc::marsrules_lineno = includeStack.back().lineno;
    marsrules_path             = includeStack.back().path;
    MarsRulesYacc::marsrules_in     = includeStack.back().in;

    includeStack.pop_back();

    return 0;
}


int MarsRulesParser::line() {
    return MarsRulesYacc::marsrules_lineno;
}

void MarsRulesParser::include(const PathName& path) {
    std::string p = std::string(path);
    if (p[0] != '/')
        p = std::string(PathName(marsrules_path).dirName()) + "/" + std::string(path);

    Log::info() << "Including " << p << " from " << marsrules_path << " at " << MarsRulesYacc::marsrules_lineno << std::endl;

    includeStack.push_back(Include());

    FILE* in = ::fopen(p.c_str(), "r");
    if (!in)
        throw CantOpenFile(p);

    MarsRulesYacc::marsrules_lineno = 0;
    marsrules_path             = p;
    MarsRulesYacc::marsrules_in     = in;
}

//----------------------------------------------------------------------------------------------------------------------


template <class T>
static T* readFile(const PathName& path, T*& ptr) {
    LOG_DEBUG_LIB(LibMetkit) << "readFile: " << path << std::endl;

    includeStack.clear();

    marsrules_path = path;

    FILE* in = ::fopen(marsrules_path.c_str(), "r");
    if (!in)
        throw CantOpenFile(path);

    MarsRulesYacc::marsrules_lineno = 0;
    MarsRulesYacc::marsrules_in     = in;
#if YYDEBUG
    MarsRulesYacc::marsrules_debug = eckit::Resource<long>("$YYDEBUG;-marsrules_debug;marsrules_debug", 0);
#endif

    MarsRulesYacc::marsrules_parse();

    fclose(in);

    return ptr;
}

QueuePermission* MarsRulesParser::parsePermissionFile(const PathName& path) {
    LOG_DEBUG_LIB(LibMetkit) << "MarsParser::parsePermissionFile: " << path << std::endl;

    AutoLock<Mutex> lock(local_mutex);
    marsrules_permissions = nullptr;
    return readFile(path, marsrules_permissions);
}


AccessRule* MarsRulesParser::parseAccessFile(const PathName& path) {
    LOG_DEBUG_LIB(LibMetkit) << "MarsParser::parseAccessFile: " << path << std::endl;

    AutoLock<Mutex> lock(local_mutex);
    marsrules_accesses = nullptr;
    return readFile(path, marsrules_accesses);
}

//----------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------


typedef std::map<std::string, MarsExpressionFactory*, std::less<std::string> > EMap;
static EMap* m = 0;

MarsExpressionFactory::MarsExpressionFactory(const std::string& name) {
    LOG_DEBUG_LIB(LibMetkit) << "MarsExpressionFactory::MarsExpressionFactory: " << name << std::endl;

    if (m == 0)
        m = new EMap;
    (*m)[name] = this;
}

MarsExpression* MarsExpressionFactory::create(const std::string& s, const Value& v) {
    LOG_DEBUG_LIB(LibMetkit) << "MarsExpressionFactory::create: " << s << std::endl;

    AutoLock<Mutex> lock(local_mutex);
    ASSERT(m != 0);
    EMap::iterator j = m->find(s);
    if (j == m->end()) {
        LOG_DEBUG_LIB(LibMetkit) << "MarsExpressionFactory::create: available accessors:"
                              << "" << std::endl;
        for (EMap::iterator i = m->begin(); i != m->end(); ++i)
            LOG_DEBUG_LIB(LibMetkit) << "MarsExpressionFactory::create: '" << i->first << "'" << std::endl;

        Log::error() << "" << std::endl;
        std::ostringstream os;
        os << "No accessor named " << s << std::endl;
        throw eckit::SeriousBug(os.str());
    }
    return (*j).second->make(v);
}

//----------------------------------------------------------------------------------------------------------------------


class AccessMarsRequest : public MarsExpression {

    std::string param_;

    Value eval(const MarsTaskProxy& task) const override {
        LOG_DEBUG_LIB(LibMetkit) << "AccessMarsRequest::eval: " << std::endl;

        std::vector<Value> v;
        task.getRequestValues(param_, v);
        return Value(v);
    }

    void print(std::ostream& s) const override { s << "request(" << param_ << ")"; }

public:
    explicit AccessMarsRequest(const std::string& p) : param_(p) {}
};

class AccessMarsRequestFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessMarsRequest(v); }

public:
    AccessMarsRequestFactory() : MarsExpressionFactory("request") {}
};

static AccessMarsRequestFactory accessMarsRequestFactory;

//----------------------------------------------------------------------------------------------------------------------


class AccessMarsEnviron : public MarsExpression {

    std::string param_;

    Value eval(const MarsTaskProxy& task) const override {
        std::vector<Value> v;
        task.getEnvironValues(param_, v);
        return Value(v);
    }

    void print(std::ostream& s) const override { s << "environ(" << param_ << ")"; }

public:
    explicit AccessMarsEnviron(const std::string& p) : param_(p) {}
};

class AccessMarsEnvironFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessMarsEnviron(v); }

public:
    AccessMarsEnvironFactory() : MarsExpressionFactory("environ") {}
};

static AccessMarsEnvironFactory accessMarsEnvironFactory;

//----------------------------------------------------------------------------------------------------------------------


class AccessMarsUser : public AccessMarsEnviron {
public:
    AccessMarsUser() : AccessMarsEnviron("user") {}
};

class AccessMarsUserFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessMarsUser(); }

public:
    AccessMarsUserFactory() : MarsExpressionFactory("user") {}
};

static AccessMarsUserFactory accessMarsUserFactory;


//----------------------------------------------------------------------------------------------------------------------


class AccessMarsHost : public AccessMarsEnviron {
public:
    AccessMarsHost() : AccessMarsEnviron("host") {}
};

class AccessMarsHostFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessMarsHost(); }

public:
    AccessMarsHostFactory() : MarsExpressionFactory("host") {}
};

//----------------------------------------------------------------------------------------------------------------------


class AccessMarsAnyone : public MarsExpression {

    // Anyone is always true

    Value eval(const MarsTaskProxy& task) const override { return Value(true); }

    void print(std::ostream& s) const override { s << "anyone()"; }

public:
    AccessMarsAnyone() = default;
};

class AccessMarsAnyoneFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessMarsAnyone(); }

public:
    AccessMarsAnyoneFactory() : MarsExpressionFactory("anyone") {}
};

static AccessMarsAnyoneFactory accessMarsAnyoneFactory;

//----------------------------------------------------------------------------------------------------------------------


class AccessMarsDenied : public MarsExpression {

    Value eval(const MarsTaskProxy& task) const override { return Value(task.denied()); }

    void print(std::ostream& s) const override { s << "access_denied()"; }

public:
    AccessMarsDenied() {}
};

class AccessMarsDeniedFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessMarsDenied(); }

public:
    AccessMarsDeniedFactory() : MarsExpressionFactory("access_denied") {}
};

static AccessMarsDeniedFactory accessMarsDeniedFactory;
//----------------------------------------------------------------------------------------------------------------------


class AccessMarsGranted : public MarsExpression {

    Value eval(const MarsTaskProxy& task) const override { return Value(!task.denied()); }

    void print(std::ostream& s) const override { s << "access_granted()"; }

public:
    AccessMarsGranted() {}
};

class AccessMarsGrantedFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessMarsGranted(); }

public:
    AccessMarsGrantedFactory() : MarsExpressionFactory("access_granted") {}
};

static AccessMarsGrantedFactory accessMarsGrantedFactory;

//----------------------------------------------------------------------------------------------------------------------


class AccessMarsAuthenticated : public MarsExpression {

    Value eval(const MarsTaskProxy& task) const override {
        Log::info() << "AccessMarsAuthenticated" << std::endl;
        return Value(task.authenticated());
    }

    void print(std::ostream& s) const override { s << "authenticated()"; }

public:
    AccessMarsAuthenticated() = default;
};

class AccessMarsAuthenticatedFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessMarsAuthenticated(); }

public:
    AccessMarsAuthenticatedFactory() : MarsExpressionFactory("authenticated") {}
};

static AccessMarsAuthenticatedFactory accessMarsAuthenticatedFactory;
//----------------------------------------------------------------------------------------------------------------------


class AccessMarsBeforeSchedule : public MarsExpression {

    Value eval(const MarsTaskProxy& task) const override {
        Log::info() << "AccessMarsBeforeSchedule" << std::endl;
        return Value(task.beforeSchedule());
    }

    void print(std::ostream& s) const override { s << "before_schedule()"; }

public:
    AccessMarsBeforeSchedule() = default;
};

class AccessMarsBeforeScheduleFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessMarsBeforeSchedule(); }

public:
    AccessMarsBeforeScheduleFactory() : MarsExpressionFactory("before_schedule") {}
};

static AccessMarsBeforeScheduleFactory accessMarsBeforeScheduleFactory;
//----------------------------------------------------------------------------------------------------------------------

class AccessOwnerOrUser : public MarsExpression {
    Value eval(const MarsTaskProxy& task) const override {

        std::vector<std::string> v;
        task.getEnvironValues("owner", v);
        if (v.size() == 0) {
            task.getEnvironValues("user", v);
        }

        if (v.size() == 0) {
            return "unknown";
        }

        return v[0];
    }
    void print(std::ostream& s) const override { s << "owner_or_user()"; }
};

class AccessOwnerOrUserFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessOwnerOrUser(); }

public:
    AccessOwnerOrUserFactory() : MarsExpressionFactory("owner_or_user") {}
};

static AccessOwnerOrUserFactory accessOwnerOrUserFactory;

//----------------------------------------------------------------------------------------------------------------------
class AccessCostLayout : public MarsExpression {
    Value eval(const MarsTaskProxy& task) const override { return task.cost().layout_; }
    void print(std::ostream& s) const override { s << "layout()"; }
};

class AccessCostLayoutFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessCostLayout(); }

public:
    AccessCostLayoutFactory() : MarsExpressionFactory("layouts") {}
};

static AccessCostLayoutFactory accessCostLayoutFactory;

//----------------------------------------------------------------------------------------------------------------------


class AccessCostMedia : public MarsExpression {
    Value eval(const MarsTaskProxy& task) const override { return task.cost().media_.size(); }
    void print(std::ostream& s) const override { s << "media()"; }
};

class AccessCostMediaFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessCostMedia(); }

public:
    AccessCostMediaFactory() : MarsExpressionFactory("media") {}
};

static AccessCostMediaFactory accessCostMediaFactory;

//----------------------------------------------------------------------------------------------------------------------

class AccessCostFields : public MarsExpression {
    Value eval(const MarsTaskProxy& task) const override { return task.cost().onLineFields_ + task.cost().offLineFields_; }
    void print(std::ostream& s) const override { s << "fields()"; }
};

class AccessCostFieldsFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessCostFields(); }

public:
    AccessCostFieldsFactory() : MarsExpressionFactory("fields") {}
};

static AccessCostFieldsFactory accessCostFieldsFactory;

//----------------------------------------------------------------------------------------------------------------------


class AccessCostSize : public MarsExpression {
    Value eval(const MarsTaskProxy& task) const override { return (long long)(task.cost().onLine_ + task.cost().offLine_); }
    void print(std::ostream& s) const override { s << "size()"; }
};

class AccessCostSizeFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessCostSize(); }

public:
    AccessCostSizeFactory() : MarsExpressionFactory("size") {}
};

static AccessCostSizeFactory accessCostSizeFactory;

//----------------------------------------------------------------------------------------------------------------------


class AccessCostUnavailable : public MarsExpression {
    Value eval(const MarsTaskProxy& task) const override { return (long long)(task.cost().unavailable_); }
    void print(std::ostream& s) const override { s << "unavailable()"; }
};

class AccessCostUnavailableFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessCostUnavailable(); }

public:
    AccessCostUnavailableFactory() : MarsExpressionFactory("unavailable") {}
};

static AccessCostUnavailableFactory accessCostUnavailableFactory;

//----------------------------------------------------------------------------------------------------------------------


class AccessCostOffsite : public MarsExpression {
    Value eval(const MarsTaskProxy& task) const override { return (long long)(task.cost().offsite_); }
    void print(std::ostream& s) const override { s << "offsite()"; }
};

class AccessCostOffsiteFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessCostOffsite(); }

public:
    AccessCostOffsiteFactory() : MarsExpressionFactory("offsite") {}
};

static AccessCostOffsiteFactory accessCostOffsiteFactory;

//----------------------------------------------------------------------------------------------------------------------


class AccessCostDamaged : public MarsExpression {
    Value eval(const MarsTaskProxy& task) const override { return (long long)(task.cost().damaged_.size()); }
    void print(std::ostream& s) const override { s << "damaged_tapes()"; }
};

class AccessCostDamagedFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessCostDamaged(); }

public:
    AccessCostDamagedFactory() : MarsExpressionFactory("damaged_tapes") {}
};

static AccessCostDamagedFactory accessCostDamagedFactory;

//----------------------------------------------------------------------------------------------------------------------


class AccessCostLibraries : public MarsExpression {

    Value eval(const MarsTaskProxy& task) const override {
        std::vector<Value> v(task.cost().libraries_.begin(), task.cost().libraries_.end());
        return Value(v);
    }

    void print(std::ostream& s) const override { s << "libraries()"; }

public:
    AccessCostLibraries() = default;
};

class AccessCostLibrariesFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new AccessCostLibraries(); }

public:
    AccessCostLibrariesFactory() : MarsExpressionFactory("libraries") {}
};

static AccessCostLibrariesFactory accessCostLibrariesFactory;

//----------------------------------------------------------------------------------------------------------------------
// Obsolute

class DayOfMonth : public MarsExpression {

    Value eval(const MarsTaskProxy& task) const override {
        DateTime now;

        std::vector<Date> dates;
        task.getRequestValues("date", dates);

        std::vector<Time> times;
        task.getRequestValues("time", times);

        if (times.empty()) {
            times.push_back(0);
        }

        DateTime last;

        for (size_t i = 0; i < dates.size(); ++i) {

            for (size_t j = 0; j < times.size(); ++j) {
                DateTime dt(dates[i], times[j]);

                if (i == 0 && j == 0) {
                    last = dt;
                }
                else {
                    last = std::max(last, dt);
                }
            }
        }

        double diff = last - now;
        diff        = diff / 60.0 / 60.0 / 24.0;

        Log::info() << "day_of_month_hour now=" << now << ", last=" << last << " diff " << int(diff * 24) << std::endl;

        return int(diff * 24);
    }

    void print(std::ostream& s) const override { s << "day_of_month_hour()"; }
};

class DayOfMonthFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new DayOfMonth(); }

public:
    DayOfMonthFactory() : MarsExpressionFactory("day_of_month_hour") {}
};

static DayOfMonthFactory dayOfMonthFactory;

//----------------------------------------------------------------------------------------------------------------------

class MaximumStep : public MarsExpression {

    Value eval(const MarsTaskProxy& task) const override {

        std::vector<std::string> steps;
        task.getRequestValues("step", steps);

        int max = 0;
        for (auto s : steps) {
            metkit::mars::StepRange r(s);
            if (r.to() > max) {

                max = r.to();
            }
        }

        return max;
    }

    void print(std::ostream& s) const override { s << "maximum_step()"; }
};

class MaximumStepFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new MaximumStep(); }

public:
    MaximumStepFactory() : MarsExpressionFactory("maximum_step") {}
};

static MaximumStepFactory maximumStepFactory;

//----------------------------------------------------------------------------------------------------------------------

class HoursFromBaseTime : public MarsExpression {

    Value eval(const MarsTaskProxy& task) const override {
        DateTime now;

        std::vector<Date> dates;
        task.getRequestValues("date", dates);

        std::vector<Time> times;
        task.getRequestValues("time", times);

        if (times.empty()) {
            times.push_back(0);
        }

        DateTime last;

        for (size_t i = 0; i < dates.size(); ++i) {

            for (size_t j = 0; j < times.size(); ++j) {
                DateTime dt(dates[i], times[j]);

                if (i == 0 && j == 0) {
                    last = dt;
                }
                else {
                    last = std::max(last, dt);
                }
            }
        }

        double diff = now - last;
        diff        = diff / 60.0 / 60.0 / 24.0;

        Log::info() << "hours_from_base_time now=" << now << ", last=" << last << " diff " << int(diff * 24)
                    << std::endl;

        return int(diff * 24);
    }

    void print(std::ostream& s) const override { s << "hours_from_base_time()"; }
};

class HoursFromBaseTimeFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new HoursFromBaseTime(); }

public:
    HoursFromBaseTimeFactory() : MarsExpressionFactory("hours_from_base_time") {}
};

static HoursFromBaseTimeFactory hoursFromBaseTimeFactory;

//----------------------------------------------------------------------------------------------------------------------

class HoursFromValidTime : public MarsExpression {

    Value eval(const MarsTaskProxy& task) const override {
        DateTime now;

        std::vector<Date> dates;
        task.getRequestValues("date", dates);

        std::vector<Time> times;
        task.getRequestValues("time", times);

        std::vector<std::string> steps;
        task.getRequestValues("step", steps);

        std::vector<std::string> fcmonths;
        task.getRequestValues("fcmonth", fcmonths);

        if (times.empty()) {
            times.push_back(0);
        }

        if (!fcmonths.empty()) {
            ASSERT(steps.empty());
            Translator<std::string, long> s2l;
            Translator<long, std::string> l2s;

            for (size_t k = 0; k < fcmonths.size(); ++k) {
                steps.push_back(l2s(s2l(fcmonths[k]) * 30 * 24));
            }
        }

        if (steps.empty()) {
            steps.push_back("0");
        }


        DateTime last;

        for (size_t k = 0; k < steps.size(); ++k) {

            Seconds step = Seconds(metkit::mars::StepRange(steps[k]).to() * 60 * 60);

            for (size_t i = 0; i < dates.size(); ++i) {

                for (size_t j = 0; j < times.size(); ++j) {

                    DateTime base(dates[i], times[j]);


                    DateTime dt = base + step;

                    if (i == 0 && j == 0 && k == 0) {
                        last = dt;
                    }
                    else {
                        last = std::max(last, dt);
                    }
                }
            }
        }

        double diff = last - now;
        diff        = diff / 60.0 / 60.0 / 24.0;

        Log::info() << "hours_from_valid_time now=" << now << ", last=" << last << " diff " << int(diff * 24)
                    << std::endl;

        return int(diff * 24);
    }

    void print(std::ostream& s) const override { s << "hours_from_valid_time()"; }
};

class HoursFromValidTimeFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new HoursFromValidTime(); }

public:
    HoursFromValidTimeFactory() : MarsExpressionFactory("hours_from_valid_time") {}
};

static HoursFromValidTimeFactory hoursFromValidTimeFactory;


//----------------------------------------------------------------------------------------------------------------------

class PublicExperiment : public MarsExpression {

    Value eval(const MarsTaskProxy& task) const override {

        std::vector<std::string> expvers;
        task.getRequestValues("expver", expvers);
        ASSERT(expvers.size() == 1);

        eckit::PathName path("~/etc/publicExperiments");

        std::ifstream in(path.localPath());
        if (!in) {
            throw CantOpenFile(path);
        }
        std::string s;
        while(in >> s) {
            if(s == expvers[0]) {
                return 1;
            }
        }

        return 0;

    }

    void print(std::ostream& s) const override { s << "public_experiment()"; }
};

class PublicExperimentFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new PublicExperiment(); }

public:
    PublicExperimentFactory() : MarsExpressionFactory("public_experiment") {}
};

static PublicExperimentFactory publicExperimentFactory;

//----------------------------------------------------------------------------------------------------------------------


class Intent : public MarsExpression {

    Value eval(const MarsTaskProxy& task) const override {
        return Value(task.intentOnly());
    }

    void print(std::ostream& s) const override { s << "intent()"; }

public:
    Intent() = default;
};

class IntentFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new Intent(); }

public:
    IntentFactory() : MarsExpressionFactory("intent") {}
};

static IntentFactory intentFactory;

//----------------------------------------------------------------------------------------------------------------------


class FileExists : public MarsExpression {
    PathName path_;
    Value eval(const MarsTaskProxy& task) const override { return path_.exists(); }
    void print(std::ostream& s) const override { s << "exists(" << path_ << ")"; }
public:
    explicit FileExists(const std::string& p) : path_(p) {}
};

class FileExistsFactory : public MarsExpressionFactory {
    MarsExpression* make(const Value& v) override { return new FileExists(v); }

public:
    FileExistsFactory() : MarsExpressionFactory("exists") {}
};

static FileExistsFactory fileExistsFactory;

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit::mars::rules
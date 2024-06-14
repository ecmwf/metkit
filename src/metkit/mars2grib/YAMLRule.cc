/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "eckit/config/Resource.h"
#include "eckit/log/Log.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/types/Types.h"
#include "eckit/utils/StringTools.h"

#include "eckit/config/LocalConfiguration.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/utils/Overloaded.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/Mars2GribException.h"
#include "metkit/mars2grib/YAMLRule.h"

#include <ostream>



namespace metkit::mars2grib {

//----------------------------------------------------------------------------------------------------------------------

namespace YAMLAction {

using LogTrace = std::vector<Log>;

using MappedActions = std::unordered_map<std::string, std::unique_ptr<Action>>;

using ScalarCodesValue = std::variant<long, double, std::string>;
using KeyValuePairs    = std::vector<std::pair<std::string, ScalarCodesValue>>;

void printLogTrace(const LogTrace& logTrace, std::ostream& os) {
    os << "MARS2GRIB YamlRule LogTrace with " << logTrace.size() << "entries: " << std::endl;
    unsigned int n = 1;
    for (const auto& log : logTrace) {
        os << std::endl
           << " " << n << ": " << std::endl;
        if (log.action) {
            log.action->print(os);
        }
        if (log.customMessage) {
            os << *log.customMessage << std::endl;
        }
    }
}

std::string stringifyLogTrace(const LogTrace& logTrace) {
    std::ostringstream ss;
    printLogTrace(logTrace, ss);
    return ss.str();
};

struct Mapping : Action {
    virtual ~Mapping() = default;

    Mapping(bool useInitialDict, bool nullIsDefault, bool notFoundIsDefault, const std::string& lookUpKey, std::unique_ptr<Action> defaultAction, MappedActions&& mappedActions) :
        useInitialDict_{useInitialDict}, nullIsDefault_{nullIsDefault}, notFoundIsDefault_{notFoundIsDefault}, lookUpKey_{lookUpKey}, defaultAction_{std::move(defaultAction)}, mappedActions_{std::move(mappedActions)} {}

    void apply(LogTrace& logTrace, const eckit::ValueMap& initial, eckit::ValueMap& workDict, KeySetter& out) override {
        logTrace.push_back(Log{this, {}});
        Action* action;

        auto& dict = (useInitialDict_ ? initial : workDict);

        auto applyNext = [&](Action& action) { action.apply(logTrace, initial, workDict, out); };

        auto searchLookUpKey = dict.find(lookUpKey_);
        if (searchLookUpKey == dict.end()) {
            if (notFoundIsDefault_ && defaultAction_) {
                applyNext(*defaultAction_.get());
                return;
            }

            std::ostringstream ss;
            ss << "Key \"" << lookUpKey_ << "\" is not available in " << (useInitialDict_ ? "intitil " : "work") << "dictionary.";
            logTrace.push_back(Log{NULL, ss.str()});
            throw Mars2GribException(stringifyLogTrace(logTrace), Here());
        }

        if (searchLookUpKey->second.isNil()) {
            if (!nullIsDefault_) {
                std::ostringstream ss;
                ss << "Value for key \"" << lookUpKey_ << "\" is NULL and thus can not be mapped.";
                logTrace.push_back(Log{NULL, ss.str()});
                throw Mars2GribException(stringifyLogTrace(logTrace), Here());
            }
            if (!defaultAction_) {
                std::ostringstream ss;
                ss << "Value for key \"" << lookUpKey_ << "\" is NULL but no default action is given.";
                logTrace.push_back(Log{NULL, ss.str()});
                throw Mars2GribException(stringifyLogTrace(logTrace), Here());
            }
            applyNext(*defaultAction_.get());
            return;
        }

        std::string val = searchLookUpKey->second;
        logTrace.push_back(Log{this, std::string("{") + lookUpKey_ + std::string(": ") + val + std::string("}")});

        if (auto searchVal = mappedActions_.find(val); searchVal == mappedActions_.end()) {
            if (!defaultAction_) {
                std::ostringstream ss;
                ss << "Value \"" << val << "\" for key \"" << lookUpKey_ << "\" is not mapped to an action and no default action is given.";
                logTrace.push_back(Log{NULL, ss.str()});
                throw Mars2GribException(stringifyLogTrace(logTrace), Here());
            }
            applyNext(*defaultAction_.get());
            return;
        }
        else {
            applyNext(*searchVal->second.get());
            return;
        }
    };

    void print(std::ostream& os) override {
        os << "mars2grib::YAMLAction::Mapping with " << (useInitialDict_ ? "initial-key" : "work-key") << " = " << lookUpKey_;
    };

    bool useInitialDict_;
    bool nullIsDefault_;
    bool notFoundIsDefault_;
    std::string lookUpKey_;

    std::unique_ptr<Action> defaultAction_;  // Can be null
    MappedActions mappedActions_;
};

struct Failure : Action {
    virtual ~Failure() = default;

    Failure(const std::string& reason) :
        reason_{reason} {}

    void apply(LogTrace& logTrace, const eckit::ValueMap& inital, eckit::ValueMap& workDict, KeySetter& out) override {
        logTrace.push_back(Log{this, {}});
        throw Mars2GribException(stringifyLogTrace(logTrace), Here());
    };

    void print(std::ostream& os) override {
        os << "mars2grib::YAMLAction::Failure with Reason: " << reason_;
    };

    std::string reason_;
};

struct Write : Action {

    virtual ~Write() = default;

    Write(std::optional<KeyValuePairs> all,
          std::optional<KeyValuePairs> out,
          std::optional<KeyValuePairs> work) :
        all_{std::move(all)}, out_{std::move(out)}, work_{std::move(work)} {}

    void writeOut(const KeyValuePairs& writeDict, KeySetter& out) {
        for (const auto& kv : writeDict) {
            std::visit(eckit::Overloaded{
                           [&out, &kv](long val) {
                               out.setValue(kv.first, val);
                           },
                           [&out, &kv](double val) {
                               out.setValue(kv.first, val);
                           },
                           [&out, &kv](const std::string& val) {
                               out.setValue(kv.first, val);
                           },
                       },
                       kv.second);
        }
    };

    void writeWork(const KeyValuePairs& writeDict, eckit::ValueMap& work) {
        for (const auto& kv : writeDict) {
            std::visit(eckit::Overloaded{
                           [&work, &kv](long val) {
                               work.insert_or_assign(kv.first, val);
                           },
                           [&work, &kv](double val) {
                               work.insert_or_assign(kv.first, val);
                           },
                           [&work, &kv](const std::string& val) {
                               work.insert_or_assign(kv.first, val);
                           },
                       },
                       kv.second);
        }
    };

    void apply(LogTrace& logTrace, const eckit::ValueMap& inital, eckit::ValueMap& workDict, KeySetter& outSetter) override {
        logTrace.push_back(Log{this, {}});
        if (all_) {
            writeWork(*all_, workDict);
            writeOut(*all_, outSetter);
        }
        if (out_) {
            writeOut(*out_, outSetter);
        }
        if (work_) {
            writeWork(*work_, workDict);
        }
    };

    void print(std::ostream& os) override {
        os << "mars2grib::YAMLAction::Write";
    };


    std::optional<KeyValuePairs> all_;
    std::optional<KeyValuePairs> out_;
    std::optional<KeyValuePairs> work_;
};

void parseKeyValuePairs(KeyValuePairs& pairs, const eckit::LocalConfiguration& map, LogTrace& logTrace) {
    for (const auto& key : map.keys()) {
        if (map.isIntegral(key)) {
            pairs.emplace_back(key, map.getLong(key));
        }
        else if (map.isFloatingPoint(key)) {
            pairs.emplace_back(key, map.getDouble(key));
        }
        else if (map.isString(key)) {
            pairs.emplace_back(key, map.getString(key));
        }
        else {
            logTrace.push_back(Log{NULL, std::string("Only support writing integers, doubles or strings. Check the type for value of key \"") + key + std::string("\"")});
            throw Mars2GribException(std::string("Failure while parsing configuration: ") + stringifyLogTrace(logTrace), Here());
        }
    }
};

void parseKeyValuePairs(KeyValuePairs& pairs, const eckit::LocalConfiguration& conf, std::string const& key, LogTrace& logTrace) {
    if (conf.isSubConfiguration(key)) {
        parseKeyValuePairs(pairs, conf.getSubConfiguration(key), logTrace);
        return;
    }
    if (conf.isList(key)) {
        for (const auto& map: conf.getSubConfigurations(key)) {
            parseKeyValuePairs(pairs, map, logTrace);
        }
        return;
    }
    logTrace.push_back(Log{NULL, std::string("The configuration for key ") + key + std::string("\" must be a map or list of maps")});
    throw Mars2GribException(std::string("Failure while parsing configuration: ") + stringifyLogTrace(logTrace), Here());
};


std::unique_ptr<Action> buildAction(const eckit::LocalConfiguration& conf, LogTrace& logTrace) {
    // Action is a mapping
    if (conf.has("key")) {
        bool hasDict              = conf.has("dict");
        bool hasDefault           = conf.has("default");
        bool hasValueMap          = conf.has("value-map");
        bool hasNullIsDefault     = conf.has("null-is-default");
        bool hasNotFoundIsDefault = conf.has("not-found-is-default");


        std::string key = conf.getString("key");
        logTrace.push_back(Log{NULL, std::string("Build mapping action for key: ") + key});
        bool useInitialDict    = hasDict ? (conf.getString("dict") == "initial") : false;
        bool nullIsDefault     = hasNullIsDefault ? conf.getBool("null-is-default") : true;
        bool notFoundIsDefault = hasNotFoundIsDefault ? conf.getBool("not-found-is-default") : true;

        std::unique_ptr<Action> defaultAction;
        if (hasDefault) {
            LogTrace cpyLogTrace{logTrace};
            cpyLogTrace.push_back(Log{NULL, std::string("Building default action")});
            defaultAction = buildAction(conf.getSubConfiguration("default"), cpyLogTrace);
        }

        if (!hasValueMap) {
            logTrace.push_back(Log{NULL, "Key \"value-map\" expected."});
            std::ostringstream oss;
            oss << "Failure while parsing configuration: " << stringifyLogTrace(logTrace);
            throw Mars2GribException(oss.str(), Here());
        }

        auto valueMap = conf.getSubConfiguration("value-map");
        MappedActions mappedActions;
        for (const auto& val : valueMap.keys()) {
            LogTrace cpyLogTrace{logTrace};
            std::ostringstream oss;
            oss << "Building action for key-value pair " <<  key << ": " << val;
            cpyLogTrace.push_back(Log{NULL, oss.str()});
            mappedActions.insert_or_assign(val, buildAction(valueMap.getSubConfiguration(val), cpyLogTrace));
        }

        return std::make_unique<Mapping>(useInitialDict, nullIsDefault, notFoundIsDefault, key, std::move(defaultAction), std::move(mappedActions));
    }

    // Action is an output
    bool hasWrite     = conf.has("write");
    bool hasWriteOut  = conf.has("write-out");
    bool hasWriteWork = conf.has("write-work");
    if (hasWrite || hasWriteOut || hasWriteWork) {
        logTrace.push_back(Log{NULL, std::string("Build write action")});
        KeyValuePairs writeAll;
        KeyValuePairs writeOut;
        KeyValuePairs writeWork;

        if (hasWrite) {
            LogTrace cpyLogTrace{logTrace};
            cpyLogTrace.push_back(Log{NULL, std::string("Parsing key-value pairs for \"write\" map.")});
            parseKeyValuePairs(writeAll, conf, "write", cpyLogTrace);
        }
        if (hasWriteOut) {
            LogTrace cpyLogTrace{logTrace};
            cpyLogTrace.push_back(Log{NULL, std::string("Parsing key-value pairs for \"write-out\" map.")});
            parseKeyValuePairs(writeOut, conf, "write-out", cpyLogTrace);
        }
        if (hasWriteWork) {
            LogTrace cpyLogTrace{logTrace};
            cpyLogTrace.push_back(Log{NULL, std::string("Parsing key-value pairs for \"write-work\" map.")});
            parseKeyValuePairs(writeWork, conf, "write-work", cpyLogTrace);
        }

        return std::make_unique<Write>(std::move(writeAll), std::move(writeOut), std::move(writeWork));
    }

    if (conf.has("fail")) {
        return std::make_unique<Failure>(conf.getString("fail"));
    }

    logTrace.push_back(Log{NULL, "Unknown action"});
    throw Mars2GribException(std::string("Failure while parsing configuration: ") + stringifyLogTrace(logTrace), Here());
}


std::unique_ptr<Action> buildAction(const eckit::LocalConfiguration& conf, const std::string& source) {
    LogTrace logTrace;
    logTrace.push_back(Log{NULL, source});
    return buildAction(conf, logTrace);
}

};  // namespace YAMLAction


YAMLRule::YAMLRule(const eckit::LocalConfiguration& conf, const std::string& ruleSource) :
    action_{YAMLAction::buildAction(conf, ruleSource)}, ruleSource_{ruleSource} {};
YAMLRule::YAMLRule(const eckit::LocalConfiguration& conf) :
    YAMLRule(conf, "Unknown source"){};
YAMLRule::YAMLRule(const eckit::PathName& path) :
    YAMLRule(eckit::LocalConfiguration{eckit::YAMLConfiguration{path}}, path){};


namespace {
eckit::PathName getPath(const RuleConfiguration& conf) {
    if (!conf.has("file")) {
        std::ostringstream oss;
        oss << "Configuration for YAMLRule must have a key \"file\": " << conf;
        throw Mars2GribException(oss.str(), Here());
    }
    return conf.getString("file");
}
}  // namespace

YAMLRule::YAMLRule(const RuleConfiguration& conf) :
    YAMLRule(getPath(conf)){};

void YAMLRule::apply(const eckit::ValueMap& initial, eckit::ValueMap& workDict, KeySetter& out) {
    YAMLAction::LogTrace logTrace;
    logTrace.push_back(YAMLAction::Log{NULL, ruleSource_});

    action_->apply(logTrace, initial, workDict, out);
};


//----------------------------------------------------------------------------------------------------------------------

static RuleBuilder<YAMLRule> SelectBuilder("yaml");

//----------------------------------------------------------------------------------------------------------------------


}  // namespace metkit::mars2grib

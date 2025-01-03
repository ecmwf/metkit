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
#include "eckit/parser/YAMLParser.h"
#include "eckit/utils/Overloaded.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/Mars2GribException.h"
#include "metkit/mars2grib/ValueMapSetter.h"
#include "metkit/mars2grib/YAMLRule.h"

#include <ostream>
#include <unordered_map>
#include <variant>


namespace metkit::mars2grib {

//----------------------------------------------------------------------------------------------------------------------

namespace YAMLAction {

using LogTrace = std::vector<Log>;

using MappedActions = std::unordered_map<std::string, std::unique_ptr<Action>>;

using ScalarCodesValue = std::variant<long, double, std::string, NullOrMissing>;


struct GenericValueLookUp;

// Options for looking up key when writing
using ValueLookUp   = std::variant<ScalarCodesValue, std::unique_ptr<GenericValueLookUp>>;
using KeyValuePairs = std::vector<std::pair<std::string, ValueLookUp>>;

std::unique_ptr<Action> buildAction(const eckit::LocalConfiguration& conf, LogTrace& logTrace);
std::unique_ptr<GenericValueLookUp> buildLookUp(const eckit::LocalConfiguration& conf, LogTrace& logTrace);

//----------------------------------------------------------------------------------------------------------------------

struct GenericValueLookUp : Printable {
    virtual ~GenericValueLookUp()                                                                                       = default;
    virtual ScalarCodesValue apply(LogTrace& logTrace, const eckit::ValueMap& initial, eckit::ValueMap& workDict) const = 0;
};

ScalarCodesValue toScalarCodesValue(LogTrace& logTrace, const ValueLookUp& val, const eckit::ValueMap& initial, eckit::ValueMap& workDict) {
    return std::visit(eckit::Overloaded{
                          [](const ScalarCodesValue& val) {
                              return val;
                          },
                          [&](const std::unique_ptr<GenericValueLookUp>& val) {
                              return val->apply(logTrace, initial, workDict);
                          }},
                      val);
}

//----------------------------------------------------------------------------------------------------------------------


void printLogTrace(const LogTrace& logTrace, std::ostream& os) {
    os << "MARS2GRIB YamlRule LogTrace with " << logTrace.size() << "entries: " << std::endl;
    unsigned int n = 1;
    for (const auto& log : logTrace) {
        os << std::endl
           << " " << n << ": " << std::endl;
        if (log.printable) {
            log.printable->print(os);
        }
        if (log.customMessage) {
            os << *log.customMessage << std::endl;
        }
        ++n;
    }
}

std::string stringifyLogTrace(const LogTrace& logTrace) {
    std::ostringstream ss;
    printLogTrace(logTrace, ss);
    return ss.str();
};

//----------------------------------------------------------------------------------------------------------------------

struct Mapping : Action {
    virtual ~Mapping() = default;

    Mapping(bool useInitialDict, bool nullIsDefault, bool notFoundIsDefault, const std::string& lookUpKey, std::unique_ptr<Action> defaultAction, std::unique_ptr<Action> notFoundAction, MappedActions&& mappedActions) :
        useInitialDict_{useInitialDict}, nullIsDefault_{nullIsDefault}, notFoundIsDefault_{notFoundIsDefault}, lookUpKey_{lookUpKey}, defaultAction_{std::move(defaultAction)}, notFoundAction_{std::move(notFoundAction)}, mappedActions_{std::move(mappedActions)} {}

    void apply(LogTrace& logTrace, const eckit::ValueMap& initial, eckit::ValueMap& workDict, KeySetter& out) const override {
        logTrace.push_back(Log{this, {}});
        Action* action;

        auto& dict = (useInitialDict_ ? initial : workDict);

        auto applyNext = [&](Action& action) { action.apply(logTrace, initial, workDict, out); };

        auto searchLookUpKey = dict.find(lookUpKey_);
        if (searchLookUpKey == dict.end()) {
            if (notFoundAction_) {
                applyNext(*notFoundAction_.get());
                return;
            } else if (notFoundIsDefault_ && defaultAction_) {
                applyNext(*defaultAction_.get());
                return;
            }

            std::ostringstream ss;
            ss << "Key \"" << lookUpKey_ << "\" is not available in " << (useInitialDict_ ? "initial " : "work") << "dictionary.";
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

    void print(std::ostream& os) const override {
        os << "mars2grib::YAMLAction::Mapping with " << (useInitialDict_ ? "initial-key" : "work-key") << " = " << lookUpKey_;
    };

    bool useInitialDict_;
    bool nullIsDefault_;
    bool notFoundIsDefault_;
    std::string lookUpKey_;

    std::unique_ptr<Action> defaultAction_;  // Can be null
    std::unique_ptr<Action> notFoundAction_;  // Can be null
    MappedActions mappedActions_;
};

struct Failure : Action {
    virtual ~Failure() = default;

    Failure(const std::string& reason) :
        reason_{reason} {}

    void apply(LogTrace& logTrace, const eckit::ValueMap& initial, eckit::ValueMap& workDict, KeySetter& out) const override {
        logTrace.push_back(Log{this, {}});
        throw Mars2GribException(stringifyLogTrace(logTrace), Here());
    };

    void print(std::ostream& os) const override {
        os << "mars2grib::YAMLAction::Failure with Reason: " << reason_;
    };

    std::string reason_;
};

struct Pass : Action {
    virtual ~Pass() = default;

    Pass(const std::string& logMsg = "") :
        logMsg_{logMsg} {}

    void apply(LogTrace& logTrace, const eckit::ValueMap& initial, eckit::ValueMap& workDict, KeySetter& out) const override {
        logTrace.push_back(Log{this, {}});
    };

    void print(std::ostream& os) const override {
        os << "mars2grib::YAMLAction::Pass";
        if (!logMsg_.empty()) {
            os << " with message: " << logMsg_;
        }
    };

    std::string logMsg_;
};

struct Write : Action {

    virtual ~Write() = default;

    Write(std::optional<KeyValuePairs> all,
          std::optional<KeyValuePairs> out,
          std::optional<KeyValuePairs> work) :
        all_{std::move(all)}, out_{std::move(out)}, work_{std::move(work)} {}

    void write(LogTrace& logTrace, const eckit::ValueMap& initial, eckit::ValueMap& workDict, const KeyValuePairs& writeDict, KeySetter& out) const {
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
                           [&out, &kv](NullOrMissing) {
                               out.setValue(kv.first, NullOrMissing{});
                           },
                       },
                       toScalarCodesValue(logTrace, kv.second, initial, workDict));
        }
    };

    void apply(LogTrace& logTrace, const eckit::ValueMap& initial, eckit::ValueMap& workDict, KeySetter& outSetter) const override {
        logTrace.push_back(Log{this, {}});
        ValueMapSetter workSetter{workDict};
        if (all_) {
            write(logTrace, initial, workDict, *all_, workSetter);
            write(logTrace, initial, workDict, *all_, outSetter);
        }
        if (out_) {
            write(logTrace, initial, workDict, *out_, outSetter);
        }
        if (work_) {
            write(logTrace, initial, workDict, *work_, workSetter);
        }
    };

    void print(std::ostream& os) const override {
        os << "mars2grib::YAMLAction::Write";
    };


    std::optional<KeyValuePairs> all_;
    std::optional<KeyValuePairs> out_;
    std::optional<KeyValuePairs> work_;
};


std::optional<ScalarCodesValue> parseScalarValue(const eckit::LocalConfiguration& map, const std::string& key) {
    if (map.isIntegral(key)) {
        return map.getLong(key);
    }
    else if (map.isFloatingPoint(key)) {
        return map.getDouble(key);
    }
    else if (map.isString(key)) {
        return map.getString(key);
    }
    else if (map.isNull(key)) {
        return NullOrMissing{};
    }
    return {};
}

ValueLookUp parseValueLookUp(const eckit::LocalConfiguration& map, const std::string& key, LogTrace& logTrace) {
    if (auto scalarVal = parseScalarValue(map, key); scalarVal) {
        return *scalarVal;
    }
    else if (map.isSubConfiguration(key)) {
        return buildLookUp(map.getSubConfiguration(key), logTrace);
    }
    else {
        logTrace.push_back(Log{NULL, std::string("Only support writing integers, doubles, strings, null or a special lookup via a map. Check the type for value of key \"") + key + std::string("\"")});
        throw Mars2GribException(std::string("Failure while parsing configuration: ") + stringifyLogTrace(logTrace), Here());
    }
};

void parseKeyValuePairs(KeyValuePairs& pairs, const eckit::LocalConfiguration& map, LogTrace& logTrace) {
    for (const auto& key : map.keys()) {
        pairs.emplace_back(key, parseValueLookUp(map, key, logTrace));
    }
};

void parseKeyValuePairs(KeyValuePairs& pairs, const eckit::LocalConfiguration& conf, std::string const& key, LogTrace& logTrace) {
    if (conf.isSubConfiguration(key)) {
        parseKeyValuePairs(pairs, conf.getSubConfiguration(key), logTrace);
        return;
    }
    if (conf.isList(key)) {
        for (const auto& map : conf.getSubConfigurations(key)) {
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

        // Allow separate action than default
        bool hasNotFound = conf.has("not-found");




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
        
        std::unique_ptr<Action> notFoundAction;
        if (hasNotFound) {
            LogTrace cpyLogTrace{logTrace};
            cpyLogTrace.push_back(Log{NULL, std::string("Building not-found action")});
            notFoundAction = buildAction(conf.getSubConfiguration("not-found"), cpyLogTrace);
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
            oss << "Building action for key-value pair " << key << ": " << val;
            cpyLogTrace.push_back(Log{NULL, oss.str()});
            mappedActions.insert_or_assign(val, buildAction(valueMap.getSubConfiguration(val), cpyLogTrace));
        }

        return std::make_unique<Mapping>(useInitialDict, nullIsDefault, notFoundIsDefault, key, std::move(defaultAction), std::move(notFoundAction), std::move(mappedActions));
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

    if (conf.has("pass")) {
        return std::make_unique<Pass>(conf.isString("pass") ? conf.getString("pass") : std::string(""));
    }

    logTrace.push_back(Log{NULL, "Unknown action"});
    throw Mars2GribException(std::string("Failure while parsing configuration: ") + stringifyLogTrace(logTrace), Here());
}


std::unique_ptr<Action> buildAction(const eckit::LocalConfiguration& conf, const std::string& source) {
    LogTrace logTrace;
    logTrace.push_back(Log{NULL, source});
    return buildAction(conf, logTrace);
}

//----------------------------------------------------------------------------------------------------------------------

// ValueLookUp
struct DictValueLookUp : GenericValueLookUp {
    virtual ~DictValueLookUp() = default;

    DictValueLookUp(bool useInitialDict, bool nullIsDefault, bool notFoundIsDefault, const std::string& lookUpKey, std::optional<ValueLookUp> defaultValue = {}, std::optional<ValueLookUp> notFoundValue = {}) :
        useInitialDict_{useInitialDict}, nullIsDefault_{nullIsDefault}, notFoundIsDefault_{notFoundIsDefault}, lookUpKey_{lookUpKey}, default_{std::move(defaultValue)}, notFound_{std::move(notFoundValue)} {
    }


    ScalarCodesValue apply(LogTrace& logTrace, const eckit::ValueMap& initial, eckit::ValueMap& workDict) const override {
        logTrace.push_back(Log{this, {}});
        const eckit::ValueMap& map = useInitialDict_ ? initial : workDict;

        if (auto searchKey = map.find(lookUpKey_); searchKey != map.end()) {
            if (searchKey->second.isNumber()) {
                return static_cast<long>(searchKey->second);
            }
            if (searchKey->second.isDouble()) {
                return static_cast<double>(searchKey->second);
            }
            if (searchKey->second.isString()) {
                return static_cast<std::string>(searchKey->second);
            }
            if (searchKey->second.isNil()) {
                if (nullIsDefault_) {
                    if (!default_) {
                        std::ostringstream oss;
                        oss << "Looked up value for key \"" << lookUpKey_ << "\" is null and null-is-default is used but no default value has been configured.";
                        logTrace.push_back(Log{NULL, oss.str()});
                        throw Mars2GribException(std::string("Failure while looking up a key: ") + stringifyLogTrace(logTrace), Here());
                    }
                    return toScalarCodesValue(logTrace, *default_, initial, workDict);
                }
                return NullOrMissing{};
            }

            std::ostringstream oss;
            oss << "Looked up value for key \"" << lookUpKey_ << "\" is neither a long, double, string nor null: " << searchKey->second;
            logTrace.push_back(Log{NULL, oss.str()});
            throw Mars2GribException(std::string("Failure while looking up a key: ") + stringifyLogTrace(logTrace), Here());
        }

        if (notFoundIsDefault_) {
            if (!default_) {
                std::ostringstream oss;
                oss << "Key \"" << lookUpKey_ << "\" not found and not-found-is-default is used but no default value has been configured.";
                logTrace.push_back(Log{NULL, oss.str()});
                throw Mars2GribException(std::string("Failure while looking up a key: ") + stringifyLogTrace(logTrace), Here());
            }
            return toScalarCodesValue(logTrace, *default_, initial, workDict);
        }

        std::ostringstream oss;
        oss << "No key \"" << lookUpKey_ << "\" found in value map.";
        logTrace.push_back(Log{NULL, oss.str()});
        throw Mars2GribException(std::string("Failure while looking up a key: ") + stringifyLogTrace(logTrace), Here());
    };

    void print(std::ostream& os) const override {
        os << "mars2grib::YAMLAction::DictValueLookUp{}";
    };

    bool useInitialDict_;
    bool nullIsDefault_;
    bool notFoundIsDefault_;
    std::string lookUpKey_;
    std::optional<ValueLookUp> default_;
    std::optional<ValueLookUp> notFound_;
};


enum class BinaryOperation
{
    Add,
    Subtract,
    Multiply,
    Divide,
};

std::optional<BinaryOperation> parseBinaryOperation(const std::string& opStr) {
    static const std::unordered_map<std::string, BinaryOperation> map{
        {"add", BinaryOperation::Add},
        {"subtract", BinaryOperation::Subtract},
        {"multiply", BinaryOperation::Multiply},
        {"divide", BinaryOperation::Divide},
        {"+", BinaryOperation::Add},
        {"-", BinaryOperation::Subtract},
        {"*", BinaryOperation::Multiply},
        {"/", BinaryOperation::Divide}};

    if (auto searchOp = map.find(opStr); searchOp != map.end()) {
        return searchOp->second;
    }

    return {};
}

std::ostream& operator<<(std::ostream& os, BinaryOperation op) {
    switch (op) {
        case BinaryOperation::Add:
            os << "add";
            break;
        case BinaryOperation::Subtract:
            os << "subtract";
            break;
        case BinaryOperation::Multiply:
            os << "multiply";
            break;
        case BinaryOperation::Divide:
            os << "divide";
            break;
    }
    return os;
}

std::variant<double, long> toNumeric(LogTrace& logTrace, const ScalarCodesValue& v) {
    return std::visit(eckit::Overloaded{
                          [&](double v) -> std::variant<double, long> { return v; },
                          [&](long v) -> std::variant<double, long> { return v; },
                          [&](const std::string& v) -> std::variant<double, long> {
                              eckit::Value testVal = eckit::YAMLParser::decodeString(v);
                              if (testVal.isNumber()) {
                                  return static_cast<long>(testVal);
                              }
                              if (testVal.isDouble()) {
                                  return static_cast<long>(testVal);
                              }
                              std::ostringstream oss;
                              oss << "Can not convert string to a numeric representation: " << v;
                              logTrace.push_back(Log{NULL, oss.str()});
                              throw Mars2GribException(std::string("Failure while converting to numeric: ") + stringifyLogTrace(logTrace), Here());
                          },
                          [&](NullOrMissing) -> std::variant<double, long> { return static_cast<long>(0); },
                      },
                      v);
}

ScalarCodesValue handleOp(LogTrace& logTrace, BinaryOperation op, const ScalarCodesValue& lhs, const ScalarCodesValue& rhs) {
    return std::visit([op](const auto& l, const auto& r) -> ScalarCodesValue {
        switch (op) {
            case BinaryOperation::Add:
                return l + r;
            case BinaryOperation::Subtract:
                return l - r;
            case BinaryOperation::Multiply:
                return l * r;
            case BinaryOperation::Divide:
                return l / r;
            default:
                NOTIMP;
        }
    },
                      toNumeric(logTrace, lhs), toNumeric(logTrace, rhs));
}


struct BinaryOperationLookUp : GenericValueLookUp {
    virtual ~BinaryOperationLookUp() = default;

    BinaryOperationLookUp(BinaryOperation op, ValueLookUp&& lhs, ValueLookUp&& rhs) :
        op_{op}, lhs_{std::move(lhs)}, rhs_{std::move(rhs)} {
    }


    ScalarCodesValue apply(LogTrace& logTrace, const eckit::ValueMap& initial, eckit::ValueMap& workDict) const override {
        logTrace.push_back(Log{this, {}});
        auto lhs = toScalarCodesValue(logTrace, lhs_, initial, workDict);
        auto rhs = toScalarCodesValue(logTrace, rhs_, initial, workDict);

        return handleOp(logTrace, op_, lhs, rhs);
    };

    void print(std::ostream& os) const override {
        os << "mars2grib::YAMLAction::BinaryOperationLookUp{}";
    };


    BinaryOperation op_;
    ValueLookUp lhs_;
    ValueLookUp rhs_;
};


std::unique_ptr<GenericValueLookUp> buildLookUp(const eckit::LocalConfiguration& conf, LogTrace& logTrace) {  // Action is a mapping
    if (conf.has("key")) {
        bool hasDict              = conf.has("dict");
        bool hasDefault           = conf.has("default");
        bool hasNullIsDefault     = conf.has("null-is-default");
        bool hasNotFoundIsDefault = conf.has("not-found-is-default");
        
        bool hasNotFound = conf.has("not-found");

        std::string key = conf.getString("key");
        logTrace.push_back(Log{NULL, std::string("Build dict lookup for key: ") + key});
        bool useInitialDict    = hasDict ? (conf.getString("dict") == "initial") : false;
        bool nullIsDefault     = hasNullIsDefault ? conf.getBool("null-is-default") : true;
        bool notFoundIsDefault = hasNotFoundIsDefault ? conf.getBool("not-found-is-default") : true;

        std::optional<ValueLookUp> defaultValue;
        if (hasDefault) {
            LogTrace cpyLogTrace{logTrace};
            cpyLogTrace.push_back(Log{NULL, std::string("Building default lookup")});
            defaultValue = parseValueLookUp(conf, "default", cpyLogTrace);
        }
        
        std::optional<ValueLookUp> notFoundValue;
        if (hasNotFound) {
            LogTrace cpyLogTrace{logTrace};
            cpyLogTrace.push_back(Log{NULL, std::string("Building not-found lookup")});
            notFoundValue = parseValueLookUp(conf, "not-found", cpyLogTrace);
        }

        return std::make_unique<DictValueLookUp>(useInitialDict, nullIsDefault, notFoundIsDefault, key, std::move(defaultValue), std::move(notFoundValue));
    }

    if (conf.has("op")) {
        bool hasLhs = conf.has("lhs");
        bool hasRhs = conf.has("rhs");

        if (!hasLhs || !hasRhs) {
            logTrace.push_back(Log{NULL, "Expected keys \"lhs\" and \"rhs\" together with \"op\" to create a binary operation."});
            throw Mars2GribException(std::string("Failure while parsing configuration: ") + stringifyLogTrace(logTrace), Here());
        }


        std::string opStr = conf.getString("op");
        auto op           = parseBinaryOperation(opStr);
        if (!op) {
            std::ostringstream oss;
            oss << "Can not parse to a known binary operation: " << opStr;
            logTrace.push_back(Log{NULL, oss.str()});
            throw Mars2GribException(std::string("Failure while parsing configuration: ") + stringifyLogTrace(logTrace), Here());
        }

        LogTrace cpyLogTrace1{logTrace};
        cpyLogTrace1.push_back(Log{NULL, std::string("Parsing lhs")});
        auto lhs = parseValueLookUp(conf, "lhs", cpyLogTrace1);

        LogTrace cpyLogTrace2{logTrace};
        cpyLogTrace2.push_back(Log{NULL, std::string("Parsing rhs")});
        auto rhs = parseValueLookUp(conf, "rhs", cpyLogTrace2);

        return std::make_unique<BinaryOperationLookUp>(*op, std::move(lhs), std::move(rhs));
    }

    logTrace.push_back(Log{NULL, "Unknown LookUp"});
    throw Mars2GribException(std::string("Failure while parsing configuration: ") + stringifyLogTrace(logTrace), Here());
}

//----------------------------------------------------------------------------------------------------------------------

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

void YAMLRule::apply(const eckit::ValueMap& initial, eckit::ValueMap& workDict, KeySetter& out) const {
    YAMLAction::LogTrace logTrace;
    logTrace.push_back(YAMLAction::Log{NULL, ruleSource_});

    action_->apply(logTrace, initial, workDict, out);
};


//----------------------------------------------------------------------------------------------------------------------

static RuleBuilder<YAMLRule> SelectBuilder("yaml");

//----------------------------------------------------------------------------------------------------------------------


}  // namespace metkit::mars2grib

#include <eckit/config/LocalConfiguration.h>
#include <eckit/config/YAMLConfiguration.h>
#include <eckit/exception/Exceptions.h>
#include <iostream>
#include <string>
#include "eckit/parser/YAMLParser.h"
#include "eckit/types/Types.h"
#include "eckit/utils/StringTools.h"
#include "metkit/config/LibMetkit.h"


#include <eckit/exception/Exceptions.h>
#include <eckit/value/Value.h>

#include <string>
#include <vector>

struct MarsLanguageValidationError : public eckit::Exception {
    MarsLanguageValidationError(const std::string& msg, const eckit::CodeLocation& loc = Here()) :
        eckit::Exception(msg, loc) {}
};

void validateMarsKeyValue(const eckit::Value& language, const std::string& key, const std::string& value) {
    // --------------------------------------------------
    // 1. key must exist
    // --------------------------------------------------
    if (!language.contains(key)) {
        throw MarsLanguageValidationError("Unknown MARS key: '" + key + "'");
    }

    const eckit::Value entry = language[key];

    // --------------------------------------------------
    // 2. category must be 'data'
    // --------------------------------------------------
    bool hasType   = entry.contains("type");
    bool hasValues = entry.contains("values");

    eckit::Value TypeInfo;
    bool isEnum = false;
    if (hasType) {
        TypeInfo = entry["type"];
        for (auto& val : TypeInfo.as<std::vector<eckit::Value>>()) {
            if (val[0].as<std::string>() == "enum") {
                isEnum = true;
            }
        }
    }

    if (isEnum && hasValues) {
        eckit::Value Values = entry["values"];
        for (auto& val : values.as<std::vector<eckit::Value>>()) {
            // std::cout << "Checking value: " << val[0] << ", " << val[1] << ";" << std::endl;
            if (val[0].as<std::string>() == value) {
                return true;  // valid value
            }
        }
    }

    bool isInteger = false;
    if (hasType) {
        TypeInfo = entry["type"];
        for (auto& val : TypeInfo.as<std::vector<eckit::Value>>()) {
            if (val[0].as<std::string>() == "enum") {
                isEnum = true;
            }
        }
    }


    // --------------------------------------------------
    // 3. enum validation (if present)
    // --------------------------------------------------
    if (!entry.contains("values")) {
        return;  // no enum → nothing to validate
    }

    const auto values = entry["values"].as<std::vector<eckit::Value>>();

    for (const auto& v : values) {
        // enum entries are [code, description]
        if (v.isList() && v.size() > 0 && v[0].as<std::string>() == value) {
            return;  // valid
        }
    }

    // --------------------------------------------------
    // 4. invalid enum value
    // --------------------------------------------------
    throw MarsLanguageValidationError("Invalid value '" + value + "' for MARS key '" + key + "'");
}

#include <eckit/filesystem/PathName.h>
#include <eckit/parser/YAMLParser.h>

#include <iostream>

int main() {
    try {
        const eckit::Value languageYaml =
            eckit::YAMLParser::decodeFile(eckit::PathName("/ec/res4/hpcperm/mavm/ba/metkit-bundle/language.yaml"));

        const eckit::Value fieldDefs = languageYaml["_field"];

        validateMarsKeyValue(fieldDefs, "class", "odx");

        std::cout << "Key/value is valid\n";
    }
    catch (const eckit::Exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}

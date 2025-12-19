#include <eckit/config/LocalConfiguration.h>
#include <eckit/config/YAMLConfiguration.h>
#include <eckit/exception/Exceptions.h>
#include <iostream>
#include <string>
#include "eckit/parser/YAMLParser.h"
#include "eckit/types/Types.h"
#include "eckit/utils/StringTools.h"
#include "metkit/config/LibMetkit.h"


//------------------------------------------------------------
// Helper: validate a single key/value against language.yaml
//------------------------------------------------------------
bool validateMarsKeyValue(const eckit::Value& language, const std::string& key, const std::string& value) {


    // 1) verify key exists
    // const LocalConfiguration& language = lang.getSubConfiguration("_field");
    // std::cout << language << std::endl;
    // if (!language.has(key)) {
    //     std::cout << "Key not found: " << key << std::endl;
    //     return false;  // unknown key
    // }

    // std::cout << "Validating key: " << language.contains(key) << std::endl;
    const eckit::Value entry    = language[key];
    const eckit::Value defaults = entry["defaults"];
    const eckit::Value values   = entry["values"];

    // std::cout << "Key found: " << key << " with defaults: " << defaults << " and values: " << values << std::endl;

    // 2) validate value according to key definition
    for (auto& val : values.as<std::vector<eckit::Value>>()) {
        // std::cout << "Checking value: " << val[0] << ", " << val[1] << ";" << std::endl;
        if (val[0].as<std::string>() == value) {
            return true;  // valid value
        }
    }

    // If enum
    // if (keyCfg.has("type") && keyCfg.getString("type") == "enum") {
    //     const auto& values = keyCfg.getSubConfigurations("values");
    //     std::cout << "Allowed values for key " << key << ": ";
    //     for (const auto& v : values) {
    //         // enum entries are usually [code, description]
    //         std::cout << "check for: " << v.getString(0) << " ";
    //         if (v.getString(0) == value) {
    //             return true;
    //         }
    //     }
    //     return false; // enum but value not allowed
    // }

    // If integer
    // if (keyCfg.has("type") && keyCfg.getString("type") == "integer") {
    //     try {
    //         std::stol(value);
    //         return true;
    //     } catch (...) {
    //         return false;
    //     }
    // }

    // If string (or no strict type)
    return false;
}

//------------------------------------------------------------
// Example usage
//------------------------------------------------------------
int main() {
    // try {
    // Load language.yaml
    eckit::Value languages_ =
        eckit::YAMLParser::decodeFile(eckit::PathName("/ec/res4/hpcperm/mavm/ba/metkit-bundle/language.yaml"));
    //    LibMetkit::languageYamlFile().asString()));
    // std::cout << languages_ << std::endl;
    const eckit::Value language = languages_["_field"];
    // std::cout << language << std::endl;
    // const eckit::Value verbs = language.keys();
    // for (size_t i = 0; i < verbs.size(); ++i) {
    //     std::cout << "Verb: " << verbs[i] << std::endl;
    // }
    // std::cout << language["class"] << std::endl;
    /// language(YAMLConfiguration("/ec/res4/hpcperm/mavm/ba/metkit-bundle/language.yaml"));
    /// eckit::LocalConfiguration language = languages_;
    /// //.as<eckit::LocalConfiguration>().getSubConfiguration("_fields"); std::cout << language << ": " <<
    /// language.has("_field") << language.has("class") << std::endl;

    // Example key/value
    std::string key   = "class";
    std::string value = "odx";

    if (!validateMarsKeyValue(languages_["_field"], key, value)) {
        std::cerr << "Invalid key/value: " << key << "=" << value << std::endl;
        return 1;
    }
    else {
        std::cout << "Valid key/value: " << key << "=" << value << std::endl;
    }

    // std::cout << "Valid key/value: " << key << "=" << value << std::endl;
    // }
    // catch (const Exception& e) {
    //     std::cerr << e.what() << std::endl;
    //     return 1;
    // }

    return 0;
}

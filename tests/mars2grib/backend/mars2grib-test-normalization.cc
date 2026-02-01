#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <tuple>
#include <cassert>
#include <iostream>
#include <string>
#include <stdexcept>

// eckit include


#include "eckit/parser/YAMLParser.h"
#include "eckit/value/Value.h"
#include <eckit/config/LocalConfiguration.h>


//------------------------------------------------------------------------------
// Minimal validator
//------------------------------------------------------------------------------
void checkKeyword(const eckit::Value& fields,
                  const std::string& key,
                  const std::string& value) {

    if (!fields.contains(key)) {
        throw std::runtime_error("Unknown keyword: " + key);
    }

    const auto& entry = fields[key];

    // Ignore non-data entries (exactly as discussed)
    if (entry.contains("category") &&
        entry["category"].as<std::string>() != "data") {
        return;
    }

    // Enumerated values
    // if (entry.contains("values")) {
    //     for (const auto& v : entry["values"]) {
    //         if (v[0].as<std::string>() == value) {
    //             return;
    //         }
    //     }
    //
    //     throw std::runtime_error(
    //         "Invalid value '" + value + "' for keyword '" + key + "'"
    //     );
    // }
}

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main(int argc, char** argv) {

    if (argc != 4) {
        std::cerr
            << "Usage:\n"
            << "  " << argv[0] << " <language.yaml> <keyword> <value>\n";
        return 1;
    }

    try {
        const std::string languageFile = argv[1];
        const std::string keyword      = argv[2];
        const std::string value        = argv[3];

        eckit::Value language =
            eckit::YAMLParser::decodeFile(languageFile);

        const eckit::Value& fields = language["_field"];

        checkKeyword(fields, keyword, value);

        std::cout << "OK: " << keyword << "=" << value << "\n";
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
}

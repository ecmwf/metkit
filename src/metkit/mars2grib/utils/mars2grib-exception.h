#pragma once

#include <exception>
#include <optional>
#include <string>
#include <typeinfo>

#include "eckit/exception/Exceptions.h"

namespace metkit::mars2grib::utils::exceptions {

// ==========================================================
// Base exception (no metadata)
// ==========================================================
class Mars2GribGenericException : public eckit::Exception, public std::nested_exception {
public:

    Mars2GribGenericException(std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        eckit::Exception(reason, loc) {}
};


// ==========================================================
// Rules exception (no metadata)
// ==========================================================
class Mars2GribRulesException : public eckit::Exception, public std::nested_exception {
public:

    Mars2GribRulesException(std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        eckit::Exception(reason, loc) {}
};

// ==========================================================
// Dict Layer Exception
// ==========================================================
class Mars2GribDictException : public Mars2GribGenericException {
public:

    Mars2GribDictException(std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        Mars2GribGenericException(reason, loc) {}
};


// ==========================================================
// Deduction Layer Exception (empty for now)
// ==========================================================
class Mars2GribDeductionException : public Mars2GribGenericException {
public:

    Mars2GribDeductionException(std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        Mars2GribGenericException(reason, loc) {}
};


// ==========================================================
// Concept Layer Exception (with metadata!)
// ==========================================================
class Mars2GribConceptException : public Mars2GribGenericException {
public:

    Mars2GribConceptException(std::string name, std::string variant, std::string stage, std::string section,
                              std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        Mars2GribGenericException(reason, loc),
        conceptName_(std::move(name)),
        conceptVariant_(std::move(variant)),
        stage_(std::move(stage)),
        section_(std::move(section)) {}

    const std::optional<std::string>& conceptName() const { return conceptName_; }
    const std::optional<std::string>& conceptVariant() const { return conceptVariant_; }
    const std::optional<std::string>& stage() const { return stage_; }
    const std::optional<std::string>& section() const { return section_; }

private:

    std::optional<std::string> conceptName_;
    std::optional<std::string> conceptVariant_;
    std::optional<std::string> stage_;
    std::optional<std::string> section_;
};


// ==========================================================
// Encoder Layer Exception
// ==========================================================
class Mars2GribEncoderException : public Mars2GribGenericException {
public:

    Mars2GribEncoderException(std::string reason, std::string marsDict_json, std::string geoDict_json,
                              std::string parDict_json, std::string optDict_json, std::string encoderCfg_json,
                              const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        Mars2GribGenericException(reason, loc),
        marsDict_json_(std::move(marsDict_json)),
        geoDict_json_(std::move(geoDict_json)),
        parDict_json_(std::move(parDict_json)),
        optDict_json_(std::move(optDict_json)),
        encoderCfg_json_(std::move(encoderCfg_json)) {}

    const std::string& marsDict_json() const { return marsDict_json_; }
    const std::string& geoDict_json() const { return geoDict_json_; }
    const std::string& parDict_json() const { return parDict_json_; }
    const std::string& optDict_json() const { return optDict_json_; }
    const std::string& encoderCfg_json() const { return encoderCfg_json_; }

private:

    const std::string marsDict_json_;
    const std::string geoDict_json_;
    const std::string parDict_json_;
    const std::string optDict_json_;
    const std::string encoderCfg_json_;
};

// ==========================================================
// Print exception stack
// ==========================================================
inline void printExceptionStack(const std::exception& e, std::ostream& os, std::size_t level = 0) {
    const std::string indent(level * 2, ' ');

    // stampa tipo + messaggio
    os << indent << "- [" << typeid(e).name() << "] " << e.what() << '\n';

    // verifica eccezione annidata
    try {
        std::rethrow_if_nested(e);
    }
    catch (const std::exception& nested) {
        printExceptionStack(nested, os, level + 1);
    }
    catch (...) {
        os << indent << "  - [unknown non-std exception]\n";
    }
};

}  // namespace metkit::mars2grib::utils::exceptions

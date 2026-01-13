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

    virtual ~Mars2GribGenericException() = default;

    virtual void printFrame(std::ostream& os, const std::string& pad) const {

        const auto& loc = location();

        os << pad << "+ file:     " << loc.file() << "\n"
           << pad << "+ function: " << loc.func() << "\n"
           << pad << "+ line:     " << loc.line() << "\n"
           << pad << "+ link:     " << loc.file() << ":" << loc.line() << "\n"
           << pad << "+ message:  " << what() << "\n";
    }
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
// Validation Layer Exception (empty for now)
// ==========================================================
class Mars2GribValidationException : public Mars2GribGenericException {
public:

    Mars2GribValidationException(std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
        Mars2GribGenericException(reason, loc) {}
};


// ==========================================================
// Tables Layer Exception (empty for now)
// ==========================================================
class Mars2GribTableException : public Mars2GribGenericException {
public:

    Mars2GribTableException(std::string reason, const eckit::CodeLocation& loc = eckit::CodeLocation()) :
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

    void printFrame(std::ostream& os, const std::string& pad) const override {

        Mars2GribGenericException::printFrame(os, pad);

        auto print_opt = [&](const char* k, const std::optional<std::string>& v) {
            if (v)
                os << pad << "+ " << k << ": " << *v << "\n";
        };

        print_opt("concept", conceptName_);
        print_opt("variant", conceptVariant_);
        print_opt("stage", stage_);
        print_opt("section", section_);
    }

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

    void printFrame(std::ostream& os, const std::string& pad) const override {

        Mars2GribGenericException::printFrame(os, pad);

        os << pad << "+ marsDict:   " << marsDict_json_ << "\n"
           << pad << "+ geoDict:    " << geoDict_json_ << "\n"
           << pad << "+ parDict:    " << parDict_json_ << "\n"
           << pad << "+ optDict:    " << optDict_json_ << "\n"
           << pad << "+ encoderCfg: " << encoderCfg_json_ << "\n";
    }

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

inline constexpr int tabSize  = 4;
inline constexpr int lineSize = 120;

inline std::string indent(std::size_t level) {
    return std::string(level * tabSize, ' ');
}

inline void printExtendedStack(const std::exception& e, std::size_t level = 0, std::size_t frame = 1) {

    const std::string pad = indent(level);

    std::cerr << pad << "+ " << std::string(lineSize, '=') << std::endl
              << pad << "+ frame " << frame << std::endl
              << pad << "+ " << std::string(lineSize, '-') << std::endl;

    if (const auto* me = dynamic_cast<const Mars2GribGenericException*>(&e)) {
        me->printFrame(std::cerr, pad);
    }
    else {
        std::cerr << pad << "+ message: " << e.what() << std::endl;
    }

    std::cerr << pad << "+ " << std::string(lineSize, '+') << std::endl;

    try {
        std::rethrow_if_nested(e);
    }
    catch (const std::exception& nested) {
        printExtendedStack(nested, level + 1, frame + 1);
    }
}


inline std::string joinNumbers(const std::vector<long>& vec) {
    std::string s{"{"};
    for (size_t i = 0; i < vec.size(); ++i) {
        s += std::to_string(vec[i]);
        if (i + 1 < vec.size()) {
            s += ", ";
        }
    }
    s += "}";
    return s;
}

inline std::string joinNumbersDouble(const std::vector<double>& vec) {
    std::string s{"{"};
    for (size_t i = 0; i < vec.size(); ++i) {
        s += std::to_string(vec[i]);
        if (i + 1 < vec.size()) {
            s += ", ";
        }
    }
    s += "}";
    return s;
}

#define MARS2GRIB_CONCEPT_RETHROW(CONCEPTNAME, MESSAGE)                                             \
    std::throw_with_nested(Mars2GribConceptException(std::string(CONCEPTNAME##Name),                \
                                                     std::string(CONCEPTNAME##TypeName<Variant>()), \
                                                     std::to_string(Stage), std::to_string(Section), MESSAGE, Here()))


#define MARS2GRIB_CONCEPT_THROW(CONCEPTNAME, MESSAGE)                                                                  \
    do {                                                                                                               \
        throw Mars2GribConceptException(std::string(CONCEPTNAME##Name), std::string(CONCEPTNAME##TypeName<Variant>()), \
                                        std::to_string(Stage), std::to_string(Section), MESSAGE, Here());              \
    } while (0)

}  // namespace metkit::mars2grib::utils::exceptions

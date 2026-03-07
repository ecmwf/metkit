#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <eckit/types/DateTime.h>
#include "eckit/exception/Exceptions.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::deductions::detail {


enum class Period {
    Daily,
    Monthly
};

enum class StatOp {
    Average,
    Minimum,
    Maximum,
    StdDev
};


// =============================================================
// Decoded block
// =============================================================

struct StatTypeBlock {
    Period period;
    StatOp op;
};

// =============================================================
// Utilities
// =============================================================

// Count number of blocks in stattype (Fortran-equivalent logic)
inline std::size_t countBlocks(std::string_view stattype) {
    if (stattype.empty())
        return 0;

    std::size_t blocks = 1;
    for (char c : stattype) {
        if (c == '_')
            ++blocks;
    }
    return blocks;
}

// Compute month length in hours (Julian / truncated-Gregorian rule)
inline long previousMonthLengthHours(int year, int month) {

    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    if (month < 1 || month > 12) {
        throw Mars2GribGenericException("Invalid month (must be 1..12)", Here());
    }

    switch (month) {
        case 1:
        case 2:
        case 4:
        case 6:
        case 8:
        case 9:
        case 11:
            return 31 * 24;

        case 5:
        case 7:
        case 10:
        case 12:
            return 30 * 24;

        case 3:
            return ((year % 4) == 0 ? 29 : 28) * 24;
    }

    throw Mars2GribGenericException("Unreachable", Here());
}

// Compute month length in hours (Julian / truncated-Gregorian rule)
inline long monthLengthHours(int year, int month) {

    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    if (month < 1 || month > 12) {
        throw Mars2GribGenericException("Invalid month (must be 1..12)", Here());
    }

    switch (month) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31 * 24;

        case 4:
        case 6:
        case 9:
        case 11:
            return 30 * 24;

        case 2:
            return ((year % 4) == 0 ? 29 : 28) * 24;
    }

    throw Mars2GribGenericException("Unreachable", Here());
}

// =============================================================
// Decoding helpers
// =============================================================

inline Period decodePeriod_orThrow(std::string_view s) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    if (s == "da")
        return Period::Daily;
    if (s == "mo")
        return Period::Monthly;
    throw Mars2GribGenericException("Invalid period token: " + std::string(s), Here());
}

inline StatOp decodeOp_orThrow(std::string_view s) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    if (s == "av")
        return StatOp::Average;
    if (s == "mn")
        return StatOp::Minimum;
    if (s == "mx")
        return StatOp::Maximum;
    if (s == "sd")
        return StatOp::StdDev;
    throw Mars2GribGenericException("Invalid operation token: " + std::string(s), Here());
}

// =============================================================
// Parser + semantic validation
// =============================================================

inline std::vector<StatTypeBlock> parseStatType_or_throw(const std::string& stattype) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    std::vector<StatTypeBlock> blocks;

    std::size_t pos = 0;
    while (pos < stattype.size()) {

        if (pos + 4 > stattype.size()) {
            throw Mars2GribGenericException("Invalid stattype format", Here());
        }

        auto period = decodePeriod_orThrow(stattype.substr(pos, 2));
        auto op     = decodeOp_orThrow(stattype.substr(pos + 2, 2));

        blocks.push_back({period, op});

        pos += 4;
        if (pos < stattype.size()) {
            if (stattype[pos] != '_') {
                throw Mars2GribGenericException("Invalid stattype separator (expected '_')", Here());
            }
            ++pos;
        }
    }

    // Semantic validation: only one mo, one da, correct order
    int moIndex = -1;
    int daIndex = -1;

    for (std::size_t i = 0; i < blocks.size(); ++i) {
        if (blocks[i].period == Period::Monthly) {
            if (moIndex != -1)
                throw Mars2GribGenericException("Invalid stattype: more than one 'mo'", Here());
            moIndex = static_cast<int>(i);
        }
        if (blocks[i].period == Period::Daily) {
            if (daIndex != -1)
                throw Mars2GribGenericException("Invalid stattype: more than one 'da'", Here());
            daIndex = static_cast<int>(i);
        }
    }

    if (moIndex != -1 && daIndex != -1 && moIndex > daIndex) {
        throw Mars2GribGenericException("Invalid stattype order: 'mo' must precede 'da'", Here());
    }

    return blocks;
}

// =============================================================
// Pretty printing (test/debug)
// =============================================================

inline const char* toString(Period p) {
    switch (p) {
        case Period::Daily:
            return "Daily";
        case Period::Monthly:
            return "Monthly";
    }
    return "Unknown";
}

inline const char* toString(StatOp op) {
    switch (op) {
        case StatOp::Average:
            return "Average";
        case StatOp::Minimum:
            return "Minimum";
        case StatOp::Maximum:
            return "Maximum";
        case StatOp::StdDev:
            return "StandardDeviation";
    }
    return "Unknown";
}

inline void prettyPrint(const std::vector<StatTypeBlock>& blocks) {
    std::cout << "Decoded stattype (" << blocks.size() << " block(s)):\n";
    for (std::size_t i = 0; i < blocks.size(); ++i) {
        std::cout << "  [" << i << "] "
                  << "Period = " << toString(blocks[i].period) << ", Operation = " << toString(blocks[i].op) << '\n';
    }
}


// If unit is missing default is hours!!!
inline long toSeconds_or_throw(std::string_view step) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    if (step.empty()) {
        throw Mars2GribGenericException("Empty step string", Here());
    }

    // Split numeric part and optional unit
    std::size_t pos = 0;
    while (pos < step.size() && std::isdigit(static_cast<unsigned char>(step[pos]))) {
        ++pos;
    }

    if (pos == 0) {
        throw Mars2GribGenericException("Invalid step format (no numeric part): " + std::string(step), Here());
    }

    long value = 0;
    try {
        value = std::stol(std::string(step.substr(0, pos)));
    }
    catch (...) {
        throw Mars2GribGenericException("Invalid numeric value in step: " + std::string(step), Here());
    }

    // Default unit: hours
    char unit = 'h';
    if (pos < step.size()) {
        if (pos + 1 != step.size()) {
            throw Mars2GribGenericException("Invalid step format (trailing characters): " + std::string(step), Here());
        }
        unit = step[pos];
    }

    switch (unit) {
        case 'h':  // hours
            return value * 3600L;
        case 'm':  // minutes
            return value * 60L;
        case 's':  // seconds
            return value;
        case 'd':  // days
            return value * 86400L;
        default:
            throw Mars2GribGenericException(std::string("Unknown step unit: '") + unit + "'", Here());
    }
}

eckit::Date convert_YYYYMMDD2Date_or_throw(long YYYYMMDD) {

    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    long YYYY = YYYYMMDD / 10000;
    long MM   = (YYYYMMDD / 100) % 100;
    long DD   = YYYYMMDD % 100;

    // @todo Validate YYYY, MM, DD ranges?

    try {
        return eckit::Date(YYYY, MM, DD);
    }
    catch (const eckit::Exception& e) {
        throw Mars2GribGenericException("Invalid date value: " + std::string(e.what()), Here());
    }
}


eckit::Time convert_hhmmss2Time_or_throw(long hhmmss) {

    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    long hh = hhmmss / 10000;
    long mm = (hhmmss / 100) % 100;
    long ss = hhmmss % 100;

    // @todo Validate hh, mm, ss ranges?

    try {
        return eckit::Time(hh, mm, ss);
    }
    catch (const eckit::Exception& e) {
        throw Mars2GribGenericException("Invalid time value: " + std::string(e.what()), Here());
    }
}


}  // namespace metkit::mars2grib::backend::deductions::detail

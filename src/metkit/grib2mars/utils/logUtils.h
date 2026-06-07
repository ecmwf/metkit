/// @file logUtils.h
/// @brief Logging macros used by grib2mars.
#pragma once

#include "eckit/log/Log.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/grib2mars/utils/generalUtils.h"

/// @brief Log a validation or guard check.
#define GRIB2MARS_LOG_CHECK(msg)                                                        \
    do {                                                                                \
        LOG_DEBUG_LIB(LibMetkit) << "           GRIB2MARS-CHECK: " << msg << std::endl; \
    } while (0)


/// @brief Log a successful match.
#define GRIB2MARS_LOG_MATCH(msg)                                                        \
    do {                                                                                \
        LOG_DEBUG_LIB(LibMetkit) << "           GRIB2MARS-MATCH: " << msg << std::endl; \
    } while (0)


/// @brief Log a resolved value.
#define GRIB2MARS_LOG_RESOLVE(msg)                                                        \
    do {                                                                                  \
        LOG_DEBUG_LIB(LibMetkit) << "           GRIB2MARS-RESOLVE: " << msg << std::endl; \
    } while (0)


/// @brief Log an overridden value.
#define GRIB2MARS_LOG_OVERRIDE(msg)                                                        \
    do {                                                                                   \
        LOG_DEBUG_LIB(LibMetkit) << "           GRIB2MARS-OVERRIDE: " << msg << std::endl; \
    } while (0)

/// @brief Log a defaulted value.
#define GRIB2MARS_LOG_DEFAULT(msg)                                                        \
    do {                                                                                  \
        LOG_DEBUG_LIB(LibMetkit) << "           GRIB2MARS-DEFAULT: " << msg << std::endl; \
    } while (0)

/// @brief Log a concept invocation.
#define GRIB2MARS_LOG_CONCEPT(CONCEPTNAME)                                                                    \
    do {                                                                                                      \
        LOG_DEBUG_LIB(LibMetkit) << "       GRIB2MARS-CONCEPT:"                                               \
                                 << "[Concept " << std::string(CONCEPTNAME##Name) << "] "                     \
                                 << "Op called: "                                                             \
                                 << "Stage=" << Stage << ", "                                                 \
                                 << "Section=" << Section << ", "                                             \
                                 << "Variant=" << std::string(CONCEPTNAME##TypeName<Variant>()) << std::endl; \
    } while (0)

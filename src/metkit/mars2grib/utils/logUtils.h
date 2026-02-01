#pragma once

#include "eckit/log/Log.h"
#include "metkit/config/LibMetkit.h"

#define MARS2GRIB_LOG_CHECK(msg)                                                        \
    do {                                                                                \
        LOG_DEBUG_LIB(LibMetkit) << "           MARS2GRIB-CHECK: " << msg << std::endl; \
    } while (0)


#define MARS2GRIB_LOG_MATCH(msg)                                                        \
    do {                                                                                \
        LOG_DEBUG_LIB(LibMetkit) << "           MARS2GRIB-MATCH: " << msg << std::endl; \
    } while (0)


#define MARS2GRIB_LOG_RESOLVE(msg)                                                        \
    do {                                                                                  \
        LOG_DEBUG_LIB(LibMetkit) << "           MARS2GRIB-RESOLVE: " << msg << std::endl; \
    } while (0)


#define MARS2GRIB_LOG_OVERRIDE(msg)                                                        \
    do {                                                                                   \
        LOG_DEBUG_LIB(LibMetkit) << "           MARS2GRIB-OVERRIDE: " << msg << std::endl; \
    } while (0)

#define MARS2GRIB_LOG_CONCEPT(CONCEPTNAME)                                                                    \
    do {                                                                                                      \
        LOG_DEBUG_LIB(LibMetkit) << "       MARS2GRIB-CONCEPT:"                                               \
                                 << "[Concept " << std::string(CONCEPTNAME##Name) << "] "                     \
                                 << "Op called: "                                                             \
                                 << "Stage=" << Stage << ", "                                                 \
                                 << "Section=" << Section << ", "                                             \
                                 << "Variant=" << std::string(CONCEPTNAME##TypeName<Variant>()) << std::endl; \
    } while (0)

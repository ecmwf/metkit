#pragma once

#include "eckit/log/Log.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2mars/utils/generalUtils.h"

#define MARS2MARS_LOG_CHECK(msg)                                                        \
    do {                                                                                \
        LOG_DEBUG_LIB(LibMetkit) << "           MARS2MARS-CHECK: " << msg << std::endl; \
    } while (0)


#define MARS2MARS_LOG_MATCH(msg)                                                        \
    do {                                                                                \
        LOG_DEBUG_LIB(LibMetkit) << "           MARS2MARS-MATCH: " << msg << std::endl; \
    } while (0)


#define MARS2MARS_LOG_RESOLVE(msg)                                                        \
    do {                                                                                  \
        LOG_DEBUG_LIB(LibMetkit) << "           MARS2MARS-RESOLVE: " << msg << std::endl; \
    } while (0)


#define MARS2MARS_LOG_OVERRIDE(msg)                                                        \
    do {                                                                                   \
        LOG_DEBUG_LIB(LibMetkit) << "           MARS2MARS-OVERRIDE: " << msg << std::endl; \
    } while (0)

#define MARS2MARS_LOG_DEFAULT(msg)                                                        \
    do {                                                                                  \
        LOG_DEBUG_LIB(LibMetkit) << "           MARS2MARS-DEFAULT: " << msg << std::endl; \
    } while (0)

#define MARS2MARS_LOG_CONCEPT(CONCEPTNAME)                                                                    \
    do {                                                                                                      \
        LOG_DEBUG_LIB(LibMetkit) << "       MARS2MARS-CONCEPT:"                                               \
                                 << "[Concept " << std::string(CONCEPTNAME##Name) << "] "                     \
                                 << "Op called: "                                                             \
                                 << "Stage=" << Stage << ", "                                                 \
                                 << "Section=" << Section << ", "                                             \
                                 << "Variant=" << std::string(CONCEPTNAME##TypeName<Variant>()) << std::endl; \
    } while (0)

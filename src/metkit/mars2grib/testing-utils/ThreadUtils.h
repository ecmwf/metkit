/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <pthread.h>
#include <sched.h>
#include <exception>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"

namespace metkit::mars2grib::testing_utils::run_tests {

/// Return the first `count` CPUs of the calling thread's affinity mask.
inline std::vector<int> availableCpus(std::size_t count) {
    cpu_set_t available;
    CPU_ZERO(&available);
    if (pthread_getaffinity_np(pthread_self(), sizeof(available), &available) != 0) {
        throw eckit::Exception("Unable to obtain process CPU affinity", Here());
    }

    std::vector<int> cpus;
    cpus.reserve(count);
    for (int cpu = 0; cpu < CPU_SETSIZE && cpus.size() < count; ++cpu) {
        if (CPU_ISSET(cpu, &available)) {
            cpus.push_back(cpu);
        }
    }
    if (cpus.size() != count) {
        throw eckit::UserError("Requested " + std::to_string(count) + " threads, but only " +
                                   std::to_string(cpus.size()) + " CPUs are available",
                               Here());
    }
    return cpus;
}

inline void bindCurrentThread(int cpu) {
    cpu_set_t affinity;
    CPU_ZERO(&affinity);
    CPU_SET(cpu, &affinity);
    if (pthread_setaffinity_np(pthread_self(), sizeof(affinity), &affinity) != 0) {
        throw eckit::Exception("Unable to bind thread to CPU " + std::to_string(cpu), Here());
    }
}

[[noreturn]] inline void rethrowAsEckit(std::exception_ptr error, const std::string& context) {
    try {
        std::rethrow_exception(error);
    }
    catch (const eckit::Exception&) {
        throw;
    }
    catch (const std::exception& exception) {
        throw eckit::Exception(context + ": " + exception.what(), Here());
    }
    catch (...) {
        throw eckit::Exception(context + ": unknown exception", Here());
    }
}

}  // namespace metkit::mars2grib::testing_utils::run_tests

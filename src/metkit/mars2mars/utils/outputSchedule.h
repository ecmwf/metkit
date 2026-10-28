// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file outputSchedule.h
/// @brief The forecast steps the model writes, per MARS context, read from `mars2mars-output-schedule.yaml`.
#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "eckit/config/YAMLConfiguration.h"
#include "eckit/filesystem/PathName.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"

namespace metkit::mars2mars::utils {

/// @brief Output schedule of the model: the forecast steps written for a (class, stream, type, time) context.
///
/// The schedule is a list of consecutive step bands. A band covers the steps after the end of the previous
/// band up to `to` (inclusive), every `by` hours. The window of a field whose statistic runs "since the
/// previous post-processing" is the distance from its step to the previous scheduled step, i.e. the `by` of
/// the band holding the step. Step 0 has no previous step and therefore no window.
///
/// The default file is `share/metkit/mars2mars-output-schedule.yaml`; another file can be given through the
/// `outputScheduleFile` option. Files are parsed once and cached.
class OutputSchedule {
public:

    struct Band {
        long to;
        long by;
    };

    struct Context {
        std::string marsClass;
        std::string stream;
        std::string type;
        long hour;

        std::string str() const {
            return "class=" + marsClass + ",stream=" + stream + ",type=" + type + ",time=" + hhmm(hour);
        }
    };

    /// @brief Hour of a MARS `time` read as an integer, in either `hhmm` or `hhmmss` form.
    static long hourOfMarsTime(long time) { return time >= 10000 ? time / 10000 : time / 100; }

    /// @brief Hour of a MARS `time` written as text (`0`, `00`, `0000`, `120000`).
    static long hourOfMarsTime(const std::string& time) { return hourOfMarsTime(std::stol(time)); }

    /// @brief The schedule of `path`, or the default file when `path` is empty.
    static const OutputSchedule& get(const std::string& path = "") {
        static std::mutex mutex;
        static std::map<std::string, std::unique_ptr<OutputSchedule>> cache;

        const std::lock_guard<std::mutex> lock(mutex);
        auto it = cache.find(path);
        if (it == cache.end()) {
            const eckit::PathName file =
                path.empty() ? LibMetkit::configFile("mars2mars-output-schedule.yaml") : eckit::PathName{path};
            it = cache.emplace(path, std::unique_ptr<OutputSchedule>(new OutputSchedule(file))).first;
        }
        return *it->second;
    }

    /// @brief Whether the context has a schedule.
    bool has(const Context& context) const { return schedules_.find(key(context)) != schedules_.end(); }

    /// @brief Window (hours) ending at `step`: the distance to the previous scheduled step.
    ///
    /// Returns `std::nullopt` for step 0 (no previous step). Throws when the context has no schedule or
    /// when `step` is not one of its output steps.
    std::optional<long> window(const Context& context, long step) const {
        using exceptions::Mars2marsGenericException;

        const auto it = schedules_.find(key(context));
        if (it == schedules_.end()) {
            throw Mars2marsGenericException("No output schedule for " + context.str() + " in " + file_ +
                                                ": the window of a field on a single step cannot be derived",
                                            Here());
        }

        if (step == 0) {
            return std::nullopt;
        }

        long previousEnd = 0;
        for (const auto& band : it->second) {
            if (step <= band.to) {
                if ((step - previousEnd) % band.by != 0) {
                    break;
                }
                return band.by;
            }
            previousEnd = band.to;
        }

        throw Mars2marsGenericException(
            "step " + std::to_string(step) + " is not an output step of " + context.str() + " in " + file_, Here());
    }

private:

    explicit OutputSchedule(const eckit::PathName& file) : file_(file.asString()) {
        using exceptions::Mars2marsGenericException;

        if (!file.exists()) {
            throw Mars2marsGenericException("Output schedule file " + file_ + " does not exist", Here());
        }

        const eckit::YAMLConfiguration conf(file);
        for (const auto& schedule : conf.getSubConfigurations("schedules")) {
            std::vector<Band> bands;
            long previousEnd = 0;
            for (const auto& band : schedule.getSubConfigurations("bands")) {
                const Band b{band.getLong("to"), band.getLong("by")};
                if (b.by <= 0 || b.to <= previousEnd || (b.to - previousEnd) % b.by != 0) {
                    throw Mars2marsGenericException("Invalid band {to: " + std::to_string(b.to) +
                                                        ", by: " + std::to_string(b.by) + "} after step " +
                                                        std::to_string(previousEnd) + " in " + file_,
                                                    Here());
                }
                bands.push_back(b);
                previousEnd = b.to;
            }
            if (bands.empty()) {
                throw Mars2marsGenericException("A schedule without bands in " + file_, Here());
            }

            for (const auto& time : schedule.getStringVector("time")) {
                const Context context{schedule.getString("class"), schedule.getString("stream"),
                                      schedule.getString("type"), hourOfMarsTime(time)};
                if (!schedules_.emplace(key(context), bands).second) {
                    throw Mars2marsGenericException("Duplicate schedule for " + context.str() + " in " + file_, Here());
                }
            }
        }
    }

    static std::string hhmm(long hour) { return (hour < 10 ? "0" : "") + std::to_string(hour) + "00"; }

    static std::string key(const Context& c) {
        return c.marsClass + ":" + c.stream + ":" + c.type + ":" + std::to_string(c.hour);
    }

    std::string file_;
    std::map<std::string, std::vector<Band>> schedules_;
};

}  // namespace metkit::mars2mars::utils

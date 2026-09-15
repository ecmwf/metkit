/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2grib/utils/type_traits_name.h"

namespace metkit::mars2grib::testing_utils {

struct MiscKeyRequirement {
    std::string key;
    std::optional<std::string> type;
    bool mandatory{false};
    std::optional<std::string> defaultValue;
};

class MiscRequirementsDictionary {
public:
    explicit MiscRequirementsDictionary(const eckit::LocalConfiguration& mars) : mars_{mars} {}

    const eckit::LocalConfiguration& mars() const { return mars_; }

    const std::vector<MiscKeyRequirement>& requirements() const { return requirements_; }

    void record_optional(std::string_view key) const { merge(key, std::nullopt, false, std::nullopt); }

    template <typename T>
    void record_optional(std::string_view key) const {
        merge(key, std::string{utils::type_name<T>()}, false, std::nullopt);
    }

    template <typename T>
    void record_mandatory(std::string_view key, const std::optional<T>& defaultValue) const {
        merge(key, std::string{utils::type_name<T>()}, true,
              defaultValue ? std::optional<std::string>{stringify(*defaultValue)} : std::nullopt);
    }

private:
    template <typename T>
    static std::string stringify(const T& value) {
        if constexpr (std::is_same_v<T, bool>) {
            return value ? "true" : "false";
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            return value;
        }
        else if constexpr (std::is_floating_point_v<T>) {
            std::ostringstream out;
            out << std::setprecision(17) << value;
            return out.str();
        }
        else if constexpr (std::is_integral_v<T>) {
            return std::to_string(value);
        }
        else {
            std::ostringstream out;
            bool first = true;
            for (const auto& item : value) {
                if (!first) {
                    out << ',';
                }
                first = false;
                out << std::setprecision(17) << item;
            }
            return out.str();
        }
    }

    void merge(std::string_view key, std::optional<std::string> type, bool mandatory,
               std::optional<std::string> defaultValue) const {
        for (auto& requirement : requirements_) {
            if (requirement.key != key) {
                continue;
            }

            if (type) {
                if (!requirement.type) {
                    requirement.type = std::move(type);
                }
                else if (*requirement.type != *type && requirement.type->find(*type) == std::string::npos) {
                    *requirement.type += "|" + *type;
                }
            }
            requirement.mandatory = requirement.mandatory || mandatory;
            if (defaultValue) {
                requirement.defaultValue = std::move(defaultValue);
            }
            return;
        }

        requirements_.push_back(
            MiscKeyRequirement{std::string{key}, std::move(type), mandatory, std::move(defaultValue)});
    }

    const eckit::LocalConfiguration& mars_;
    mutable std::vector<MiscKeyRequirement> requirements_;
};

}  // namespace metkit::mars2grib::testing_utils

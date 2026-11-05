#pragma once

#include <algorithm>
#include <cstddef>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "eckit/exception/Exceptions.h"

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/codes/api/KeyIterator.h"

#include "metkit/grib2mars/utils/grib2marsExceptions.h"

namespace metkit::grib2mars::rules::impl {

class MarsTopology {
public:

    using container_type = std::vector<std::string>;
    using const_iterator = container_type::const_iterator;

    MarsTopology() = delete;

    explicit MarsTopology(const metkit::codes::CodesHandle& grib) : MarsTopology(make_MarsKeywords(grib)) {}

    MarsTopology(const MarsTopology&)            = default;
    MarsTopology& operator=(const MarsTopology&) = default;

    MarsTopology(MarsTopology&&)            = default;
    MarsTopology& operator=(MarsTopology&&) = default;

    ~MarsTopology() = default;

    const_iterator begin() const {
        using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

        try {
            return keywords_.begin();
        }
        catch (...) {
            std::throw_with_nested(
                Grib2MarsGenericException("Failed to access begin iterator of grib2mars MarsTopology", Here()));
        }
    }

    const_iterator end() const {
        using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

        try {
            return keywords_.end();
        }
        catch (...) {
            std::throw_with_nested(
                Grib2MarsGenericException("Failed to access end iterator of grib2mars MarsTopology", Here()));
        }
    }

    bool contains(const std::string& keyword) const {
        using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

        try {
            const std::unordered_set<std::string>::const_iterator it = index_.find(keyword);

            return it != index_.end();
        }
        catch (...) {
            std::throw_with_nested(Grib2MarsGenericException(
                "Failed to check MARS keyword `" + keyword + "` in grib2mars MarsTopology", Here()));
        }
    }

    std::size_t size() const {
        using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

        try {
            return keywords_.size();
        }
        catch (...) {
            std::throw_with_nested(Grib2MarsGenericException("Failed to get size of grib2mars MarsTopology", Here()));
        }
    }

    bool empty() const {
        using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

        try {
            return keywords_.empty();
        }
        catch (...) {
            std::throw_with_nested(
                Grib2MarsGenericException("Failed to check whether grib2mars MarsTopology is empty", Here()));
        }
    }

private:

    explicit MarsTopology(std::vector<std::string> keywords) :
        keywords_(std::move(keywords)), index_(make_index(keywords_)) {}

    static bool contains_keyword(const std::vector<std::string>& keywords, const std::string& keyword) {
        using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

        try {
            const std::vector<std::string>::const_iterator it = std::find(keywords.begin(), keywords.end(), keyword);

            return it != keywords.end();
        }
        catch (...) {
            std::throw_with_nested(Grib2MarsGenericException(
                "Failed to check MARS keyword `" + keyword + "` while building grib2mars MarsTopology", Here()));
        }
    }

    static void append_unique(std::vector<std::string>& keywords, const std::string& keyword) {
        using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

        try {
            const bool alreadyPresent = contains_keyword(keywords, keyword);

            if (!alreadyPresent) {
                keywords.push_back(keyword);
            }
        }
        catch (...) {
            std::throw_with_nested(Grib2MarsGenericException(
                "Failed to append MARS keyword `" + keyword + "` while building grib2mars MarsTopology", Here()));
        }
    }

    static void remove_keyword(std::vector<std::string>& keywords, const std::string& keyword) {
        using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

        try {
            keywords.erase(std::remove(keywords.begin(), keywords.end(), keyword), keywords.end());
        }
        catch (...) {
            std::throw_with_nested(Grib2MarsGenericException(
                "Failed to remove MARS keyword `" + keyword + "` while building grib2mars MarsTopology", Here()));
        }
    }

    static void append_ecCodes_MarsNamespace_keywords(const metkit::codes::CodesHandle& grib,
                                                      std::vector<std::string>& keywords) {
        using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

        try {
            for (const metkit::codes::KeyIterator& key : grib.keys(metkit::codes::namespaces::mars)) {
                const std::string keyword = key.name();
                append_unique(keywords, keyword);
            }
        }
        catch (...) {
            std::throw_with_nested(Grib2MarsGenericException(
                "Failed to extract ecCodes MARS namespace while building grib2mars MarsTopology", Here()));
        }
    }

    static void append_packing_keyword(const metkit::codes::CodesHandle& grib, std::vector<std::string>& keywords) {
        using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

        try {
            const bool hasPackingType = grib.has("packingType");

            if (hasPackingType) {
                append_unique(keywords, "packing");
            }
        }
        catch (...) {
            std::throw_with_nested(Grib2MarsGenericException(
                "Failed to add MARS keyword `packing` while building grib2mars MarsTopology", Here()));
        }
    }

    static void append_geometry_keyword(const metkit::codes::CodesHandle& grib, std::vector<std::string>& keywords) {
        using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

        try {
            const bool hasGridType = grib.has("gridType");

            if (!hasGridType) {
                return;
            }

            const std::string gridType = grib.getString("gridType");

            if (gridType == "sh") {
                remove_keyword(keywords, "grid");
                append_unique(keywords, "truncation");
                return;
            }

            remove_keyword(keywords, "truncation");
            append_unique(keywords, "grid");
        }
        catch (...) {
            std::throw_with_nested(
                Grib2MarsGenericException("Failed to select MARS geometry keyword `grid`/`truncation` "
                                          "while building grib2mars MarsTopology",
                                          Here()));
        }
    }

    static void append_required_keywords(std::vector<std::string>& keywords) {
        using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

        try {
            append_unique(keywords, "class");
            append_unique(keywords, "stream");
            append_unique(keywords, "type");
            append_unique(keywords, "origin");
        }
        catch (...) {
            std::throw_with_nested(Grib2MarsGenericException(
                "Failed to append required MARS keywords while building grib2mars MarsTopology", Here()));
        }
    }

    static std::vector<std::string> make_MarsKeywords(const metkit::codes::CodesHandle& grib) {
        using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

        try {
            std::vector<std::string> keywords;

            append_ecCodes_MarsNamespace_keywords(grib, keywords);

            // MARS keywords required by mars2grib but not always injected by ecCodes.
            append_packing_keyword(grib, keywords);
            append_geometry_keyword(grib, keywords);
            append_required_keywords(keywords);

            return keywords;
        }
        catch (...) {
            std::throw_with_nested(
                Grib2MarsGenericException("Failed to build MARS keyword list for grib2mars MarsTopology", Here()));
        }
    }

    static std::unordered_set<std::string> make_index(const std::vector<std::string>& keywords) {
        using metkit::grib2mars::utils::exceptions::Grib2MarsGenericException;

        try {
            std::unordered_set<std::string> index;

            for (const std::string& keyword : keywords) {
                index.insert(keyword);
            }

            return index;
        }
        catch (...) {
            std::throw_with_nested(
                Grib2MarsGenericException("Failed to build keyword index for grib2mars MarsTopology", Here()));
        }
    }

private:

    const std::vector<std::string> keywords_;
    const std::unordered_set<std::string> index_;
};

}  // namespace metkit::grib2mars::rules::impl
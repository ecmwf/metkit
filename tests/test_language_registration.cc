// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

// Keyword registration, categories and the typing of parameters.
// The test is run twice: with the language served from language.bin and with METKIT_FORCE_BINFILE_CREATION (YAML
// parsing only, keywords are registered lazily as the languages are parsed).
// The CASEs run in order and the first one relies on no language having been loaded yet.

#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "eckit/config/YAMLConfiguration.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/testing/Test.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsRequest.h"

namespace metkit::mars::test {

using strings = std::vector<std::string>;

// typed parameters refer to the types of a language: languages are obtained from MarsLanguage::get(), which keeps
// them for the lifetime of the process (MarsLanguage(const std::string&) is only kept, deprecated, for compatibility)
static_assert(!std::is_constructible_v<MarsLanguage, MarsVerb>);

//----------------------------------------------------------------------------------------------------------------------

CASE("a name stored before its keyword is registered is updated, not duplicated") {
    MarsRequest r("retrieve");
    r.values("target", strings{"a"});

    MarsLanguage::get("retrieve");  // registers "target" if the language is parsed from YAML

    r.values("target", strings{"b"});

    size_t count = 0;
    for (const auto& p : r) {
        count += (p.name() == "target") ? 1 : 0;
    }
    EXPECT_EQUAL(count, 1);
    EXPECT_EQUAL(r.values("target"), strings{"b"});
    EXPECT(r.has(MarsLanguage::keyword("target")));
}

CASE("environ requests store known keywords without a language definition") {
    MarsRequest env("environ");
    EXPECT_NO_THROW(env.values("class", strings{"od"}));
    EXPECT_NO_THROW(env.values("user", strings{"someone"}));
    EXPECT_EQUAL(env.values("class"), strings{"od"});
}

CASE("keyword categories follow the language definition") {
    const eckit::Value languages = eckit::YAMLParser::decodeFile(LibMetkit::languageYamlFile());
    const eckit::Value verbs     = languages.keys();
    for (size_t i = 0; i < verbs.size(); ++i) {
        std::string verb = verbs[i];
        if (verb[0] == '_' || !MarsLanguage::verb_or_default(verb)) {
            continue;
        }
        MarsLanguage::get(verb);
    }

    const std::map<std::string, Category> categories{{"data", Category::Data},
                                                     {"derived", Category::Derived},
                                                     {"postproc", Category::PostProc},
                                                     {"sink", Category::Sink}};

    // the category is global to a keyword: only check the keywords that have the same category in every language
    std::map<std::string, std::set<Category>> expected;
    for (size_t i = 0; i < verbs.size(); ++i) {
        std::string verb = verbs[i];
        if (verb[0] == '_' || !MarsLanguage::verb_or_default(verb)) {
            continue;
        }
        eckit::Value lang = languages[verb];
        eckit::Value keys = lang.keys();
        for (size_t j = 0; j < keys.size(); ++j) {
            std::string kw = keys[j];
            if (kw[0] == '_') {
                continue;
            }
            expected[kw].insert(lang[kw].contains("category") ? categories.at(lang[kw]["category"])
                                                              : Category::Other);
        }
    }

    // "day" is registered by MarsLanguage::init() as Category::Derived, before any language is parsed
    expected.erase("day");

    size_t checked = 0;
    std::vector<std::string> mismatches;
    for (const auto& [kw, cats] : expected) {
        if (cats.size() != 1) {
            eckit::Log::info() << "keyword '" << kw << "' has different categories in different languages"
                               << std::endl;
            continue;
        }
        Category actual = MarsLanguage::category(MarsLanguage::keyword(kw));
        if (actual != *cats.begin()) {
            eckit::Log::info() << "keyword '" << kw << "': expected category " << static_cast<int>(*cats.begin())
                               << ", got " << static_cast<int>(actual) << std::endl;
            mismatches.push_back(kw);
        }
        ++checked;
    }
    EXPECT(checked > 0);
    EXPECT_EQUAL(mismatches, strings{});

    EXPECT(MarsLanguage::category(MarsLanguage::keyword("grid")) == Category::PostProc);
    EXPECT(MarsLanguage::category(MarsLanguage::keyword("target")) == Category::Sink);
}

CASE("addKeyword of an existing name does not shift the category of the next keyword") {
    MarsLanguage::addKeyword("class", Category::Other);  // already registered
    MarsKeyword k = MarsLanguage::addKeyword("test_registration_new_keyword", Category::PostProc);
    EXPECT(MarsLanguage::category(k) == Category::PostProc);
    EXPECT(MarsLanguage::category(MarsLanguage::keyword("class")) == Category::Data);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}

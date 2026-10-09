/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Runs in its own process, with a METKIT_HOME holding language.bin but no language.yaml:
// the language must be served from the binary file even when get(Verb) is the first entry point.

#include <cstdint>
#include <string>

#include "eckit/testing/Test.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/Dictionary.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Serialize.h"

namespace metkit::mars::test {

/// Looks up a verb in the binary file directly, without initialising MarsLanguage
static Verb verb_from_binary_file(const std::string& name) {
    MemFile file{LibMetkit::languageBinaryFile().localPath()};
    EXPECT_EQUAL(std::string{file.readString(4)}, "LANG");
    EXPECT_EQUAL(file.read16(), LibMetkit::binaryFilesVersion());

    const uint32_t numVerbs = file.read32();
    for (uint32_t i = 0; i < numVerbs; ++i) {
        if (file.readString() == name) {
            return static_cast<Verb>(i + 1);  // index 0 is reserved for "not found"
        }
    }
    return 0;
}

CASE("get(Verb) as first entry point uses the binary language file") {
    EXPECT(LibMetkit::languageBinaryFile().exists());
    EXPECT(!LibMetkit::languageYamlFile().exists());

    const Verb retrieve = verb_from_binary_file("retrieve");
    EXPECT(retrieve != 0);

    const MarsLanguage* language = nullptr;
    EXPECT_NO_THROW(language = &MarsLanguage::get(retrieve));
    EXPECT(language != nullptr);
    EXPECT_EQUAL(language, &MarsLanguage::get("retrieve"));
    EXPECT_NO_THROW(language->type("param"));
}

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}

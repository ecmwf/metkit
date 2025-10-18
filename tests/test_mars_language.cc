
#include <assert.h>
#include "eckit/testing/Test.h"
#include "metkit/mars/MarsLanguage.h"


namespace metkit::mars::test {

CASE("retrieve_best_match_param_matching") {
    const auto language = MarsLanguage("retrieve");

    // Strict is defaulted to true and this is not matching
    auto match = language.bestMatch("parameter", {"parameter"}, false, false, false, {});

    EXPECT(match == "parameter");
};


CASE("retrieve_best_match_param_not_matching") {
    const auto language = MarsLanguage("retrieve");

    // Strict is defaulted to true and this is not matching
    auto match = language.bestMatch("param", {"parameter"}, false, false, false, {});

    // TODO:(TKR) THIS IS MENTAL
    EXPECT(match == "parameter");

    auto empty= language.bestMatch("param", {"car"}, false, false, false, {});

    EXPECT(empty == "");
};

CASE("retrieve_best_match_param_not_matching_throw") {
    const auto language = MarsLanguage("retrieve");

    // Strict is defaulted to true and this is not matching
    auto match = language.bestMatch("param", {"parameter"}, true, false, false, {});

    // TODO:(TKR) THIS IS MENTAL
    EXPECT(match == "parameter");

    EXPECT_THROWS(language.bestMatch("param", {"car"}, true, false, false, {}));
};


CASE("retrieve_best_match_param_not_matching") {
    const auto language = MarsLanguage("retrieve");

    // Strict is defaulted to true and this is not matching
    auto match = language.bestMatch("param", {"parameter"}, false, false, true, {});

    // TODO:(TKR) THIS IS MENTAL
    EXPECT(match == "parameter");

    match = language.bestMatch("par", {"parameter"}, false, false, true, {});

    // TODO:(TKR) THIS IS MENTAL
    EXPECT(match == "parameter");

    match = language.bestMatch("par", {"car"}, false, false, true, {});

    EXPECT(match == "");
};


}  // namespace metkit::mars::test
int main(int argc, char** argv) {

    putenv("METKIT_LANGUAGE_STRICT_MODE=0");
    auto res = eckit::testing::run_tests(argc, argv);
    unsetenv("METKIT_LANGUAGE_STRICT_MODE");
    return res;
}

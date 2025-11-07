#include <assert.h>
#include "eckit/testing/Test.h"
#include "metkit/mars/MarsLanguage.h"


namespace metkit::mars::test {

CASE("retrieve_best_match_param_not_matching") {
    const auto language = MarsLanguage("retrieve");

    // Strict is defaulted to true and this is not matching
    EXPECT_THROWS(language.bestMatch("param", {"parameter"}, false, false, true, {}));
};

CASE("retrieve_best_match_param_matching") {
    const auto language = MarsLanguage("retrieve");

    // Strict is defaulted to true and this is not matching
    auto match = language.bestMatch("param", {"parameter", "param"}, false, false, true, {});

    EXPECT(match == "param");
};

}  // namespace metkit::mars::test
int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}

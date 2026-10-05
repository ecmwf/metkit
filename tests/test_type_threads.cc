/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <atomic>
#include <cstddef>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/value/Value.h"

#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Type.h"
#include "metkit/mars/TypesFactory.h"

namespace metkit::mars::test {

//----------------------------------------------------------------------------------------------------------------------

// Meaningful under ThreadSanitizer: the lazy loading of an enum values file must not race with readers of the flags.
CASE("test_enum_lazy_values_file_concurrent_flag_reads") {

    eckit::Value settings = eckit::Value::makeOrderedMap();
    settings["type"]      = "enum";
    settings["multiple"]  = true;
    settings["values"]    = "obstype.yaml";  // grouped values: loading them sets the "has groups" flag

    // a fresh instance, so that its values file has not been loaded yet
    std::shared_ptr<const Type> type = TypesFactory::build(MarsLanguage::addKeyword("obstype"), settings);

    constexpr size_t readers = 4;
    std::atomic<size_t> ready{0};
    std::atomic<bool> stop{false};

    std::atomic<bool> flagsOk{true};
    std::vector<std::thread> threads;
    threads.reserve(readers);
    for (size_t i = 0; i < readers; ++i) {
        threads.emplace_back([&] {
            ++ready;
            while (!stop) {
                if (!type->multiple() || !type->flatten()) {
                    flagsOk = false;
                }
            }
        });
    }

    while (ready < readers) {
        std::this_thread::yield();
    }

    std::vector<std::string> values{"conv"};
    type->expand(values);  // triggers the lazy load of obstype.yaml
    EXPECT(values.size() > 1);

    stop = true;
    for (auto& thread : threads) {
        thread.join();
    }
    EXPECT(flagsOk);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}

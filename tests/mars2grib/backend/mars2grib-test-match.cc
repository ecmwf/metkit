#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <tuple>
#include <cassert>

// eckit include
#include <eckit/config/LocalConfiguration.h>

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

#include "metkit/mars2grib/backend/concepts/MatchingCallbacksRegistry.h"

int main() {

    using Registry = metkit::mars2grib::backend::concepts_::MatchingCallbacksRegistry<eckit::LocalConfiguration,eckit::LocalConfiguration>;

    const auto& callbacks = Registry::matchingCallbacks;

    std::cout << "Hello World: " << callbacks.size() << std::endl;

    for ( const auto& f : callbacks ) {
        if ( f ) {
            eckit::LocalConfiguration xxx;
            eckit::LocalConfiguration yyy;
            std::size_t r = f( xxx, yyy );
        }
        else {
            std::cout << "Aiaiaiaiaaii" << std::endl;
        }
    }

    // Exit point
    return 0;
};

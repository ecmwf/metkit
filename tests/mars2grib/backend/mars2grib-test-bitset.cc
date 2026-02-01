#include <iostream>

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

#include "metkit/mars2grib/backend/concepts/BitSet.h"
#include "metkit/mars2grib/backend/concepts/capabilitiesRegistry.h"

int main() {

    using metkit::mars2grib::backend::concepts_::BitSet;
    using metkit::mars2grib::backend::concepts_::isMissing;
    using metkit::mars2grib::backend::concepts_::totalVariants;

    BitSet<totalVariants> bs;
    BitSet<totalVariants> bs2;

    std::cout << "test01: " << bs << std::endl;

    bs = BitSet<totalVariants>::ones();

    std::cout << "test02: " << bs << std::endl;

    bs = BitSet<totalVariants>::zero();
    bs.set(0);
    bs.set(1);
    bs.set(109);
    bs.set(110);
    std::cout << "test03: " << bs << std::endl;

    bs2 = BitSet<totalVariants>::zero();
    bs2.set(25);
    bs2.set(26);
    bs2.set(27);

    std::cout << "test04: " << bs2 << std::endl;

    auto tmp = (bs | bs2);
    std::cout << "test05: " << tmp << std::endl;

    auto tmp1 = tmp;
    std::cout << "test06: " << (tmp == tmp1) << std::endl;
    std::cout << "test07: " << (tmp == bs2) << std::endl;

    std::cout << "test08: " << tmp.count() << std::endl;

    const auto compressionMask = tmp.make_reverse_compression();

    std::cout << "Compression mask(" << BitSet<totalVariants>::nbits << ")";
    std::string separator = "[";
    for (const auto& x : compressionMask) {
        if (isMissing(x)) {
            std::cout << separator << " " << "missing";
        }
        else {
            std::cout << separator << " " << int(x);
        }
        separator = ",";
    }
    std::cout << " ]" << std::endl;

    // Exit point
    return 0;
};

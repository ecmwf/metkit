#pragma once

namespace metkit::mars2grib {

struct Options {
    bool applyChecks                   = true;
    bool enableOverride                = false;
    bool enableBitsPerValueCompression = false;
};

}  // namespace metkit::mars2grib

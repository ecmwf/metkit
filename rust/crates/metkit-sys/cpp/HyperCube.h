// metkit HyperCube bridge — wraps `metkit::hypercube::HyperCube`.
#pragma once

#include "MarsRequest.h"
#include "metkit/hypercube/HyperCube.h"

#include "rust/cxx.h"

#include <cstddef>
#include <memory>

namespace metkit_bridge {

//----------------------------------------------------------------------------------------------------------------------

/// Wraps `metkit::hypercube::HyperCube` for Rust FFI.
class HyperCubeWrapper {
    std::unique_ptr<metkit::hypercube::HyperCube> cube_;

public:

    explicit HyperCubeWrapper(const MarsRequestWrapper& request);

    size_t size() const;
    size_t count() const;
    size_t count_vacant() const;
    bool contains(const MarsRequestWrapper& request) const;
    bool clear(const MarsRequestWrapper& request);
    size_t field_ordinal(const MarsRequestWrapper& request) const;

    // Access underlying
    const metkit::hypercube::HyperCube& inner() const { return *cube_; }

    // ============== Factories ==============

    /// Build a HyperCube from a MarsRequest.
    static std::unique_ptr<HyperCubeWrapper> create(const MarsRequestWrapper& request);
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit_bridge

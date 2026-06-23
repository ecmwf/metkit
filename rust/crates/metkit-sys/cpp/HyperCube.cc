// metkit HyperCube bridge — implementation.

#include "metkit_exceptions.h"

#include "HyperCube.h"

namespace metkit_bridge {

//----------------------------------------------------------------------------------------------------------------------

HyperCubeWrapper::HyperCubeWrapper(const MarsRequestWrapper& request) :
    cube_(std::make_unique<metkit::hypercube::HyperCube>(request.inner())) {}

size_t HyperCubeWrapper::size() const {
    return cube_->size();
}

size_t HyperCubeWrapper::count() const {
    return cube_->count();
}

size_t HyperCubeWrapper::count_vacant() const {
    return cube_->countVacant();
}

bool HyperCubeWrapper::contains(const MarsRequestWrapper& request) const {
    return cube_->contains(request.inner());
}

bool HyperCubeWrapper::clear(const MarsRequestWrapper& request) {
    return cube_->clear(request.inner());
}

size_t HyperCubeWrapper::field_ordinal(const MarsRequestWrapper& request) const {
    return cube_->fieldOrdinal(request.inner());
}

//----------------------------------------------------------------------------------------------------------------------

std::unique_ptr<HyperCubeWrapper> HyperCubeWrapper::create(const MarsRequestWrapper& request) {
    return std::make_unique<HyperCubeWrapper>(request);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit_bridge

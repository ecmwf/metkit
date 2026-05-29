#include <memory>
#include <vector>

#include "metkit/mars2grib/debug/reproducer.h"
#include "metkit/codes/api/CodesTypes.h"

// ============================================================================
// MockDictionary Definition
// ============================================================================
namespace metkit::mars2grib::debug::reproducer {


class MockDictionary {
public:
    std::shared_ptr<Recorder> recorder;

    // Pipeline constructor
    explicit MockDictionary(std::shared_ptr<Recorder> rec)
        : recorder(std::move(rec)) {}

    // Output utility
    void dump(const std::string& filename) const {
        if (recorder) recorder->generate(filename);
    }

    template <typename T>
    void set(const std::string& k, const T& v) {
        recorder->record(k, v);
    }

    template <typename T>
    void set_array(const std::string& k, const std::vector<T>& v) {
        recorder->record_array(k, v.data(), v.size());
    }

    template <typename T>
    void set_array(const std::string& k, const metkit::codes::Span<T>& v) {
        recorder->record_array(k, v.data(), v.size());
    }
};

} // namespace metkit::mars2grib::debug::reproducer
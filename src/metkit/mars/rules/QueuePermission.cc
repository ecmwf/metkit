
#include "metkit/mars/rules/QueuePermission.h"

#include "eckit/utils/Tokenizer.h"

using namespace eckit;


namespace metkit::mars::rules {

//----------------------------------------------------------------------------------------------------------------------

std::string QueuePermission::info(const MarsTaskProxy& t) const {
    Tokenizer parse("$");
    std::vector<std::string> bits;
    parse(info_, bits);

    std::ostringstream oss;
    for (size_t i = 0; i < bits.size(); ++i) {
        if (i % 2) {
            std::vector<std::string> v;
            t.getEnvironValues(bits[i], v);
            if (v.size()) {
                const char* sep = "";
                for (auto o : v) {
                    oss << sep << o;
                    sep = ", ";
                }
            }
            else {
                oss << "???";
            }
        }
        else {
            oss << bits[i];
        }
    }

    return oss.str();
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit::mars::rules


#include "metkit/mars/rules/AccessRule.h"

#include "eckit/value/Value.h"

using namespace eckit;


namespace metkit::mars::rules {

//----------------------------------------------------------------------------------------------------------------------

AccessRule::AccessRule(const std::string& name, MarsExpression* e, const eckit::Value& access, const std::string& url) :
        expr_(e), name_(name), url_(url) {
    ValueMap m(access);
    for (auto j = m.begin(); j != m.end(); ++j) {
        std::set<std::string>& s = access_[(*j).first];
        eckit::fromValue(s, (*j).second);
    }
}


void AccessRule::print(std::ostream& s) const {
    s << "access " << name_ << " " << *expr_ << " " << access_;
    if (url_.size()) {
        s << " [" << url_ << "]";
    }
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace metkit::mars::rules

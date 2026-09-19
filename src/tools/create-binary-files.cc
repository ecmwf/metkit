/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/runtime/Tool.h"

#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsRequest.h"

class ConfigBinaryFiles : public eckit::Tool {
public:

    ConfigBinaryFiles(int argc, char** argv) : eckit::Tool(argc, argv) {}

    void run() override {
        const std::vector<std::string> verbs{"retrieve"};  //, "archive",     "compute", "disseminate", "flush",
                                                           // "get",      "list",        "pointdb", "read", "write"};

        for (const auto& verb : verbs) {
            metkit::mars::MarsLanguage marsLanguage(verb);
        }

        metkit::mars::MarsRequest req{"retrieve"};
        req.setValue("param", "t");
        metkit::mars::MarsExpansion{true}.expand(req);
    }
};

int main(int argc, char** argv) {
    ConfigBinaryFiles app(argc, argv);
    return app.start();
}

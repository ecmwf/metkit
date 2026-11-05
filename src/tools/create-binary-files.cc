// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include "eckit/runtime/Tool.h"

#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsRequest.h"

class ConfigBinaryFiles : public eckit::Tool {
public:

    ConfigBinaryFiles(int argc, char** argv) : eckit::Tool(argc, argv) {}

    void run() override {
        const std::vector<std::string> verbs{"retrieve", "archive", "compute", "disseminate", "flush",
                                             "get",      "list",    "pointdb", "read",        "write"};

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

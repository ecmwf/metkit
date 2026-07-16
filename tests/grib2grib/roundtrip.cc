/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence cVersion 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <cstdio>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/log/Log.h"
#include "eckit/runtime/Main.h"
#include "eckit/runtime/Tool.h"

#include "eccodes/eccodes.h"

#include "metkit/grib2mars/api/Grib2Mars.h"
#include "metkit/mars2grib/api/Mars2Grib.h"


void get_vector_double(codes_handle* h, const std::string& k, std::vector<double>& v) {
    size_t size1 = 0;
    ASSERT(CODES_SUCCESS == codes_get_size(h, k.c_str(), &size1));

    v.resize(size1);
    size_t size2 = size1;

    ASSERT(CODES_SUCCESS == codes_get_double_array(h, k.c_str(), v.data(), &size2));
    ASSERT(size2 == size1);
}


std::string get_string(codes_handle* h, const std::string& k) {
    char buffer[1024];
    size_t size = sizeof(buffer);

    ASSERT(CODES_SUCCESS == codes_get_string(h, k.c_str(), buffer, &size));

    return buffer;
}


class Roundtrip : public eckit::Tool {
    using Tool::Tool;

    void run() override {
        const auto& tool = Main::instance();
        if (tool.argc() != 2 && tool.argc() != 3) {
            eckit::Log::info() << tool.name() << " file [grid]" << std::endl;
            exit(1);
        }

        std::string path = tool.argv(1);
        std::string grid = (tool.argc() == 3 ? tool.argv(2) : "");


        auto* f = std::fopen(path.c_str(), "rb");
        ASSERT(f != nullptr);

        int err = 0;
        auto* g = codes_handle_new_from_file(0, f, PRODUCT_GRIB, &err);
        ASSERT(g != nullptr && err == 0);

        std::vector<double> values;
        get_vector_double(g, "values", values);


        {
            metkit::grib2mars::Grib2Mars grib2mars;
            metkit::mars2grib::Mars2Grib mars2grib;

            auto ch = metkit::codes::codesHandleFromGRIBHandle(g);
            ASSERT(ch);

            auto [mars, misc] = grib2mars.convert<eckit::LocalConfiguration>(*ch);

            if (!grid.empty()) {
                mars.remove("area");
                mars.remove("rotation");
                mars.remove("grid");
                mars.remove("truncation");

                // until implementation uses 'grid'
                if (std::string gridType = get_string(g, "gridType"); gridType == "sh") {
                    long J = 0;
                    ASSERT(CODES_SUCCESS == codes_get_long(g, "J", &J));
                    mars.set("truncation", J);
                }
                else {
                    mars.set("grid", grid);
                }
            }

            const auto h = mars2grib.encode(values, mars, misc);
            ASSERT(h);

            auto isMessageValid = h->getLong("isMessageValid");
            eckit::Log::info() << "isMessageValid=" << isMessageValid << std::endl;
            eckit::Log::info() << "messageSize=" << h->messageSize() << std::endl;
        }

        codes_handle_delete(g);
        ASSERT(std::fclose(f) == 0);
    }
};


int main(int argc, char** argv) {
    return Roundtrip(argc, argv).start();
}

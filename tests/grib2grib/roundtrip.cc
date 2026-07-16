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
#include <cstring>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/geo/Grid.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
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


template <typename T>
using Option = eckit::option::SimpleOption<T>;


class Roundtrip : public eckit::Tool {
    using Tool::Tool;

    static void usage(const std::string& name) {
        eckit::Log::info() << name << " file [grid]" << std::endl;
        exit(1);
    }

    void run() override {

        std::vector<eckit::option::Option*> options;
        options.push_back(new Option<std::string>("grid", "MARS grid"));
        options.push_back(new Option<bool>("gridspec", "Set grid as gridSpec"));
        options.push_back(new Option<bool>("valid", "Check isMessageValid (deafult true)"));
        options.push_back(new Option<bool>("cmp", "Check message bytes (cmp-like) (deafult false)"));

        eckit::option::CmdArgs args(usage, options, 1, 1);
        ASSERT(args.count() == 1);

        auto* f = std::fopen(args(0).c_str(), "rb");
        ASSERT(f != nullptr);

        int err = 0;
        auto* g = codes_handle_new_from_file(0, f, PRODUCT_GRIB, &err);
        ASSERT(g != nullptr && err == 0);

        std::vector<double> values;
        get_vector_double(g, "values", values);


        {
            eckit::LocalConfiguration cfg;
            cfg.set("skipSection3", args.getBool("gridspec", false));

            metkit::grib2mars::Grib2Mars grib2mars;
            metkit::mars2grib::Mars2Grib mars2grib(cfg);

            auto ch = metkit::codes::codesHandleFromGRIBHandle(g);
            ASSERT(ch);

            auto [mars, misc] = grib2mars.convert<eckit::LocalConfiguration>(*ch);

            if (args.has("grid")) {
                mars.set("grid", args.getString("grid"));
                mars.remove("area");
                mars.remove("rotation");
                mars.remove("truncation");
            }
            else if (args.has("truncation")) {
                mars.set("truncation", args.getLong("truncation"));
                mars.remove("area");
                mars.remove("grid");
                mars.remove("rotation");
            }

            const auto h = mars2grib.encode(values, mars, misc);
            ASSERT(h);

            if (args.getBool("valid", true)) {
                ASSERT(h->getLong("isMessageValid") != 0);
            }

            if (args.getBool("cmp", true)) {
                size_t size = 0;
                ASSERT(CODES_SUCCESS == codes_get_message_size(g, &size));
                ASSERT(size == h->messageSize());

                const void* input_message = nullptr;
                ASSERT(CODES_SUCCESS == codes_get_message(g, &input_message, &size));

                auto encoded = h->messageData();
                ASSERT(encoded.size() == size);
                ASSERT(std::memcmp(input_message, encoded.data(), size) == 0);
            }
        }

        codes_handle_delete(g);
        ASSERT(std::fclose(f) == 0);
    }
};


int main(int argc, char** argv) {
    return Roundtrip(argc, argv).start();
}

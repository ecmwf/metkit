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
#include <ios>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/runtime/Main.h"
#include "eckit/runtime/Tool.h"

#include "eccodes.h"
#include "metkit/grib2mars/api/Grib2Mars.h"
#include "metkit/mars2grib/api/Mars2Grib.h"


// Note: not in eccodes.h
extern "C" int codes_compare_key(codes_handle*, codes_handle*, const char* key, int compare_flags);


void get_vector_double(codes_handle* h, const std::string& k, std::vector<double>& v) {
    size_t size1 = 0;
    ASSERT(CODES_SUCCESS == codes_get_size(h, k.c_str(), &size1));

    v.resize(size1);
    size_t size2 = size1;

    ASSERT(CODES_SUCCESS == codes_get_double_array(h, k.c_str(), v.data(), &size2));
    ASSERT(size2 == size1);
}


auto print_key_value = [](const codes_handle* h, const char* key, eckit::Channel& out) {
    int type = CODES_TYPE_UNDEFINED;
    ASSERT(codes_get_native_type(h, key, &type) == CODES_SUCCESS);

    out << "'" << key << "': ";
    if (type == CODES_TYPE_UNDEFINED) {
        out << "undefined" << std::endl;
    }
    else if (type == CODES_TYPE_LONG) {
        long value = 0;
        codes_get_long(h, key, &value);
        if (value == CODES_MISSING_LONG) {
            out << "MISSING" << std::endl;
        }
        else {
            out << "long=" << value << std::endl;
        }
    }
    else if (type == CODES_TYPE_DOUBLE) {
        double value = 0;
        codes_get_double(h, key, &value);
        if (value == CODES_MISSING_DOUBLE) {
            out << "MISSING" << std::endl;
        }
        else {
            out << "double=" << value << std::endl;
        }
    }
    else if (type == CODES_TYPE_STRING) {
        std::string str(512, '\0');
        size_t size = str.size();
        codes_get_string(h, key, str.data(), &size);
        str.resize(std::strlen(str.c_str()));  // 'size' may include the trailing '\0'
        out << "string='" << str << "'" << std::endl;
    }
    else if (type == CODES_TYPE_BYTES) {
        std::vector<unsigned char> bytes(512, '\0');
        size_t size = bytes.size();
        codes_get_bytes(h, key, bytes.data(), &size);
        bytes.resize(size);
        out << std::hex;
        out << "bytes=[";
        for (size_t i = 0; i < size; ++i) {
            out << bytes[i];
        }
        out << std::dec << "]" << std::endl;
    }
    else if (type == CODES_TYPE_SECTION) {
        out << "section" << std::endl;
    }
    else if (type == CODES_TYPE_LABEL) {
        out << "label" << std::endl;
    }
    else if (type == CODES_TYPE_MISSING) {
        out << "missing" << std::endl;
    }
};


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
        options.push_back(new Option<bool>("valid", "Check isMessageValid (deafult true)"));
        options.push_back(new Option<bool>("cmp", "Check message bytes (cmp-like) (deafult false)"));
        options.push_back(new Option<bool>("keys", "Check grib key values (grib_compare-like, in-memory) (deafult false)"));

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
            cfg.set("skipSection3", true);

            metkit::grib2mars::Grib2Mars grib2mars(cfg);
            metkit::mars2grib::Mars2Grib mars2grib(cfg);

            auto ch = metkit::codes::codesHandleFromGRIBHandle(g);
            ASSERT(ch);

            auto [mars, misc] = grib2mars.convert<eckit::LocalConfiguration>(*ch);

            if (args.has("grid")) {
                mars.remove("area");
                mars.remove("rotation");
                mars.set("grid", args.getString("grid"));
            }

            const auto h = mars2grib.encode(values, mars, misc);
            ASSERT(h);

            if (args.getBool("valid", true)) {
                ASSERT(h->getLong("isMessageValid") != 0);
            }

            if (args.getBool("cmp", false)) {
                size_t size = 0;
                ASSERT(CODES_SUCCESS == codes_get_message_size(g, &size));

                eckit::Log::info() << "Original message size: " << size
                                   << ", encoded message size: " << h->messageSize() << std::endl;
                ASSERT(size == h->messageSize());

                const void* input_message = nullptr;
                ASSERT(CODES_SUCCESS == codes_get_message(g, &input_message, &size));

                auto encoded = h->messageData();
                ASSERT(encoded.size() == size);
                ASSERT(std::memcmp(input_message, encoded.data(), size) == 0);
            }

            if (args.getBool("keys", false)) {
                auto& out = eckit::Log::error();

                // release() is the last use of h, so ownership transfer is safe
                auto* h2    = reinterpret_cast<codes_handle*>(h->release());
                auto* kiter = codes_keys_iterator_new(g, CODES_KEYS_ITERATOR_SKIP_READ_ONLY, nullptr);

                auto diff = false;
                while (codes_keys_iterator_next(kiter) != 0) {
                    const auto* key = codes_keys_iterator_get_name(kiter);
                    auto err        = codes_compare_key(g, h2, key, 0);
                    if (err != CODES_SUCCESS && err != CODES_NOT_IMPLEMENTED /*key doesn't support comparison*/) {
                        out << "Key differs: '" << key << "' (" << codes_get_error_message(err) << ")" << std::endl;
                        print_key_value(g, key, out);
                        print_key_value(h2, key, out);
                        diff = true;
                    }
                }

                codes_keys_iterator_delete(kiter);
                codes_handle_delete(h2);

                ASSERT(!diff);
            }
        }

        codes_handle_delete(g);
        ASSERT(std::fclose(f) == 0);
    }
};


int main(int argc, char** argv) {
    return Roundtrip(argc, argv).start();
}

/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eccodes.h"

#include "eckit/config/Resource.h"
#include "eckit/io/Buffer.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/Option.h"
#include "eckit/runtime/Tool.h"

#include "metkit/grib/MetFile.h"

using namespace metkit;





//----------------------------------------------------------------------------------------------------------------------

class Bufr2Request;
static Bufr2Request* instance_ = nullptr;

class Bufr2Request : public eckit::Tool {
public:
    virtual void usage(const std::string& tool) const {
        eckit::Log::info() << std::endl
                           << "Usage: " << tool << " <path1> [path2] [...]" << std::endl;
    }

    virtual int numberOfPositionalArguments() const { return -1; }
    virtual int minimumPositionalArguments() const { return 1; }

    virtual void run();

    Bufr2Request(int argc, char** argv) : eckit::Tool(argc, argv) {
        ASSERT(instance_ == nullptr);
        instance_ = this;
    }

protected:  // members
    std::vector<eckit::option::Option*> options_;

    size_t nMsg_ = 0;

private:
    void list_all_kvs(codes_handle* h);
    void list_namespace_kvs(codes_handle* h);
};

static void usage(const std::string& tool) {
    ASSERT(instance_);
    instance_->usage(tool);
}

#define MAX_VAL_LEN 1024


void Bufr2Request::list_all_kvs(codes_handle* h) {
    // CODES_CHECK(codes_set_long(h, "unpack", 1), nullptr); // tell ecCodes to unpack the data
    // values

    codes_bufr_keys_iterator* kiter = codes_bufr_keys_iterator_new(h, CODES_KEYS_ITERATOR_ALL_KEYS);
    ASSERT(kiter);

    while (codes_bufr_keys_iterator_next(kiter)) {
        char* name = codes_bufr_keys_iterator_get_name(kiter);

        int keyType = 0;
        CODES_CHECK(codes_get_native_type(h, name, &keyType), nullptr);

        size_t klen = 0;
        CODES_CHECK(codes_get_size(h, name, &klen), nullptr);

        if (klen == 1) /* not array */
        {
            size_t vlen             = MAX_VAL_LEN;
            char value[MAX_VAL_LEN] = {0};
            codes_get_string(h, name, value, &vlen);
            eckit::Log::info() << name << " : " << value << std::endl;
        }
        else { /* for arrays */
            eckit::Log::info() << name << " : array(" << klen << ")" << std::endl;
        }
    }

    codes_bufr_keys_iterator_delete(kiter);
}

void Bufr2Request::list_namespace_kvs(codes_handle* h) {
    char value[128]                  = {0};
    static std::string BufrNamespace = eckit::Resource<std::string>("BufrNamespace", "mars");

    codes_keys_iterator* ks =
        codes_keys_iterator_new(h, CODES_KEYS_ITERATOR_ALL_KEYS, BufrNamespace.c_str());
    ASSERT(ks);

    while (codes_keys_iterator_next(ks)) {
        const char* name = codes_keys_iterator_get_name(ks);

        if (name[0] == '_') {
            continue;
        }

        size_t len = sizeof(value);
        ASSERT(codes_keys_iterator_get_string(ks, value, &len) == 0);
        eckit::Log::info() << name << " : " << value << std::endl;
    }
}

void Bufr2Request::run() {
    eckit::option::CmdArgs args(&::usage, options_, numberOfPositionalArguments(),
                                minimumPositionalArguments());

    static size_t bufferSize = eckit::Resource<size_t>("BufferSize", 64 * 1024 * 1024);
    eckit::Buffer buffer(bufferSize);
    long len = 0;

    for (size_t i = 0; i < args.count(); i++) {
        eckit::PathName path(args(i));
        std::cout << "Processing " << path << std::endl;

        grib::MetFile file(path);

        while ((len = file.readSome(buffer)) != 0) {

            // BufrHandle h (buffer);

            codes_handle* h = codes_handle_new_from_message(nullptr, buffer, buffer.size());

            nMsg_++;

            if (!h) {
                std::ostringstream oss;
                oss << "Error: unable to create handle for message " << nMsg_;
                throw eckit::BadValue(oss.str(), Here());
            }

            //            list_all_kvs(h);
            list_namespace_kvs(h);

            codes_handle_delete(h);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    Bufr2Request tool(argc, argv);
    return tool.start();
}

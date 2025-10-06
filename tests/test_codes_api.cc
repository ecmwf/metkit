/*
 * (C) Copyright 2025- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


// Include operator<< via LocalConfiguration
#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"

#include "metkit/codes/api/CodesAPI.h"

#include "eccodes.h"
#include "metkit/codes/api/CodesTypes.h"
#include "metkit/codes/api/KeyIterator.h"

namespace metkit::grib::test {

//-----------------------------------------------------------------------------

CASE("Print version and paths") {
    std::cout << codes::info() << std::endl;
    std::cout << "samplesPath: " << codes::samplesPath() << std::endl;
    std::cout << "definitionPath: " << codes::definitionPath() << std::endl;

    // TODO(pgeier) This is probably not safe to execute in the CI
    // EXPECT_EQUAL(codes::samplesPath(), "/MEMFS/samples");
    // EXPECT_EQUAL(codes::definitionPath(), "/MEMFS/definitions");
}

// No explicit expectations here
// However, we iterate the whole sample and print all keys
// Keys are fetched by inspecting the native type and calling a specialized getXXX
// A non-throwing behaviour is at least
CASE("Test iterate sample, getting all keys by native type on iterator") {
    using namespace codes;

    auto handle = newFromSample("GRIB2");


    for (auto& k : handle->keys()) {
        std::cout << "\t" << k.name() << ": ";
        auto valFromIt     = k.get();
        auto valFromHandle = handle->get(k.name());
        std::visit([](const auto& v) { std::cout << v << std::endl; }, valFromIt);

        if (k.name() != "sectionNumber" && k.name() != "numberOfSection") {
            EXPECT_EQUAL(valFromIt.index(), valFromHandle.index());
            std::visit(
                [&](const auto& v) {
                    std::cout << "\t(from handle): " << v << std::endl;
                    EXPECT_EQUAL(v, std::get<std::decay_t<decltype(v)>>(valFromHandle));
                },
                valFromIt);
        }
    }
}


CASE("Test iterate and rewrite keys") {
    using namespace codes;

    auto handle = newFromSample("GRIB2");

    using IF = KeyIteratorFlags;

    std::vector<std::pair<std::string, codes::Value>> values;

    for (auto& k : handle->keys(IF::SkipReadOnly)) {
        values.emplace_back(k.name(), k.get());
    }

    EXPECT(values.size() > 0);

    for (const auto& p : std::move(values)) {
        // TODO(pgeier) Raise eccodes bug about not readonly keys
        if (p.first == "validityDateTime")
            continue;
        if (p.first == "productType")
            continue;
        if (p.first == "isTemplateDeprecated")
            continue;
        if (p.first == "isTemplateExperimental")
            continue;
        if (p.first == "datasetForLocal")
            continue;
        if (p.first == "isMessageValid")
            continue;


        std::cout << "\tsetting " << p.first << ": ";
        std::visit(
            [&](const auto& val) {
                std::cout << val << std::endl;
                handle->set(p.first, val);
            },
            p.second);
    };
}

CASE("Test geo iterator") {
    using namespace codes;

    auto handle = newFromSample("GRIB2");

    long numberValues = handle->getLong("numberOfValues");

    auto lons   = handle->getDoubleArray("longitudes");
    auto lats   = handle->getDoubleArray("latitudes");
    auto values = handle->getDoubleArray("values");

    EXPECT_EQUAL(lons.size(), numberValues);
    EXPECT_EQUAL(lats.size(), numberValues);
    EXPECT_EQUAL(values.size(), numberValues);

    long count = 0;
    // TODO(pgeier) Use structured binding with C++20;  for (const auto& {longitude, latitude, value}: handle->values())
    // {}
    for (const auto& data : handle->values()) {
        // std::cout << "\t" << count << ": " << data.longitude << "/" << data.latitude << ": " << data.value <<
        // std::endl;

        EXPECT_EQUAL(lons[count], data.longitude);
        EXPECT_EQUAL(lats[count], data.latitude);
        EXPECT_EQUAL(values[count], data.value);
        ++count;
    }

    EXPECT_EQUAL(count, numberValues);
}

CASE("Test setting values") {
    using namespace codes;

    auto handle = newFromSample("GRIB2");

    long numberValues = handle->getLong("numberOfValues");

    std::vector<double> newVals;
    for (int i = 0; i < numberValues; ++i) {
        newVals.push_back((double)i);
    }

    EXPECT_NO_THROW(handle->set("values", newVals));

    auto values = handle->getDoubleArray("values");

    EXPECT_EQUAL(newVals, values);
    long count = 0;
    for (const auto& data : handle->values()) {
        EXPECT_EQUAL(newVals[count], data.value);
        ++count;
    }
    EXPECT_EQUAL(count, numberValues);
}

CASE("Test load and iterate mars keys") {
    using namespace codes;

    auto handle = newFromSample("GRIB2");

    handle->set("date", 20250101);
    handle->set("time", 1400);
    handle->set("step", (long)18);
    handle->set("param", (long)132);

    for (auto& k : handle->keys(namespaces::mars)) {
        if (k.name() == "date") {
            EXPECT_EQUAL(std::get<long>(k.get()), 20250101);
        }
        if (k.name() == "time") {
            EXPECT_EQUAL(std::get<long>(k.get()), 1400);
        }
        if (k.name() == "step") {
            EXPECT_EQUAL(std::get<long>(k.get()), 18);
        }
        if (k.name() == "levtype") {
            EXPECT_EQUAL(std::get<std::string>(k.get()), "sfc");
        }
        if (k.name() == "param") {
            EXPECT_EQUAL(std::get<long>(k.get()), 132);
        }
    }
}

CASE("Test isDefined, has, isMissing, set MARS key \"class\"") {
    using namespace codes;

    auto handle = newFromSample("GRIB2");

    EXPECT(!handle->isDefined("class"));
    EXPECT(!handle->has("class"));
    EXPECT(!handle->isDefined("anoffset"));
    EXPECT(!handle->has("anoffset"));

    // Set a local definition template with mars keys
    EXPECT_NO_THROW(handle->set("setLocalDefinition", 1));
    EXPECT_NO_THROW(handle->set("localDefinitionNumber", 15));

    // Mars directly get a "default" value instead of being set to missing
    EXPECT(handle->isDefined("class"));
    EXPECT(!handle->isMissing("class"));
    EXPECT(handle->has("class"));

    EXPECT_NO_THROW(handle->set("class", "od"));

    EXPECT_EQUAL(handle->getString("class"), std::string("od"));


    EXPECT(!handle->isMissing("class"));
    EXPECT(handle->has("class"));


    // Mars key can not set to be missing
    EXPECT_THROWS(handle->setMissing("class"));
}


CASE("Test set missing") {
    using namespace codes;

    auto handle = newFromSample("GRIB2");

    EXPECT_NO_THROW(handle->set("productDefinitionTemplateNumber", 0));

    EXPECT(handle->isDefined("scaledValueOfFirstFixedSurface"));
    EXPECT(handle->isMissing("scaledValueOfFirstFixedSurface"));
    EXPECT(!handle->has("scaledValueOfFirstFixedSurface"));

    EXPECT_NO_THROW(handle->set("scaledValueOfFirstFixedSurface", 123));
    EXPECT_EQUAL(handle->getLong("scaledValueOfFirstFixedSurface"), 123);

    EXPECT(!handle->isMissing("scaledValueOfFirstFixedSurface"));
    EXPECT(handle->has("scaledValueOfFirstFixedSurface"));

    EXPECT_NO_THROW(handle->setMissing("scaledValueOfFirstFixedSurface"));
    EXPECT(handle->isMissing("scaledValueOfFirstFixedSurface"));
    EXPECT(!handle->has("scaledValueOfFirstFixedSurface"));
}


CASE("Test copyInto and clone") {
    using namespace codes;

    auto handle = newFromSample("GRIB2");

    size_t size = handle->messageSize();
    auto bytes  = std::unique_ptr<std::uint8_t>(new std::uint8_t[size]);
    EXPECT_NO_THROW(handle->copyInto(bytes.get(), size));

    auto handle2 = handle->clone();
    EXPECT_EQUAL(handle2->messageSize(), size);

    size_t size2 = handle->messageSize();
    auto bytes2  = std::unique_ptr<std::uint8_t>(new std::uint8_t[size2]);
    EXPECT_NO_THROW(handle->copyInto(bytes2.get(), size2));

    EXPECT_EQUAL(size, size2);
    for (int i = 0; i < size; ++i) {
        EXPECT_EQUAL(bytes.get()[i], bytes2.get()[i]);
    }
}


CASE("Test copyInto and newFromMessage") {
    using namespace codes;

    auto handle = newFromSample("GRIB2");

    size_t size = handle->messageSize();
    auto bytes  = std::unique_ptr<std::uint8_t>(new std::uint8_t[size]);
    EXPECT_NO_THROW(handle->copyInto(bytes.get(), size));


    auto handle2 = newFromMessage({bytes.get(), size});
    EXPECT_EQUAL(handle2->messageSize(), size);

    size_t size2 = handle2->messageSize();
    auto bytes2  = std::unique_ptr<std::uint8_t>(new std::uint8_t[size2]);
    EXPECT_NO_THROW(handle->copyInto(bytes2.get(), size2));

    EXPECT_EQUAL(size, size2);
    for (int i = 0; i < size; ++i) {
        EXPECT_EQUAL(bytes.get()[i], bytes2.get()[i]);
    }
}

CASE("Test copyInto and newFromMessageCopy") {
    using namespace codes;

    auto handle = newFromSample("GRIB2");

    size_t size = handle->messageSize();
    auto bytes  = std::unique_ptr<std::uint8_t>(new std::uint8_t[size]);
    EXPECT_NO_THROW(handle->copyInto(bytes.get(), size));


    auto handle2 = newFromMessageCopy({bytes.get(), size});
    EXPECT_EQUAL(handle2->messageSize(), size);

    size_t size2 = handle2->messageSize();
    auto bytes2  = std::unique_ptr<std::uint8_t>(new std::uint8_t[size2]);
    EXPECT_NO_THROW(handle->copyInto(bytes2.get(), size2));

    EXPECT_EQUAL(size, size2);
    for (int i = 0; i < size; ++i) {
        EXPECT_EQUAL(bytes.get()[i], bytes2.get()[i]);
    }
}
}  // namespace metkit::grib::test


namespace std {
template <>
struct default_delete<codes_handle> {
    void operator()(codes_handle* h) { ::codes_handle_delete(h); }
};
}  // namespace std

namespace metkit::grib::test {

CASE("Test release handle") {
    using namespace codes;

    auto handle = newFromSample("GRIB2");

    std::unique_ptr<codes_handle> raw =
        std::unique_ptr<codes_handle>(reinterpret_cast<codes_handle*>(handle->release()));

    EXPECT(raw);
}

//-----------------------------------------------------------------------------

}  // namespace metkit::grib::test


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}

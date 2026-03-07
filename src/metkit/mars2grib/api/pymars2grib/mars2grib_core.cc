/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>
#include <cstdint>
#include <memory>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/runtime/Main.h"

#include "metkit/mars2grib/api/Mars2Grib.h"

namespace py = pybind11;

using metkit::mars2grib::Mars2Grib;

static eckit::LocalConfiguration dictToLocalConfig(const py::dict& dict) {
    eckit::LocalConfiguration config;
    for (auto item : dict) {
        const auto key   = item.first.cast<std::string>();
        const auto value = item.second;
        if (py::isinstance<py::str>(value)) {
            config.set(key, value.cast<std::string>());
        }
        else if (py::isinstance<py::bool_>(value)) {
            config.set(key, value.cast<bool>());
        }
        else if (py::isinstance<py::int_>(value)) {
            config.set(key, value.cast<long>());
        }
        else if (py::isinstance<py::float_>(value)) {
            config.set(key, value.cast<double>());
        }
        else if (py::isinstance<py::dict>(value)) {
            config.set(key, dictToLocalConfig(value.cast<py::dict>()));
        }
        else if (py::isinstance<py::list>(value)) {
            const py::list list = value.cast<py::list>();
            if (list.empty()) {
                config.set(key, std::vector<long>{});
            }
            else {
                bool hasFloat = false;
                for (const auto& item : list) {
                    if (py::isinstance<py::float_>(item)) {
                        hasFloat = true;
                    }
                    else if (!py::isinstance<py::int_>(item)) {
                        throw eckit::Exception{"Unsupported type in list for key '" + key + "'", Here()};
                    }
                    // else it's an int, just fall through...
                }
                if (hasFloat) {
                    std::vector<double> vector;
                    vector.reserve(list.size());
                    for (const auto& item : list) {
                        vector.push_back(item.cast<double>());
                    }
                    config.set(key, vector);
                }
                else {
                    std::vector<long> vector;
                    vector.reserve(list.size());
                    for (const auto& item : list) {
                        vector.push_back(item.cast<long>());
                    }
                    config.set(key, vector);
                }
            }
        }
        else {
            throw eckit::Exception{"Unsupported type for key '" + key + "'", Here()};
        }
    }
    return config;
}

py::bytes encode(Mars2Grib& encoder, const std::vector<double>& values, const py::dict& mars, const py::dict& misc) {
    const auto message = encoder.encode(values, dictToLocalConfig(mars), dictToLocalConfig(misc));

    const auto size = message->messageSize();

    std::vector<uint8_t> buffer(message->messageSize());
    message->copyInto(buffer.data(), buffer.size());

    return py::bytes(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

PYBIND11_MODULE(mars2grib_core, m) {
    m.def("init_bindings", []() {
        const char* args[] = {"mars2grib", ""};
        eckit::Main::initialise(1, const_cast<char**>(args));
    });
    auto mars2grib =
        py::class_<Mars2Grib>(m, "Mars2GribCore")
            .def(py::init<>())
            .def(py::init([](py::dict dict) { return std::make_unique<Mars2Grib>(dictToLocalConfig(dict)); }))
            .def("encode", &encode, py::arg("values"), py::arg("mars"), py::arg("misc"));
}

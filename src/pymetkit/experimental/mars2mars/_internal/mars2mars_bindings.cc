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
#include <memory>
#include <sstream>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/runtime/Main.h"
#include "eckit/system/Library.h"
#include "eckit/system/LibraryManager.h"

#include "metkit/mars/MarsRequest.h"
#include "metkit/mars2mars/api/Mars2Mars.h"
#include "metkit_version.h"

namespace py = pybind11;

using metkit::mars2mars::Mars2Mars;

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

std::map<std::string, std::vector<std::string>> to_dict(const metkit::mars::MarsRequest& mars_request) {
    std::map<std::string, std::vector<std::string>> res;

    const auto& parameters = mars_request.parameters();

    for (const auto& parameter : parameters) {
        res.emplace(parameter.name(), parameter.values());
    }

    return res;
}

std::map<std::string, std::vector<std::string>> to_dict(const eckit::LocalConfiguration& local_config) {
    std::map<std::string, std::vector<std::string>> res;

    const auto& keys = local_config.keys();

    for (const auto& key : keys) {
        const bool convertible = local_config.isConvertible<std::string>(key);

        // if (!convertible) {
        //     std::ostringstream buf;
        //     buf << "Element in local configuration not convertible to string: " << key;
        //     throw eckit::UserError(buf.str());
        // }

        std::string value{};
        local_config.get(key, value);
        res.emplace(std::pair<std::string, std::vector<std::string>>(key, {value}));
    }

    return res;
}


PYBIND11_MODULE(mars2mars_bindings, m) {
    py::module_::import("pymetkit_bindings");

    m.def("init_bindings", []() {
        const char* args[] = {"mars2mars", ""};
        eckit::Main::initialise(1, const_cast<char**>(args));
    });

    m.def("version_info", []() {
        std::vector<std::tuple<std::string, std::string, std::string, std::string>> dependencyInformation;

        for (const std::string& libname : eckit::system::LibraryManager::list()) {
            const eckit::system::Library& lib = eckit::system::LibraryManager::lookup(libname);
            dependencyInformation.emplace_back(lib.name(), lib.version(), lib.gitsha1(), lib.libraryPath());
        }

        return dependencyInformation;
    });

    // Compile-time mars2mars version
    m.attr("__mars2mars_build_version__") = metkit_VERSION_STR;

    auto mars2mars = py::class_<metkit::mars2mars::Mars2Mars>(m, "Mars2Mars")
                         .def(py::init<>())
                         .def(py::init([](py::dict dict) {
                             return std::make_unique<metkit::mars2mars::Mars2Mars>(dictToLocalConfig(dict));
                         }))
                         .def("convert",
                              [](Mars2Mars& mars2mars, const eckit::LocalConfiguration& configuration) {
                                  const auto& result = mars2mars.convert<eckit::LocalConfiguration>(configuration);
                                  return std::make_pair(to_dict(result.mars), to_dict(result.misc));
                              })
                         .def("convert", [](Mars2Mars& mars2mars, const metkit::mars::MarsRequest& mars_request) {
                             const metkit::mars2mars::Mars2MarsResult<metkit::mars::MarsRequest> result =
                                 mars2mars.convert<metkit::mars::MarsRequest>(mars_request);

                             return std::pair(to_dict(result.mars), to_dict(result.misc));
                         });
}

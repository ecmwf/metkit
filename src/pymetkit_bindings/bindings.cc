/*
 * (C) Copyright 2025- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include "eckit/runtime/Main.h"
#include "eckit/system/Library.h"
#include "eckit/system/LibraryManager.h"

#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit_version.h"

namespace py   = pybind11;
namespace mars = metkit::mars;

metkit::mars::MarsRequest mars_request_from_map(const std::string& verb,
                                                const std::map<std::string, std::vector<std::string>>& map) {
    eckit::ValueMap value_map;

    for (const auto& pair : map) {
        eckit::ValueList value_list;
        for (const auto& value : pair.second) {
            value_list.emplace_back(value);
        }
        value_map.emplace(eckit::Value(pair.first), value_list);
    }

    return metkit::mars::MarsRequest(verb, value_map);
}

PYBIND11_MODULE(pymetkit_bindings, m) {

    m.def("init_bindings", []() {
        char arg0[]  = "pymetkit";
        char* argv[] = {arg0, nullptr};
        eckit::Main::initialise(1, argv);
    });

    m.def("version_info", []() {
        std::vector<std::tuple<std::string, std::string, std::string, std::string>> dependencyInformation;

        for (const std::string& libname : eckit::system::LibraryManager::list()) {
            const eckit::system::Library& lib = eckit::system::LibraryManager::lookup(libname);
            dependencyInformation.emplace_back(lib.name(), lib.version(), lib.gitsha1(), lib.libraryPath());
        }

        return dependencyInformation;
    });

    // Compile-time metkit version
    m.attr("__pymetkit_build_version__") = metkit_VERSION_STR;


    //--------------------------------------------------
    // @brief MarsRequest
    //--------------------------------------------------

    py::class_<mars::MarsRequest>(m, "MarsRequest")
        .def(py::init())
        .def(py::init([](const std::string& verb) {
            mars::MarsRequest request;
            request.verb(verb);
            return request;
        }))
        .def(py::init([](const std::string& verb, const std::map<std::string, std::vector<std::string>>& values) {
            return mars_request_from_map(verb, values);
        }))
        .def("verb", [](const mars::MarsRequest& request) { return request.verb(); })
        .def("set_verb", [](mars::MarsRequest& request, const std::string& verb) { request.verb(verb); })
        .def("set", [](mars::MarsRequest& request, const std::string& param,
                       const std::vector<std::string>& values) { request.values(param, values); })
        .def("has", [](const mars::MarsRequest& request, const std::string& param) { return request.has(param); })
        .def("params", [](const mars::MarsRequest& request) { return request.params(); })
        .def("values",
             [](const mars::MarsRequest& request, const std::string& param) {
                 const std::vector<std::string>& values = request.values(param, false);
                 return std::vector<std::string>{values.begin(), values.end()};
             })
        .def("merge", [](mars::MarsRequest& request, const mars::MarsRequest& other) { request.merge(other); })
        .def("expand",
             [](const mars::MarsRequest& request, bool inherit, bool strict) {
                 mars::MarsExpansion expansion(inherit, strict);
                 return expansion.expand(request);
             })
        .def("split",
             [](const mars::MarsRequest& request, const std::vector<std::string>& keys) { return request.split(keys); })
        .def("__repr__", [](const mars::MarsRequest& request) { return request.asString(); });

    //--------------------------------------------------
    // @brief Parsing
    //--------------------------------------------------

    m.def("parse_marsrequests", [](const std::string& str, bool strict) {
        std::istringstream in(str);
        return mars::MarsRequest::parse(in, strict);
    });

    m.def("parse_marsrequest",
          [](const std::string& str, bool strict) { return mars::MarsRequest::parse(str, strict); });
}

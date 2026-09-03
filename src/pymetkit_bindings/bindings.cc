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

#include "eckit/utils/MD5.h"
#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit_version.h"

namespace py   = pybind11;
namespace mars = metkit::mars;


PYBIND11_MODULE(pymetkit_bindings, m) {

    // Internal functions
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
    m.attr("__metkit_build_version__") = metkit_VERSION_STR;

    // @brief MarsRequest
    py::class_<mars::MarsRequest>(m, "MarsRequest")
        .def(py::init())
        .def(py::init([](const std::string& verb) {
            mars::MarsRequest request;
            request.verb(verb);
            return request;
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
        .def(
            "expand",
            [](const mars::MarsRequest& request, bool inherit, bool strict) {
                mars::MarsExpansion expansion(inherit, strict);
                return expansion.expand(request);
            },
            py::call_guard<py::gil_scoped_release>())
        .def("md5",
             [](const mars::MarsRequest& request) {
                 eckit::MD5 md5;
                 request.md5(md5);
                 return md5.digest();
             })
        .def("__repr__", [](const mars::MarsRequest& request) { return request.asString(); });

    // @brief Bulk expansion
    //
    // One MarsExpansion serves the whole batch, so the per-verb MarsLanguage is
    // built once instead of once per request.
    m.def(
        "expand_marsrequests",
        [](const std::vector<mars::MarsRequest>& requests, bool inherit, bool strict) {
            mars::MarsExpansion expansion(inherit, strict);
            return expansion.expand(requests);
        },
        py::call_guard<py::gil_scoped_release>());

    // @brief Parsing
    m.def(
        "parse_marsrequests",
        [](const std::string& str, bool strict) {
            std::istringstream in(str);
            return mars::MarsRequest::parse(in, strict);
        },
        py::call_guard<py::gil_scoped_release>());

    m.def(
        "parse_marsrequest", [](const std::string& str, bool strict) { return mars::MarsRequest::parse(str, strict); },
        py::call_guard<py::gil_scoped_release>());
}

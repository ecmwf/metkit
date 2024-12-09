#include "metkit_c.h"
#include "metkit/mars/MarsExpension.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/metkit_version.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/runtime/Main.h"
#include "eckit/utils/Optional.h"

#include <functional>

extern "C" {

// ---------------------------------------------------------------------------------------------------------------------

struct metkit_request_t : public metkit::mars::MarsRequest {
    using metkit::mars::MarsRequest::MarsRequest;
    metkit_request_t(const metkit::mars::MarsRequest& k) :
        metkit::mars::MarsRequest(k) {}
};

struct metkit_requestiterator_t {
    metkit_requestiterator_t(std::vector<metkit::mars::MarsRequest> vec) :
        first(true), vector(std::move(vec)), iterator(vector.begin()) {}

    int next() {
        if (first) {
            first = false;
        }
        else {
            ++iterator;
        }
        if (iterator == vector.end()) {
            return METKIT_ITERATION_COMPLETE;
        }
        return METKIT_SUCCESS;
    }

    bool first;
    std::vector<metkit::mars::MarsRequest> vector;
    std::vector<metkit::mars::MarsRequest>::const_iterator iterator;
};

struct metkit_paramiterator_t {
    metkit_paramiterator_t(std::vector<std::string> vec) :
        first(true), vector(std::move(vec)), iterator(vector.begin()) {}

    int next() {
        if (first) {
            first = false;
        }
        else {
            ++iterator;
        }
        if (iterator == vector.end()) {
            return METKIT_ITERATION_COMPLETE;
        }
        return METKIT_SUCCESS;
    }

    bool first;
    std::vector<std::string> vector;
    std::vector<std::string>::const_iterator iterator;
};

// ---------------------------------------------------------------------------------------------------------------------
//                           ERROR HANDLING

}  // extern "C"

static thread_local std::string g_current_error_string;

const char* metkit_get_error_string(int) {
    return g_current_error_string.c_str();
}

int innerWrapFn(std::function<int()> f) {
    return f();
}

int innerWrapFn(std::function<void()> f) {
    f();
    return METKIT_SUCCESS;
}

template <typename FN>
[[nodiscard]] int tryCatch(FN&& fn) {
    try {
        return innerWrapFn(fn);
    }
    catch (const eckit::UserError& e) {
        eckit::Log::error() << "User Error: " << e.what() << std::endl;
        g_current_error_string = e.what();
        return METKIT_ERROR_USER;
    }
    catch (const eckit::AssertionFailed& e) {
        eckit::Log::error() << "Assertion Failed: " << e.what() << std::endl;
        g_current_error_string = e.what();
        return METKIT_ERROR_ASSERT;
    }
    catch (const eckit::Exception& e) {
        eckit::Log::error() << "METKIT Error: " << e.what() << std::endl;
        g_current_error_string = e.what();
        return METKIT_ERROR;
    }
    catch (const std::exception& e) {
        eckit::Log::error() << "Unknown Error: " << e.what() << std::endl;
        g_current_error_string = e.what();
        return METKIT_ERROR_UNKNOWN;
    }
    catch (...) {
        eckit::Log::error() << "Unknown Error!" << std::endl;
        g_current_error_string = "<unknown>";
        return METKIT_ERROR_UNKNOWN;
    }
}

extern "C" {

// -----------------------------------------------------------------------------
//                           HELPERS
// -----------------------------------------------------------------------------

int metkit_version(const char** version) {
    *version = metkit_version_str();
    return METKIT_SUCCESS;
}

int metkit_vcs_version(const char** sha1) {
    *sha1 = metkit_git_sha1();
    return METKIT_SUCCESS;
}

int metkit_initialise() {
    return tryCatch([] {
        static bool initialised = false;

        if (initialised) {
            eckit::Log::warning()
                << "Initialising Metkit library twice" << std::endl;
        }

        if (!initialised) {
            const char* argv[2] = {"metkit-api", 0};
            eckit::Main::initialise(1, const_cast<char**>(argv));
            initialised = true;
        }
    });
}

// -----------------------------------------------------------------------------
//                           PARSING
// -----------------------------------------------------------------------------

int metkit_parse_mars_request(const char* str, metkit_requestiterator_t** requests, bool strict) {
    return tryCatch([requests, str, strict] {
        ASSERT(requests);
        ASSERT(str);
        std::istringstream in(str);
        *requests = new metkit_requestiterator_t(metkit::mars::MarsRequest::parse(in, strict));
    });
}

// -----------------------------------------------------------------------------
//                           REQUEST
// -----------------------------------------------------------------------------

int metkit_new_request(metkit_request_t** request) {
    return tryCatch([request] {
        ASSERT(request);
        *request = new metkit_request_t();
    });
}

int metkit_free_request(const metkit_request_t* request) {
    return tryCatch([request] {
        ASSERT(request);
        delete request;
    });
}

int metkit_request_add(metkit_request_t* request, const char* param, const char* values[], int numValues) {
    return tryCatch([request, param, values, numValues] {
        ASSERT(request);
        ASSERT(param);
        ASSERT(values);
        std::string n(param);
        std::vector<std::string> vv;
        for (int i = 0; i < numValues; i++) {
            vv.push_back(std::string(values[i]));
        }
        request->values(n, vv);
    });
}

int metkit_request_set_verb(metkit_request_t* request, const char* verb) {
    return tryCatch([request, verb] {
        ASSERT(request);
        ASSERT(verb);
        request->verb(verb);
    });
}

int metkit_request_verb(const metkit_request_t* request, const char** verb) {
    return tryCatch([request, verb] {
        ASSERT(request);
        ASSERT(verb);
        *verb = request->verb().c_str();
    });
}

int metkit_request_has_param(const metkit_request_t* request, const char* param, bool* has) {
    return tryCatch([request, param, has] {
        ASSERT(request);
        ASSERT(param);
        ASSERT(has);
        *has = request->has(param);
    });
}

int metkit_request_params(const metkit_request_t* request, metkit_paramiterator_t** params) {
    return tryCatch([request, params] {
        ASSERT(request);
        ASSERT(params);
        *params = new metkit_paramiterator_t(request->params());
    });
}

int metkit_request_count_values(const metkit_request_t* request, const char* param, size_t* count) {
    return tryCatch([request, param, count] {
        ASSERT(request);
        ASSERT(param);
        ASSERT(count);
        *count = request->countValues(param);
    });
}

int metkit_request_value(const metkit_request_t* request, const char* param, int index, const char** value) {
    return tryCatch([request, param, index, value] {
        ASSERT(request);
        ASSERT(param);
        ASSERT(value);
        *value = (request->values(param, false))[index].c_str();
    });
}

int metkit_request_expand(const metkit_request_t* request, metkit_request_t* expandedRequest, bool inherit, bool strict) {
    return tryCatch([request, expandedRequest, inherit, strict] {
        ASSERT(request);
        ASSERT(expandedRequest);
        ASSERT(expandedRequest->empty());
        metkit::mars::MarsExpension expand(inherit, strict);
        *expandedRequest = std::move(expand.expand(*request));
    });
}

int metkit_request_merge(metkit_request_t* request, const metkit_request_t* otherRequest) {
    return tryCatch([request, otherRequest] {
        ASSERT(request);
        ASSERT(otherRequest);
        request->merge(*otherRequest);
    });
}

// -----------------------------------------------------------------------------
//                           REQUEST ITERATOR
// -----------------------------------------------------------------------------

int metkit_free_requestiterator(const metkit_requestiterator_t* list) {
    return tryCatch([list] {
        ASSERT(list);
        delete list;
    });
}

int metkit_requestiterator_next(metkit_requestiterator_t* list) {
    return tryCatch(std::function<int()>{[list] {
        ASSERT(list);
        return list->next();
    }});
}

int metkit_requestiterator_request(const metkit_requestiterator_t* list, metkit_request_t* request) {
    return tryCatch([list, request] {
        ASSERT(list);
        ASSERT(request);
        ASSERT(list->iterator != list->vector.end());
        ASSERT(request->empty());
        *request = std::move(*(list->iterator));
    });
}

// -----------------------------------------------------------------------------
//                           PARAM ITERATOR
// -----------------------------------------------------------------------------

int metkit_free_paramiterator(const metkit_paramiterator_t* list) {
    return tryCatch([list] {
        ASSERT(list);
        delete list;
    });
}

int metkit_paramiterator_next(metkit_paramiterator_t* list) {
    return tryCatch(std::function<int()>{[list] {
        ASSERT(list);
        return list->next();
    }});
}

int metkit_paramiterator_param(const metkit_paramiterator_t* list, const char** param) {
    return tryCatch([list, param] {
        ASSERT(list);
        ASSERT(param);
        ASSERT(list->iterator != list->vector.end());
        *param = list->iterator->c_str();
    });
}


// ---------------------------------------------------------------------------------------------------------------------

}  // extern "C"
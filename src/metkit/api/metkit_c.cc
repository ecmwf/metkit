#include "metkit_c.h"
#include "metkit/mars/MarsExpension.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/metkit_version.h"
#include "eckit/runtime/Main.h"
#include <functional>

// ---------------------------------------------------------------------------------------------------------------------

struct metkit_marsrequest_t : public metkit::mars::MarsRequest {
    using metkit::mars::MarsRequest::MarsRequest;

    metkit_marsrequest_t(metkit::mars::MarsRequest&& req) :
        metkit::mars::MarsRequest(std::move(req)) {}
};

struct metkit_requestiterator_t {
    explicit metkit_requestiterator_t(std::vector<metkit::mars::MarsRequest>&& vec) :
        vector_(std::move(vec)), iterator_(vector_.begin()) {}

    int next() {
        if (iterator_ == vector_.end()) {
            return METKIT_ITERATION_COMPLETE;
        }
        ++iterator_;
        return iterator_ == vector_.end() ? METKIT_ITERATION_COMPLETE : METKIT_SUCCESS;
    }

    void current(metkit_marsrequest_t* request)  {
        ASSERT(iterator_ != vector_.end());
        *request = std::move(*iterator_);
    }

private:

    std::vector<metkit::mars::MarsRequest> vector_;
    std::vector<metkit::mars::MarsRequest>::iterator iterator_;
};

/// @comment: (maby)
/// Not sure if there is much value in having a param iterator. We could just return an array of
/// strings (char**) using metkit_marsrequest_params.
/// I think we should metkit_paramiterator_t.
struct metkit_paramiterator_t {
    explicit metkit_paramiterator_t(std::vector<std::string> vec) :
        vector_(std::move(vec)), iterator_(vector_.begin()) {}

    int next() {
        if (iterator_ == vector_.end()) {
            return METKIT_ITERATION_COMPLETE;
        }
        ++iterator_;

        return iterator_ == vector_.end() ? METKIT_ITERATION_COMPLETE : METKIT_SUCCESS;
    }

    void current(const char** param) {
        ASSERT(iterator_ != vector_.end());
        *param = iterator_->c_str();
    }

private: 
    std::vector<std::string> vector_;
    std::vector<std::string>::iterator iterator_;
};

// ---------------------------------------------------------------------------------------------------------------------
//                           ERROR HANDLING

static thread_local std::string g_current_error_string;

const char* metkit_get_error_string(enum metkit_error_values_t err) {
    switch (err) {
        case METKIT_SUCCESS:
            return "Success";
        case METKIT_ITERATION_COMPLETE:
            return "Iteration complete";
        case METKIT_ERROR:
        case METKIT_ERROR_USER:
        case METKIT_ERROR_ASSERT:
            return g_current_error_string.c_str();
        default:
            return "<unknown>";
    }
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
        return METKIT_ERROR_UNKNOWN;
    }
}

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

int metkit_parse_marsrequest(const char* str, metkit_requestiterator_t** requests, bool strict) {
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

int metkit_new_marsrequest(metkit_marsrequest_t** request) {
    return tryCatch([request] {
        ASSERT(request);
        *request = new metkit_marsrequest_t();
    });
}

int metkit_delete_marsrequest(const metkit_marsrequest_t* request) {
    return tryCatch([request] {
        delete request;
    });
}   

int metkit_marsrequest_set(metkit_marsrequest_t* request, const char* param, const char* values[], int numValues) {
    return tryCatch([request, param, values, numValues] {
        ASSERT(request);
        ASSERT(param);
        ASSERT(values);
        std::string param_str(param);
        std::vector<std::string> values_vec;
        values_vec.reserve(numValues);
        std::copy(values, values + numValues, std::back_inserter(values_vec));

        request->values(param_str, values_vec);
    });
}

int metkit_marsrequest_set_one(metkit_marsrequest_t* request, const char* param, const char* value) {
    return metkit_marsrequest_set(request, param, &value, 1);
}

int metkit_marsrequest_set_verb(metkit_marsrequest_t* request, const char* verb) {
    return tryCatch([request, verb] {
        ASSERT(request);
        ASSERT(verb);
        request->verb(verb);
    });
}

int metkit_marsrequest_verb(const metkit_marsrequest_t* request, const char** verb) {
    return tryCatch([request, verb] {
        ASSERT(request);
        ASSERT(verb);
        *verb = request->verb().c_str();
    });
}

int metkit_marsrequest_has_param(const metkit_marsrequest_t* request, const char* param, bool* has) {
    return tryCatch([request, param, has] {
        ASSERT(request);
        ASSERT(param);
        ASSERT(has);
        *has = request->has(param);
    });
}

int metkit_marsrequest_params(const metkit_marsrequest_t* request, metkit_paramiterator_t** params) {
    return tryCatch([request, params] {
        ASSERT(request);
        ASSERT(params);
        *params = new metkit_paramiterator_t(request->params());
    });
}

int metkit_marsrequest_count_values(const metkit_marsrequest_t* request, const char* param, size_t* count) {
    return tryCatch([request, param, count] {
        ASSERT(request);
        ASSERT(param);
        ASSERT(count);
        *count = request->countValues(param);
    });
}

int metkit_marsrequest_value(const metkit_marsrequest_t* request, const char* param, int index, const char** value) {
    return tryCatch([request, param, index, value] {
        ASSERT(request);
        ASSERT(param);
        ASSERT(value);
        *value = (request->values(param, false))[index].c_str();
    });
}

int metkit_marsrequest_values(const metkit_marsrequest_t* request, const char* param, const char** values[], size_t* numValues) {
    return tryCatch([request, param, values, numValues] {
        ASSERT(request);
        ASSERT(param);
        ASSERT(values);
        ASSERT(numValues);
        const std::vector<std::string>& values_str = request->values(param);
        *numValues = values_str.size();
        *values = new const char*[values_str.size()];
        std::transform(values_str.begin(), values_str.end(), *values, [](const std::string& s) { return s.c_str(); });
    });
}

int metkit_marsrequest_expand(const metkit_marsrequest_t* request, metkit_marsrequest_t* expandedRequest, bool inherit, bool strict) {
    return tryCatch([request, expandedRequest, inherit, strict] {
        ASSERT(request);
        ASSERT(expandedRequest);
        ASSERT(expandedRequest->empty());
        metkit::mars::MarsExpension expand(inherit, strict);
        *expandedRequest = expand.expand(*request);
    });
}

int metkit_marsrequest_merge(metkit_marsrequest_t* request, const metkit_marsrequest_t* otherRequest) {
    return tryCatch([request, otherRequest] {
        ASSERT(request);
        ASSERT(otherRequest);
        request->merge(*otherRequest);
    });
}

// -----------------------------------------------------------------------------
//                           REQUEST ITERATOR
// -----------------------------------------------------------------------------

int metkit_delete_requestiterator(const metkit_requestiterator_t* it) {
    return tryCatch([it] {
        delete it;
    });
}

int metkit_requestiterator_next(metkit_requestiterator_t* it) {
    return tryCatch(std::function<int()>{[it] {
        ASSERT(it);
        return it->next();
    }});
}

int metkit_requestiterator_request(metkit_requestiterator_t* it, metkit_marsrequest_t* request) {
    return tryCatch([it, request] {
        ASSERT(it);
        ASSERT(request);
        ASSERT(request->empty());

        it->current(request);
    });
}

// -----------------------------------------------------------------------------
//                           PARAM ITERATOR
// -----------------------------------------------------------------------------

int metkit_delete_paramiterator(const metkit_paramiterator_t* it) {
    return tryCatch([it] {
        delete it;
    });
}

int metkit_paramiterator_next(metkit_paramiterator_t* it) {
    return tryCatch(std::function<int()>{[it] {
        ASSERT(it);
        return it->next();
    }});
}

int metkit_paramiterator_param(metkit_paramiterator_t* it, const char** param) {
    return tryCatch([it, param] {
        ASSERT(it);
        ASSERT(param);
        it->current(param);
    });
}


// ---------------------------------------------------------------------------------------------------------------------
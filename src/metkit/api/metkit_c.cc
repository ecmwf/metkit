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
        vector_(std::move(vec)), current_(vector_.begin()) {}

    metkit_iterator_status_t next() {
        if (first_) {
            first_ = false;

            if (current_ != vector_.end()) {
                return METKIT_ITERATOR_SUCCESS;
            }
        }

        if (current_ == vector_.end()) {
            return METKIT_ITERATOR_COMPLETE;
        }

        ++current_;
        return current_ == vector_.end() ? METKIT_ITERATOR_COMPLETE : METKIT_ITERATOR_SUCCESS;
    }

    // Note: expected to call next() before calling current()
    metkit_iterator_status_t current(metkit_marsrequest_t* out)  {
        if (first_ || current_ == vector_.end()) {
            return METKIT_ITERATOR_ERROR;
        }

        *out = std::move(*current_);
        return METKIT_ITERATOR_SUCCESS;
    }

private:
    bool first_ = true;
    std::vector<metkit::mars::MarsRequest> vector_;
    std::vector<metkit::mars::MarsRequest>::iterator current_;
};

// ---------------------------------------------------------------------------------------------------------------------
//                           ERROR HANDLING

static thread_local std::string g_current_error_string;

const char* metkit_get_error_string(metkit_error_t err) {
    switch (err) {
        case METKIT_SUCCESS:
            return "Success";
        case METKIT_ERROR:
        case METKIT_ERROR_USER:
        case METKIT_ERROR_ASSERT:
            return g_current_error_string.c_str();
        default:
            return "<unknown>";
    }
}

metkit_error_t innerWrapFn(std::function<metkit_error_t()> f) {
    return f();
}

metkit_error_t innerWrapFn(std::function<void()> f) {
    f();
    return METKIT_SUCCESS;
}

template <typename FN>
[[nodiscard]] metkit_error_t tryCatch(FN&& fn) {
    try {
        return innerWrapFn(std::forward<FN>(fn));
    }
    catch (const eckit::UserError& e) {
        g_current_error_string = e.what();
        return METKIT_ERROR_USER;
    }
    catch (const eckit::AssertionFailed& e) {
        g_current_error_string = e.what();
        return METKIT_ERROR_ASSERT;
    }
    catch (const eckit::Exception& e) {
        g_current_error_string = e.what();
        return METKIT_ERROR;
    }
    catch (const std::exception& e) {
        g_current_error_string = e.what();
        return METKIT_ERROR_UNKNOWN;
    }
    catch (...) {
        return METKIT_ERROR_UNKNOWN;
    }
}

// -----------------------------------------------------------------------------
//                           HELPERS
// -----------------------------------------------------------------------------

metkit_error_t metkit_initialise() {
    return tryCatch([] {
        static bool initialised = false;

        if (initialised) {
            eckit::Log::warning()
                << "Initialising Metkit library twice" << std::endl;
        }

        if (!initialised) {
            const char* argv[2] = {"metkit-api", nullptr};
            eckit::Main::initialise(1, const_cast<char**>(argv));
            initialised = true;
        }
    });
}

// -----------------------------------------------------------------------------
//                           PARSING
// -----------------------------------------------------------------------------

metkit_error_t metkit_parse_marsrequests(const char* str, metkit_requestiterator_t** requests, bool strict) {
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

metkit_error_t metkit_marsrequest_new(metkit_marsrequest_t** request) {
    return tryCatch([request] {
        ASSERT(request);
        *request = new metkit_marsrequest_t();
    });
}

metkit_error_t metkit_marsrequest_delete(const metkit_marsrequest_t* request) {
    return tryCatch([request] {
        delete request;
    });
}   

metkit_error_t metkit_marsrequest_set(metkit_marsrequest_t* request, const char* param, const char* values[], int numValues) {
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
    
metkit_error_t metkit_marsrequest_set_one(metkit_marsrequest_t* request, const char* param, const char* value) {
    return metkit_marsrequest_set(request, param, &value, 1);
}

metkit_error_t metkit_marsrequest_set_verb(metkit_marsrequest_t* request, const char* verb) {
    return tryCatch([request, verb] {
        ASSERT(request);
        ASSERT(verb);
        request->verb(verb);
    });
}

metkit_error_t metkit_marsrequest_verb(const metkit_marsrequest_t* request, const char** verb) {
    return tryCatch([request, verb] {
        ASSERT(request);
        ASSERT(verb);
        *verb = request->verb().c_str();
    });
}

metkit_error_t metkit_marsrequest_has_param(const metkit_marsrequest_t* request, const char* param, bool* has) {
    return tryCatch([request, param, has] {
        ASSERT(request);
        ASSERT(param);
        ASSERT(has);
        *has = request->has(param);
    });
}

metkit_error_t metkit_marsrequest_count_params(const metkit_marsrequest_t* request, size_t* count) {
    return tryCatch([request, count] {
        ASSERT(request);
        ASSERT(count);
        *count = request->params().size();
    });
}

metkit_error_t metkit_marsrequest_param(const metkit_marsrequest_t* request, size_t index, const char** param) {
    return tryCatch([request, index, param] {
        ASSERT(request);
        ASSERT(param);
        *param = strdup(request->params()[index].c_str());
    });
}

metkit_error_t metkit_marsrequest_count_values(const metkit_marsrequest_t* request, const char* param, size_t* count) {
    return tryCatch([request, param, count] {
        ASSERT(request);
        ASSERT(param);
        ASSERT(count);
        *count = request->countValues(param);
    });
}

metkit_error_t metkit_marsrequest_value(const metkit_marsrequest_t* request, const char* param, int index, const char** value) {
    return tryCatch([request, param, index, value] {
        ASSERT(request);
        ASSERT(param);
        ASSERT(value);
        *value = request->values(param, false)[index].c_str();
    });
}
metkit_error_t metkit_marsrequest_expand(const metkit_marsrequest_t* request, bool inherit, bool strict, metkit_marsrequest_t* expandedRequest) {
    return tryCatch([request, expandedRequest, inherit, strict] {
        ASSERT(request);
        ASSERT(expandedRequest);
        ASSERT(expandedRequest->empty());
        metkit::mars::MarsExpension expand(inherit, strict);
        *expandedRequest = expand.expand(*request);
    });
}

metkit_error_t metkit_marsrequest_merge(metkit_marsrequest_t* request, const metkit_marsrequest_t* otherRequest) {
    return tryCatch([request, otherRequest] {
        ASSERT(request);
        ASSERT(otherRequest);
        request->merge(*otherRequest);
    });
}

void metkit_string_delete(const char* str) {
    delete[] str;
}

// -----------------------------------------------------------------------------
//                           REQUEST ITERATOR
// -----------------------------------------------------------------------------

metkit_error_t metkit_requestiterator_delete(const metkit_requestiterator_t* it) {
    return tryCatch([it] {
        delete it;
    });
}

metkit_iterator_status_t metkit_requestiterator_next(metkit_requestiterator_t* it) {
    if (!it) return METKIT_ITERATOR_ERROR;
    
    return it->next();
}

metkit_iterator_status_t metkit_requestiterator_current(metkit_requestiterator_t* it, metkit_marsrequest_t* request) {
    if (!it || !request || !request->empty()) {
        return METKIT_ITERATOR_ERROR;
    }

    return it->current(request);
}

// ---------------------------------------------------------------------------------------------------------------------
// Bridge between C and C++
const metkit::mars::MarsRequest& metkit::mars::MarsRequest::fromOpaque(const metkit_marsrequest_t* request) {
    return *static_cast<const metkit::mars::MarsRequest*>(request);
}

// ---------------------------------------------------------------------------------------------------------------------

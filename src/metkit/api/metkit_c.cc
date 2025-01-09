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

// ---------------------------------------------------------------------------------------------------------------------
//                           ERROR HANDLING

static thread_local std::string g_current_error_string;

const char* metkit_get_error_string(metkit_error_t err) {
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

metkit_error_t innerWrapFn(std::function<int()> f) {
    return static_cast<metkit_error_t>(f());
}

metkit_error_t innerWrapFn(std::function<void()> f) {
    f();
    return METKIT_SUCCESS;
}

template <typename FN>
[[nodiscard]] metkit_error_t tryCatchEnum(FN&& fn) {
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
    return tryCatchEnum([] {
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

metkit_error_t metkit_parse_marsrequests(const char* str, metkit_requestiterator_t** requests, bool strict) {
    return tryCatchEnum([requests, str, strict] {
        ASSERT(requests);
        ASSERT(str);
        std::istringstream in(str);
        *requests = new metkit_requestiterator_t(metkit::mars::MarsRequest::parse(in, strict));
    });
}

// -----------------------------------------------------------------------------
//                           REQUEST
// -----------------------------------------------------------------------------

metkit_error_t metkit_new_marsrequest(metkit_marsrequest_t** request) {
    return tryCatchEnum([request] {
        ASSERT(request);
        *request = new metkit_marsrequest_t();
    });
}

metkit_error_t metkit_delete_marsrequest(const metkit_marsrequest_t* request) {
    return tryCatchEnum([request] {
        delete request;
    });
}   

metkit_error_t metkit_marsrequest_set(metkit_marsrequest_t* request, const char* param, const char* values[], int numValues) {
    return tryCatchEnum([request, param, values, numValues] {
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
    return tryCatchEnum([request, verb] {
        ASSERT(request);
        ASSERT(verb);
        request->verb(verb);
    });
}

metkit_error_t metkit_marsrequest_verb(const metkit_marsrequest_t* request, const char** verb) {
    return tryCatchEnum([request, verb] {
        ASSERT(request);
        ASSERT(verb);
        *verb = request->verb().c_str();
    });
}

metkit_error_t metkit_marsrequest_has_param(const metkit_marsrequest_t* request, const char* param, bool* has) {
    return tryCatchEnum([request, param, has] {
        ASSERT(request);
        ASSERT(param);
        ASSERT(has);
        *has = request->has(param);
    });
}

metkit_error_t metkit_marsrequest_count_params(const metkit_marsrequest_t* request, size_t* count) {
    return tryCatchEnum([request, count] {
        ASSERT(request);
        ASSERT(count);
        *count = request->params().size();
    });
}

metkit_error_t metkit_marsrequest_param(const metkit_marsrequest_t* request, size_t index, const char** param) {
    return tryCatchEnum([request, index, param] {
        ASSERT(request);
        ASSERT(param);
        *param = request->params()[index].c_str();
    });
}

metkit_error_t metkit_marsrequest_count_values(const metkit_marsrequest_t* request, const char* param, size_t* count) {
    return tryCatchEnum([request, param, count] {
        ASSERT(request);
        ASSERT(param);
        ASSERT(count);
        *count = request->countValues(param);
    });
}

metkit_error_t metkit_marsrequest_value(const metkit_marsrequest_t* request, const char* param, int index, const char** value) {
    return tryCatchEnum([request, param, index, value] {
        ASSERT(request);
        ASSERT(param);
        ASSERT(value);
        *value = request->values(param, false)[index].c_str();
    });
}
metkit_error_t metkit_marsrequest_expand(const metkit_marsrequest_t* request, bool inherit, bool strict, metkit_marsrequest_t* expandedRequest) {
    return tryCatchEnum([request, expandedRequest, inherit, strict] {
        ASSERT(request);
        ASSERT(expandedRequest);
        ASSERT(expandedRequest->empty());
        metkit::mars::MarsExpension expand(inherit, strict);
        *expandedRequest = expand.expand(*request);
    });
}

metkit_error_t metkit_marsrequest_merge(metkit_marsrequest_t* request, const metkit_marsrequest_t* otherRequest) {
    return tryCatchEnum([request, otherRequest] {
        ASSERT(request);
        ASSERT(otherRequest);
        request->merge(*otherRequest);
    });
}

// -----------------------------------------------------------------------------
//                           REQUEST ITERATOR
// -----------------------------------------------------------------------------

metkit_error_t metkit_delete_requestiterator(const metkit_requestiterator_t* it) {
    return tryCatchEnum([it] {
        delete it;
    });
}

metkit_error_t metkit_requestiterator_next(metkit_requestiterator_t* it) {
    return tryCatchEnum(std::function<int()>{[it] {
        ASSERT(it);
        return it->next();
    }});
}

metkit_error_t metkit_requestiterator_request(metkit_requestiterator_t* it, metkit_marsrequest_t* request) {
    return tryCatchEnum([it, request] {
        ASSERT(it);
        ASSERT(request);
        ASSERT(request->empty());

        it->current(request);
    });
}

// ---------------------------------------------------------------------------------------------------------------------
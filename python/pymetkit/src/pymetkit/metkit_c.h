
struct metkit_marsrequest_t;
typedef struct metkit_marsrequest_t metkit_marsrequest_t;
struct metkit_requestiterator_t;
typedef struct metkit_requestiterator_t metkit_requestiterator_t;
struct metkit_paramiterator_t;
typedef struct metkit_paramiterator_t metkit_paramiterator_t;

typedef enum metkit_error_values_t {
    METKIT_SUCCESS       = 0, /* Operation succeded. */
    METKIT_ERROR         = 1, /* Operation failed. */
    METKIT_ERROR_UNKNOWN = 2, /* Failed with an unknown error. */
    METKIT_ERROR_USER    = 3, /* Failed with an user error. */
    METKIT_ERROR_ASSERT  = 4  /* Failed with an assert() */
} metkit_error_t;

typedef enum metkit_iterator_status_t {
    METKIT_ITERATOR_SUCCESS  = 0, /* Operation succeded. */
    METKIT_ITERATOR_COMPLETE = 1, /* All elements have been returned */
    METKIT_ITERATOR_ERROR    = 2  /* Operation failed. */
} metkit_iterator_status_t;


const char* metkit_get_error_string(enum metkit_error_values_t err);
const char* metkit_version();
const char* metkit_git_sha1();
metkit_error_t metkit_initialise();

metkit_error_t metkit_parse_marsrequests(const char* str, metkit_requestiterator_t** requests, bool strict);
metkit_error_t metkit_marsrequest_new(metkit_marsrequest_t** request);
metkit_error_t metkit_marsrequest_delete(const metkit_marsrequest_t* request);
metkit_error_t metkit_marsrequest_set(metkit_marsrequest_t* request, const char* param, const char* values[],
                                      int numValues);
metkit_error_t metkit_marsrequest_set_one(metkit_marsrequest_t* request, const char* param, const char* value);
metkit_error_t metkit_marsrequest_set_verb(metkit_marsrequest_t* request, const char* verb);
metkit_error_t metkit_marsrequest_verb(const metkit_marsrequest_t* request, const char** verb);
metkit_error_t metkit_marsrequest_has_param(const metkit_marsrequest_t* request, const char* param, bool* has);
metkit_error_t metkit_marsrequest_params(const metkit_marsrequest_t* request, metkit_paramiterator_t** params);
metkit_error_t metkit_marsrequest_count_values(const metkit_marsrequest_t* request, const char* param, size_t* count);
metkit_error_t metkit_marsrequest_value(const metkit_marsrequest_t* request, const char* param, int index,
                                        const char** value);
metkit_error_t metkit_marsrequest_expand(const metkit_marsrequest_t* request, bool inherit, bool strict,
                                         metkit_marsrequest_t* expandedRequest);
metkit_error_t metkit_marsrequest_merge(metkit_marsrequest_t* request, const metkit_marsrequest_t* otherRequest);

metkit_error_t metkit_requestiterator_delete(const metkit_requestiterator_t* it);
metkit_iterator_status_t metkit_requestiterator_next(metkit_requestiterator_t* it);
metkit_iterator_status_t metkit_requestiterator_current(metkit_requestiterator_t* it, metkit_marsrequest_t* request);

metkit_error_t metkit_paramiterator_delete(const metkit_paramiterator_t* it);
metkit_iterator_status_t metkit_paramiterator_next(metkit_paramiterator_t* it);
metkit_iterator_status_t metkit_paramiterator_current(const metkit_paramiterator_t* it, const char** param);

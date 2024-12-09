struct metkit_request_t;
typedef struct metkit_request_t metkit_request_t;
struct metkit_requestiterator_t;
typedef struct metkit_requestiterator_t metkit_requestiterator_t;
struct metkit_paramiterator_t;
typedef struct metkit_paramiterator_t metkit_paramiterator_t;
typedef enum metkit_error_values_t
{
    METKIT_SUCCESS            = 0, /* Operation succeded. */
    METKIT_ITERATION_COMPLETE = 1, /* All elements have been returned */
    METKIT_ERROR              = 2, /* Operation failed. */
    METKIT_ERROR_UNKNOWN      = 3, /* Failed with an unknown error. */
    METKIT_ERROR_USER         = 4, /* Failed with an user error. */
    METKIT_ERROR_ASSERT       = 5  /* Failed with an assert() */
} metkit_error_enum_t;
const char* metkit_get_error_string(int err);
int metkit_version(const char** version);
int metkit_vcs_version(const char** sha1);
int metkit_initialise();
int metkit_parse_mars_request(const char* str, metkit_requestiterator_t** request, bool strict);
int metkit_new_request(metkit_request_t** request);
int metkit_free_request(const metkit_request_t* request);
int metkit_request_add(metkit_request_t* request, const char* param, const char* values[], int numValues);
int metkit_request_set_verb(metkit_request_t* request, const char* verb);
int metkit_request_verb(const metkit_request_t* request, const char** verb);
int metkit_request_has_param(const metkit_request_t* request, const char* param, bool* has);
int metkit_request_params(const metkit_request_t* request, metkit_paramiterator_t** params);
int metkit_request_count_values(const metkit_request_t* request, const char* param, size_t* count);
int metkit_request_value(const metkit_request_t* request, const char* param, int index, const char** value);
int metkit_request_expand(const metkit_request_t* request, metkit_request_t* expandedRequest, bool inherit, bool strict);
int metkit_request_merge(metkit_request_t* request, const metkit_request_t* otherRequest);
int metkit_free_requestiterator(const metkit_requestiterator_t* list);
int metkit_requestiterator_next(metkit_requestiterator_t* list);
int metkit_requestiterator_request(const metkit_requestiterator_t* list, metkit_request_t* request);
int metkit_free_paramiterator(const metkit_paramiterator_t* list);
int metkit_paramiterator_next(metkit_paramiterator_t* list);
int metkit_paramiterator_param(const metkit_paramiterator_t* list, const char** param);